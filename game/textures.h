//
// Created by Александр Тарасиди on 15.05.2024.
//

#ifndef CHESS_TEXTURE_H
#define CHESS_TEXTURE_H

#include "stdlib.h"
#include "stb_image.h"

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

sg_image* get_figures_textures() {
    sg_image *textures = malloc(sizeof(sg_image) * 12);
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

    return textures;
}

#endif //CHESS_TEXTURE_H
