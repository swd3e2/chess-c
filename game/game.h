//
// Created by wwwsh on 15.05.2024.
//

#ifndef CHESS_GAME_H
#define CHESS_GAME_H

#include "math.h"
#include "gfx.h"
#include "glfw_glue.h"
#include "textures.h"
#include "file.h"

typedef struct {
    vec3 position;
    vec4 color;
} vertex;

typedef struct {
    vec2 position;
    vec4 color;
} line_vertex;

typedef struct {
    int x;
    int y;
} position_coord;

typedef struct {
    // type is index in textures array
    int figure_type;
    bool is_selected;
    position_coord position;
    bool is_black;
} figure;

typedef struct {
    sg_buffer vertex_buffer;
    bool is_hovered;
    bool possible_move;
    bool attack_move;
    figure *figure;
} chess_square;

typedef struct {
    int is_hovered;
    int possible_move;
    int attack_move;
} squares_uniform_params;

typedef struct {
    vec2 pos_offset;
    int is_hovered;
    int is_selected;
    float scale;
} figures_uniform_params;

vertex** prepare_board_vertices();
sg_pipeline get_board_pipeline();
sg_pipeline get_figures_pipeline();
sg_pipeline get_lines_pipeline();

typedef struct {
    sg_image* textures;
    // Current position of cursor
    position_coord cursor_pos;
    // Position of last selected figure
    position_coord selected_pos;
    // Current selected figure, NULL if no figure is selected
    figure *selected_figure;
    // Chess board squares shit
    chess_square squares[8][8];
    // Who's going to move figures next
    int black_player_turn;

    figure* figures_to_draw[64];
    sg_buffer lines_b;
    sg_pipeline board_pipeline;
    sg_pipeline figures_pipeline;
    sg_pipeline lines_pipeline;
    sg_buffer ibuf;
    sg_buffer vbuf;
    sg_sampler smp;
} game_stuff;

void fill_squares_with_figures(game_stuff* g);
void create_squares_vertex_buffers(game_stuff* g, vertex **vertices);
void free_board_vertices(vertex **board_vertices);

void clear_squares_possible_move(game_stuff* g) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            g->squares[i][j].possible_move = false;
        }
    }
}

void game_mouse_click_callback(game_stuff* g, int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        figure *f = g->squares[g->cursor_pos.y][g->cursor_pos.x].figure;
        if (f != NULL) {
            if (f->is_black && !g->black_player_turn || !f->is_black && g->black_player_turn) {
                return;
            }

            clear_squares_possible_move(g);

            if (f->is_selected == true) {
                f->is_selected = false;
                g->selected_pos.x = 0;
                g->selected_pos.y = 0;
                g->selected_figure = NULL;
            } else {
                f->is_selected = true;
                g->selected_pos.x = g->cursor_pos.x;
                g->selected_pos.y = g->cursor_pos.y;
                g->selected_figure = f;
            }

            if (!f->is_black) {
                g->squares[g->cursor_pos.y + 1][g->cursor_pos.x].possible_move = true;
                g->squares[g->cursor_pos.y + 2][g->cursor_pos.x].possible_move = true;
            } else {
                g->squares[g->cursor_pos.y - 1][g->cursor_pos.x].possible_move = true;
                g->squares[g->cursor_pos.y - 2][g->cursor_pos.x].possible_move = true;
            }

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (i == g->cursor_pos.y && j == g->cursor_pos.x || g->squares[i][j].figure == NULL) {
                        continue;
                    }

                    g->squares[i][j].figure->is_selected = false;
                }
            }
        } else {
            if (g->selected_figure == NULL) {
                return;
            }

            g->squares[g->selected_pos.y][g->selected_pos.x].figure = NULL;
            g->squares[g->cursor_pos.y][g->cursor_pos.x].figure = g->selected_figure;
            g->selected_figure->is_selected = false;
            g->black_player_turn = !g->black_player_turn;
            g->selected_figure = NULL;
            clear_squares_possible_move(g);
        }
    }
}

