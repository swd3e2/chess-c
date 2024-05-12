//  texcube-glfw.c
//------------------------------------------------------------------------------
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE

#include "HandmadeMath.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33

#include "sokol_gfx.h"
#include "sokol_log.h"
#include "glfw_glue.h"

#define STB_IMAGE_IMPLEMENTATION

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

typedef struct {
    int figure_type; // 0pawn 1 rook
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
} cursorPosition;

typedef struct {
    vec2 position;
    int is_hovered;
} uniform_params;

typedef struct {
    int is_hovered;
} ps_uniform_params;

typedef struct {
    vertex vertices[64][4];
} boardResult;

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

            if (isEvenRow) {
                if (isEvenColumn) {
                    result.vertices[i * 8 + j][0].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][1].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][2].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][3].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                } else {
                    result.vertices[i * 8 + j][0].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][1].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][2].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][3].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                }
            } else {
                if (isEvenColumn) {
                    result.vertices[i * 8 + j][0].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][1].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][2].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][3].color =
                            (vec4) {.x = 0.0f, .y = 0.0f, .z = 0.0f, .r = 1.0f};
                } else {
                    result.vertices[i * 8 + j][0].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][1].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][2].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                    result.vertices[i * 8 + j][3].color =
                            (vec4) {.x = 1.0f, .y = 1.0f, .z = 1.0f, .r = 1.0f};
                }
            }
        }
    }
    return result;
}

sg_pipeline getBoardPipleine() {
    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
            .vs =
                    {
                            .source = "#version 330\n"
                                      "layout(location = 0) in vec4 position;\n"
                                      "layout(location = 1) in vec4 color0;\n"
                                      "out vec4 color;\n"
                                      "void main() {\n"
                                      "  gl_Position = position;\n"
                                      "  color = color0;\n"
                                      "}\n",
                    },
            .fs = {.source = "#version 330\n"
                             "out vec4 frag_color;\n"
                             "in vec4 color;\n"
                             "void main() {\n"
                             "  frag_color = color;\n"
                             "}\n"}});

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

char *get_file_content(char *filename) {
    char *buffer = 0;
    long length;
    FILE *f = fopen(filename, "rb");

    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
    }

    return buffer;
}

