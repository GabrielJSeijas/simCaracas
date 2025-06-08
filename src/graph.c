// src/graph.c

#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

void iniciarGrafo(GrafoCiudad *grafo) {
    grafo->zonas             = malloc(sizeof(Zona) * MAX_ZONAS);
    assert(grafo->zonas != NULL);

    grafo->totalZonas         = 0;
    grafo->capacidadReservada = MAX_ZONAS;
    grafo->totalEmpleados     = 0;
    grafo->totalDesempleados  = 0;

    pthread_rwlock_init(&grafo->cerrojoGrafo, NULL);
}

void destruirGrafo(GrafoCiudad *grafo) {
    pthread_rwlock_wrlock(&grafo->cerrojoGrafo);
    for (int i = 0; i < grafo->totalZonas; i++) {
        pthread_mutex_destroy(&grafo->zonas[i].mutexZona);
    }
    free(grafo->zonas);
    grafo->zonas             = NULL;
    grafo->totalZonas        = 0;
    grafo->capacidadReservada = 0;
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
    pthread_rwlock_destroy(&grafo->cerrojoGrafo);
}

Zona* agregarZona(GrafoCiudad *grafo,
                  const char *codigo,
                  int nivel,
                  bool esFuente)
{
    if (strlen(codigo) != 3) return NULL;

    pthread_rwlock_wrlock(&grafo->cerrojoGrafo);
    /* Verificar duplicado */
    for (int i = 0; i < grafo->totalZonas; i++) {
        if (strcmp(grafo->zonas[i].codigo, codigo) == 0) {
            pthread_rwlock_unlock(&grafo->cerrojoGrafo);
            return NULL;
        }
    }
    /* Redimensionar si es necesario */
    if (grafo->totalZonas >= grafo->capacidadReservada) {
        grafo->capacidadReservada *= 2;
        grafo->zonas = realloc(
            grafo->zonas,
            sizeof(Zona) * grafo->capacidadReservada
        );
        assert(grafo->zonas != NULL);
    }
    /* Inicializar la nueva zona */
    Zona *z = &grafo->zonas[grafo->totalZonas++];
    strncpy(z->codigo, codigo, 3);
    z->codigo[3] = '\0';
    z->nivel     = nivel;
    z->puntos    = 0;
    z->esFuente  = esFuente;
    z->disponibles = esFuente
        ? (1 << nivel) / 2
        : (1 << nivel);

    z->norte = z->sur = z->este = z->oeste = NULL;
    z->capacidadNorte = z->capacidadSur = 0;
    z->capacidadEste  = z->capacidadOeste = 0;
    z->vehiculosNorte = z->vehiculosSur = 0;
    z->vehiculosEste  = z->vehiculosOeste = 0;

    pthread_mutex_init(&z->mutexZona, NULL);
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
    return z;
}

bool conectarZonas(GrafoCiudad *grafo,
                   Zona *z1,
                   Zona *z2,
                   Direccion dir,
                   int capacidadInicial)
{
    if (!z1 || !z2 || dir == NINGUNA) return false;

    pthread_rwlock_wrlock(&grafo->cerrojoGrafo);
    pthread_mutex_lock(&z1->mutexZona);
    pthread_mutex_lock(&z2->mutexZona);

    switch (dir) {
        case NORTE:
            if (z1->norte || z2->sur) goto error;
            z1->norte          = z2;
            z1->capacidadNorte = capacidadInicial;
            z2->sur            = z1;
            z2->capacidadSur   = capacidadInicial;
            break;
        case SUR:
            if (z1->sur || z2->norte) goto error;
            z1->sur            = z2;
            z1->capacidadSur   = capacidadInicial;
            z2->norte          = z1;
            z2->capacidadNorte = capacidadInicial;
            break;
        case ESTE:
            if (z1->este || z2->oeste) goto error;
            z1->este           = z2;
            z1->capacidadEste  = capacidadInicial;
            z2->oeste          = z1;
            z2->capacidadOeste = capacidadInicial;
            break;
        case OESTE:
            if (z1->oeste || z2->este) goto error;
            z1->oeste          = z2;
            z1->capacidadOeste = capacidadInicial;
            z2->este           = z1;
            z2->capacidadEste  = capacidadInicial;
            break;
        default:
            goto error;
    }

    pthread_mutex_unlock(&z2->mutexZona);
    pthread_mutex_unlock(&z1->mutexZona);
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
    return true;

error:
    pthread_mutex_unlock(&z2->mutexZona);
    pthread_mutex_unlock(&z1->mutexZona);
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
    return false;
}

