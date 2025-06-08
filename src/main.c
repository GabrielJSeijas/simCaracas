// src/main.c

#include <stdio.h>
#include <stdlib.h>

#include "config.h"    // parsearArgumentos(), mostrarConfiguracion()
#include "graph.h"     // GrafoCiudad, iniciarGrafo(), destruirGrafo()
#include "fileio.h"    // cargarCiudadDesdeCSV(), guardarCiudadEnCSV()
#include "traffic.h"   // inicializarSistemaTrafico()
#include "menu.h"      // iniciarMenuInteractivo()

int main(int argc, char *argv[]) {
    // 1) Procesar argumentos y mostrar la configuración
    Configuracion configuracion = parsearArgumentos(argc, argv);
    mostrarConfiguracion(&configuracion);

    // 2) Inicializar el grafo de la ciudad
    GrafoCiudad grafo;
    iniciarGrafo(&grafo);

    // 3) Cargar ciudad desde CSV si se indicó archivo
    if (configuracion.archivoCarga) {
        if (!cargarCiudadDesdeCSV(&grafo, configuracion.archivoCarga)) {
            fprintf(stderr, "Error al cargar '%s'\n", configuracion.archivoCarga);
        }
    }

    // 4) Preparar el sistema de tráfico
    inicializarSistemaTrafico(&grafo);

    // 5) Arrancar el menú interactivo
    iniciarMenuInteractivo(&grafo, configuracion);

    // 6) Al salir del menú, limpiar y liberar recursos
    destruirGrafo(&grafo);
    free(configuracion.archivoCarga);

    return EXIT_SUCCESS;
}