void game_mouse_move_callback(game_stuff* g, double xpos, double ypos) {
    if (xpos < 0 || ypos < 0) return;

    double x = xpos;
    int xCell = 0;
    while (x > 75) {
        x -= 75;
        xCell++;
    }

    double y = ypos;
    int yCell = 0;
    while (y > 75) {
        y -= 75;
        yCell++;
    }

    g->cursor_pos.x = xCell;
    g->cursor_pos.y = 7 - yCell;
}

void game_init(game_stuff *g) {
    g->textures = get_figures_textures();
    vertex** board_vertices = prepare_board_vertices();

    create_squares_vertex_buffers(g, board_vertices);
    free_board_vertices(board_vertices);

    fill_squares_with_figures(g);

    float startX = 0.0f;
    float startY = 0.0f;
    float endX = startX + 0.25f;
    float endY = startY + 0.25f;

    float vertices[] = {
            // pos                 color                  texcoord
            startX, startY, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            endX, startY, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            endX, endY, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            startX, endY, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    };

    g->vbuf = sg_make_buffer(&(sg_buffer_desc) {
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .data = SG_RANGE(vertices)
    });

    // create a sampler object
    g->smp = sg_make_sampler(&(sg_sampler_desc) {
            .min_filter = SG_FILTER_NEAREST,
            .mag_filter = SG_FILTER_NEAREST,
    });

    uint16_t indices[] = {2, 1, 0, 3, 2, 0};
    g->ibuf = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_INDEXBUFFER, .data = SG_RANGE(indices) });

    g->board_pipeline = get_board_pipeline();
    g->figures_pipeline = get_figures_pipeline();
    g->lines_pipeline = get_lines_pipeline();

    line_vertex  line_vertices[] = {
            {.color = (vec4){.x = 1.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f}, .position = (vec2){.x = -1.0f, .y = -1.0f} },
            {.color = (vec4){.x = 1.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f}, .position = (vec2){.x = 1.0f, .y = 1.0f} }
    };
    g->lines_b = sg_make_buffer(&(sg_buffer_desc){
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .data = SG_RANGE(line_vertices)
    });
}

void game_frame_play(game_stuff* g) {
    memset(g->figures_to_draw, 0, sizeof(g->figures_to_draw));

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            g->squares[i][j].is_hovered = false;
            if (g->squares[i][j].figure == NULL) {
                continue;
            }

            figure *f = g->squares[i][j].figure;

            f->position.x = j;
            f->position.y = 7-i;

            g->figures_to_draw[i*8+j] = f;
        }
    }

    if (g->cursor_pos.y > -1 && g->cursor_pos.x > -1) {
        g->squares[g->cursor_pos.y][g->cursor_pos.x].is_hovered = true;
    }

    sg_apply_pipeline(g->board_pipeline);

    squares_uniform_params s_params = {0};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            s_params.is_hovered = g->squares[i][j].is_hovered;
            s_params.possible_move = g->squares[i][j].possible_move;
            s_params.attack_move = g->squares[i][j].attack_move;

            sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &s_params,.size = sizeof(s_params)});
            sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range) {.ptr = &s_params,.size = sizeof(s_params)});
            sg_apply_bindings(&(sg_bindings) { .vertex_buffers = g->squares[i][j].vertex_buffer, .index_buffer = g->ibuf });
            sg_draw(0, 6, 1);
        }
    }
    sg_apply_pipeline(g->figures_pipeline);

    figures_uniform_params params;
    for (int i = 0; i < 64; i++) {
        figure *fig = *(g->figures_to_draw + i);
        if (fig == NULL) {
            continue;
        }

        params.is_selected = fig->is_selected;
        params.scale = 0;

        if (fig->is_selected == true) {
            params.scale = 1.02f;
        }

        params.pos_offset.x = -(1.0f - 2.0f * fig->position.x * 0.125f);
        params.pos_offset.y = (1.0f - 2.0f * fig->position.y * 0.125f) - 0.25f;

        params.is_hovered = 1;
        sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &params,.size = sizeof(params)});
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range) {.ptr = &params,.size = sizeof(params)});

        int offset = 0;
        if (fig->is_black) {
            offset = offset + 6;
        }
        sg_image figure_image = g->textures[fig->figure_type + offset];

        sg_apply_bindings(&(sg_bindings) {
                .vertex_buffers = g->vbuf,
                .index_buffer = g->ibuf,
                .fs = {.images[0] = figure_image, .samplers[0] = g->smp }
        });
        sg_draw(0, 6, 1);
    }

    sg_apply_pipeline(g->lines_pipeline);
    sg_apply_bindings(&(sg_bindings){
            .vertex_buffers = g->lines_b
    });

    sg_draw(0, 2, 1);
}