Zona* buscarZona(GrafoCiudad *grafo,
                 const char *codigo)
{
    pthread_rwlock_rdlock(&grafo->cerrojoGrafo);
    for (int i = 0; i < grafo->totalZonas; i++) {
        if (strcmp(grafo->zonas[i].codigo, codigo) == 0) {
            pthread_rwlock_unlock(&grafo->cerrojoGrafo);
            return &grafo->zonas[i];
        }
    }
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
    return NULL;
}

void mostrarGrafo(GrafoCiudad *grafo) {
    pthread_rwlock_rdlock(&grafo->cerrojoGrafo);
    printf("La ciudad tiene %d zonas:\n", grafo->totalZonas);
    for (int i = 0; i < grafo->totalZonas; i++) {
        Zona *z = &grafo->zonas[i];
        pthread_mutex_lock(&z->mutexZona);

        printf("[%s] Nivel %d %s | Puntos: %d | ",
               z->codigo,
               z->nivel,
               z->esFuente ? "Fuente" : "Sumidero",
               z->puntos);

        if (z->esFuente) {
            printf("Desempleados: %d\n", z->disponibles);
        } else {
            printf("Puestos libres: %d\n", z->disponibles);
        }

        printf(" Conexiones: N(%s:%d/%d) S(%s:%d/%d) E(%s:%d/%d) O(%s:%d/%d)\n",
               z->norte   ? z->norte->codigo   : "--",
               z->vehiculosNorte, z->capacidadNorte,
               z->sur     ? z->sur->codigo     : "--",
               z->vehiculosSur,   z->capacidadSur,
               z->este    ? z->este->codigo    : "--",
               z->vehiculosEste,  z->capacidadEste,
               z->oeste   ? z->oeste->codigo   : "--",
               z->vehiculosOeste, z->capacidadOeste);

        pthread_mutex_unlock(&z->mutexZona);
    }
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
}

Direccion direccionOpuesta(Direccion dir) {
    switch (dir) {
        case NORTE: return SUR;
        case SUR:   return NORTE;
        case ESTE:  return OESTE;
        case OESTE: return ESTE;
        default:    return NINGUNA;
    }
}

bool ampliarArteria(GrafoCiudad *grafo,
                   Zona *z1,
                   Zona *z2,
                   int nuevaCapacidad)
{
    bool modificado = false;
    pthread_rwlock_wrlock(&grafo->cerrojoGrafo);
    pthread_mutex_lock(&z1->mutexZona);
    pthread_mutex_lock(&z2->mutexZona);

    if (z1->norte == z2) {
        z1->capacidadNorte = nuevaCapacidad;
        z2->capacidadSur   = nuevaCapacidad;
        modificado = true;
    } else if (z1->sur == z2) {
        z1->capacidadSur   = nuevaCapacidad;
        z2->capacidadNorte = nuevaCapacidad;
        modificado = true;
    } else if (z1->este == z2) {
        z1->capacidadEste  = nuevaCapacidad;
        z2->capacidadOeste = nuevaCapacidad;
        modificado = true;
    } else if (z1->oeste == z2) {
        z1->capacidadOeste = nuevaCapacidad;
        z2->capacidadEste  = nuevaCapacidad;
        modificado = true;
    }

    pthread_mutex_unlock(&z2->mutexZona);
    pthread_mutex_unlock(&z1->mutexZona);
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
    return modificado;
}
