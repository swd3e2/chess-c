//
// Created by wwwsh on 15.05.2024.
//

#ifndef CHESS_MENU_H
#define CHESS_MENU_H

#include "gfx.h"
#include "math.h"
#define SOKOL_DEBUGTEXT_IMPL
#include "sokol_debugtext.h"

#define NUM_FONTS  (3)
#define FONT_KC854 (0)
#define FONT_C64   (1)
#define FONT_ORIC  (2)


sg_pipeline get_menu_pipeline();

typedef struct {
    int is_hovered;
} menu_uniform_params;

typedef struct {
    vec2 position;
    vec4 color;
} button_vertex;

typedef struct {
    sg_buffer button;
    sg_buffer ibuf;
    sg_pipeline menu_pipeline;
} menu_stuff;

void menu_init(menu_stuff *m) {
    sdtx_setup(&(sdtx_desc_t){
            .fonts = {
                    [FONT_KC854] = sdtx_font_kc854(),
                    [FONT_C64]   = sdtx_font_c64(),
                    [FONT_ORIC]  = sdtx_font_oric()
            },
            .logger.func = slog_func,
    });

    m->menu_pipeline = get_menu_pipeline();

    float startX = -0.5f;
    float startY = -0.125f;
    float endX = 0.5f;
    float endY = 0.125f;

    button_vertex vertices[] = {
            {.position = (vec2){ .x = startX, .y = startY}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
            {.position = (vec2){ .x = endX, .y = startY}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
            {.position = (vec2){ .x = endX, .y = endY}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
            {.position = (vec2){ .x = startX, .y = endY}, .color = (vec4){.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f}},
    };

    m->button = sg_make_buffer(&(sg_buffer_desc){
        .data = (sg_range) {
            .ptr = vertices,
            .size = sizeof(button_vertex) * 4
        }
    });
    uint16_t indices[] = {2, 1, 0, 3, 2, 0};
    m->ibuf = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_INDEXBUFFER, .data = SG_RANGE(indices) });
}

void menu_frame_play(menu_stuff *m) {

    sg_apply_pipeline(m->menu_pipeline);
    menu_uniform_params params;
    params.is_hovered = false;
    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &params,.size = sizeof(params)});
    sg_apply_bindings(&(sg_bindings) {
            .vertex_buffers = m->button,
            .index_buffer = m->ibuf,
    });
    sg_draw(0, 6, 1);

    sdtx_canvas(600 * 0.5f, 600 * 0.5f);
    sdtx_origin(16.0f, 18.0f);
    sdtx_font(FONT_KC854);
    sdtx_color3b(1.0f, 0.0f, 0.0f);
    sdtx_printf("Start");
    sdtx_draw();
}

sg_pipeline get_menu_pipeline() {
    char *vs_shader_content = get_file_content("game/shaders/menu_vs.glsl");
    char *ps_shader_content = get_file_content("game/shaders/menu_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
        .vs = {
            .source = vs_shader_content,
            .uniform_blocks[0] = {
                .size = sizeof(menu_uniform_params),
                .uniforms = {
                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                },
            },
        },
        .fs = {
            .source = ps_shader_content,
            .uniform_blocks[0] = {
                .size = sizeof(menu_uniform_params),
                .uniforms = {
                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                },
            },
        }
    });

    // create pipeline object
    return sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                {.format = SG_VERTEXFORMAT_FLOAT2},
                {.format = SG_VERTEXFORMAT_FLOAT4},
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .cull_mode = SG_CULLMODE_BACK
    });
}

#endif //CHESS_MENU_H
