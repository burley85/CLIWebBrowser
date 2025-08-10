#ifndef FORMATTER_H
#define FORMATTER_H

struct format* init_format();
void free_format(struct format* format);
void reset_format(struct format* format);
char* format_string(struct format* format);

void set_text_color(struct format* format, unsigned char r, unsigned char g, unsigned char b);
void set_background_color(struct format* format, unsigned char r, unsigned char g, unsigned char b);
void set_bold(struct format* format);
void clear_bold(struct format* format);
void set_italic(struct format* format);
void clear_italic(struct format* format);
void set_underline(struct format* format);
void clear_underline(struct format* format);
void set_strike(struct format* format);
void clear_strike(struct format* format);

#endif