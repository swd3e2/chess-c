//  texcube-glfw.c
//------------------------------------------------------------------------------
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#define SOKOL_GLCORE
#include "HandmadeMath.h"

#define SOKOL_GLCORE33
#define STB_IMAGE_IMPLEMENTATION

#include "game.h"
#include "menu.h"
#include "gfx.h"
#include "textures.h"

#define NUM_FONTS  (3)
#define FONT_KC854 (0)
#define FONT_C64   (1)
#define FONT_ORIC  (2)

enum GAME_STATE {
    MENU,
    GAME
};

enum GAME_STATE state = MENU;

// Game data to render/play
game_stuff g = {0};
// Menu data to render
menu_stuff m = {0};

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    if (state == GAME) {
        game_mouse_move_callback(&g, xpos, ypos);
    } else {
        float window_coord_x = -1.0f + (2 * (xpos / 600.0f));
        float window_coord_y = 1.0f - (2 * (ypos / 600.0f));

        if (b_inside_corners(&m.start_button, window_coord_x, window_coord_y)) {
            m.start_button.is_hovered = true;
        } else {
            m.start_button.is_hovered = false;
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    switch (state) {
        case GAME:
            game_mouse_click_callback(&g, button, action);
            break;
        case MENU:
            if (!m.start_button.is_hovered) {
                return;
            }
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                state = GAME;
            }
            break;
    }
}

char* vec3_to_json(vec3 *v) {
    size_t needed = snprintf(NULL, 0, "{\"x\":%f,\"y\":%f,\"z\":%f}", v->x, v->y, v->z)+1;
    char * buffer = malloc(needed);
    sprintf(buffer, "{\"x\":%f,\"y\":%f,\"z\":%f}", v->x, v->y, v->z);
    return buffer;
}
char* vec4_to_json(vec4 *v) {
    size_t needed = snprintf(NULL, 0, "{\"x\":%f,\"y\":%f,\"z\":%f,\"r\":%f}", v->x, v->y, v->z, v->r)+1;
    char * buffer = malloc(needed);
    sprintf(buffer, "{\"x\":%f,\"y\":%f,\"z\":%f,\"r\":%f}", v->x, v->y, v->z, v->r);
    return buffer;
}

char* vertex_to_json(vertex* v) {
    char *pos = vec3_to_json(&v->position);
    char *color = vec4_to_json(&v->color);

    size_t needed = snprintf(NULL, 0, "{\"pos\":%s,\"col\":%s}", pos, color)+1;
    char * buffer = malloc(needed);
    sprintf(buffer, "{\"pos\":%s,\"col\":%s}", pos, color);

    free(pos);
    free(color);

    return buffer;
}

int main() {
    vertex v = (vertex){
        .position = (vec3){.x = 1.0f, .y = 2.0f, .z = 3.0f},
        .color = (vec4){.x = 1.0f, .y = 2.0f, .z = 3.0f, .r = 4.0f},
    };
    char* vertex_json = vertex_to_json(&v);
    printf("%s\n", vertex_json);
    free(vertex_json);
    // create GLFW window and initialize GL
    glfw_init(&(glfw_desc_t) {
        .title = "Chess-C",
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

    sdtx_setup(&(sdtx_desc_t){
            .fonts = {
                    [FONT_KC854] = sdtx_font_kc854(),
                    [FONT_C64]   = sdtx_font_c64(),
                    [FONT_ORIC]  = sdtx_font_oric()
            },
            .logger.func = slog_func,
    });

    menu_init(&m);
    game_init(&g);

    sg_pass_action pass_action = {0};
    while (!glfwWindowShouldClose(glfw_window())) {
        sg_begin_pass(&(sg_pass) {.action = pass_action, .swapchain = glfw_swapchain()});

        switch (state) {
            case GAME:
                game_frame_play(&g);
                break;
            case MENU:
                menu_frame_play(&m);
                break;
        }

        sg_end_pass();
        sg_commit();

        glfwSwapBuffers(glfw_window());
        glfwPollEvents();
    }
    free(g.textures);
    sg_shutdown();
    glfwTerminate();
}

