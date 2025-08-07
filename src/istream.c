#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "logger.h"
#include "istream.h"

struct istream {
    char* start;
    char* pos;
    unsigned int full_length;
    unsigned int length;
};

struct istream* init_stream(char* data) {
    struct istream* s = malloc(sizeof(struct istream));
    if (!s) {
        errorf("Failed to allocate memory for stream\n");
        return NULL;
    }

    s->start = malloc(strlen(data) + 1);
    if (!s->start) {
        errorf("Failed to allocate memory for stream data\n");
        free(s);
        return NULL;
    }
    strcpy(s->start, data);
    s->start[strlen(data)] = '\0';

    s->pos = s->start;
    s->full_length = strlen(data);
    s->length = s->full_length;

    return s;
}

int delete_istream(struct istream* s) {
    free(s->start);
    free(s);
    return 1;
}

int istrm_invalid(struct istream* s) {
    if(!s) return -1;

    if(!s->start) return -2;
    if(s->start > s->pos) return -3;
    

    if(!s->pos) return -4;
    if(s->pos < s->start) return -5;
    if(s->pos > s->start + s->full_length) return -6;

    if(s->full_length < s->length) return -7;
    if(s->full_length != strlen(s->start)) return -8;

    if(s->length != strlen(s->pos)) return -9;
    if(s->length != s->full_length - (s->pos - s->start)) return -10;

    return 0;
}

unsigned int istrm_pos(struct istream* s) {
    return (unsigned int)(s->pos - s->start);
}

unsigned int istrm_seek(struct istream* s, unsigned int offset) {
    if (offset > s->full_length) {
        errorf("Seek offset %u exceeds stream length %u\n", offset, s->full_length);
        return 0;
    }
    
    s->pos = s->start + offset;
    s->length = s->full_length - offset;

    return offset;
}

unsigned int istrm_length(struct istream* s) {
    return s->length;
}

char istrm_peek(struct istream* s) {
    if(s->length <= 0) return '\0';
    return *(s->pos);
}

unsigned int istrm_peek_n(struct istream* s, unsigned int n, char* buffer) {
    if(n > s->length) n = s->length;
    strncpy(buffer, s->pos, n);
    buffer[n] = '\0';
    return n;   
}

void istrm_skip_whitespace(struct istream* s) {
    while(isspace(istrm_peek(s)) && s->length > 0) {
        s->pos++;
        s->length--;
    }
}

int istrm_match(struct istream* s, char* match) {
    unsigned int str_len = strlen(match);

    if(s->length >= str_len && !strncmp(s->pos, match, str_len)) {
        s->pos += str_len;
        s->length -= str_len;
        return 1;
    }
    return 0;
}

char istrm_next(struct istream* s) {
    if(s->length <= 0) return '\0';
    char c = *(s->pos);
    s->pos++;
    s->length--;
    return c;
}

int istrm_expect(struct istream* s, char* expected, int log_level) { 
    if(istrm_match(s, expected)) return 1;
    else
        logmsgf(log_level, "Expected '%s' but found '%.*s'\n", expected, strlen(expected), s->pos);
    return 0;
}

int istrm_skip_thru(struct istream* s, char* expected) {
    char* found = strstr(s->pos, expected);
    if (found) {
        unsigned int offset = found - s->pos;
        s->pos += offset + strlen(expected);
        s->length -= offset + strlen(expected);
        return 1;
    }
    s->pos += s->length;
    return 0;
}

char* istrm_skip_thru_any(struct istream* s, char** expected, unsigned int count) {
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

    s->pos += s->length;
    return NULL;
}

char* istrm_get_word(struct istream* s) {
    if(!isalnum(istrm_peek(s))) return NULL;
    char* start = s->pos;

    char c = istrm_next(s);
    while(c && isalnum(c)) {
        c = istrm_next(s);
    }
    if(c){
        s->pos--;
        s->length++;
    }

    unsigned int word_length = s->pos - start;
    if (word_length == 0) return NULL;

    char* word = malloc(word_length + 1);
    if (!word) {
        errorf("Failed to allocate memory for word\n");
        return NULL;
    }
    strncpy(word, start, word_length);
    word[word_length] = '\0';
    return word;
}

int istrm_copy(struct istream* s, char* dest, unsigned int n) {
    if (n > s->length) n = s->length;
    strncpy(dest, s->pos, n);
    s->pos += n;
    s->length -= n;
    return n;
}

char* istrm_remaining(struct istream* s) {
    char* remaining = malloc(s->length + 1);
    if (!remaining) {
        errorf("Failed to allocate memory for remaining stream data\n");
        return NULL;
    }
    strncpy(remaining, s->pos, s->length);
    remaining[s->length] = '\0';

    s->pos += s->length;
    s->length = 0;

    return remaining;
}