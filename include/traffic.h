#ifndef TRAFFIC_H
#define TRAFFIC_H

#include "graph.h"

/// Caravana de vehículos moviéndose de una zona a otra
typedef struct Caravana {
    Zona            *zonaActual;        ///< Zona de partida o posición actual
    Zona            *zonaDestino;       ///< Zona de llegada
    int              cantidadVehiculos; ///< Número de vehículos en la caravana
    struct Caravana *siguiente;         ///< Siguiente caravana en la lista
} Caravana;

/// Inicializa las estructuras del sistema de tráfico.
/// @param ciudad Puntero al grafo de la ciudad
void inicializarSistemaTrafico(GrafoCiudad *ciudad);

/// Simula el tráfico matutino durante 'ticksPorDia' ticks.
/// @param ciudad      Puntero al grafo de la ciudad
/// @param ticksPorDia Número de ticks a simular
void simularTraficoMatutino(GrafoCiudad *ciudad, int ticksPorDia);

/// Simula el tráfico vespertino durante 'ticksPorDia' ticks.
/// @param ciudad      Puntero al grafo de la ciudad
/// @param ticksPorDia Número de ticks a simular
void simularTraficoVespertino(GrafoCiudad *ciudad, int ticksPorDia);

/// Actualiza el flujo de vehículos para todas las caravanas activas.
/// @param ciudad Puntero al grafo de la ciudad
void actualizarFlujosTrafico(GrafoCiudad *ciudad);

/// Libera la memoria de una lista de caravanas.
/// @param caravana Puntero a la primera caravana de la lista
void liberarCaravanas(Caravana *caravana);

#endif // TRAFFIC_H
