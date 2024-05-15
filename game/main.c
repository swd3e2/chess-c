//  texcube-glfw.c
//------------------------------------------------------------------------------
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE

#include "HandmadeMath.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define STB_IMAGE_IMPLEMENTATION

#include "sokol_gfx.h"
#include "sokol_log.h"
#include "glfw_glue.h"
#include "file.h"

#include "stb_image.h"

typedef struct {
    float x, y;
} vec2;

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float x, y, z, r;
} vec4;

typedef struct {
    vec3 position;
    vec4 color;
} vertex;

typedef struct {
    int x;
    int y;
} position_coord;

const char *filenames[] = {
        "assets/w_pawn_1x_ns.png",
        "assets/w_rook_1x_ns.png",
        "assets/w_bishop_1x_ns.png",
        "assets/w_king_1x_ns.png",
        "assets/w_knight_1x_ns.png",
        "assets/w_queen_1x_ns.png",
        "assets/b_pawn_1x_ns.png",
        "assets/b_rook_1x_ns.png",
        "assets/b_bishop_1x_ns.png",
        "assets/b_king_1x_ns.png",
        "assets/b_knight_1x_ns.png",
        "assets/b_queen_1x_ns.png",
};

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
    figure *figure;
} chess_square;

typedef struct {
    int is_hovered;
    int possible_move;
} squares_uniform_params;

typedef struct {
    vec2 pos_offset;
    int is_hovered;
    int is_selected;
    float scale;
} figures_uniform_params;

typedef struct {
    vertex vertices[64][4];
} boardResult;


boardResult fill_board();
sg_pipeline get_board_pipleine();
sg_pipeline get_figures_pipleine();

void clear_squares_possible_move();

// Guess for what is this variable?
position_coord cursor_pos = {};
position_coord selected_pos = {};
figure *selected_figure;
// Figures textures
sg_image *textures;
// Chess board squares shit
chess_square squares[8][8] = {0};
// Who's going to move figures next
int black_player_turn = 0;

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
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
    cursor_pos.x = xCell;
    cursor_pos.y = 7 - yCell;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        figure *f = squares[cursor_pos.y][cursor_pos.x].figure;
        if (f != NULL) {
            if (f->is_black && !black_player_turn || !f->is_black && black_player_turn) {
                return;
            }

            clear_squares_possible_move();

            if (f->is_selected == true) {
                f->is_selected = false;
                selected_pos.x = 0;
                selected_pos.y = 0;
                selected_figure = NULL;
            } else {
                f->is_selected = true;
                selected_pos.x = cursor_pos.x;
                selected_pos.y = cursor_pos.y;
                selected_figure = f;
            }

            if (!f->is_black) {
                squares[cursor_pos.y + 1][cursor_pos.x].possible_move = true;
                squares[cursor_pos.y + 2][cursor_pos.x].possible_move = true;
            } else {
                squares[cursor_pos.y - 1][cursor_pos.x].possible_move = true;
                squares[cursor_pos.y - 2][cursor_pos.x].possible_move = true;
            }

            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (i == cursor_pos.y && j == cursor_pos.x || squares[i][j].figure == NULL) {
                        continue;
                    }

                    squares[i][j].figure->is_selected = false;
                }
            }
        } else {
            if (selected_figure == NULL) {
                return;
            }
            squares[selected_pos.y][selected_pos.x].figure = NULL;
            squares[cursor_pos.y][cursor_pos.x].figure = selected_figure;
            selected_figure->is_selected = false;
            black_player_turn = !black_player_turn;
            selected_figure = NULL;
            clear_squares_possible_move();
        }
    }
}

void clear_squares_possible_move() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            squares[i][j].possible_move = false;
        }
    }
}

