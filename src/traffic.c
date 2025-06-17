#include "traffic.h"
#include "graph.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

/// Umbrales de saturación para emitir señales (no usados en este ejemplo)
#define UMBRAL_CAPACIDAD_1 0.5f
#define UMBRAL_CAPACIDAD_2 1.0f
#define UMBRAL_CAPACIDAD_3 1.5f

static Caravana *caravanasMatutinas  = NULL;
static Caravana *caravanasVespertinas = NULL;
static pthread_mutex_t mutexTrafico   = PTHREAD_MUTEX_INITIALIZER;

void inicializarSistemaTrafico(GrafoCiudad *ciudad) {
    (void)ciudad; // Marcar como usado para silenciar warning
    caravanasMatutinas  = NULL;
    caravanasVespertinas = NULL;
}

void simularDia(GrafoCiudad *grafo, Configuracion configuracion) {
    int ticksPorFase = configuracion.ticksPorDia / 2;

    simularTraficoMatutino(grafo, ticksPorFase);
    actualizarPuntosZona(grafo, true);

    simularTraficoVespertino(grafo, ticksPorFase);
    actualizarPuntosZona(grafo, false);

    // Limpia la pantalla antes del resumen
    printf("\033[2J\033[H");

    // SOLO IMPRIME EL RESUMEN Y EL MENÚ
    printf("\nResumen del día:\n");
    imprimirCiudadEnGridAvanzado(grafo);
    pthread_rwlock_rdlock(&grafo->cerrojoGrafo);
    printf("  - Empleados    : %d\n", grafo->totalEmpleados);
    printf("  - Desempleados : %d\n", grafo->totalDesempleados);
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
}



void simularTraficoMatutino(GrafoCiudad *ciudad, int ticksPorDia) {
    pthread_rwlock_rdlock(&ciudad->cerrojoGrafo);
    for (int i = 0; i < ciudad->totalZonas; i++) {
        if (ciudad->zonas[i].esFuente) {
            pthread_mutex_lock(&ciudad->zonas[i].mutexZona);
            int vehiculos = (1 << ciudad->zonas[i].nivel) / 2 - ciudad->zonas[i].disponibles;
            if (vehiculos > 0 && ciudad->zonas[i].este && !ciudad->zonas[i].este->esFuente) {
                Caravana *nueva = malloc(sizeof(Caravana));
                nueva->zonaActual        = &ciudad->zonas[i];
                nueva->zonaDestino       = ciudad->zonas[i].este;
                nueva->cantidadVehiculos = vehiculos;
                nueva->siguiente         = caravanasMatutinas;
                caravanasMatutinas      = nueva;
            }
            pthread_mutex_unlock(&ciudad->zonas[i].mutexZona);
        }
    }
    pthread_rwlock_unlock(&ciudad->cerrojoGrafo);

    for (int t = 0; t < ticksPorDia; t++) {
        // LIMPIAR PANTALLA
        printf("\033[2J\033[H");

        printf("--- Simulando MAÑANA ---\n");
        printf("Tick %d/%d\n\n", t+1, ticksPorDia);

        // Imprime el grafo en grid avanzado
        imprimirCiudadEnGridAvanzado(ciudad);

        actualizarFlujosTrafico(ciudad);
        usleep(500000);
    }
}

void simularTraficoVespertino(GrafoCiudad *ciudad, int ticksPorDia) {
    for (int t = 0; t < ticksPorDia; t++) {
        // LIMPIAR PANTALLA
        printf("\033[2J\033[H");
        printf("--- Simulando TARDE ---\n");
        printf("Tick %d/%d\n\n", t+1, ticksPorDia);
        imprimirCiudadEnGridAvanzado(ciudad);
        actualizarFlujosTrafico(ciudad);
        usleep(500000);
    }
}

void actualizarFlujosTrafico(GrafoCiudad *ciudad) {
    pthread_mutex_lock(&mutexTrafico);

    Caravana *actual = caravanasMatutinas;
    while (actual) {
        Zona *origen = actual->zonaActual;
        Zona *dest   = actual->zonaDestino;

        // Solo manejamos caravanas hacia el este en este ejemplo
        if (origen->este == dest) {
            int capLibre = origen->capacidadEste - origen->vehiculosEste;
            if (capLibre > 0) {
                int mover = capLibre < actual->cantidadVehiculos
                            ? capLibre
                            : actual->cantidadVehiculos;

                origen->vehiculosEste += mover;
                actual->cantidadVehiculos -= mover;

                if (actual->cantidadVehiculos == 0) {
                    // Caravana completó su viaje
                    Caravana *aLiberar = actual;
                    actual             = actual->siguiente;
                    free(aLiberar);
                    continue;
                }
            }
        }

        actual = actual->siguiente;
    }

    pthread_mutex_unlock(&mutexTrafico);
}

void liberarCaravanas(Caravana *caravana) {
    while (caravana) {
        Caravana *sig = caravana->siguiente;
        free(caravana);
        caravana = sig;
    }
}
