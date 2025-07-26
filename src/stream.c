#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "logger.h"
#include "stream.h"

struct stream {
    char* start;
    char* pos;
    int full_length;
    int length;
};

struct stream* init_stream(char* data) {
    struct stream* s = malloc(sizeof(struct stream));
    if (!s) {
        errorf("Failed to allocate memory for stream\n");
        return NULL;
    }
    
    s->start = data;
    s->pos = data;
    s->full_length = strlen(data);
    s->length = s->full_length;

    return s;
}

int strm_length(struct stream* s) {
    return s->length;
}

char strm_peek(struct stream* s) {
    if(s->length <= 0) return '\0';
    return *(s->pos);
}

int strm_peek_n(struct stream* s, int n, char* buffer) {
    if(n > s->length) n = s->length;
    strncpy(buffer, s->pos, n);
    buffer[n] = '\0';
    return n;   
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
        logmsgf(log_level, "Expected '%s' but found '%.*s'\n", expected, strlen(expected), s->pos);
    return 0;
}

int strm_skip_thru(struct stream* s, char* expected) {
    char* found = strstr(s->pos, expected);
    if (found) {
        int offset = found - s->pos;
        s->pos += offset + strlen(expected);
        s->length -= offset + strlen(expected);
        return 1;
    }
    return 0;
}

char* strm_skip_thru_any(struct stream* s, char** expected, int count) {
    char* first_match_pos = NULL;
    int match_index = -1;

    for(int i = 0; i < count; i++) {
        char* found = strstr(s->pos, expected[i]);
        if(found) {
            if (!first_match_pos || found < first_match_pos) {
                first_match_pos = found;
                match_index = i;
            }
        }
    }

    if (first_match_pos) {
        s->length -= (first_match_pos - s->pos) + strlen(expected[match_index]);
        s->pos = first_match_pos + strlen(expected[match_index]);
        return expected[match_index];
    }

    return NULL;
}