#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "config.h"
#include "graph.h"
#include "fileio.h"
#include "traffic.h"
#include "utils.h"
#include "pipes.h"
#include "process.h"

// Variables globales para pipes y PIDs
int tuberiaZonaEsc = -1, tuberiaZonaLec = -1, tuberiaTransitoEsc = -1, tuberiaTransitoLec = -1;
pid_t pidZona = -1, pidTransito = -1;

// Prototipo de hiloZona (debe estar en process.h también)
void* hiloZona(void* arg);

// Muestra las opciones del menú principal
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
    // Procesar argumentos y configuración
    Configuracion configuracion = parsearArgumentos(argc, argv);
    mostrarConfiguracion(&configuracion);

    // Inicializar el grafo
    GrafoCiudad grafo;
    iniciarGrafo(&grafo);
    grafo.nivelMaximoZonas = configuracion.nivelMaximoZona;

    // Cargar ciudad desde CSV si se especifica
    if (configuracion.archivoCarga) {
        printf("Cargando ciudad desde '%s'...\n", configuracion.archivoCarga);
        if (!cargarCiudadDesdeCSV(&grafo, configuracion.archivoCarga)) {
            fprintf(stderr, "Error: No se pudo cargar '%s'\n", configuracion.archivoCarga);
            printf("Iniciando con grafo vacío\n");
        } else {
            printf("Ciudad cargada exitosamente con %d zonas\n", grafo.totalZonas);
            asignarCoordenadas(&grafo);
        }
    }

    // Inicializar procesos hijos y pipes
    reiniciarProcesosHijos(&grafo, configuracion);

    // Menú principal de simulación
    int salir = 0, dia = 1;
    char entrada[16];
    while (!salir) {
        printf("\n==============================\n");
        printf("        >>> Día %d <<<\n", dia);
        printf("==============================\n");

        if (dia == 1 ) {
            printf("Simulación iniciada. Este es el mundo actual...\n\n");
            imprimirCiudadEnGridAvanzado(&grafo);
        }

        while (1) {
            mostrarOpcionesMenuPrincipal();
            if (!fgets(entrada, sizeof(entrada), stdin)) break;
            int opcion = atoi(entrada);

            switch (opcion) {
                case 1:
                    agregarZonaMenu(&grafo);
                    reiniciarProcesosHijos(&grafo, configuracion);
                    break;
                case 2:
                    agregarArteriaMenu(&grafo);
                    reiniciarProcesosHijos(&grafo, configuracion);
                    break;
                case 3:
                    ampliarArteriaMenu(&grafo);
                    reiniciarProcesosHijos(&grafo, configuracion);
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
        if (!salir && entrada[0] == '5') {
            // 1. Enviar INICIO_DIA a los hijos por pipes
            enviarInicioDia(tuberiaZonaEsc);
            enviarInicioDia(tuberiaTransitoEsc);

            // 2. Simular la mañana (empleados van a trabajar, tráfico, etc)
            simularTraficoMatutino(&grafo, configuracion.ticksPorDia / 2);
            sumarPuntosSumideros(&grafo);

            // 3. Tareas por zona en hilos (sólo locales a cada zona)
            pthread_t hilos[MAX_ZONAS];
            for (int i = 0; i < grafo.totalZonas; i++) {
                pthread_create(&hilos[i], NULL, hiloZona, &grafo.zonas[i]);
            }
            for (int i = 0; i < grafo.totalZonas; i++) {
                pthread_join(hilos[i], NULL);
            }

            // 4. Actualizar puntos de zona después de la mañana
            actualizarPuntosZona(&grafo, true);

            // 5. Simular la tarde (empleados regresan)
            simularTraficoVespertino(&grafo, configuracion.ticksPorDia / 2);
            sumarPuntosFuentes(&grafo);
            actualizarPuntosZona(&grafo, false);

            // 6. Actualizar totales empleados/desempleados
            actualizarTotalesEmpleo(&grafo);

            // 7. Esperar respuestas de los hijos
            MensajeDia msgZona, msgTransito;
            recibirMensaje(tuberiaZonaLec, &msgZona, sizeof(MensajeDia));
            recibirMensaje(tuberiaTransitoLec, &msgTransito, sizeof(MensajeDia));

            printf("\033[2J\033[H"); // Limpia la pantalla
            printf("\n--- Estado final del día ---\n");
            imprimirCiudadEnGridAvanzado(&grafo);
            mostrarResumenEmpleo(&grafo);

        }
        dia++;
    }

    // Finalización y liberación de recursos
    if (pidZona > 0) { kill(pidZona, SIGTERM); waitpid(pidZona, NULL, 0); }
    if (pidTransito > 0) { kill(pidTransito, SIGTERM); waitpid(pidTransito, NULL, 0); }
    if (tuberiaZonaEsc != -1) close(tuberiaZonaEsc);
    if (tuberiaZonaLec != -1) close(tuberiaZonaLec);
    if (tuberiaTransitoEsc != -1) close(tuberiaTransitoEsc);
    if (tuberiaTransitoLec != -1) close(tuberiaTransitoLec);
    destruirGrafo(&grafo);
    free(configuracion.archivoCarga);
    printf("\nSimulación finalizada.\n");
    return 0;
}
