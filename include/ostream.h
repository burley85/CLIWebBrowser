#ifndef OSTREAM_H
#define OSTREAM_H

struct ostream;

struct ostream* init_ostream();
int delete_ostream(struct ostream* s);
int ostrm_write(struct ostream* s, char* data, unsigned int length);
int ostrm_length(struct ostream* s);
struct ostream* ostrm_copy(struct ostream* s);
char* ostrm_to_string(struct ostream* s);

#endif // OSTREAM_H