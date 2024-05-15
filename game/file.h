//
// Created by Александр Тарасиди on 13.05.2024.
//

#ifndef CHESS_FILE_H
#define CHESS_FILE_H

char *get_file_content(char *filename) {
    char *buffer = 0;
    long length;
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return buffer;
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length+1);
    if (buffer) {
        fread(buffer, 1, length, f);
    }
    fclose(f);

#ifdef WIN32
    buffer[length] = '\0';
#endif

    return buffer;
}

#endif //CHESS_FILE_H
