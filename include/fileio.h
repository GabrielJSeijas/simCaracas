#ifndef FILEIO_H
#define FILEIO_H

#include <stdbool.h>
#include "graph.h"   // Para GrafoCiudad y Zona

// Definición de constantes para manejo de archivos
#define MAX_LINEA_CSV 1024
#define NUM_COLUMNAS_CSV 12

/**
 * @brief Carga la configuración de la ciudad desde un archivo CSV
 * 
 * @param ciudad Puntero al grafo de la ciudad
 * @param rutaArchivo Ruta al archivo CSV de entrada
 * @return true si la carga fue exitosa
 * @return false si hubo errores al cargar
 */
bool cargarCiudadDesdeCSV(GrafoCiudad *ciudad, const char *rutaArchivo);

/**
 * @brief Guarda el estado actual de la ciudad en un archivo CSV
 * 
 * @param ciudad Puntero constante al grafo de la ciudad
 * @param rutaArchivo Ruta donde se guardará el archivo
 * @return true si se guardó correctamente
 * @return false si hubo errores al guardar
 */
bool guardarCiudadEnCSV(const GrafoCiudad *ciudad, const char *rutaArchivo);

#endif // FILEIO_H