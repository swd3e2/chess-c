//
// Created by wwwsh on 16.05.2024.
//

#ifndef CHESS_BUTTON_H
#define CHESS_BUTTON_H

#include "gfx.h"
#include "math.h"
#include "vertex.h"

typedef struct {
    vec2 bottom_left;
    vec2 top_right;
} corners;

typedef struct {
    bool is_hovered;
    sg_buffer vbuf;
    sg_buffer ibuf;
    corners corners;
    sg_image texture;
    sg_sampler smp;
} button;


typedef struct {
    corners corners;
    sg_image texture;
    sg_sampler smp;
} button_desc;

button button_create(button_desc* desc) {
    vertex vertices[] = {
        {
            .position = (vec2){ .x = desc->corners.bottom_left.x, .y = desc->corners.bottom_left.y},
            .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f},
            .uv = (vec2){.x = 1.0f, .y = 1.0f}
        },
        {
            .position = (vec2){ .x = desc->corners.top_right.x, .y = desc->corners.bottom_left.y},
            .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f},
            .uv = (vec2){.x = 0.0f, .y = 1.0f}
        },
        {
            .position = (vec2){ .x = desc->corners.top_right.x, .y = desc->corners.top_right.y},
            .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f},
            .uv = (vec2){.x = 0.0f, .y = 0.0f}
        },
        {
            .position = (vec2){ .x = desc->corners.bottom_left.x, .y = desc->corners.top_right.y},
            .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f},
            .uv = (vec2){.x = 1.0f, .y = 0.0f}
        },
    };

    uint16_t indices[] = {2, 1, 0, 3, 2, 0};

    return (button){
        .corners = desc->corners,
        .texture = desc->texture,
        .smp = desc->smp,
        .vbuf = sg_make_buffer(&(sg_buffer_desc){.data = SG_RANGE(vertices)}),
        .ibuf = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_INDEXBUFFER, .data = SG_RANGE(indices) })
    };
}

bool b_inside_corners(button *b, float x, float y) {
    return x > b->corners.bottom_left.x && x < b->corners.top_right.x && y > b->corners.bottom_left.y && y < b->corners.top_right.y;
}

void button_render(button *b) {
    int is_hovered;
    is_hovered = b->is_hovered;
    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &is_hovered,.size = sizeof(int)});
    sg_apply_bindings(&(sg_bindings) {
            .vertex_buffers = b->vbuf,
            .index_buffer = b->ibuf,
            .fs = {.images[0] = b->texture, .samplers[0] = b->smp }
    });
    sg_draw(0, 6, 1);
}

#endif //CHESS_BUTTON_H
