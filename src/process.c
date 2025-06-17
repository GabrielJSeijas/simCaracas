// src/process.c

#include "process.h"
#include "graph.h"
#include "pipes.h"
#include <unistd.h>    // pipe(), fork(), pause()
#include <sys/wait.h>  // waitpid()
#include <stdlib.h>    // exit()
#include <stdio.h>     // perror()

void iniciarProcesosHijos(GrafoCiudad *grafo, Configuracion configuracion) {
    int tuberiaZona[2], tuberiaTransito[2];

    if (pipe(tuberiaZona) == -1 || pipe(tuberiaTransito) == -1) {
        perror("Error al crear tuberías");
        exit(EXIT_FAILURE);
    }

    pid_t pidZona = fork();
    if (pidZona < 0) {
        perror("Error en fork (zona)");
        exit(EXIT_FAILURE);
    }
    if (pidZona == 0) {
        close(tuberiaZona[1]);
        procesoZona(grafo, tuberiaZona[0], configuracion);
        exit(EXIT_SUCCESS);
    }

    pid_t pidTransito = fork();
    if (pidTransito < 0) {
        perror("Error en fork (tránsito)");
        exit(EXIT_FAILURE);
    }
    if (pidTransito == 0) {
        close(tuberiaTransito[1]);
        procesoTransito(grafo, tuberiaTransito[0], configuracion);
        exit(EXIT_SUCCESS);
    }

    // Proceso padre: cierra extremos de lectura
    close(tuberiaZona[0]);
    close(tuberiaTransito[0]);
    bucleProcesoPrincipal(grafo,
                          tuberiaZona[1],
                          tuberiaTransito[1],
                          configuracion);

    // Esperar a que terminen los hijos
    waitpid(pidZona, NULL, 0);
    waitpid(pidTransito, NULL, 0);
}

void* hiloZona(void* arg) {
    Zona *z = (Zona*)arg;
    pthread_mutex_lock(&z->mutexZona);
    
    // Implementación temporal - reemplaza con tu lógica real
    printf("Hilo zona procesando %s\n", z->codigo);
    
    pthread_mutex_unlock(&z->mutexZona);
    return NULL;
}
void procesoZona(GrafoCiudad *grafo, int tuberiaLectura, Configuracion config) {
      (void)config; // Marcar como no usado para silenciar warning
    
    MensajeZona msg;
    
    while (read(tuberiaLectura, &msg, sizeof(MensajeZona)) > 0) {
        switch (msg.tipo) {
            case INICIO_DIA:
                // Lanzar hilos para cada zona
                for (int i = 0; i < grafo->totalZonas; i++) {
                    pthread_t hilo;
                    pthread_create(&hilo, NULL, hiloZona, &grafo->zonas[i]);
                }
                break;
            case FIN_DIA:
                // Actualizar estadísticas
                break;
                case ACTUALIZACION_ZONA:
    case ACTUALIZACION_TRAFICO:
        // Manejar estos casos o dejarlos vacíos
        break;
    default:
        fprintf(stderr, "Tipo de mensaje desconocido: %d\n", msg.tipo);
    
        }
    }
}


void procesoTransito(GrafoCiudad *grafo,
                     int tuberiaLectura,
                     Configuracion configuracion)
{
    (void)grafo;
    (void)tuberiaLectura;
    (void)configuracion;
    // Aquí se leerían ActualizacionTransito de la tubería y se lanzarían hilos
    for (;;) pause();
}

void bucleProcesoPrincipal(GrafoCiudad *grafo,
                           int tuberiaZona,
                           int tuberiaTransito,
                           Configuracion configuracion)
{
    (void)grafo;
    (void)tuberiaZona;
    (void)tuberiaTransito;
    (void)configuracion;
    // Aquí se enviarían INICIO_DIA y FIN_DIA a los procesos hijos
    for (;;) pause();
}

static void buscarEmpleoParaZona(Zona* zona) {
    pthread_mutex_lock(&zona->mutexZona);
    if (zona->esFuente) {
        // Lógica para buscar empleo para residentes
        printf("Buscando empleo para residentes en %s\n", zona->codigo);
    }
    pthread_mutex_unlock(&zona->mutexZona);
}

static void buscarEmpleadosParaZona(Zona* zona) {
    pthread_mutex_lock(&zona->mutexZona);
    if (!zona->esFuente) {
        // Lógica para buscar empleados para puestos de trabajo
        printf("Buscando empleados para puestos en %s\n", zona->codigo);
    }
    pthread_mutex_unlock(&zona->mutexZona);
}