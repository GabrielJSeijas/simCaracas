#include "pipes.h"
#include <unistd.h>   // Para write() y read()

// Envía una actualización de zona a través de la tubería
void enviarActualizacionZona(int fdTuberia, const Zona *zona) {
    MensajeZona mensaje;
    mensaje.tipo = ACTUALIZACION_ZONA;
    mensaje.zona = *zona;
    write(fdTuberia, &mensaje, sizeof(mensaje));
}

// Envía una actualización de tránsito a través de la tubería
void enviarActualizacionTransito(int fdTuberia,
                                 const ActualizacionTransito *actualizacion) {
    write(fdTuberia, actualizacion, sizeof(*actualizacion));
}

// Recibe un mensaje de la tubería y verifica que se haya leído completamente
bool recibirMensaje(int fdTuberia, void *destinoMensaje, size_t tam) {
    return read(fdTuberia, destinoMensaje, tam) == (ssize_t)tam;
}

// Envía el mensaje de inicio de día a través de la tubería
void enviarInicioDia(int fdTuberia) {
    MensajeDia mensaje = { .tipo = INICIO_DIA };
    write(fdTuberia, &mensaje, sizeof(mensaje));
}

// Envía el mensaje de fin de día a través de la tubería
void enviarFinDia(int fdTuberia) {
    MensajeDia mensaje = { .tipo = FIN_DIA };
    write(fdTuberia, &mensaje, sizeof(mensaje));
}
