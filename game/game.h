//
// Created by wwwsh on 15.05.2024.
//

#ifndef CHESS_GAME_H
#define CHESS_GAME_H

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#include "math.h"
#include "gfx.h"
#include "glfw_glue.h"
#include "textures.h"
#include "file.h"
#include <time.h>
#include "arena.h"

#define PAWN 0
#define ROOK 1
#define BISHOP 2
#define KING 3
#define KNIGHT 4
#define QUEEN 5

//
//#define COLOR_SQUARE_WHITE (vec4){.x = 0.3f,.y = 0.3f,.z = 0.3f,.r = 1.0f, }
//#define COLOR_SQUARE_BLACK (vec4){.x = 0.6f,.y = 0.6f,.z = 0.6f,.r = 1.0f, }

#define COLOR_SQUARE_WHITE (vec4){.x = 124.0f / 255.0f,.y = 76.0f / 255.0f,.z = 62.0f / 255.0f,.r = 1.0f, }
#define COLOR_SQUARE_BLACK (vec4){.x = 81.0f / 255.0f,.y = 42.0f / 255.0f,.z = 42.0f / 255.0f,.r = 1.0f, }

typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;
    vec2 position;
    sg_image image;
} select_square;

typedef struct {
    vec3 position;
    vec4 color;
} vertex2;

typedef struct {
    vec2 position;
    vec4 color;
} line_vertex;

typedef struct {
    int x;
    int y;
} position_coord;

bool pc_is_valid(position_coord pc) {
    return pc.x >= 0 && pc.x < 8 && pc.y >= 0 && pc.y < 8;
}

typedef struct {
    // type is index in textures array
    int figure_type;
    bool is_selected;
    position_coord position;
    bool is_black;
} figure;

typedef struct {
    sg_buffer vertex_buffer;
    bool possible_move;
    bool attack_move;
    figure *figure;
} chess_square;

typedef struct {
    int possible_move;
    int attack_move;
} squares_uniform_params;

typedef struct {
    vec2 pos_offset;
    int is_selected;
} figures_uniform_params;

vertex2** prepare_board_vertices();
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

    select_square s_square;
    bool game_over;

    arena arena;
} game_stuff;

void restart(game_stuff* g);

void fill_squares_with_figures(game_stuff* g);

void create_squares_vertex_buffers(game_stuff* g, vertex2 **vertices);

void free_board_vertices(vertex2 **board_vertices);

void sq_check_possible_move(game_stuff* g, figure* f);

void sq_clear_possible_move(game_stuff* g) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            g->squares[i][j].possible_move = false;
        }
    }
}

void game_mouse_click_callback(game_stuff* g, int button, int action) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
        return;
    }

    if (g->game_over) {
        restart(g);
        return;
    }

    figure *f = g->squares[g->cursor_pos.y][g->cursor_pos.x].figure;
    if (f == NULL) {
        if (g->selected_figure == NULL) {
            return;
        }

        if (!g->squares[g->cursor_pos.y][g->cursor_pos.x].possible_move) {
            return;
        }

        g->squares[g->selected_pos.y][g->selected_pos.x].figure = NULL;
        g->squares[g->cursor_pos.y][g->cursor_pos.x].figure = g->selected_figure;
        g->selected_figure->is_selected = false;
        g->black_player_turn = !g->black_player_turn;
        g->selected_figure = NULL;
        sq_clear_possible_move(g);
        return;
    }

    // Deselecting same figure
    if (f == g->selected_figure) {
        f->is_selected = false;
        g->selected_pos.x = 0;
        g->selected_pos.y = 0;
        g->selected_figure = NULL;
        sq_clear_possible_move(g);
        return;
    }

    if (g->selected_figure != NULL && f->is_black != g->selected_figure->is_black) {
        if (!g->squares[g->cursor_pos.y][g->cursor_pos.x].possible_move) {
            return;
        }

        figure* figure_to_kill = g->squares[g->cursor_pos.y][g->cursor_pos.x].figure;
        if (figure_to_kill->figure_type == KING) {
            g->game_over = 1;
        }
        printf("%d\n", figure_to_kill->figure_type);
        g->squares[g->selected_pos.y][g->selected_pos.x].figure = NULL;
        g->squares[g->cursor_pos.y][g->cursor_pos.x].figure = g->selected_figure;
        g->selected_figure->is_selected = false;
        g->black_player_turn = !g->black_player_turn;
        g->selected_figure = NULL;
        sq_clear_possible_move(g);
        return;
    }

    // were working with figure
    if (f->is_black && !g->black_player_turn || !f->is_black && g->black_player_turn) {
        return;
    }

    sq_clear_possible_move(g);

    f->is_selected = true;
    g->selected_pos.x = g->cursor_pos.x;
    g->selected_pos.y = g->cursor_pos.y;
    g->selected_figure = f;

    sq_check_possible_move(g, f);

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == g->cursor_pos.y && j == g->cursor_pos.x || g->squares[i][j].figure == NULL) {
                continue;
            }

            g->squares[i][j].figure->is_selected = false;
        }
    }
}

