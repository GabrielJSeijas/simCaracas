#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "graph.h"
#include "fileio.h"

// Agrega una zona
void agregarZonaMenu(GrafoCiudad *grafo) {
    char codigo[4];
    int nivel;
    char resp[8];

    printf("Código de zona (3 letras): ");
    if (!fgets(codigo, sizeof(codigo), stdin)) return;
    codigo[strcspn(codigo, "\r\n")] = '\0';

    printf("Nivel (potencia de 2): ");
    char nivelStr[16];
    if (!fgets(nivelStr, sizeof(nivelStr), stdin)) return;
    nivel = atoi(nivelStr);

    printf("¿Es fuente? (s/n): ");
    if (!fgets(resp, sizeof(resp), stdin)) return;

    if (agregarZona(grafo, codigo, nivel, (resp[0]=='s'||resp[0]=='S')))
        printf("Zona %s agregada (nivel %d, %s).\n",
                codigo, nivel,
                (resp[0]=='s'||resp[0]=='S') ? "fuente" : "sumidero");
    else
        printf("Error al agregar la zona %s.\n", codigo);
}

// Agrega una arteria vial
void agregarArteriaMenu(GrafoCiudad *grafo) {
    char origen[4], destino[4];
    int capacidad;

    printf("Zona origen: ");
    if (!fgets(origen, sizeof(origen), stdin)) return;
    origen[strcspn(origen, "\r\n")] = '\0';

    printf("Zona destino: ");
    if (!fgets(destino, sizeof(destino), stdin)) return;
    destino[strcspn(destino, "\r\n")] = '\0';

    printf("Capacidad inicial: ");
    char capStr[16];
    if (!fgets(capStr, sizeof(capStr), stdin)) return;
    capacidad = atoi(capStr);

    Zona *z1 = buscarZona(grafo, origen);
    Zona *z2 = buscarZona(grafo, destino);
    if (!z1 || !z2) {
        puts("Alguna zona no existe.");
    } else if (!conectarZonas(grafo, z1, z2, ESTE, capacidad)) {
        puts("No se pudo crear la arteria (quizá ya existe).");
    } else {
        printf("Arteria %s->%s creada (cap=%d).\n", origen, destino, capacidad);
    }
}

// Amplía una arteria vial
void ampliarArteriaMenu(GrafoCiudad *grafo) {
    char origen[4], destino[4];
    int nuevaCap;

    printf("Zona origen: ");
    if (!fgets(origen, sizeof(origen), stdin)) return;
    origen[strcspn(origen, "\r\n")] = '\0';

    printf("Zona destino: ");
    if (!fgets(destino, sizeof(destino), stdin)) return;
    destino[strcspn(destino, "\r\n")] = '\0';

    printf("Nueva capacidad: ");
    char capStr[16];
    if (!fgets(capStr, sizeof(capStr), stdin)) return;
    nuevaCap = atoi(capStr);

    Zona *z1 = buscarZona(grafo, origen);
    Zona *z2 = buscarZona(grafo, destino);
    if (!z1 || !z2) {
        puts("Alguna zona no existe.");
    } else if (!ampliarArteria(grafo, z1, z2, nuevaCap)) {
        puts("No se pudo ampliar la arteria.");
    } else {
        printf("Arteria %s->%s ampliada a %d.\n", origen, destino, nuevaCap);
    }
}

// Guardar el grafo actual
void guardarGrafoMenu(GrafoCiudad *grafo) {
    char nombreArchivo[128];

    printf("Nombre de archivo CSV: ");
    if (!fgets(nombreArchivo, sizeof(nombreArchivo), stdin)) return;
    nombreArchivo[strcspn(nombreArchivo, "\r\n")] = '\0';

    if (guardarCiudadEnCSV(grafo, nombreArchivo))
        printf("Grafo guardado en '%s'.\n", nombreArchivo);
    else
        printf("Error al guardar en '%s'.\n", nombreArchivo);
}
