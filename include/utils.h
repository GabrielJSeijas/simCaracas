#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "graph.h"      // define CityGraph y Zone

// Comprueba que 'code' tenga exactamente 3 letras A–Z
bool is_valid_zone_code(const char *code);

// Calcula puntos requeridos: (2^level)²
int calculate_required_points(int level);

// Imprime el estado de tráfico; toma bloqueo de lectura del grafo
void print_traffic_status(CityGraph *graph);

// Imprime los detalles de una zona; toma bloqueo del mutex de la zona
void print_zone_details(Zone *zone);

// Muestra las opciones del menú principal
void print_menu_options(void);

#endif // UTILS_H
