#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "ostream.h"

struct ostream {
    char* start;
    char* pos;
    unsigned int length;
    unsigned int allocated;
};

struct ostream* init_ostream() {
    int starting_size = 1;

    struct ostream* s = malloc(sizeof(struct ostream));
    if (!s) {
        errorf("Failed to allocate memory for stream\n");
        return NULL;
    }

    s->start = malloc(starting_size);
    if (!s->start) {
        errorf("Failed to allocate memory for stream data\n");
        free(s);
        return NULL;
    }
    memset(s->start, 0, starting_size);

    s->pos = s->start;
    s->length = 0;
    s->allocated = starting_size;
    return s;
}

int delete_ostream(struct ostream* s) {
    free(s->start);
    free(s);
    return 1;
}

int ostrm_write(struct ostream* s, char* data, unsigned int length) {
    if (s->length + length > s->allocated - 1) {
        s->allocated *= 2;
        char* new_buffer = realloc(s->start, s->allocated);
        if (!new_buffer) {
            errorf("Failed to reallocate memory for stream data\n");
            return 0;
        }
        s->start = new_buffer;
        memset(s->start + s->length, 0, s->allocated - s->length);
    }

    memcpy(s->pos, data, length);
    s->pos += length;
    s->length += length;
    return 1;
}

int ostrm_length(struct ostream* s) {
    return s->length;
}

struct ostream* ostrm_copy(struct ostream* old){
    int starting_size = old->length + 1;

    struct ostream* new = malloc(sizeof(struct ostream));
    if (!new) {
        errorf("Failed to allocate memory for stream\n");
        return NULL;
    }

    new->start = malloc(starting_size);
    if (!new->start) {
        errorf("Failed to allocate memory for stream data\n");
        free(new);
        return NULL;
    }

    memcpy(new->start, old->start, old->length + 1);
    new->pos = new->start + old->length;
    new->length = old->length;
    new->allocated = starting_size;
    return new;
}