//
// Created by wwwsh on 16.05.2024.
//

#ifndef CHESS_BUTTON_H
#define CHESS_BUTTON_H

#include "gfx.h"
#include "math.h"

typedef struct {
    vec2 position;
    vec4 color;
} button_vertex;

typedef struct {
    vec2 bottom_left;
    vec2 top_right;
} corners;

typedef struct {
    char* text;
    bool is_hovered;
    sg_buffer vbuf;
    sg_buffer ibuf;
    corners corners;
} button;


typedef struct {
    corners corners;
    char* text;
} button_desc;

button create_button(button_desc* desc) {
    button_vertex vertices[] = {
        {.position = (vec2){ .x = desc->corners.bottom_left.x, .y = desc->corners.bottom_left.y}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
        {.position = (vec2){ .x = desc->corners.top_right.x, .y = desc->corners.bottom_left.y}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
        {.position = (vec2){ .x = desc->corners.top_right.x, .y = desc->corners.top_right.y}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
        {.position = (vec2){ .x = desc->corners.bottom_left.x, .y = desc->corners.top_right.y}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
    };

    button res;
    res.corners = desc->corners;
    res.vbuf = sg_make_buffer(&(sg_buffer_desc){
        .data = (sg_range) {
            .ptr = vertices,
            .size = sizeof(button_vertex) * 4
        }
    });

    uint16_t indices[] = {2, 1, 0, 3, 2, 0};
    res.ibuf = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_INDEXBUFFER, .data = SG_RANGE(indices) });

    return res;
}

bool b_inside_corners(button *b, float x, float y) {
    return x > b->corners.bottom_left.x && x < b->corners.top_right.x && y > b->corners.bottom_left.y && y < b->corners.top_right.y;
}

void b_render(button *b) {
    int is_hovered;
    is_hovered = b->is_hovered;
    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &is_hovered,.size = sizeof(int)});
    sg_apply_bindings(&(sg_bindings) {
            .vertex_buffers = b->vbuf,
            .index_buffer = b->ibuf,
    });
    sg_draw(0, 6, 1);

    sdtx_canvas(600 * 0.5f, 600 * 0.5f);
    sdtx_origin(16.0f, 18.0f);
    sdtx_font(1);
    sdtx_color3b(1.0f, 0.0f, 0.0f);
    sdtx_printf("Start");
    sdtx_draw();
}

#endif //CHESS_BUTTON_H
