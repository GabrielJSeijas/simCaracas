#ifndef ZONE_H
#define ZONE_H

#include <stdbool.h>
#include "graph.h"

// Suma puntos a los sumideros del grafo
void sumarPuntosSumideros(GrafoCiudad *grafo);
// Suma puntos a las fuentes del grafo
void sumarPuntosFuentes(GrafoCiudad *grafo);

// Nodo para lista ordenada de zonas según prioridad
typedef struct NodoListaZona {
    Zona                   *zona;       // Puntero a la zona
    int                     prioridad;  // Prioridad (disponibles o residentes)
    struct NodoListaZona   *siguiente;  // Siguiente nodo en la lista
} NodoListaZona;

// Crea un nodo para la lista de prioridades
NodoListaZona* crearNodoListaZona(Zona *zona, int prioridad);

// Inserta una zona en la lista ordenada por prioridad descendente
void insertarNodoPorPrioridad(NodoListaZona **lista,
                              Zona *zona,
                              int prioridad);

// Extrae la zona de mayor prioridad (cabeza de la lista)
Zona* extraerZonaMayorPrioridad(NodoListaZona **lista);

// Libera todos los nodos de la lista de prioridades
void liberarListaZonas(NodoListaZona *lista);

// Búsqueda BFS priorizada por prioridad en la cola
// @param inicio         Zona de partida
// @param buscarSumidero true = buscar sumidero; false = buscar fuente con disponibles 
// @param grafo          GrafoCiudad donde buscar
void buscarZonaBFS(Zona *inicio,
                   bool buscarSumidero,
                   GrafoCiudad *grafo);

// Asigna residentes de una zona fuente a un sumidero (empleo),
// actualiza disponibles y estadísticas globales
void asignarResidentesATrabajo(Zona *fuente,
                               Zona *sumidero,
                               GrafoCiudad *grafo);

// Actualiza los puntos de cada zona según la fase del día.
// esManana=true: puntos para sumideros; false: puntos para fuentes.
void actualizarPuntosZona(GrafoCiudad *grafo,
                          bool esManana);

#endif // ZONE_H
