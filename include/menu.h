#ifndef MENU_H
#define MENU_H

#include "graph.h"
#include "config.h"

/// Bucle interactivo principal.
/// @param grafo        Puntero al grafo de la ciudad
/// @param configuracion Parámetros de simulación parseados
void iniciarMenuInteractivo(GrafoCiudad *grafo, Configuracion configuracion);

#endif // MENU_H
