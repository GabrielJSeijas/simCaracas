#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "graph.h"  // GrafoCiudad, Zona

// Verifica si el código tiene exactamente 3 letras
bool esCodigoZonaValido(const char *codigo);

// Calcula los puntos requeridos según el nivel: (2^nivel)^2
int calcularPuntosRequeridos(int nivel);

// Muestra el estado del tráfico en cada zona
void mostrarEstadoTrafico(GrafoCiudad *grafo);

// Muestra información detallada de una zona
void mostrarDetallesZona(Zona *zona);

// Imprime las opciones del menú principal
void mostrarOpcionesMenuPrincipal(void);

// Asigna coordenadas (x, y) a cada zona del grafo
void asignarCoordenadas(GrafoCiudad *grafo);

// Imprime el grafo de la ciudad en formato de cuadrícula
void imprimirCiudadEnGridAvanzado(GrafoCiudad *grafo);

// Muestra un resumen del empleo en la ciudad
void mostrarResumenEmpleo(const GrafoCiudad *ciudad);

// Actualiza los totales de empleados y desempleados en la ciudad
void actualizarTotalesEmpleo(GrafoCiudad *ciudad);

#endif // UTILS_H
