//
// Created by Александр Тарасиди on 15.05.2024.
//

#ifndef CHESS_TEXTURE_H
#define CHESS_TEXTURE_H

#include "stdlib.h"
#include "stb_image.h"

sg_image* get_multiple_textures(const char** filenames, int size) {
    sg_image *textures = malloc(sizeof(sg_image) * 12);
    for (int i = 0; i < size; i++) {
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

        stbi_image_free(data);
    }

    return textures;
}

sg_image get_texture(const char* filename) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data == NULL) {
        printf("could not find file %s\n", filename);
        return (sg_image){};
    }
    sg_image img = sg_make_image(&(sg_image_desc) {
            .width = width,
            .height = height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .data.subimage[0][0] = (sg_range) {.ptr = data, .size = width * height * 4}}
    );
    stbi_image_free(data);

    return img;
}

#endif //CHESS_TEXTURE_H
