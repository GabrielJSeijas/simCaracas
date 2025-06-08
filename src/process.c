#include "process.h"
#include "graph.h"
#include "pipes.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void create_child_processes(CityGraph *graph, Config config) {
    int pipe_zone[2], pipe_traffic[2];
    
    // Crear pipes para comunicación
    if (pipe(pipe_zone) == -1 || pipe(pipe_traffic) == -1) {
        perror("Error creando pipes");
        exit(EXIT_FAILURE);
    }
    
    // Proceso de zonas
    pid_t zone_pid = fork();
    if (zone_pid == 0) {
        close(pipe_zone[1]); // Cerrar extremo de escritura
        zone_process(graph, pipe_zone[0], config);
        exit(EXIT_SUCCESS);
    } else if (zone_pid < 0) {
        perror("Error creando proceso de zonas");
        exit(EXIT_FAILURE);
    }
    
    // Proceso de tráfico
    pid_t traffic_pid = fork();
    if (traffic_pid == 0) {
        close(pipe_traffic[1]); // Cerrar extremo de escritura
        traffic_process(graph, pipe_traffic[0], config);
        exit(EXIT_SUCCESS);
    } else if (traffic_pid < 0) {
        perror("Error creando proceso de tráfico");
        exit(EXIT_FAILURE);
    }
    
    // Proceso principal
    close(pipe_zone[0]);
    close(pipe_traffic[0]);
    
    // Bucle principal de comunicación
    main_process_loop(graph, pipe_zone[1], pipe_traffic[1], config);
    
    // Esperar a los procesos hijos
    waitpid(zone_pid, NULL, 0);
    waitpid(traffic_pid, NULL, 0);
}