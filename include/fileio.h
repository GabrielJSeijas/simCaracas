#ifndef FILEIO_H
#define FILEIO_H

#include "graph.h"

bool load_city_from_csv(CityGraph *graph, const char *filename);
bool save_city_to_csv(const CityGraph *graph, const char *filename);

#endif