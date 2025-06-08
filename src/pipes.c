// src/pipes.c

#include "pipes.h"
#include <unistd.h>   // write(), read()

void enviarActualizacionZona(int fdTuberia, const Zona *zona) {
    MensajeZona mensaje;
    mensaje.tipo = ACTUALIZACION_ZONA;
    mensaje.zona = *zona;
    write(fdTuberia, &mensaje, sizeof(mensaje));
}

void enviarActualizacionTransito(int fdTuberia,
                                 const ActualizacionTransito *actualizacion) {
    write(fdTuberia, actualizacion, sizeof(*actualizacion));
}

bool recibirMensaje(int fdTuberia, void *destinoMensaje, size_t tam) {
    return read(fdTuberia, destinoMensaje, tam) == (ssize_t)tam;
}

void enviarInicioDia(int fdTuberia) {
    MensajeDia mensaje = { .tipo = INICIO_DIA };
    write(fdTuberia, &mensaje, sizeof(mensaje));
}

void enviarFinDia(int fdTuberia) {
    MensajeDia mensaje = { .tipo = FIN_DIA };
    write(fdTuberia, &mensaje, sizeof(mensaje));
}
