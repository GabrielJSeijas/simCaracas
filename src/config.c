#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config parse_args(int argc, char *argv[]) {
    Config config = {
        .day_ticks = 64,
        .tick_duration = 1.0f,
        .max_zone_level = 10,
        .load_file = NULL
    };
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i+1 < argc) {
            config.day_ticks = atoi(argv[++i]);
            if (config.day_ticks <= 0) config.day_ticks = 64;
        } 
        else if (strcmp(argv[i], "-t") == 0 && i+1 < argc) {
            config.tick_duration = atof(argv[++i]);
            if (config.tick_duration <= 0) config.tick_duration = 1.0f;
        } 
        else if (strcmp(argv[i], "-z") == 0 && i+1 < argc) {
            config.max_zone_level = atoi(argv[++i]);
            if (config.max_zone_level <= 0) config.max_zone_level = 10;
        } 
        else if (strcmp(argv[i], "-f") == 0 && i+1 < argc) {
            config.load_file = strdup(argv[++i]);
        }
    }
    
    return config;
}

void print_config(const Config *config) {
    printf("Configuración del simulador:\n");
    printf(" - Duración del día: %d ticks\n", config->day_ticks);
    printf(" - Duración del tick: %.2f segundos\n", config->tick_duration);
    printf(" - Nivel máximo de zonas: %d\n", config->max_zone_level);
    printf(" - Archivo a cargar: %s\n", config->load_file ? config->load_file : "Ninguno");
}