// src/zone.c

#include "zone.h"
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Crea un nodo para la lista prioritaria de zonas
NodoListaZona* crearNodoListaZona(Zona *zona, int prioridad) {
    NodoListaZona *nodo = malloc(sizeof(NodoListaZona));
    if (!nodo) return NULL;
    nodo->zona      = zona;
    nodo->prioridad = prioridad;
    nodo->siguiente = NULL;
    return nodo;
}

// Inserta un nodo en la lista prioritaria según la prioridad
void insertarNodoPorPrioridad(NodoListaZona **lista,
                              Zona *zona,
                              int prioridad)
{
    NodoListaZona *nuevo = crearNodoListaZona(zona, prioridad);
    if (!nuevo) return;

    pthread_mutex_lock(&zona->mutexZona);
    pthread_mutex_unlock(&zona->mutexZona);

    if (*lista == NULL || prioridad > (*lista)->prioridad) {
        nuevo->siguiente = *lista;
        *lista           = nuevo;
    } else {
        NodoListaZona *actual = *lista;
        while (actual->siguiente &&
               actual->siguiente->prioridad >= prioridad)
        {
            actual = actual->siguiente;
        }
        nuevo->siguiente   = actual->siguiente;
        actual->siguiente  = nuevo;
    }
}

// Extrae la zona con mayor prioridad de la lista
Zona* extraerZonaMayorPrioridad(NodoListaZona **lista) {
    if (*lista == NULL) return NULL;
    NodoListaZona *cabeza = *lista;
    Zona *resultado       = cabeza->zona;
    *lista                = cabeza->siguiente;
    free(cabeza);
    return resultado;
}

// Libera la memoria de la lista de zonas
void liberarListaZonas(NodoListaZona *lista) {
    while (lista) {
        NodoListaZona *sig = lista->siguiente;
        free(lista);
        lista = sig;
    }
}

// Búsqueda BFS priorizada por disponibilidad para encontrar fuente o sumidero
void buscarZonaBFS(Zona *inicio,
                   bool buscarSumidero,
                   GrafoCiudad *grafo)
{
    NodoListaZona *colaPrioritaria = NULL;
    bool *visitado = calloc(grafo->totalZonas, sizeof(bool));
    if (!visitado) return;

    pthread_mutex_lock(&inicio->mutexZona);
    int prioridadInicio = inicio->disponibles;
    pthread_mutex_unlock(&inicio->mutexZona);

    insertarNodoPorPrioridad(&colaPrioritaria, inicio, prioridadInicio);
    visitado[inicio - grafo->zonas] = true;

    while (colaPrioritaria) {
        Zona *actual = extraerZonaMayorPrioridad(&colaPrioritaria);

        bool encontrado = false;
        pthread_mutex_lock(&actual->mutexZona);
        if ((buscarSumidero && !actual->esFuente) ||
            (!buscarSumidero &&  actual->esFuente && actual->disponibles > 0))
        {
            encontrado = true;
        }
        pthread_mutex_unlock(&actual->mutexZona);

        if (encontrado) {
            asignarResidentesATrabajo(inicio, actual, grafo);
            liberarListaZonas(colaPrioritaria);
            free(visitado);
            return;
        }

        Zona *vecinos[4] = {
            actual->norte,
            actual->sur,
            actual->este,
            actual->oeste
        };
        for (int i = 0; i < 4; i++) {
            Zona *vec = vecinos[i];
            if (vec && !visitado[vec - grafo->zonas]) {
                visitado[vec - grafo->zonas] = true;
                pthread_mutex_lock(&vec->mutexZona);
                int prio = vec->disponibles;
                pthread_mutex_unlock(&vec->mutexZona);
                insertarNodoPorPrioridad(&colaPrioritaria, vec, prio);
            }
        }
    }

    liberarListaZonas(colaPrioritaria);
    free(visitado);
}

// Asigna residentes de una fuente a un sumidero
void asignarResidentesATrabajo(Zona *fuente, Zona *sumidero, GrafoCiudad *grafo)
{
     (void)grafo; 
    pthread_mutex_lock(&fuente->mutexZona);
    pthread_mutex_lock(&sumidero->mutexZona);

    int aAsignar = fuente->disponibles;
    if (sumidero->disponibles < aAsignar) {
        aAsignar = sumidero->disponibles;
    }

    if (aAsignar > 0) {
        fuente->disponibles      -= aAsignar;
        sumidero->disponibles    -= aAsignar;
        sumidero->empleadosPresentes += aAsignar;

        printf("Asignados %d residentes de %s a %s\n",
               aAsignar, fuente->codigo, sumidero->codigo);
    }

    pthread_mutex_unlock(&sumidero->mutexZona);
    pthread_mutex_unlock(&fuente->mutexZona);
}

// Actualiza los puntos y niveles de las zonas según la hora del día
void actualizarPuntosZona(GrafoCiudad *grafo,
                          bool esMañana)
{
    pthread_rwlock_wrlock(&grafo->cerrojoGrafo);

    for (int i = 0; i < grafo->totalZonas; i++) {
        Zona *z = &grafo->zonas[i];
        pthread_mutex_lock(&z->mutexZona);

        int nivel    = z->nivel;
        int capacidad = (1 << nivel);
        int requerido = capacidad * capacidad;

        if (esMañana && !z->esFuente) {
            int ocupados = capacidad - z->disponibles;
            z->puntos += ocupados;
            if (z->puntos >= requerido) {
                z->nivel++;
                z->disponibles = (1 << z->nivel);
                printf("¡%s subió a nivel %d!\n", z->codigo, z->nivel);
            }
        }
        else if (!esMañana && z->esFuente) {
            int empleados = (capacidad / 2) - z->disponibles;
            z->puntos += empleados;
            if (z->puntos >= requerido) {
                z->nivel++;
                z->disponibles = (1 << z->nivel) / 2;
                printf("¡%s subió a nivel %d!\n", z->codigo, z->nivel);
            }
        }

        pthread_mutex_unlock(&z->mutexZona);
    }

    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
}

// Suma puntos SOLO a los sumideros al finalizar la mañana
void sumarPuntosSumideros(GrafoCiudad *grafo) {
    for (int i = 0; i < grafo->totalZonas; i++) {
        Zona *z = &grafo->zonas[i];
        if (!z->esFuente) { // Es sumidero
            z->puntos += z->empleadosPresentes;
            // Chequea si sube de nivel
            int capacidad = 1 << z->nivel;
            int puntosParaSubir = capacidad * capacidad;
            if (z->puntos >= puntosParaSubir) {
                z->nivel++;
                printf("%s subió a nivel %d\n", z->codigo, z->nivel);
                z->puntos = 0; // O deja el excedente, según la lógica deseada
            }
        }
    }
}

// Suma puntos SOLO a las fuentes al finalizar la tarde
void sumarPuntosFuentes(GrafoCiudad *grafo) {
    for (int i = 0; i < grafo->totalZonas; i++) {
        Zona *z = &grafo->zonas[i];
        if (z->esFuente) {
            z->puntos += z->empleadosPresentes;
            int capacidad = 1 << z->nivel;
            int puntosParaSubir = capacidad * capacidad;
            if (z->puntos >= puntosParaSubir) {
                z->nivel++;
                printf("%s subió a nivel %d\n", z->codigo, z->nivel);
                z->puntos = 0;
            }
        }
    }
}
