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
        //menu_move_callback(&g, button, action);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (state == GAME) {
        game_mouse_click_callback(&g, button, action);
    } else {
        //menu_mouse_callback(&g, button, action);
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

    menu_init(&m);
    game_init(&g);

    sg_pass_action pass_action = {0};
    while (!glfwWindowShouldClose(glfw_window())) {
        sg_begin_pass(&(sg_pass) {.action = pass_action, .swapchain = glfw_swapchain()});

        if (state == GAME) {
            game_frame_play(&g);
        } else {
            menu_frame_play(&m);
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

