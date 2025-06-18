#ifndef PROCESS_H
#define PROCESS_H

#include "graph.h"
#include "config.h"

// Inicializa los procesos hijos y sus pipes
void iniciarProcesosHijos(GrafoCiudad *grafo, Configuracion configuracion,
                          int *tuberiaZonaEsc, int *tuberiaZonaLec,
                          int *tuberiaTransitoEsc, int *tuberiaTransitoLec,
                          pid_t *pidZona, pid_t *pidTransito);

// Proceso encargado de la gestión de una zona
void procesoZona(GrafoCiudad *grafo,
                 int tuberiaLectura,
                 int tuberiaEscritura,
                 Configuracion configuracion);

// Proceso encargado de la gestión del tránsito
void procesoTransito(GrafoCiudad *grafo,
                     int tuberiaLectura,
                     int tuberiaEscritura,
                     Configuracion configuracion);

// Función para el hilo de la zona
void* hiloZona(void* arg);

// Bucle principal del proceso padre
void bucleProcesoPrincipal(GrafoCiudad *grafo,
                           int tuberiaZonaEsc, int tuberiaZonaLec,
                           int tuberiaTransitoEsc, int tuberiaTransitoLec,
                           Configuracion configuracion);

// Reinicia los procesos hijos en caso de ser necesario
void reiniciarProcesosHijos(GrafoCiudad *grafo, Configuracion configuracion);

#endif // PROCESS_H
