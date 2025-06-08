#ifndef MENU_H
#define MENU_H

#include "graph.h"
#include "config.h"

/// Arranca el bucle interactivo del menú.
/// @param city   Puntero al grafo de la ciudad ya inicializado
/// @param config Configuración de la simulación (day_ticks, etc.)
void menu_loop(CityGraph *city, Config config);

#endif // MENU_H
