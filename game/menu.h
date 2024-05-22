//
// Created by wwwsh on 15.05.2024.
//

#ifndef CHESS_MENU_H
#define CHESS_MENU_H

#include "gfx.h"
#include "math.h"
#include "button.h"
#include "textures.h"
#include "time.h"
#include <windows.h>

sg_pipeline get_menu_pipeline();
sg_pipeline get_background_menu_pipeline();

unsigned short getMilliseconds() {
    SYSTEMTIME st;
    GetSystemTime(&st);

    return st.wMilliseconds;
}

typedef struct {
    int is_hovered;
} menu_uniform_params;

typedef struct {
    vec2 uv_offset;
    float time;
} background_uniform_params;

typedef struct {
    button start_button;
    button exit_button;
    sg_pipeline menu_pipeline;
    sg_pipeline background_pipeline;
    sg_buffer vb;
    sg_buffer ib;
    sg_image background_texture;
    sg_sampler smp;
} menu_stuff;

void menu_mouse_move_callback(menu_stuff* m, double xpos, double ypos) {

}

void menu_mouse_click_callback(menu_stuff* g, int button, int action) {

}

void menu_init(menu_stuff *m) {
    m->menu_pipeline = get_menu_pipeline();
    m->background_pipeline = get_background_menu_pipeline();

    m->smp = sg_make_sampler(&(sg_sampler_desc) {
            .min_filter = SG_FILTER_NEAREST,
            .mag_filter = SG_FILTER_NEAREST,
    });

    vertex* v = vertex_create_quad(-1.0f, -1.0f, 1.0f, 1.0f);
    m->vb = sg_make_buffer(&(sg_buffer_desc){
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .data = (sg_range){
                .size = sizeof(vertex) * 4,
                .ptr = v
            }
    });
    vertex_free_memory(v);

    uint16_t indices[] = {2, 1, 0, 3, 2, 0};
    m->ib = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices)
    });
    m->background_texture = get_texture("assets/background.png");
    m->start_button = button_create(&(button_desc) {
            .corners = (corners) {
                    .bottom_left = {.x = -0.3f, .y = -0.125f - 0.25f},
                    .top_right = {.x = 0.3f, .y = 0.125f - 0.25f}
            },
            .texture = get_texture("assets/start.png"),
            .smp = m->smp
    });

    m->exit_button = button_create(&(button_desc) {
            .corners = (corners) {
                    .bottom_left = {.x = -0.3f, .y = -0.125f - 0.6},
                    .top_right = {.x = 0.3f, .y = 0.125f - 0.6}
            },
            .texture = get_texture("assets/exit.png"),
            .smp = m->smp
    });
}

void menu_frame_play(menu_stuff *m) {
    unsigned short millis = getMilliseconds();
    sg_apply_pipeline(m->background_pipeline);
    background_uniform_params p = {.time = 1.0f, .uv_offset = {.x = ((float)millis) / 8000.0f, .y = ((float)millis) / 8000.0f}};
    sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range){.size = sizeof(background_uniform_params), .ptr = &p});
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = m->vb,
        .index_buffer = m->ib,
        .fs = {
            .images[0] = m->background_texture,
            .samplers[0] = m->smp
        }
    });
    sg_draw(0, 6, 1);

    sg_apply_pipeline(m->menu_pipeline);
    button_render(&m->start_button);
    button_render(&m->exit_button);
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
            .images[0].used = true,
            .samplers[0].used = true,
            .image_sampler_pairs[0] = {.used = true, .glsl_name = "tex", .image_slot = 0, .sampler_slot = 0},
        }
    });

    free(vs_shader_content);
    free(ps_shader_content);

    return sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                {.format = SG_VERTEXFORMAT_FLOAT2},
                {.format = SG_VERTEXFORMAT_FLOAT4},
                {.format = SG_VERTEXFORMAT_FLOAT2},
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .cull_mode = SG_CULLMODE_BACK
    });
}

sg_pipeline get_background_menu_pipeline() {
    char *vs_shader_content = get_file_content("game/shaders/menu_background_vs.glsl");
    char *ps_shader_content = get_file_content("game/shaders/menu_background_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
        .vs = {
            .source = vs_shader_content,
            .uniform_blocks[0] = {
                .size = sizeof(background_uniform_params),
                .uniforms = {
                    {.name = "uv_offset", .type = SG_UNIFORMTYPE_FLOAT2},
                    {.name = "time", .type = SG_UNIFORMTYPE_FLOAT},
                },
            },
        },
        .fs = {
            .source = ps_shader_content,
            .uniform_blocks[0] = {
                .size = sizeof(background_uniform_params),
                .uniforms = {
                    {.name = "uv_offset", .type = SG_UNIFORMTYPE_FLOAT2},
                    {.name = "time", .type = SG_UNIFORMTYPE_FLOAT},
                },
            },
            .images[0].used = true,
            .samplers[0].used = true,
            .image_sampler_pairs[0] = {.used = true, .glsl_name = "tex", .image_slot = 0, .sampler_slot = 0},
        }
    });

    free(vs_shader_content);
    free(ps_shader_content);

    return sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                {.format = SG_VERTEXFORMAT_FLOAT2},
                {.format = SG_VERTEXFORMAT_FLOAT4},
                {.format = SG_VERTEXFORMAT_FLOAT2},
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .cull_mode = SG_CULLMODE_BACK
    });
}

#endif //CHESS_MENU_H