void free_board_vertices(vertex **board_vertices) {
    if (board_vertices == NULL) {
        return;
    }

    for (int i = 0; i < 64; i++) {
        if (board_vertices[i] == NULL) {
            continue;
        }
        free(board_vertices[i]);
    }
    free(board_vertices);
}

void create_squares_vertex_buffers(game_stuff* g, vertex **vertices) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            g->squares[i][j].vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
                    .type = SG_BUFFERTYPE_VERTEXBUFFER,
                    .data = (sg_range){
                            .ptr = vertices[i * 8 + j],
                            .size = 4 * sizeof(vertex)
                    }
            });
        }
    }
}

sg_pipeline get_board_pipeline() {
    char *vs_shader_content = get_file_content("game/shaders/square_vs.glsl");
    char *ps_shader_content = get_file_content("game/shaders/square_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
            .vs = {
                    .source = vs_shader_content,
                    .uniform_blocks[0] = {
                            .size = sizeof(squares_uniform_params),
                            .uniforms = {
                                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "possible_move", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "attack_move", .type = SG_UNIFORMTYPE_INT},
                            },
                    },
            },
            .fs = {
                    .source = ps_shader_content,
                    .uniform_blocks[0] = {
                            .size = sizeof(squares_uniform_params),
                            .uniforms = {
                                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "possible_move", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "attack_move", .type = SG_UNIFORMTYPE_INT},
                            },
                    },
            }
    });

    // create pipeline object
    return sg_make_pipeline(&(sg_pipeline_desc) {
            .layout = {
                    .attrs = {
                            {.format = SG_VERTEXFORMAT_FLOAT3},
                            {.format = SG_VERTEXFORMAT_FLOAT4},
                    }
            },
            .shader = shd,
            .index_type = SG_INDEXTYPE_UINT16,
            .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
            .cull_mode = SG_CULLMODE_BACK
    });
}

sg_pipeline get_lines_pipeline() {
    char *vs_shader_content = get_file_content("game/shaders/line_vs.glsl");
    char *ps_shader_content = get_file_content("game/shaders/line_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
            .vs = { .source = vs_shader_content },
            .fs = { .source = ps_shader_content }
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
            .index_type = SG_INDEXTYPE_NONE,
            .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
            .cull_mode = SG_CULLMODE_BACK,
            .primitive_type = SG_PRIMITIVETYPE_LINES
    });
}

sg_pipeline get_figures_pipeline() {
    char *vs_shader_content = get_file_content("game/shaders/figures_vs.glsl");
    char *ps_shader_content = get_file_content("game/shaders/figures_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
            .vs = {
                    .uniform_blocks[0] = {
                            .size = sizeof(figures_uniform_params),
                            .uniforms = {
                                    {.name = "pos_offset", .type = SG_UNIFORMTYPE_FLOAT2},
                                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "is_selected", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "scale", .type = SG_UNIFORMTYPE_FLOAT}
                            },
                    },
                    .source = vs_shader_content,
            },
            .fs = {
                    .uniform_blocks[0] = {
                            .size = sizeof(figures_uniform_params),
                            .uniforms = {
                                    {.name = "pos_offset", .type = SG_UNIFORMTYPE_FLOAT2},
                                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "is_selected", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "scale", .type = SG_UNIFORMTYPE_FLOAT}
                            }
                    },
                    .images[0].used = true,
                    .samplers[0].used = true,
                    .image_sampler_pairs[0] = {.used = true, .glsl_name = "tex", .image_slot = 0, .sampler_slot = 0},
                    .source = ps_shader_content
            }
    });


    // create pipeline object
    return sg_make_pipeline(&(sg_pipeline_desc) {
            .layout = {.attrs = {
                    [0] = {.format = SG_VERTEXFORMAT_FLOAT3},
                    [1] = {.format = SG_VERTEXFORMAT_FLOAT4},
                    [2] = {.format = SG_VERTEXFORMAT_FLOAT2}
            }},
            .shader = shd,
            .index_type = SG_INDEXTYPE_UINT16,
            .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
            .alpha_to_coverage_enabled = true,
            .cull_mode = SG_CULLMODE_BACK
    });
}

