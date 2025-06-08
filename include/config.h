#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int day_ticks;          // Duración del día en ticks
    float tick_duration;    // Duración de un tick en segundos
    int max_zone_level;     // Nivel máximo de zonas
    char *load_file;        // Archivo a cargar (NULL si no hay)
} Config;

// Funciones para manejar configuración
Config parse_args(int argc, char *argv[]);
void print_config(const Config *config);

#endif // CONFIG_H