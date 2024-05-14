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
    figure *figure;
} chess_square;

typedef struct {
    int xCell;
    int yCell;
} cursor_position;

typedef struct {
    int is_hovered;
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

chess_square squares[8][8] = {0};

boardResult fillBoard() {
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

            vec4 color = {0};
            if ((isEvenRow && isEvenColumn) || !isEvenRow && !isEvenColumn) {
                color.x = color.y = color.z = color.r = 1.0f;
            }

            result.vertices[i*8+j][0].color = color;
            result.vertices[i*8+j][1].color = color;
            result.vertices[i*8+j][2].color = color;
            result.vertices[i*8+j][3].color = color;
        }
    }
    return result;
}

sg_pipeline getBoardPipleine() {
    const char *vs_shader_content = get_file_content("game/shaders/square_vs.glsl");
    const char *ps_shader_content = get_file_content("game/shaders/square_ps.glsl");

    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
        .vs = {
            .source = vs_shader_content,
            .uniform_blocks[0] = {
                .size = sizeof(squares_uniform_params),
                .uniforms = { {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT} },
            },
        },
        .fs = {
            .source = ps_shader_content,
            .uniform_blocks[0] = {
                .size = sizeof(squares_uniform_params),
                .uniforms = { {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT} },
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



sg_pipeline getFiguresPipleine() {
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

cursor_position cursorPos = {};

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
    cursorPos.xCell = xCell;
    cursorPos.yCell = yCell;
}



sg_image *textures;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (squares[i][j].figure == NULL) continue;
                squares[i][j].figure->is_selected = 0;
            }
        }
        if (squares[cursorPos.xCell][cursorPos.yCell].figure != NULL) {
            squares[cursorPos.xCell][cursorPos.yCell].figure->is_selected = 1;
            //todo now i need to calculate possible positions
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

    boardResult result = fillBoard();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            squares[i][j].vertex_buffer = sg_make_buffer(&(sg_buffer_desc) { .type = SG_BUFFERTYPE_VERTEXBUFFER, .data = SG_RANGE(result.vertices[i * 8 + j]) });
        }
    }

    for (int i =0; i < 8; i++) {
        squares[i][0].figure = malloc(sizeof(figure));
    }

    squares[0][0].figure->figure_type = 1;
    squares[7][0].figure->figure_type = 1;
    squares[1][0].figure->figure_type = 4;
    squares[6][0].figure->figure_type = 4;
    squares[2][0].figure->figure_type = 2;
    squares[5][0].figure->figure_type = 2;
    squares[3][0].figure->figure_type = 3;
    squares[4][0].figure->figure_type = 5;

    for (int i = 0; i < 8; i++) {
        figure *f = malloc(sizeof(figure));
        f->figure_type = 0;
        squares[i][1].figure = f;
    }

    for (int i =0; i < 8; i++) {
        squares[i][7].figure = malloc(sizeof(figure));
        squares[i][7].figure->is_black = 1;
    }

    squares[0][7].figure->figure_type = 1;
    squares[7][7].figure->figure_type = 1;
    squares[1][7].figure->figure_type = 4;
    squares[6][7].figure->figure_type = 4;
    squares[2][7].figure->figure_type = 2;
    squares[5][7].figure->figure_type = 2;
    squares[3][7].figure->figure_type = 3;
    squares[4][7].figure->figure_type = 5;

    for (int i = 0; i < 8; i++) {
        figure *f = malloc(sizeof(figure));
        f->figure_type = 0;
        f->is_black = 1;
        squares[i][6].figure = f;
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

    sg_pipeline board_pipeline = getBoardPipleine();
    sg_pipeline figures_pipeline = getFiguresPipleine();

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

                f->position.x = i;
                f->position.y = j;

                figures_to_draw[i*8+j] = f;
            }
        }

        squares[7-cursorPos.yCell][cursorPos.xCell].is_hovered = true;

        sg_begin_pass(&(sg_pass) {.action = pass_action, .swapchain = glfw_swapchain()});
        sg_apply_pipeline(board_pipeline);

        squares_uniform_params s_params = {0};
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                s_params.is_hovered = squares[i][j].is_hovered;
                sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {.ptr = &s_params,.size = sizeof(s_params)});
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

            params.is_selected = 0;
            params.scale = 0;

            if (fig->is_selected == 1) {
                params.is_selected = 1;
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
