#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "graph.h"
#include "fileio.h"
#include "traffic.h"
#include "utils.h"

// Declaraciones de funciones de acciones del menú (deberían estar en menu.c/.h o aquí directamente)
void agregarZonaMenu(GrafoCiudad *grafo);
void agregarArteriaMenu(GrafoCiudad *grafo);
void ampliarArteriaMenu(GrafoCiudad *grafo);
void guardarGrafoMenu(GrafoCiudad *grafo);

void mostrarOpcionesMenuPrincipal(void) {
    printf("\n==== Menú SimCaracas ====\n");
    printf("1) Agregar zona\n");
    printf("2) Agregar arteria vial\n");
    printf("3) Ampliar arteria vial\n");
    printf("4) Guardar grafo actual\n");
    printf("5) Simular siguiente día\n");
    printf("6) Salir\n");
    printf("Seleccione una opción: ");
}

int main(int argc, char *argv[]) {
    // 1) Procesar argumentos
    Configuracion configuracion = parsearArgumentos(argc, argv);
    mostrarConfiguracion(&configuracion);

    // 2) Inicializar el grafo
    GrafoCiudad grafo;
    iniciarGrafo(&grafo);
    grafo.nivelMaximoZonas = configuracion.nivelMaximoZona;

    // 3) Cargar ciudad desde CSV
    if (configuracion.archivoCarga) {
        printf("Cargando ciudad desde '%s'...\n", configuracion.archivoCarga);
        if (!cargarCiudadDesdeCSV(&grafo, configuracion.archivoCarga)) {
            fprintf(stderr, "Error: No se pudo cargar '%s'\n", configuracion.archivoCarga);
            printf("Iniciando con grafo vacío\n");
        } else {
            printf("Ciudad cargada exitosamente con %d zonas\n", grafo.totalZonas);
            printf("\n");
            printf("\n");
            asignarCoordenadas(&grafo);  // <--- COORDENADAS
            imprimirCiudadEnGridAvanzado(&grafo); // <--- GRID
        }
    }

    printf("\n=== Comienza la simulación ===\n");

    int salir = 0, dia = 1;
    while (!salir) {
        printf("\n==============================\n");
        printf("        >>> Día %d <<<\n          ", dia);
        printf("==============================\n");

        // 1. Simula el día completo (mañana y tarde, etc.)
        simularDia(&grafo, configuracion);

        // 2. Muestra el grafo actualizado al terminar el día
        mostrarGrafo(&grafo);

        // 3. Menú de acciones
        while (1) {
            mostrarOpcionesMenuPrincipal();
            char entrada[16];
            if (!fgets(entrada, sizeof(entrada), stdin)) break;
            int opcion = atoi(entrada);

            switch (opcion) {
                case 1:
                    agregarZonaMenu(&grafo);
                    break;
                case 2:
                    agregarArteriaMenu(&grafo);
                    break;
                case 3:
                    ampliarArteriaMenu(&grafo);
                    break;
                case 4:
                    guardarGrafoMenu(&grafo);
                    break;
                case 5: // Simular siguiente día
                    goto fin_menu;
                case 6: // Salir
                    salir = 1;
                    goto fin_menu;
                default:
                    printf("Opción inválida. Intente de nuevo.\n");
            }
        }
    fin_menu:
        dia++;
    }

    destruirGrafo(&grafo);
    free(configuracion.archivoCarga);
    printf("\nSimulación finalizada.\n");
    return 0;
}