int main() {
    // create GLFW window and initialize GL
    glfw_init(&(glfw_desc_t) {
        .title = "texcube-glfw.c",
        .width = 600,
        .height = 600,
        .sample_count = 4
    });
    glfwSetCursorPosCallback(glfw_window(), cursor_position_callback);
    glfwSetMouseButtonCallback(glfw_window(), mouse_button_callback);

    // setup sokol_gfx
    sg_setup(&(sg_desc) {
            .environment = glfw_environment(),
            .logger.func = slog_func,
    });
    assert(sg_isvalid());

    textures = malloc(sizeof(sg_image) * 12);
    for (int i = 0; i < 12; i++) {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(filenames[i], &width, &height, &nrChannels, 0);
        if (data == NULL) {
            printf("could not find file %s\n", filenames[i]);
            continue;
        }
        sg_image img = sg_make_image(&(sg_image_desc) {
                .width = width,
                .height = height,
                .pixel_format = SG_PIXELFORMAT_RGBA8,
                .data.subimage[0][0] = (sg_range) {.ptr = data, .size = width * height * 4}}
        );
        textures[i].id = img.id;
    }

    boardResult result = fill_board();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            squares[i][j].vertex_buffer = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_VERTEXBUFFER, .data = SG_RANGE(result.vertices[i * 8 + j]) });
        }
    }

    for (int i =0; i < 8; i++) {
        squares[0][i].figure = malloc(sizeof(figure));
    }

    squares[0][0].figure->figure_type = 1;
    squares[0][7].figure->figure_type = 1;
    squares[0][1].figure->figure_type = 4;
    squares[0][6].figure->figure_type = 4;
    squares[0][2].figure->figure_type = 2;
    squares[0][5].figure->figure_type = 2;
    squares[0][3].figure->figure_type = 3;
    squares[0][4].figure->figure_type = 5;

    for (int i = 0; i < 8; i++) {
        figure *f = malloc(sizeof(figure));
        f->figure_type = 0;
        squares[1][i].figure = f;
    }

    for (int i =0; i < 8; i++) {
        squares[7][i].figure = malloc(sizeof(figure));
        squares[7][i].figure->is_black = 1;
    }

    squares[7][0].figure->figure_type = 1;
    squares[7][7].figure->figure_type = 1;
    squares[7][1].figure->figure_type = 4;
    squares[7][6].figure->figure_type = 4;
    squares[7][2].figure->figure_type = 2;
    squares[7][5].figure->figure_type = 2;
    squares[7][3].figure->figure_type = 3;
    squares[7][4].figure->figure_type = 5;

    for (int i = 0; i < 8; i++) {
        figure *f = malloc(sizeof(figure));
        f->figure_type = 0;
        f->is_black = 1;
        squares[6][i].figure = f;
    }

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

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .data = SG_RANGE(vertices)
    });

    // create a sampler object
    sg_sampler smp = sg_make_sampler(&(sg_sampler_desc) {
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
    });

    // create an index buffer for the cube
    uint16_t indices[] = {2, 1, 0, 3, 2, 0};
    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_INDEXBUFFER, .data = SG_RANGE(indices) });

    sg_pipeline board_pipeline = get_board_pipleine();
    sg_pipeline figures_pipeline = get_figures_pipleine();

    // default pass action
    sg_pass_action pass_action = {0};

    figure* figures_to_draw[64];
    while (!glfwWindowShouldClose(glfw_window())) {
        memset(figures_to_draw, 0, sizeof(figures_to_draw));

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                squares[i][j].is_hovered = false;
                if (squares[i][j].figure == NULL) {
                    continue;
                }

                figure *f = squares[i][j].figure;

                f->position.x = j;
                f->position.y = 7-i;

                figures_to_draw[i*8+j] = f;
            }
        }

        if (cursor_pos.y > -1 && cursor_pos.x > -1) {
            squares[cursor_pos.y][cursor_pos.x].is_hovered = true;
        }

        sg_begin_pass(&(sg_pass) {.action = pass_action, .swapchain = glfw_swapchain()});
        sg_apply_pipeline(board_pipeline);

        squares_uniform_params s_params = {0};
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                s_params.is_hovered = squares[i][j].is_hovered;
                s_params.possible_move = squares[i][j].possible_move;
                sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &s_params,.size = sizeof(s_params)});
                sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range) {.ptr = &s_params,.size = sizeof(s_params)});
                sg_apply_bindings(&(sg_bindings) { .vertex_buffers = squares[i][j].vertex_buffer, .index_buffer = ibuf });
                sg_draw(0, 6, 1);
            }
        }
        sg_apply_pipeline(figures_pipeline);

        figures_uniform_params params;
        for (int i = 0; i < 64; i++) {
            figure *fig = *(figures_to_draw + i);
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
            sg_image figure_image = textures[fig->figure_type + offset];

            sg_apply_bindings(&(sg_bindings) {
                .vertex_buffers = vbuf,
                .index_buffer = ibuf,
                .fs = {.images[0] = figure_image, .samplers[0] = smp }
            });
            sg_draw(0, 6, 1);
        }

        sg_end_pass();

        sg_commit();
        glfwSwapBuffers(glfw_window());
        glfwPollEvents();
    }
    sg_shutdown();
    glfwTerminate();
}

