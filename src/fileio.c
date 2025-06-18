#include "fileio.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINEA_CSV 1024
#define NUM_COLUMNAS_CSV 12

// Elimina espacios y comillas de un campo CSV
static char* limpiar_token(char* token) {
    if (!token) return "";
    size_t len = strlen(token);
    if (len >= 2 && token[0] == '"' && token[len-1] == '"') {
        token[len-1] = '\0';
        token++;
    }
    while (isspace((unsigned char)*token)) token++;
    char *end = token + strlen(token) - 1;
    while (end > token && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return token;
}

// Divide una línea CSV en exactamente 12 columnas
static int partir_csv_12campos(char *linea, char *partes[NUM_COLUMNAS_CSV]) {
    int n = 0;
    char *p = linea;
    char *end;
    while (n < NUM_COLUMNAS_CSV) {
        end = strchr(p, ',');
        if (end) {
            *end = '\0';
            partes[n++] = limpiar_token(p);
            p = end + 1;
            if (*p == ',' || *p == '\0') {
                if (n < NUM_COLUMNAS_CSV) partes[n++] = "";
                if (*p) p++;
            }
        } else {
            partes[n++] = limpiar_token(p);
            break;
        }
    }
    while (n < NUM_COLUMNAS_CSV)
        partes[n++] = "";
    return n;
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

    // Estructura auxiliar para guardar conexiones de cada zona
    typedef struct {
        char codigo[8];
        char vecinos[4][8]; // NORTE, SUR, ESTE, OESTE
        int capacidades[4];
    } ZonaConexiones;
    ZonaConexiones temp[1024]; // Máximo 1024 zonas
    int count = 0;

    // PASADA 1: Agregar zonas y guardar conexiones
    if (!fgets(linea, sizeof(linea), fp)) {
        fclose(fp);
        fprintf(stderr, "Archivo vacío o sin cabecera\n");
        return false;
    }

    unsigned int line_num = 1;
    while (fgets(linea, sizeof(linea), fp)) {
        line_num++;
        linea[strcspn(linea, "\r\n")] = '\0';
        if (strlen(linea) == 0) continue;

        char *partes[NUM_COLUMNAS_CSV];
        int n = partir_csv_12campos(linea, partes);
        if (n != NUM_COLUMNAS_CSV) {
            fprintf(stderr, "Error en línea %d: Número incorrecto de columnas (%d)\n", line_num, n);
            continue;
        }

        const char *codigo = partes[0];
        if (strlen(codigo) != 3) {
            fprintf(stderr, "Código de zona inválido en línea %d: %s\n", line_num, codigo);
            continue;
        }

        bool esFuente = (partes[1][0] == 'F' || partes[1][0] == 'f');
        int nivel = atoi(partes[2]);
        int puntos = atoi(partes[3]);
        if (nivel < 1 || nivel > ciudad->nivelMaximoZonas) {
            fprintf(stderr, "Nivel inválido en línea %d: %d (debe ser 1-%d)\n", line_num, nivel, ciudad->nivelMaximoZonas);
            continue;
        }

        Zona *z = agregarZona(ciudad, codigo, nivel, esFuente);
        if (!z) {
            fprintf(stderr, "Error agregando zona %s en línea %d\n", codigo, line_num);
            continue;
        }
        z->puntos = puntos;

        // Guardar conexiones para la segunda pasada
        strncpy(temp[count].codigo, codigo, 7); temp[count].codigo[7] = '\0';
        for (int i = 0; i < 4; i++) {
            strncpy(temp[count].vecinos[i], partes[4 + i*2], 7);
            temp[count].vecinos[i][7] = '\0';
            temp[count].capacidades[i] = atoi(partes[5 + i*2]);
        }
        count++;
    }

    fclose(fp);

    // PASADA 2: Conectar zonas según las conexiones guardadas
    for (int i = 0; i < count; i++) {
        Zona *z = buscarZona(ciudad, temp[i].codigo);
        for (int d = 0; d < 4; d++) {
            if (strlen(temp[i].vecinos[d]) > 0) {
                Zona *vecina = buscarZona(ciudad, temp[i].vecinos[d]);
                if (vecina) {
                    conectarZonas(ciudad, z, vecina, (Direccion)d, temp[i].capacidades[d]);
                } else {
                    fprintf(stderr, "Zona vecina no encontrada: %s para %s\n", temp[i].vecinos[d], temp[i].codigo);
                }
            }
        }
    }

    // Calcular estadísticas iniciales de la ciudad
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

    // Escribir cabecera CSV
    fprintf(fp, "codigo,tipo,nivel,puntos,"
                "norte,cap_norte,"
                "sur,cap_sur,"
                "este,cap_este,"
                "oeste,cap_oeste\n");

    // Escribir datos de cada zona
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