vertex** prepare_board_vertices() {
    vertex **vertices = malloc(64 * sizeof(vertex*));
    float width = 1.0f / 8;
    float height = 1.0f / 8;

    for (int i = 0; i < 8; i++) {
        bool isEvenRow = i % 2 > 0;
        for (int j = 0; j < 8; j++) {
            vertices[i * 8 + j] = malloc(4 * sizeof(vertex));
            bool isEvenColumn = j % 2 > 0;

            float startX = j * width * 2.0f - 1.0f;
            float startY = i * height * 2.0f - 1.0f;
            float endX = startX + (width * 2);
            float endY = startY + (height * 2);

            vertices[i * 8 + j][0].position = (vec3) {.x = startX, .y = startY, .z = -1.0f};
            vertices[i * 8 + j][1].position = (vec3) {.x = endX, .y = startY, .z = -1.0f};
            vertices[i * 8 + j][2].position = (vec3) {.x = endX, .y = endY, .z = -1.0f};
            vertices[i * 8 + j][3].position = (vec3) {.x = startX, .y = endY, .z = -1.0f};

            vec4 color = (vec4){.x = 0.15f,.y = 0.15f,.z = 0.15f,.r = 1.0f, };
            if ((isEvenRow && isEvenColumn) || !isEvenRow && !isEvenColumn) {
                color.x = color.y = color.z = color.r = 0.72f;
            }

            vertices[i*8+j][0].color = color;
            vertices[i*8+j][1].color = color;
            vertices[i*8+j][2].color = color;
            vertices[i*8+j][3].color = color;
        }
    }

    return vertices;
}

void fill_squares_with_figures(game_stuff* g) {
    for (int i =0; i < 8; i++) {
        g->squares[0][i].figure = calloc(1, sizeof(figure));
    }

    g->squares[0][0].figure->figure_type = 1;
    g->squares[0][7].figure->figure_type = 1;
    g->squares[0][1].figure->figure_type = 4;
    g->squares[0][6].figure->figure_type = 4;
    g->squares[0][2].figure->figure_type = 2;
    g->squares[0][5].figure->figure_type = 2;
    g->squares[0][3].figure->figure_type = 3;
    g->squares[0][4].figure->figure_type = 5;

    for (int i = 0; i < 8; i++) {
        figure *f = calloc(1, sizeof(figure));
        g->squares[1][i].figure = f;
    }

    for (int i =0; i < 8; i++) {
        g->squares[7][i].figure = calloc(1, sizeof(figure));
        g->squares[7][i].figure->is_black = 1;
    }

    g->squares[7][0].figure->figure_type = 1;
    g->squares[7][7].figure->figure_type = 1;
    g->squares[7][1].figure->figure_type = 4;
    g->squares[7][6].figure->figure_type = 4;
    g->squares[7][2].figure->figure_type = 2;
    g->squares[7][5].figure->figure_type = 2;
    g->squares[7][3].figure->figure_type = 3;
    g->squares[7][4].figure->figure_type = 5;

    for (int i = 0; i < 8; i++) {
        figure *f = calloc(1, sizeof(figure));
        f->is_black = 1;
        g->squares[6][i].figure = f;
    }
}

#endif //CHESS_GAME_H
