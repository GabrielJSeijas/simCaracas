#include "traffic.h"
#include "graph.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "zone.h"   // Para actualizarPuntosZona
#include "utils.h"  // Para imprimirCiudadEnGridAvanzado, actualizarTotalesEmpleo, mostrarResumenEmpleo

#define UMBRAL_CAPACIDAD_1 0.5f
#define UMBRAL_CAPACIDAD_2 1.0f
#define UMBRAL_CAPACIDAD_3 1.5f

static Caravana *caravanasMatutinas  = NULL;
static Caravana *caravanasVespertinas = NULL;
static pthread_mutex_t mutexTrafico __attribute__((unused)) = PTHREAD_MUTEX_INITIALIZER;

// Inicializa el sistema de tráfico
void inicializarSistemaTrafico(GrafoCiudad *ciudad) {
    (void)ciudad; // Silencia warning de variable no usada
    caravanasMatutinas  = NULL;
    caravanasVespertinas = NULL;
}

// Simula un día completo (mañana y tarde)
void simularDia(GrafoCiudad *grafo, Configuracion configuracion) {
    int ticksPorFase = configuracion.ticksPorDia / 2;

    simularTraficoMatutino(grafo, ticksPorFase);
    actualizarPuntosZona(grafo, true);

    simularTraficoVespertino(grafo, ticksPorFase);
    actualizarPuntosZona(grafo, false);

    actualizarTotalesEmpleo(grafo);  

    printf("\nResumen del día:\n");
    imprimirCiudadEnGridAvanzado(grafo);
    mostrarResumenEmpleo(grafo);
}

// Simula el tráfico matutino
void simularTraficoMatutino(GrafoCiudad *ciudad, int ticksPorDia) {
    for (int t = 0; t < ticksPorDia; t++) {
        simularFlujoVecinosInmediatos(ciudad);

        printf("\033[2J\033[H");
        fflush(stdout);
        printf("--- Simulando MAÑANA ---\n");
        printf("Tick %d/%d\n\n", t+1, ticksPorDia);
        imprimirCiudadEnGridAvanzado(ciudad);

        usleep(600000);
    }
}

// Simula el tráfico vespertino
void simularTraficoVespertino(GrafoCiudad *ciudad, int ticksPorDia) {
    for (int t = 0; t < ticksPorDia; t++) {
        simularFlujoVecinosInmediatos(ciudad);

        printf("\033[2J\033[H");
        fflush(stdout);
        printf("--- Simulando TARDE ---\n");
        printf("Tick %d/%d\n\n", t+1, ticksPorDia);
        imprimirCiudadEnGridAvanzado(ciudad);

        usleep(600000);
    }
}

// Simula el flujo de empleados entre zonas vecinas inmediatas
void simularFlujoVecinosInmediatos(GrafoCiudad *ciudad) {
    for (int i = 0; i < ciudad->totalZonas; i++) {
        Zona *origen = &ciudad->zonas[i];
        if (!origen->esFuente || origen->disponibles <= 0)
            continue;

        struct { Zona *vec; int *cap; int *veh; } dirs[] = {
            {origen->norte, &origen->capacidadNorte, &origen->vehiculosNorte},
            {origen->sur,   &origen->capacidadSur,   &origen->vehiculosSur},
            {origen->este,  &origen->capacidadEste,  &origen->vehiculosEste},
            {origen->oeste, &origen->capacidadOeste, &origen->vehiculosOeste},
        };

        for (int d = 0; d < 4; d++) {
            Zona *dest = dirs[d].vec;
            if (!dest || dest->esFuente) continue; // Solo se mueve a sumideros

            int capLibre = *dirs[d].cap - *dirs[d].veh;
            int puestosLibres = dest->disponibles;

            int mover = origen->disponibles;
            if (capLibre < mover) mover = capLibre;
            if (puestosLibres < mover) mover = puestosLibres;
            if (mover <= 0) continue;

            *dirs[d].veh += mover;          // Ocupa la vía
            origen->disponibles -= mover;   // Salen desempleados de la fuente
            dest->disponibles -= mover;     // Se ocupan los puestos libres
            dest->empleadosPresentes += mover; // Llegan empleados a sumidero
        }
    }
}
