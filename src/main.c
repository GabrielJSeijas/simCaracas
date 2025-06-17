// src/main.c

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "graph.h"
#include "fileio.h"
#include "traffic.h"
#include "menu.h"
#include "process.h"  // Asegúrate de incluir esto

int main(int argc, char *argv[]) {
    // 1) Procesar argumentos
    Configuracion configuracion = parsearArgumentos(argc, argv);
    mostrarConfiguracion(&configuracion);

    // 2) Inicializar el grafo
    GrafoCiudad grafo;
    iniciarGrafo(&grafo);
    grafo.nivelMaximoZonas = configuracion.nivelMaximoZona; // Importante!

    // 3) Cargar ciudad desde CSV
    if (configuracion.archivoCarga) {
        printf("Cargando ciudad desde '%s'...\n", configuracion.archivoCarga);
        if (!cargarCiudadDesdeCSV(&grafo, configuracion.archivoCarga)) {
            fprintf(stderr, "Error: No se pudo cargar '%s'\n", 
                   configuracion.archivoCarga);
            // Continuar con grafo vacío
            printf("Iniciando con grafo vacío\n");
        } else {
            printf("Ciudad cargada exitosamente con %d zonas\n", 
                  grafo.totalZonas);
        }
    }

    // 4) Mostrar estado inicial del grafo
    mostrarGrafo(&grafo);

    // 5) Iniciar procesos hijos (zona y tráfico)
    iniciarProcesosHijos(&grafo, configuracion);

    // 6) Preparar sistema de tráfico
    inicializarSistemaTrafico(&grafo);

    // 7) Iniciar menú interactivo
    iniciarMenuInteractivo(&grafo, configuracion);

    // 8) Limpieza
    destruirGrafo(&grafo);
    free(configuracion.archivoCarga);

    return EXIT_SUCCESS;
}