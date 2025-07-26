#ifndef STREAM_H
#define STREAM_H

struct stream {
    char* start;
    char* pos;
    int full_length;
    int length;
};

void init_stream(struct stream* s, char* data);
int strm_length(struct stream* s);
char strm_peek(struct stream* s);
void skip_whitespace(struct stream* s);
int strm_match(struct stream* s, char* match);
int strm_expect(struct stream* s, char* expected, int log_level);

#endif // STREAM_H