#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parsea los argumentos de línea de comandos y retorna la configuración resultante
Configuracion parsearArgumentos(int argc, char *argv[]) {
    // Valores por defecto
    Configuracion configuracion = {
        .ticksPorDia      = 12,
        .segundosPorTick  = 1.0f,
        .nivelMaximoZona  = 10,
        .archivoCarga     = NULL
    };
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            configuracion.ticksPorDia = atoi(argv[++i]);
            if (configuracion.ticksPorDia <= 0) {
                configuracion.ticksPorDia = 64;
            }
        }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            configuracion.segundosPorTick = atof(argv[++i]);
            if (configuracion.segundosPorTick <= 0.0f) {
                configuracion.segundosPorTick = 1.0f;
            }
        }
        else if (strcmp(argv[i], "-z") == 0 && i + 1 < argc) {
            configuracion.nivelMaximoZona = atoi(argv[++i]);
            if (configuracion.nivelMaximoZona <= 0) {
                configuracion.nivelMaximoZona = 10;
            }
        }
        else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            // Guardar una copia del nombre de archivo
            configuracion.archivoCarga = strdup(argv[++i]);
        }
    }
    
    return configuracion;
}

// Muestra la configuración actual por pantalla
void mostrarConfiguracion(const Configuracion *config) {
    printf("Parámetros de la simulación:\n");
    printf("  • Ticks por día      : %d\n",   config->ticksPorDia);
    printf("  • Segundos por tick  : %.2f\n", config->segundosPorTick);
    printf("  • Nivel máximo zonas : %d\n",   config->nivelMaximoZona);
    printf("  • Archivo inicial    : %s\n\n",
           config->archivoCarga ? config->archivoCarga : "Ninguno");
}
