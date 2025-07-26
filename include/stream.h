#ifndef STREAM_H
#define STREAM_H

struct stream;

struct stream* init_stream(char* data);
int strm_length(struct stream* s);
char strm_peek(struct stream* s);
int strm_peek_n(struct stream* s, int n, char* buffer);
void skip_whitespace(struct stream* s);
int strm_match(struct stream* s, char* match);
int strm_expect(struct stream* s, char* expected, int log_level);
int strm_skip_thru(struct stream* s, char* expected);
char* strm_skip_thru_any(struct stream* s, char** expected, int count);

#endif // STREAM_H