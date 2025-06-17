#include "fileio.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINEA_CSV 1024
#define NUM_COLUMNAS_CSV 12

// Función auxiliar para limpiar espacios y comillas
static char* limpiar_token(char* token) {
    if (!token) return token;
    
    // Eliminar comillas si existen
    if (token[0] == '"' && token[strlen(token)-1] == '"') {
        token[strlen(token)-1] = '\0';
        token++;
    }
    
    // Eliminar espacios en blanco alrededor
    while (isspace((unsigned char)*token)) token++;
    
    char *end = token + strlen(token) - 1;
    while (end > token && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    
    return token;
}

bool cargarCiudadDesdeCSV(GrafoCiudad *ciudad, const char *rutaArchivo) {
    if (!ciudad || !rutaArchivo) {
        fprintf(stderr, "Parámetros inválidos\n");
        return false;
    }

    FILE *fp = fopen(rutaArchivo, "r");
    if (!fp) {
        fprintf(stderr, "Error abriendo '%s': %s\n", rutaArchivo, strerror(errno));
        return false;
    }

    char linea[MAX_LINEA_CSV];
    
    // Saltar cabecera
    if (!fgets(linea, sizeof(linea), fp)) {
        fclose(fp);
        fprintf(stderr, "Archivo vacío o sin cabecera\n");
        return false;
    }

    unsigned int line_num = 1;
    while (fgets(linea, sizeof(linea), fp)) {
        line_num++;
        
        // Eliminar salto de línea
        linea[strcspn(linea, "\r\n")] = '\0';
        
        // Saltar líneas vacías
        if (strlen(linea) == 0) continue;

        char *partes[NUM_COLUMNAS_CSV];
        char *saveptr = NULL;
        int n = 0;

        for (char *tok = strtok_r(linea, ",", &saveptr); 
             tok && n < NUM_COLUMNAS_CSV; 
             tok = strtok_r(NULL, ",", &saveptr)) {
            partes[n++] = limpiar_token(tok);
        }

        if (n != NUM_COLUMNAS_CSV) {
            fprintf(stderr, "Error en línea %d: Número incorrecto de columnas (%d)\n", 
                    line_num, n);
            continue;
        }

        // Procesar datos de la zona
        const char *codigo = partes[0];
        if (strlen(codigo) != 3) {
            fprintf(stderr, "Código de zona inválido en línea %d: %s\n", line_num, codigo);
            continue;
        }

        bool esFuente = (partes[1][0] == 'F' || partes[1][0] == 'f');
        int nivel = atoi(partes[2]);
        int puntos = atoi(partes[3]);

        // Validar nivel
        if (nivel < 1 || nivel > ciudad->nivelMaximoZonas) {
            fprintf(stderr, "Nivel inválido en línea %d: %d (debe ser 1-%d)\n", 
                    line_num, nivel, ciudad->nivelMaximoZonas);
            continue;
        }

        // Agregar zona al grafo
        Zona *z = agregarZona(ciudad, codigo, nivel, esFuente);
        if (!z) {
            fprintf(stderr, "Error agregando zona %s en línea %d\n", codigo, line_num);
            continue;
        }
        z->puntos = puntos;

        // Estructura temporal para conexiones
        struct {
            Direccion dir;
            const char *vecina;
            int capacidad;
        } conexiones[4] = {
            {NORTE, partes[4],  atoi(partes[5])},
            {SUR,   partes[6],  atoi(partes[7])},
            {ESTE,  partes[8],  atoi(partes[9])},
            {OESTE, partes[10], atoi(partes[11])}
        };

        // Procesar conexiones
        for (int i = 0; i < 4; i++) {
            if (strlen(conexiones[i].vecina) > 0) {
                Zona *vecina = buscarZona(ciudad, conexiones[i].vecina);
                if (vecina) {
                    if (!conectarZonas(ciudad, z, vecina, conexiones[i].dir, conexiones[i].capacidad)) {
                        fprintf(stderr, "Error conectando %s con %s en línea %d\n", 
                                z->codigo, vecina->codigo, line_num);
                    }
                } else {
                    fprintf(stderr, "Zona vecina no encontrada: %s en línea %d\n", 
                            conexiones[i].vecina, line_num);
                }
            }
        }
    }

    fclose(fp);

    // Calcular estadísticas iniciales
    pthread_rwlock_wrlock(&ciudad->cerrojoGrafo);
    
    ciudad->totalEmpleados = 0;
    ciudad->totalDesempleados = 0;
    
    for (int i = 0; i < ciudad->totalZonas; i++) {
        Zona *z = &ciudad->zonas[i];
        if (z->esFuente) {
            ciudad->totalDesempleados += z->disponibles;
        } else {
            ciudad->totalEmpleados += (1 << z->nivel) - z->disponibles;
        }
    }
    
    pthread_rwlock_unlock(&ciudad->cerrojoGrafo);
    
    printf("Estadísticas iniciales:\n");
    printf(" - Total zonas: %d\n", ciudad->totalZonas);
    printf(" - Empleados: %d\n", ciudad->totalEmpleados);
    printf(" - Desempleados: %d\n", ciudad->totalDesempleados);
    
    return true;
}

bool guardarCiudadEnCSV(const GrafoCiudad *ciudad, const char *rutaArchivo) {
    if (!ciudad || !rutaArchivo) {
        fprintf(stderr, "Parámetros inválidos\n");
        return false;
    }

    FILE *fp = fopen(rutaArchivo, "w");
    if (!fp) {
        fprintf(stderr, "Error creando '%s': %s\n", rutaArchivo, strerror(errno));
        return false;
    }

    // Escribir cabecera
    fprintf(fp, "codigo,tipo,nivel,puntos,"
                "norte,cap_norte,"
                "sur,cap_sur,"
                "este,cap_este,"
                "oeste,cap_oeste\n");

    // Escribir cada zona
    for (int i = 0; i < ciudad->totalZonas; i++) {
        const Zona *z = &ciudad->zonas[i];
        
        fprintf(fp, "%s,%c,%d,%d,"
                    "%s,%d,"
                    "%s,%d,"
                    "%s,%d,"
                    "%s,%d\n",
                z->codigo,
                z->esFuente ? 'F' : 'S',
                z->nivel,
                z->puntos,
                z->norte   ? z->norte->codigo   : "",
                z->capacidadNorte,
                z->sur     ? z->sur->codigo     : "",
                z->capacidadSur,
                z->este    ? z->este->codigo    : "",
                z->capacidadEste,
                z->oeste   ? z->oeste->codigo   : "",
                z->capacidadOeste);
    }

    fclose(fp);
    return true;
}