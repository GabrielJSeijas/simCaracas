#include "pipes.h"
#include "graph.h"
#include <unistd.h>
#include <string.h>

void send_zone_update(int pipe_fd, const Zone *zone) {
    ZoneMessage msg;
    msg.type = ZONE_UPDATE;
    msg.zone = *zone;
    
    write(pipe_fd, &msg, sizeof(msg));
}

void send_traffic_update(int pipe_fd, TrafficUpdate *update) {
    write(pipe_fd, update, sizeof(*update));
}

bool receive_message(int pipe_fd, void *buffer, size_t size) {
    return read(pipe_fd, buffer, size) == (ssize_t)size;
}

void send_day_start(int pipe_fd) {
    DayMessage msg = { .type = DAY_START };
    write(pipe_fd, &msg, sizeof(msg));
}

void send_day_end(int pipe_fd) {
    DayMessage msg = { .type = DAY_END };
    write(pipe_fd, &msg, sizeof(msg));
}