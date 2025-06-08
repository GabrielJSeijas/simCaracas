// src/fileio.c

#include "fileio.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LONGITUD_LINEA 1024

bool cargarCiudadDesdeCSV(GrafoCiudad *ciudad,
                          const char *rutaArchivo)
{
    FILE *fp = fopen(rutaArchivo, "r");
    if (!fp) {
        fprintf(stderr, "No se pudo abrir '%s': %s\n",
                rutaArchivo, strerror(errno));
        return false;
    }

    char linea[LONGITUD_LINEA];
    // Saltar cabecera
    if (!fgets(linea, sizeof linea, fp)) {
        fclose(fp);
        return false;
    }

    while (fgets(linea, sizeof linea, fp)) {
        char *partes[12];
        char *saveptr = NULL;
        int   n       = 0;

        for (char *tok = strtok_r(linea, ",", &saveptr);
             tok && n < 12;
             tok = strtok_r(NULL, ",", &saveptr))
        {
            // Quitar comillas si existen
            if (tok[0]=='"' && tok[strlen(tok)-1]=='"') {
                tok[strlen(tok)-1] = '\0';
                tok++;
            }
            partes[n++] = tok;
        }
        if (n < 12) continue;

        const char *codigo   = partes[0];
        bool        esFuente = (partes[1][0]=='F');
        int         nivel    = atoi(partes[2]);
        int         puntos   = atoi(partes[3]);

        // Crear la zona en el grafo
        Zona *z = agregarZona(ciudad, codigo, nivel, esFuente);
        if (!z) continue;
        z->puntos = puntos;

        struct {
            Direccion dir;
            const char *vecina;
            const char *cap;
        } conns[4] = {
            { NORTE, partes[4],  partes[5]  },
            { SUR,   partes[6],  partes[7]  },
            { ESTE,  partes[8],  partes[9]  },
            { OESTE, partes[10], partes[11] }
        };

        for (int i = 0; i < 4; i++) {
            if (conns[i].vecina[0] != '\0') {
                Zona *otra = buscarZona(ciudad, conns[i].vecina);
                if (otra) {
                    int cap = atoi(conns[i].cap);
                    conectarZonas(ciudad, z, otra, conns[i].dir, cap);
                }
            }
        }
    }

    fclose(fp);
    return true;
}

bool guardarCiudadEnCSV(const GrafoCiudad *ciudad,
                        const char *rutaArchivo)
{
    FILE *fp = fopen(rutaArchivo, "w");
    if (!fp) {
        fprintf(stderr, "No se pudo crear '%s': %s\n",
                rutaArchivo, strerror(errno));
        return false;
    }

    // Cabecera
    fprintf(fp,
        "codigo,tipo,nivel,puntos,"
        "norte,cap_norte,"
        "sur,cap_sur,"
        "este,cap_este,"
        "oeste,cap_oeste\n");

    for (int i = 0; i < ciudad->totalZonas; i++) {
        const Zona *z = &ciudad->zonas[i];
        fprintf(fp,
            "%s,%c,%d,%d,"
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
            z->capacidadOeste
        );
    }

    fclose(fp);
    return true;
}
