#ifndef STREAM_H
#define STREAM_H

struct stream;

struct stream* init_stream(char* data);
unsigned int strm_pos(struct stream* s);
unsigned int strm_seek(struct stream* s, unsigned int offset);
unsigned int strm_length(struct stream* s);
char strm_peek(struct stream* s);
unsigned int strm_peek_n(struct stream* s, unsigned int n, char* buffer);
void skip_whitespace(struct stream* s);
int strm_match(struct stream* s, char* match);
int strm_expect(struct stream* s, char* expected, int log_level);
int strm_skip_thru(struct stream* s, char* expected);
char* strm_skip_thru_any(struct stream* s, char** expected, unsigned int count);

#endif // STREAM_H