void sq_check_possible_move(game_stuff* g, figure* f) {
    position_coord cursor_pos = g->cursor_pos;
    int cursor_pos_x = cursor_pos.x;
    int cursor_pos_y = cursor_pos.y;

    switch (f->figure_type) {
        case PAWN: {
            int y_movement = 1;
            if (f->is_black) {
                y_movement = -1;
            }

            // Attack
            position_coord attack_coords[2] = {
                    {.x = cursor_pos_x - 1, .y = cursor_pos_y + y_movement},
                    {.x = cursor_pos_x + 1, .y = cursor_pos_y + y_movement},
            };

            for (int i = 0; i < 2; i++) {
                position_coord next_attack_coords = attack_coords[i];
                if (!pc_is_valid(next_attack_coords)) continue;
                chess_square* sq = &g->squares[next_attack_coords.y][next_attack_coords.x];
                if (sq->figure == NULL || sq->figure->is_black == f->is_black) continue;
                sq->possible_move = true;
            }

            // Move
            for (int i = 0; i < 2; i++) {
                position_coord next_coords = (position_coord){.y = cursor_pos_y + y_movement * (i + 1), .x = cursor_pos_x};
                if (!pc_is_valid(next_coords)) break;
                chess_square* sq = &g->squares[next_coords.y][next_coords.x];
                if (sq->figure != NULL) break;
                sq->possible_move = true;
            }
            break;
        }
        case ROOK: {
            position_coord moves[] = {
                (position_coord ){.x = -1, .y = 0},
                (position_coord ){.x = 1,  .y = 0},
                (position_coord ){.x = 0,  .y = 1},
                (position_coord ){.x = 0,  .y = -1},
            };

            for (int i = 0; i < 4; i++) {
                position_coord current_position = cursor_pos;

                bool has_found_enemy = false;
                while(true) {
                    current_position.x += moves[i].x;
                    current_position.y += moves[i].y;

                    if (!pc_is_valid(current_position)) break;
                    chess_square *sq = &g->squares[current_position.y][current_position.x];
                    if (sq->figure != NULL) {
                        if (sq->figure->is_black == f->is_black) break;

                        sq->possible_move = true;
                        break;
                    }
                    sq->possible_move = true;
                };
            }
            break;
        }
        case BISHOP: {
            position_coord moves[] = {
                    (position_coord ){.x = -1, .y =  1},
                    (position_coord ){.x =  1, .y = -1},
                    (position_coord ){.x =  1, .y =  1},
                    (position_coord ){.x = -1, .y = -1},
            };

            for (int i = 0; i < 4; i++) {
                position_coord current_position = cursor_pos;

                while(true) {
                    current_position.x += moves[i].x;
                    current_position.y += moves[i].y;

                    if (!pc_is_valid(current_position)) break;
                    chess_square *sq = &g->squares[current_position.y][current_position.x];
                    if (sq->figure != NULL) {
                        if (sq->figure->is_black == f->is_black) break;

                        sq->possible_move = true;
                        break;
                    }
                    sq->possible_move = true;
                };
            }
            break;
        }
        case KING: {
            position_coord moves[] = {
                    (position_coord ){.x =  1, .y =  0},
                    (position_coord ){.x = -1, .y =  0},
                    (position_coord ){.x =  0, .y = -1},
                    (position_coord ){.x =  0, .y =  1},
                    (position_coord ){.x =  1, .y =  1},
                    (position_coord ){.x =  1, .y = -1},
                    (position_coord ){.x = -1, .y =  1},
                    (position_coord ){.x = -1, .y = -1},
            };

            for (int i = 0; i < 8; i++) {
                position_coord current_position = cursor_pos;

                current_position.x += moves[i].x;
                current_position.y += moves[i].y;

                if (!pc_is_valid(current_position)) continue;
                chess_square *sq = &g->squares[current_position.y][current_position.x];
                if (sq->figure != NULL) {
                    if (sq->figure->is_black == f->is_black) continue;

                    sq->possible_move = true;
                    continue;
                }
                sq->possible_move = true;
            }
            break;
        }
        case KNIGHT: {
            position_coord moves[] = {
                    (position_coord ){.x =  1, .y = 2},
                    (position_coord ){.x = -1, .y = 2},

                    (position_coord ){.x =  1, .y = -2},
                    (position_coord ){.x = -1, .y = -2},

                    (position_coord ){.x =  2, .y =  1},
                    (position_coord ){.x =  2, .y = -1},

                    (position_coord ){.x =  -2, .y =  1},
                    (position_coord ){.x =  -2, .y = -1},
            };

            for (int i = 0; i < 8; i++) {
                position_coord current_position = cursor_pos;

                current_position.x += moves[i].x;
                current_position.y += moves[i].y;

                if (!pc_is_valid(current_position)) continue;
                chess_square *sq = &g->squares[current_position.y][current_position.x];
                if (sq->figure != NULL) {
                    if (sq->figure->is_black == f->is_black) continue;

                    sq->possible_move = true;
                    continue;
                }
                sq->possible_move = true;
            }
            break;
        }
        case QUEEN: {
            position_coord moves[] = {
                    (position_coord ){.x = -1, .y = 0},
                    (position_coord ){.x = 1,  .y = 0},
                    (position_coord ){.x = 0,  .y = 1},
                    (position_coord ){.x = 0,  .y = -1},
                    (position_coord ){.x = -1, .y =  1},
                    (position_coord ){.x =  1, .y = -1},
                    (position_coord ){.x =  1, .y =  1},
                    (position_coord ){.x = -1, .y = -1},
            };

            for (int i = 0; i < 8; i++) {
                position_coord current_position = cursor_pos;

                while(true) {
                    current_position.x += moves[i].x;
                    current_position.y += moves[i].y;

                    if (!pc_is_valid(current_position)) break;
                    chess_square *sq = &g->squares[current_position.y][current_position.x];
                    if (sq->figure != NULL) {
                        if (sq->figure->is_black == f->is_black) break;

                        sq->possible_move = true;
                        break;
                    }
                    sq->possible_move = true;
                };
            }
            break;
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

    g->cursor_pos.x = min(7, xCell);
    g->cursor_pos.y = min(7, 7 - yCell);
}

void game_init(game_stuff *g) {
    g->arena = arena_create(sizeof(figure) * 32);

    const char* figure_texture_filenames[] = {
        "assets/w_pawn.png",
        "assets/w_rook.png",
        "assets/w_bishop.png",
        "assets/w_king.png",
        "assets/w_knight.png",
        "assets/w_queen.png",
        "assets/b_pawn.png",
        "assets/b_rook.png",
        "assets/b_bishop.png",
        "assets/b_king.png",
        "assets/b_knight.png",
        "assets/b_queen.png",
    };
    g->textures = get_multiple_textures(figure_texture_filenames, sizeof(figure_texture_filenames) / sizeof(char*));
    vertex2** board_vertices = prepare_board_vertices();

    create_squares_vertex_buffers(g, board_vertices);
    free_board_vertices(board_vertices);

    fill_squares_with_figures(g);

    float startX = 0.0f;
    float startY = 0.0f;
    float endX = startX + 0.20f;
    float endY = startY + 0.20f;

    float vertices[] = {
            // pos                 color                        texcoord
            startX, startY, -1.0f,  0.0f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
            endX, startY, -1.0f,    0.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,
            endX, endY, -1.0f,      0.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
            startX, endY, -1.0f,    0.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
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

    float startX2 = 0.0f;
    float startY2 = 0.0f;
    float endX2 = startX2 + 0.25f;
    float endY2 = startY2 + 0.25f;
    float vertices2[] = {
            // pos                 color                  texcoord
            startX2, startY2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            endX2, startY2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            endX2, endY2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            startX2, endY2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    };

    g->s_square.vbuf = sg_make_buffer(&(sg_buffer_desc){
            .type = SG_BUFFERTYPE_VERTEXBUFFER,
            .data = SG_RANGE(vertices2)
    });
    g->s_square.ibuf = g->ibuf;
    g->s_square.image = get_texture("assets/square_possible_move.png");
}

void game_frame_play(game_stuff* g) {
    memset(g->figures_to_draw, 0, sizeof(g->figures_to_draw));

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (g->squares[i][j].figure == NULL) {
                continue;
            }

            figure *f = g->squares[i][j].figure;

            f->position.x = j;
            f->position.y = 7-i;

            g->figures_to_draw[i*8+j] = f;
        }
    }

    sg_apply_pipeline(g->board_pipeline);

    squares_uniform_params s_params = {0};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
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

        params.pos_offset.x = -(1.0f - 2.0f * fig->position.x * 0.125f) + 0.02f;
        params.pos_offset.y = (1.0f - 2.0f * fig->position.y * 0.125f) - 0.25f + 0.02f;

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

    if (g->game_over) {
        sdtx_canvas(600 * 0.5f, 600 * 0.5f);
        sdtx_origin(10.0f, 18.0f);
        sdtx_font(1);
        sdtx_color3b(205.0f, 205.0f, 205.0f);
        if (g->black_player_turn) {
            sdtx_printf("GAME OVER WHITE WON");
        } else {
            sdtx_printf("GAME OVER BLACK WON");
        }

        sdtx_canvas(600 * 0.5f, 600 * 0.5f);
        sdtx_font(1);
        sdtx_color3b(205.0f, 205.0f, 205.0f);
        sdtx_origin(9.0f, 19.0f);
        sdtx_printf("CLICK TO START NEW GAME");
        sdtx_draw();

        return;
    }

    // Cursor things
    chess_square * cursor_square = &g->squares[g->cursor_pos.y][g->cursor_pos.x];
    if ((cursor_square->figure != NULL && cursor_square->figure->is_black == g->black_player_turn) || cursor_square->possible_move) {
        params.pos_offset.x = -(1.0f - 2.0f * g->cursor_pos.x * 0.125f);
        params.pos_offset.y = -(1.0f - 2.0f * g->cursor_pos.y * 0.125f);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range) {.ptr = &params,.size = sizeof(params)});
        sg_apply_bindings(&(sg_bindings) {
                .vertex_buffers = g->s_square.vbuf,
                .index_buffer = g->s_square.ibuf,
                .fs = { .images[0] = g->s_square.image, .samplers[0] = g->smp }
        });
        sg_draw(0, 6, 1);
    }

    figure* selected_figure = g->selected_figure;
    if (selected_figure != NULL) {
        params.pos_offset.x = -(1.0f - 2.0f * g->selected_pos.x * 0.125f);
        params.pos_offset.y = -(1.0f - 2.0f * g->selected_pos.y * 0.125f);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range) {.ptr = &params,.size = sizeof(params)});
        sg_apply_bindings(&(sg_bindings) {
                .vertex_buffers = g->s_square.vbuf,
                .index_buffer = g->s_square.ibuf,
                .fs = { .images[0] = g->s_square.image, .samplers[0] = g->smp }
        });
        sg_draw(0, 6, 1);
    }

