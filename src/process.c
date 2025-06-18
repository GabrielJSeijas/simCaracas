// src/process.c

#include "process.h"
#include "graph.h"
#include "pipes.h"
#include <unistd.h>    // pipe(), fork(), close(), exit()
#include <sys/wait.h>  // waitpid()
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "traffic.h"
#include "zone.h"
#include <signal.h>

extern int tuberiaZonaEsc, tuberiaZonaLec, tuberiaTransitoEsc, tuberiaTransitoLec;
extern pid_t pidZona, pidTransito;

// Inicia los procesos hijos y configura las tuberías de comunicación
void iniciarProcesosHijos(GrafoCiudad *grafo, Configuracion configuracion,
                          int *tuberiaZonaEsc, int *tuberiaZonaLec,
                          int *tuberiaTransitoEsc, int *tuberiaTransitoLec,
                          pid_t *pidZona, pid_t *pidTransito)
{
    int pipeZonaPadreAHijo[2], pipeZonaHijoAPadre[2];
    int pipeTransitoPadreAHijo[2], pipeTransitoHijoAPadre[2];

    // Crear pipes para zona
    if (pipe(pipeZonaPadreAHijo) == -1 || pipe(pipeZonaHijoAPadre) == -1) {
        perror("Error al crear tuberías zona");
        exit(EXIT_FAILURE);
    }
    // Crear pipes para tráfico
    if (pipe(pipeTransitoPadreAHijo) == -1 || pipe(pipeTransitoHijoAPadre) == -1) {
        perror("Error al crear tuberías transito");
        exit(EXIT_FAILURE);
    }

    pid_t zonaPID = fork();
    if (zonaPID < 0) { perror("Error en fork (zona)"); exit(EXIT_FAILURE); }
    if (zonaPID == 0) {
        // Proceso hijo Zona
        close(pipeZonaPadreAHijo[1]); // Solo lee del padre
        close(pipeZonaHijoAPadre[0]); // Solo escribe al padre
        // Cerrar pipes de tráfico
        close(pipeTransitoPadreAHijo[0]); close(pipeTransitoPadreAHijo[1]);
        close(pipeTransitoHijoAPadre[0]); close(pipeTransitoHijoAPadre[1]);
        procesoZona(grafo, pipeZonaPadreAHijo[0], pipeZonaHijoAPadre[1], configuracion);
        exit(EXIT_SUCCESS);
    }

    pid_t transitoPID = fork();
    if (transitoPID < 0) { perror("Error en fork (tránsito)"); exit(EXIT_FAILURE); }
    if (transitoPID == 0) {
        // Proceso hijo Tráfico
        close(pipeTransitoPadreAHijo[1]); // Solo lee del padre
        close(pipeTransitoHijoAPadre[0]); // Solo escribe al padre
        // Cerrar pipes de zona
        close(pipeZonaPadreAHijo[0]); close(pipeZonaPadreAHijo[1]);
        close(pipeZonaHijoAPadre[0]); close(pipeZonaHijoAPadre[1]);
        procesoTransito(grafo, pipeTransitoPadreAHijo[0], pipeTransitoHijoAPadre[1], configuracion);
        exit(EXIT_SUCCESS);
    }

    // Proceso padre: cerrar extremos no usados
    close(pipeZonaPadreAHijo[0]);      // Solo escribe
    close(pipeZonaHijoAPadre[1]);      // Solo lee
    close(pipeTransitoPadreAHijo[0]);  // Solo escribe
    close(pipeTransitoHijoAPadre[1]);  // Solo lee

    *tuberiaZonaEsc = pipeZonaPadreAHijo[1];
    *tuberiaZonaLec = pipeZonaHijoAPadre[0];
    *tuberiaTransitoEsc = pipeTransitoPadreAHijo[1];
    *tuberiaTransitoLec = pipeTransitoHijoAPadre[0];
    *pidZona = zonaPID;
    *pidTransito = transitoPID;
}

// Función que ejecuta cada hilo de zona
void* hiloZona(void* arg) {
    Zona *z = (Zona*)arg;
    pthread_mutex_lock(&z->mutexZona);

    // Sumar puntos por empleados presentes
    if (z->empleadosPresentes > 0) {
        z->puntos += z->empleadosPresentes;
    }

    pthread_mutex_unlock(&z->mutexZona);
    return NULL;
}

// Proceso hijo: Zonas
void procesoZona(GrafoCiudad *grafo, int tuberiaLectura, int tuberiaEscritura, Configuracion config) {
     (void)config;
    while (1) {
        MensajeDia msg;
        if (!recibirMensaje(tuberiaLectura, &msg, sizeof(MensajeDia))) continue;

        if (msg.tipo == INICIO_DIA) {
            // Lanzar hilos para cada zona
            pthread_t hilos[MAX_ZONAS];
            for (int i = 0; i < grafo->totalZonas; i++) {
                pthread_create(&hilos[i], NULL, hiloZona, &grafo->zonas[i]);
            }
            for (int i = 0; i < grafo->totalZonas; i++) {
                pthread_join(hilos[i], NULL);
            }

            // Actualizar información de zonas
            actualizarPuntosZona(grafo, true);
            actualizarPuntosZona(grafo, false);
            actualizarTotalesEmpleo(grafo);

            // Enviar FIN_DIA al padre
            enviarFinDia(tuberiaEscritura);
        }
    }
}

// Proceso hijo: Tráfico
void procesoTransito(GrafoCiudad *grafo, int tuberiaLectura, int tuberiaEscritura, Configuracion configuracion) {
    while (1) {
        MensajeDia msg;
        if (!recibirMensaje(tuberiaLectura, &msg, sizeof(MensajeDia))) continue;

        if (msg.tipo == INICIO_DIA) {
            // Simulación de tráfico matutino y vespertino
            simularTraficoMatutino(grafo, configuracion.ticksPorDia / 2);
            simularTraficoVespertino(grafo, configuracion.ticksPorDia / 2);

            // Enviar FIN_DIA al padre
            enviarFinDia(tuberiaEscritura);
        }
    }
}

// Reinicia los procesos hijos y sus tuberías
void reiniciarProcesosHijos(GrafoCiudad *grafo, Configuracion configuracion) {
    if (pidZona > 0) { kill(pidZona, SIGTERM); waitpid(pidZona, NULL, 0); }
    if (pidTransito > 0) { kill(pidTransito, SIGTERM); waitpid(pidTransito, NULL, 0); }

    if (tuberiaZonaEsc != -1) close(tuberiaZonaEsc);
    if (tuberiaZonaLec != -1) close(tuberiaZonaLec);
    if (tuberiaTransitoEsc != -1) close(tuberiaTransitoEsc);
    if (tuberiaTransitoLec != -1) close(tuberiaTransitoLec);

    iniciarProcesosHijos(grafo, configuracion,
                         &tuberiaZonaEsc, &tuberiaZonaLec,
                         &tuberiaTransitoEsc, &tuberiaTransitoLec,
                         &pidZona, &pidTransito);
}
