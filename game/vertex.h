//
// Created by wwwsh on 22.05.2024.
//

#ifndef CHESS_VERTEX_H
#define CHESS_VERTEX_H

#include "math.h"

typedef struct {
    vec2 position;
    vec4 color;
    vec2 uv;
} vertex;

vertex* vertex_create_quad(float start_x, float start_y, float end_x, float end_y) {
    vertex* vertices = malloc(sizeof(vertex)*4);
    vertices[0] = (vertex){.position = (vec2){.x = start_x, .y = start_y},   .color = (vec4){.x = 0.0f,.y = 0.0f,.z = 0.0f,.r = 1.0f}, .uv = (vec2){.x = 1.0f,.y = 1.0f}};
    vertices[1] = (vertex){.position = (vec2){.x = end_x, .y = start_y},     .color = (vec4){.x = 0.0f,.y = 0.0f,.z = 0.0f,.r = 1.0f}, .uv = (vec2){.x = 0.0f,.y = 1.0f}};
    vertices[2] = (vertex){.position = (vec2){.x = end_x, .y = end_y},       .color = (vec4){.x = 0.0f,.y = 0.0f,.z = 0.0f,.r = 1.0f}, .uv = (vec2){.x = 0.0f,.y = 0.0f}};
    vertices[3] = (vertex){.position = (vec2){.x = start_x, .y = end_y},     .color = (vec4){.x = 0.0f,.y = 0.0f,.z = 0.0f,.r = 1.0f}, .uv = (vec2){.x = 1.0f,.y = 0.0f}};
    return vertices;
}

void vertex_free_memory(vertex* m) {
    free(m);
}

#endif //CHESS_VERTEX_H
