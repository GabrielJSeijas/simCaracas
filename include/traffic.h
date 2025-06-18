#ifndef TRAFFIC_H
#define TRAFFIC_H

#include "config.h"
#include "graph.h"

// Estructura que representa una caravana de vehículos
typedef struct Caravana {
    Zona            *zonaActual;        // Zona de partida o posición actual
    Zona            *zonaDestino;       // Zona de llegada
    int              cantidadVehiculos; // Número de vehículos en la caravana
    struct Caravana *siguiente;         // Siguiente caravana en la lista
} Caravana;

// Inicializa las estructuras del sistema de tráfico
void inicializarSistemaTrafico(GrafoCiudad *ciudad);

// Simula el tráfico matutino durante 'ticksPorDia' ticks
void simularTraficoMatutino(GrafoCiudad *ciudad, int ticksPorDia);

// Simula el tráfico vespertino durante 'ticksPorDia' ticks
void simularTraficoVespertino(GrafoCiudad *ciudad, int ticksPorDia);

// Simula un día completo de tráfico en la ciudad
void simularDia(GrafoCiudad *grafo, Configuracion configuracion);

void simularFlujoVecinosInmediatos(GrafoCiudad *ciudad);

#endif // TRAFFIC_H
