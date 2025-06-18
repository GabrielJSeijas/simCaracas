#ifndef FILEIO_H
#define FILEIO_H

#include <stdbool.h>
#include "graph.h"   // GrafoCiudad y Zona

#define MAX_LINEA_CSV 1024
#define NUM_COLUMNAS_CSV 12

/**
 * Carga la configuración de la ciudad desde un archivo CSV.
 * @param ciudad Puntero al grafo de la ciudad.
 * @param rutaArchivo Ruta al archivo CSV de entrada.
 * @return true si la carga fue exitosa, false en caso contrario.
 */
bool cargarCiudadDesdeCSV(GrafoCiudad *ciudad, const char *rutaArchivo);

/**
 * Guarda el estado actual de la ciudad en un archivo CSV.
 * @param ciudad Puntero constante al grafo de la ciudad.
 * @param rutaArchivo Ruta donde se guardará el archivo.
 * @return true si se guardó correctamente, false en caso de error.
 */
bool guardarCiudadEnCSV(const GrafoCiudad *ciudad, const char *rutaArchivo);

#endif // FILEIO_H
