//
// Created by wwwsh on 21.05.2024.
//

#ifndef CHESS_ARENA_H
#define CHESS_ARENA_H
#include "stdlib.h"

typedef struct {
    void* data;
    void* current_p;
} arena;

arena arena_create(size_t size) {
    void* data = malloc(size);
    memset(data, 0, size);
    return (arena){
        .data = data,
        .current_p = data
    };
}

void* arena_get_memory(arena *a, size_t s) {
    void* p = a->current_p;
    a->current_p = ((char *) a->current_p) + s;
    return p;
}

void arena_restart(arena *a) {
    a->current_p = a->data;
}

void arena_free(arena *a) {
    free(a->data);
}

#endif //CHESS_ARENA_H
