// src/utils.c

#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define UMBRAL_SATURACION_1 0.5f
#define UMBRAL_SATURACION_2 1.0f
#define UMBRAL_SATURACION_3 1.5f

bool esCodigoZonaValido(const char *codigo) {
    if (strlen(codigo) != 3) return false;
    for (int i = 0; i < 3; i++) {
        if (!isalpha((unsigned char)codigo[i])) return false;
    }
    return true;
}

int calcularPuntosRequeridos(int nivel) {
    int capacidad = 1 << nivel; // 2^nivel
    return capacidad * capacidad;
}

void mostrarEstadoTrafico(GrafoCiudad *grafo) {
    // Lock de lectura global
    pthread_rwlock_rdlock(&grafo->cerrojoGrafo);
    printf("\nEstado del tráfico:\n");
    for (int i = 0; i < grafo->totalZonas; i++) {
        // Lock por zona
        pthread_mutex_lock(&grafo->zonas[i].mutexZona);

        printf("[%s] ", grafo->zonas[i].codigo);
        if (grafo->zonas[i].norte) {
            float proporcion = (float)grafo->zonas[i].vehiculosNorte
                              / (float)grafo->zonas[i].capacidadNorte;
            printf("N:%d", grafo->zonas[i].vehiculosNorte);
            if (proporcion >= UMBRAL_SATURACION_3)     printf("!!! ");
            else if (proporcion >= UMBRAL_SATURACION_2) printf("!! ");
            else if (proporcion >= UMBRAL_SATURACION_1) printf("! ");
            else                                      printf(" ");
        }

        // Aquí podrías añadir Sur, Este y Oeste de forma similar...

        printf("\n");
        pthread_mutex_unlock(&grafo->zonas[i].mutexZona);
    }
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
}

void mostrarDetallesZona(Zona *zona) {
    pthread_mutex_lock(&zona->mutexZona);
    printf("Código           : %s\n", zona->codigo);
    printf("Nivel            : %d\n", zona->nivel);
    if (zona->esFuente)
        printf("Desempleados     : %d\n", zona->disponibles);
    else
        printf("Puestos libres   : %d\n", zona->disponibles);
    printf("Puntos acumulados: %d\n", zona->puntos);
    pthread_mutex_unlock(&zona->mutexZona);
}

void mostrarOpcionesMenuPrincipal(void) {
    printf("\n==== Menú SimCaracas ====\n");
    printf("1) Agregar zona\n");
    printf("2) Agregar arteria vial\n");
    printf("3) Ampliar arteria vial\n");
    printf("4) Guardar grafo actual\n");
    printf("5) Salir\n");
    printf("Seleccione una opción: ");
}
