#include "config.h"
#include "graph.h"
#include "process.h"
#include "menu.h"

int main(int argc, char *argv[]) {
    Config config = parse_args(argc, argv);
    CityGraph city;
    
    graph_init(&city);
    
    if (config.load_file) {
        if (!load_city_from_csv(&city, config.load_file)) {
            fprintf(stderr, "Error cargando el archivo %s\n", config.load_file);
        }
    }
    
    init_traffic_system(&city);
    create_child_processes(&city, config);
    
    // Bucle principal del menú
    while (1) {
        print_menu_options();
        handle_menu_input(&city);
    }
    
    graph_destroy(&city);
    if (config.load_file) free(config.load_file);
    return 0;
}