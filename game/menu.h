//
// Created by wwwsh on 15.05.2024.
//

#ifndef CHESS_MENU_H
#define CHESS_MENU_H

#include "gfx.h"
#include "math.h"

#include "button.h"



sg_pipeline get_menu_pipeline();

typedef struct {
    int is_hovered;
} menu_uniform_params;

typedef struct {
    button start_button;
    button exit_button;
    sg_pipeline menu_pipeline;
} menu_stuff;


void menu_mouse_move_callback(menu_stuff* m, double xpos, double ypos) {

}

void menu_mouse_click_callback(menu_stuff* g, int button, int action) {

}

void menu_init(menu_stuff *m) {
    m->menu_pipeline = get_menu_pipeline();

    m->start_button = create_button(&(button_desc){
        .corners = (corners) {
            .bottom_left = (vec2){.x = -0.5f, .y = -0.125f},
            .top_right = (vec2){.x = 0.5f, .y = 0.125f}
        },
        .text = "Start"
    });
}

void menu_frame_play(menu_stuff *m) {
    sg_apply_pipeline(m->menu_pipeline);
    b_render(&m->start_button);
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

    free(vs_shader_content);
    free(ps_shader_content);

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
