#ifndef FILEIO_H
#define FILEIO_H

#include <stdbool.h>
#include "graph.h"   // para GrafoCiudad y Zona

/// Carga la configuración de la ciudad desde un CSV.
/// @param ciudad      Puntero al grafo de la ciudad
/// @param rutaArchivo Ruta al archivo CSV de entrada
/// @return true si la carga fue exitosa, false en caso contrario
bool cargarCiudadDesdeCSV(GrafoCiudad *ciudad,
                          const char *rutaArchivo);

/// Guarda el grafo de la ciudad en un CSV.
/// @param ciudad      Puntero al grafo de la ciudad
/// @param rutaArchivo Ruta al archivo CSV de salida
/// @return true si la operación fue exitosa, false en caso contrario
bool guardarCiudadEnCSV(const GrafoCiudad *ciudad,
                        const char *rutaArchivo);

#endif // FILEIO_H
