// messages.h
#ifndef MESSAGES_H
#define MESSAGES_H

typedef enum {
    INICIO_DIA,
    FIN_DIA,
    ACTUALIZAR_ZONA,
    ACTUALIZAR_TRANSITO
} TipoMensaje;

typedef struct {
    TipoMensaje tipo;
    char codigoZona[4];
    int valor;
} MensajeZona;

typedef struct {
    TipoMensaje tipo;
    char origen[4];
    char destino[4];
    int vehiculos;
} MensajeTransito;

#endif