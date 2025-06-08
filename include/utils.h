#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

// Funciones utilitarias generales
bool is_valid_zone_code(const char *code);
int calculate_required_points(int level);
void print_traffic_status(const CityGraph *graph);
void print_zone_details(const Zone *zone);
void print_menu_options();

#endif // UTILS_H