sg_pipeline get_board_pipleine() {
    const char *vs_shader_content = get_file_content("game/shaders/square_vs.glsl");
    const char *ps_shader_content = get_file_content("game/shaders/square_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
            .vs = {
                    .source = vs_shader_content,
                    .uniform_blocks[0] = {
                            .size = sizeof(squares_uniform_params),
                            .uniforms = {
                                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT},
                                    {.name = "possible_move", .type = SG_UNIFORMTYPE_INT},
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
            .cull_mode = SG_CULLMODE_BACK});
}

sg_pipeline get_figures_pipleine() {
    const char *vs_shader_content = get_file_content("game/shaders/figures_vs.glsl");
    const char *ps_shader_content = get_file_content("game/shaders/figures_ps.glsl");

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

    free(vs_shader_content);
    free(ps_shader_content);

    // create pipeline object
    return sg_make_pipeline(&(sg_pipeline_desc) {
            .layout = {.attrs = {[0] = {.format = SG_VERTEXFORMAT_FLOAT3},
                    [1] = {.format = SG_VERTEXFORMAT_FLOAT4},
                    [2] = {.format = SG_VERTEXFORMAT_FLOAT2}}},
            .shader = shd,
            .index_type = SG_INDEXTYPE_UINT16,
            .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
            .cull_mode = SG_CULLMODE_BACK});
}

boardResult fill_board() {
    boardResult result;
    float width = 1.0f / 8;
    float height = 1.0f / 8;

    for (int i = 0; i < 8; i++) {
        bool isEvenRow = i % 2 > 0;
        for (int j = 0; j < 8; j++) {
            bool isEvenColumn = j % 2 > 0;

            float startX = j * width * 2.0f - 1.0f;
            float startY = i * height * 2.0f - 1.0f;
            float endX = startX + (width * 2);
            float endY = startY + (height * 2);

            result.vertices[i * 8 + j][0].position = (vec3) {.x = startX, .y = startY, .z = -1.0f};
            result.vertices[i * 8 + j][1].position = (vec3) {.x = endX, .y = startY, .z = -1.0f};
            result.vertices[i * 8 + j][2].position = (vec3) {.x = endX, .y = endY, .z = -1.0f};
            result.vertices[i * 8 + j][3].position = (vec3) {.x = startX, .y = endY, .z = -1.0f};

            vec4 color = (vec4){.x = 0.15f,.y = 0.15f,.z = 0.15f,.r = 1.0f, };
            if ((isEvenRow && isEvenColumn) || !isEvenRow && !isEvenColumn) {
                color.x = color.y = color.z = color.r = 0.72f;
            }

            result.vertices[i*8+j][0].color = color;
            result.vertices[i*8+j][1].color = color;
            result.vertices[i*8+j][2].color = color;
            result.vertices[i*8+j][3].color = color;
        }
    }
    return result;
}