sg_pipeline getFiguresPipleine() {
    const char *vs_shader_content = get_file_content("game/shaders/default_vs.glsl");
    const char *ps_shader_content = get_file_content("game/shaders/default_ps.glsl");
    printf("%s\n", vs_shader_content);
    printf("%s\n", ps_shader_content);
    // create shader, note the combined-image-sampler description
    sg_shader shd = sg_make_shader(&(sg_shader_desc) {
            .vs = {
                    .uniform_blocks[0] = {
                            .size = sizeof(uniform_params),
                            .uniforms = {
                                    {.name = "posOffset", .type = SG_UNIFORMTYPE_FLOAT2},
                                    {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT}
                            },
                    },
                    .source = vs_shader_content,
            },
            .fs = {
                    .uniform_blocks[0] = {
                            .size = sizeof(ps_uniform_params),
                            .uniforms = {
                                {.name = "is_hovered", .type = SG_UNIFORMTYPE_INT}
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
            .layout = {.attrs = {[0] = {.format = SG_VERTEXFORMAT_FLOAT3},
                    [1] = {.format = SG_VERTEXFORMAT_FLOAT4},
                    [2] = {.format = SG_VERTEXFORMAT_FLOAT2}}},
            .shader = shd,
            .index_type = SG_INDEXTYPE_UINT16,
            .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
            .cull_mode = SG_CULLMODE_BACK});
}

cursorPosition cursorPos = {};

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

const char *filenames[] = {
        "w_pawn_1x_ns.png",
        "w_rook_1x_ns.png",
        "w_bishop_1x_ns.png",
        "w_king_1x_ns.png",
        "w_knight_1x_ns.png",
        "w_queen_1x_ns.png",
        "b_pawn_1x_ns.png",
        "b_rook_1x_ns.png",
        "b_bishop_1x_ns.png",
        "b_king_1x_ns.png",
        "b_knight_1x_ns.png",
        "b_queen_1x_ns.png",
};

sg_image *textures;
figure *figures;

figure *selected_figure;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        for (int i = 0; i < 32; i++) {
            if (figures[i].position.x == cursorPos.xCell && figures[i].position.y == cursorPos.yCell) {
                selected_figure = &figures[i];
            }
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

    figures = malloc(sizeof(figure) * 32);
    for (int i = 0; i < 8; i++) {
        figures[i] = (figure) {.figure_type = 0, .position = (position_coord) {.x = i, .y = 1}};
    }
    figures[8] = (figure) {.figure_type = 1, .position = (position_coord) {.x = 0, .y = 0}};
    figures[9] = (figure) {.figure_type = 1, .position = (position_coord) {.x = 7, .y = 0}};
    figures[10] = (figure) {.figure_type = 4, .position = (position_coord) {.x = 1, .y = 0}};
    figures[11] = (figure) {.figure_type = 4, .position = (position_coord) {.x = 6, .y = 0}};
    figures[12] = (figure) {.figure_type = 2, .position = (position_coord) {.x = 2, .y = 0}};
    figures[13] = (figure) {.figure_type = 2, .position = (position_coord) {.x = 5, .y = 0}};
    figures[14] = (figure) {.figure_type = 3, .position = (position_coord) {.x = 3, .y = 0}};
    figures[15] = (figure) {.figure_type = 5, .position = (position_coord) {.x = 4, .y = 0}};

    for (int i = 0; i < 8; i++) {
        figures[i + 16] = (figure) {.figure_type = 0, .position = (position_coord) {.x = i, .y = 6}, .is_black = true};
    }
    figures[24] = (figure) {.figure_type = 1, .position = (position_coord) {.x = 0, .y = 7}, .is_black = true};
    figures[25] = (figure) {.figure_type = 1, .position = (position_coord) {.x = 7, .y = 7}, .is_black = true};
    figures[26] = (figure) {.figure_type = 4, .position = (position_coord) {.x = 1, .y = 7}, .is_black = true};
    figures[27] = (figure) {.figure_type = 4, .position = (position_coord) {.x = 6, .y = 7}, .is_black = true};
    figures[28] = (figure) {.figure_type = 2, .position = (position_coord) {.x = 2, .y = 7}, .is_black = true};
    figures[29] = (figure) {.figure_type = 2, .position = (position_coord) {.x = 5, .y = 7}, .is_black = true};
    figures[30] = (figure) {.figure_type = 3, .position = (position_coord) {.x = 3, .y = 7}, .is_black = true};
    figures[31] = (figure) {.figure_type = 5, .position = (position_coord) {.x = 4, .y = 7}, .is_black = true};
    boardResult result = fillBoard();

    chess_square squares[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            squares[i][j].vertex_buffer = sg_make_buffer(&(sg_buffer_desc) {.type = SG_BUFFERTYPE_VERTEXBUFFER, .data = SG_RANGE(result.vertices[i * 8 + j])});
        }
    }

    float startX = 0.0f;
    float startY = 0.0f;
    float endX = startX + 0.25f;
    float endY = startY + 0.25f;

    // cube vertex buffer
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
    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc) {
            .type = SG_BUFFERTYPE_INDEXBUFFER, .data = SG_RANGE(indices)});

    sg_pipeline boardPipleine = getBoardPipleine();
    sg_pipeline figuresPipeline = getFiguresPipleine();

    // default pass action
    sg_pass_action pass_action = {0};


    figure *figures_to_draw[32];
    while (!glfwWindowShouldClose(glfw_window())) {
        memset(figures_to_draw, 0, sizeof(figures_to_draw));
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                squares[i][j].is_hovered = false;
                if (squares[i][j].figure != NULL) {
                    //todo: add figures to draw after figures will be added in cells
                }
            }
        }
        squares[cursorPos.xCell][cursorPos.yCell].is_hovered = true;

        sg_begin_pass(&(sg_pass) {.action = pass_action, .swapchain = glfw_swapchain()});

        sg_apply_pipeline(boardPipleine);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                sg_apply_bindings(&(sg_bindings) {
                        .vertex_buffers = squares[i][j].vertex_buffer,
                        .index_buffer = ibuf,
                });
                sg_draw(0, 6, 1);
            }
        }

        sg_apply_pipeline(figuresPipeline);

        uniform_params params;
        ps_uniform_params ps_params;
        for (int i = 0; i < 32; i++) {
            if (&figures[i] == selected_figure) {
                printf("some figure is selected pos x %d y %d\n", selected_figure->position.x,
                       selected_figure->position.y);
            }
            params.position.x = -(1.0f - 2.0f * figures[i].position.x * 0.125f);
            params.position.y = (1.0f - 2.0f * figures[i].position.y * 0.125f) - 0.25f;

            ps_params.is_hovered = 1;
            sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, &(sg_range) {
                    .ptr = &ps_params,
                    .size = sizeof(ps_params)
            });
            sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &(sg_range) {
                    .ptr = &params,
                    .size = sizeof(params)
            });

            int offset = 0;
            if (figures[i].is_black) {
                offset = offset + 6;
            }
            sg_image figure_image = textures[figures[i].figure_type + offset];

            sg_apply_bindings(&(sg_bindings) {
                    .vertex_buffers = vbuf,
                    .index_buffer = ibuf,
                    .fs = {.images[0] = figure_image, .samplers[0] = smp}
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
