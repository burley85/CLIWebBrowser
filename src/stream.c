#include <stdio.h>
#include <string.h>

#include "logger.h"

struct stream {
    char* start;
    char* pos;
    int full_length;
    int length;
};

void init_stream(struct stream* s, char* data) {
    s->start = data;
    s->pos = data;
    s->full_length = strlen(data);
    s->length = s->full_length;
}

int strm_length(struct stream* s) {
    return s->length;
}

char strm_peek(struct stream* s) {
    if (s->length <= 0) return '\0';
    return *(s->pos);
}

void skip_whitespace(struct stream* s) {
    while(isspace(strm_peek(s)) && s->length > 0) {
        s->pos++;
        s->length--;
    }
}

int strm_match(struct stream* s, char* match) {
    skip_whitespace(s);

    int str_len = strlen(match);

    if(s->length >= str_len && !strncmp(s->pos, match, str_len)) {
        s->pos += str_len;
        s->length -= str_len;
        return 1;
    }
    return 0;
}

int strm_expect(struct stream* s, char* expected, int log_level) { 
    if(strm_match(s, expected)) return 1;
    else
        logf(log_level, "Expected '%s' but found '%.*s'\n", expected, strlen(expected), s->pos);
    return 0;
}