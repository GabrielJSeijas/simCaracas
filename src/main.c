// src/main.c

#include <stdio.h>
#include <stdlib.h>

#include "config.h"      // Config, parse_args(), print_config() 
#include "graph.h"       // CityGraph, graph_init(), graph_destroy() :contentReference[oaicite:0]{index=0}
#include "fileio.h"      // load_city_from_csv() :contentReference[oaicite:1]{index=1}
#include "traffic.h"     // init_traffic_system() :contentReference[oaicite:2]{index=2}
#include "menu.h"        // menu_loop()

int main(int argc, char *argv[]) {
    /* 1) Parsear argumentos de configuración */
    Config config = parse_args(argc, argv);
    print_config(&config);

    /* 2) Inicializar el grafo de la ciudad */
    CityGraph city;
    graph_init(&city);

    /* 3) Si se indicó archivo de carga, intentar leerlo */
    if (config.load_file) {
        if (!load_city_from_csv(&city, config.load_file)) {
            fprintf(stderr, "Error cargando el archivo '%s'\n", config.load_file);
        }
    }

    /* 4) Inicializar subsistema de tráfico (hilos, caravanas, etc.) */
    init_traffic_system(&city);

    /* 5) Entrar en el menú interactivo */
    menu_loop(&city, config);

    /* 6) Al salir del menú: liberar recursos y cerrar */
    graph_destroy(&city);
    if (config.load_file) {
        free(config.load_file);
    }

    return EXIT_SUCCESS;
}
