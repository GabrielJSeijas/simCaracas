#ifndef PROCESS_H
#define PROCESS_H

#include "graph.h"
#include "config.h"

/// Crea y lanza los procesos hijos (zona y tránsito)
void iniciarProcesosHijos(GrafoCiudad *grafo, Configuracion configuracion);

/// Ciclo de vida del proceso encargado de manejar zonas
/// @param grafo           Puntero al grafo de la ciudad
/// @param tuberiaLectura  Descriptor de lectura de la tubería de zona
/// @param configuracion   Parámetros de simulación
void procesoZona(GrafoCiudad *grafo,
                 int tuberiaLectura,
                 Configuracion configuracion);

/// Ciclo de vida del proceso encargado de manejar tránsito
/// @param grafo           Puntero al grafo de la ciudad
/// @param tuberiaLectura  Descriptor de lectura de la tubería de tránsito
/// @param configuracion   Parámetros de simulación
void procesoTransito(GrafoCiudad *grafo,
                     int tuberiaLectura,
                     Configuracion configuracion);

/// Bucle principal del proceso padre para coordinar el día
/// @param grafo              Puntero al grafo de la ciudad
/// @param tuberiaZona        Descriptor de escritura para zona
/// @param tuberiaTransito    Descriptor de escritura para tránsito
/// @param configuracion      Parámetros de simulación
void bucleProcesoPrincipal(GrafoCiudad *grafo,
                           int tuberiaZona,
                           int tuberiaTransito,
                           Configuracion configuracion);

#endif // PROCESS_H
