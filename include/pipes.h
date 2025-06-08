#ifndef PIPES_H
#define PIPES_H

#include "graph.h"

typedef enum {
    ZONE_UPDATE,
    TRAFFIC_UPDATE,
    DAY_START,
    DAY_END
} MessageType;

typedef struct {
    MessageType type;
    Zone zone;
} ZoneMessage;

typedef struct {
    MessageType type;
} DayMessage;

typedef struct {
    char from[4];
    char to[4];
    int vehicles;
} TrafficUpdate;

// Funciones de comunicación
void send_zone_update(int pipe_fd, const Zone *zone);
void send_traffic_update(int pipe_fd, TrafficUpdate *update);
bool receive_message(int pipe_fd, void *buffer, size_t size);
void send_day_start(int pipe_fd);
void send_day_end(int pipe_fd);

#endif