//    sg_apply_pipeline(g->lines_pipeline);
//    sg_apply_bindings(&(sg_bindings){
//            .vertex_buffers = g->lines_b
//    });
//
//    sg_draw(0, 2, 1);
}

void free_board_vertices(vertex2 **board_vertices) {
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

void create_squares_vertex_buffers(game_stuff* g, vertex2 **vertices) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            g->squares[i][j].vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {
                    .type = SG_BUFFERTYPE_VERTEXBUFFER,
                    .data = (sg_range){
                            .ptr = vertices[i * 8 + j],
                            .size = 4 * sizeof(vertex2)
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
                                    {.name = "is_selected", .type = SG_UNIFORMTYPE_INT},
                            },
                    },
                    .source = vs_shader_content,
            },
            .fs = {
                    .uniform_blocks[0] = {
                            .size = sizeof(figures_uniform_params),
                            .uniforms = {
                                    {.name = "pos_offset", .type = SG_UNIFORMTYPE_FLOAT2},
                                    {.name = "is_selected", .type = SG_UNIFORMTYPE_INT},
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

vertex2** prepare_board_vertices() {
    vertex2 **vertices = malloc(64 * sizeof(vertex2*));
    float width = 1.0f / 8;
    float height = 1.0f / 8;

    for (int i = 0; i < 8; i++) {
        bool isEvenRow = i % 2 > 0;
        for (int j = 0; j < 8; j++) {
            vertices[i * 8 + j] = malloc(4 * sizeof(vertex2));
            bool isEvenColumn = j % 2 > 0;

            float startX = j * width * 2.0f - 1.0f;
            float startY = i * height * 2.0f - 1.0f;
            float endX = startX + (width * 2);
            float endY = startY + (height * 2);

            vertices[i * 8 + j][0].position = (vec3) {.x = startX, .y = startY, .z = -1.0f};
            vertices[i * 8 + j][1].position = (vec3) {.x = endX, .y = startY, .z = -1.0f};
            vertices[i * 8 + j][2].position = (vec3) {.x = endX, .y = endY, .z = -1.0f};
            vertices[i * 8 + j][3].position = (vec3) {.x = startX, .y = endY, .z = -1.0f};

            vec4 color = COLOR_SQUARE_WHITE;
            if ((isEvenRow && isEvenColumn) || !isEvenRow && !isEvenColumn) {
                color = COLOR_SQUARE_BLACK;
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
        g->squares[0][i].figure = arena_get_memory(&g->arena, sizeof(figure));//calloc(1, sizeof(figure));
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
        figure *f = arena_get_memory(&g->arena, sizeof(figure));
        g->squares[1][i].figure = f;
    }

    for (int i =0; i < 8; i++) {
        g->squares[7][i].figure = arena_get_memory(&g->arena,  sizeof(figure));
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
        figure *f = arena_get_memory(&g->arena, sizeof(figure));
        f->is_black = 1;
        g->squares[6][i].figure = f;
    }


//    g->squares[2][0].figure = g->squares[7][3].figure;
//    g->squares[7][3].figure = NULL;
//    g->squares[5][0].figure = g->squares[0][3].figure;
//    g->squares[0][3].figure = NULL;
}

void restart(game_stuff* g) {
    g->game_over = false;
    g->black_player_turn = false;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            g->squares[i][j].figure = NULL;
        }
    }
    arena_restart(&g->arena);
    fill_squares_with_figures(g);
}

#endif //CHESS_GAME_H
