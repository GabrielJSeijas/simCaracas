// src/process.c

#include "process.h"   // create_child_processes, zone_process, traffic_process, main_process_loop :contentReference[oaicite:1]{index=1}
#include "graph.h"
#include "pipes.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

void create_child_processes(CityGraph *graph, Config config) {
    int pipe_zone[2], pipe_traffic[2];
    if (pipe(pipe_zone)==-1 || pipe(pipe_traffic)==-1) {
        perror("Error creando pipes");
        exit(EXIT_FAILURE);
    }

    pid_t pidZ = fork();
    if (pidZ < 0) {
        perror("fork zonas");
        exit(EXIT_FAILURE);
    }
    if (pidZ == 0) {
        close(pipe_zone[1]);
        zone_process(graph, pipe_zone[0], config);
        exit(EXIT_SUCCESS);
    }

    pid_t pidT = fork();
    if (pidT < 0) {
        perror("fork tráfico");
        exit(EXIT_FAILURE);
    }
    if (pidT == 0) {
        close(pipe_traffic[1]);
        traffic_process(graph, pipe_traffic[0], config);
        exit(EXIT_SUCCESS);
    }

    // Proceso principal
    close(pipe_zone[0]);
    close(pipe_traffic[0]);
    main_process_loop(graph, pipe_zone[1], pipe_traffic[1], config);

    waitpid(pidZ, NULL, 0);
    waitpid(pidT, NULL, 0);
}

void zone_process(CityGraph *graph, int read_pipe, Config config) {
    (void)graph;
    (void)read_pipe;
    (void)config;
    // Aquí iría el ciclo de lectura de ZoneMessage vía read_pipe
    // y creación de hilos para manejar cada zona.
    // Por ahora, un stub que no hace nada:
    for (;;) pause();
}

void traffic_process(CityGraph *graph, int read_pipe, Config config) {
    (void)graph;
    (void)read_pipe;
    (void)config;
    // Stub: recibir TrafficUpdate y lanzar hilos por caravana
    for (;;) pause();
}

void main_process_loop(CityGraph *graph,
                       int zone_pipe,
                       int traffic_pipe,
                       Config config)
{
    (void)graph;
    (void)zone_pipe;
    (void)traffic_pipe;
    (void)config;
    // Stub: el proceso principal podría distribuir “DAY_START”/“DAY_END”
    // a los pipes hijos cada tick, etc. Por ahora, mantenemos vivo:
    for (;;) pause();
}
