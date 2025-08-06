#ifndef ISTREAM_H
#define ISTREAM_H

struct istream;

struct istream* init_stream(char* data);
int delete_stream(struct istream* s);
unsigned int istrm_pos(struct istream* s);
unsigned int istrm_seek(struct istream* s, unsigned int offset);
unsigned int istrm_length(struct istream* s);
char istrm_peek(struct istream* s);
unsigned int istrm_peek_n(struct istream* s, unsigned int n, char* buffer);
void skip_whitespace(struct istream* s);
int istrm_match(struct istream* s, char* match);
char istrm_next(struct istream* s);
int istrm_expect(struct istream* s, char* expected, int log_level);
int istrm_skip_thru(struct istream* s, char* expected);
char* istrm_skip_thru_any(struct istream* s, char** expected, unsigned int count);
char* istrm_get_word(struct istream* s);
int istrm_copy(struct istream* s, char* dest, unsigned int n);
char* istrm_remaining(struct istream* s);

#endif // ISTREAM_H