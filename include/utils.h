#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "graph.h"  // GrafoCiudad, Zona

/// Verifica si el código tiene exactamente 3 letras.
bool esCodigoZonaValido(const char *codigo);

/// Calcula los puntos requeridos según el nivel ( (2^nivel)² ).
int calcularPuntosRequeridos(int nivel);

/// Muestra por pantalla el estado del tráfico en cada zona.
void mostrarEstadoTrafico(GrafoCiudad *grafo);

/// Muestra información detallada de una zona.
void mostrarDetallesZona(Zona *zona);

/// Imprime en pantalla las opciones del menú principal.
void mostrarOpcionesMenuPrincipal(void);

#endif // UTILS_H
