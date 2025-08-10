#include <string.h>
#include <stdlib.h>

#include "logger.h"
#include "formatter.h"

struct format {
    int bold;
    int italic;
    int underline;
    int strike;

    unsigned char fg_red;
    unsigned char fg_green;
    unsigned char fg_blue;

    unsigned char bg_red;
    unsigned char bg_green;
    unsigned char bg_blue;

    int up_to_date;

    int (*print)(char*);
};

struct format* init_format() {
    struct format* format = malloc(sizeof(struct format));
    if(!format){
        errorf("Failed to allocate memory for console format");
        return NULL;
    }
    reset_format(format);
    return format;
}

void free_format(struct format* format) {
    free(format);
}

void reset_format(struct format* format) {
    format->up_to_date = 0;

    format->bold = 0;
    format->italic = 0;
    format->underline = 0;
    format->strike = 0;

    format->fg_red = 0;
    format->fg_green = 0;
    format->fg_blue = 0;
    
    format->bg_red = 255;
    format->bg_green = 255;
    format->bg_blue = 255;
}

char* format_string(struct format* format){
    if(format->up_to_date) return "";
    
    static char buffer[1024] = "\033[0";

    if(format->bold) sprintf(buffer + strlen(buffer), ";1");
    if(format->italic) sprintf(buffer + strlen(buffer), ";3");
    if(format->underline) sprintf(buffer + strlen(buffer), ";4");
    if(format->strike) sprintf(buffer + strlen(buffer), ";9");

    sprintf(buffer + strlen(buffer), ";38;2;%d;%d;%d", format->fg_red, format->fg_green, format->fg_blue);
    sprintf(buffer + strlen(buffer), ";48;2;%d;%d;%dm", format->bg_red, format->bg_green, format->bg_blue);

    format->up_to_date = 1;
    return buffer;
}

void set_text_color(struct format* format, unsigned char r, unsigned char g, unsigned char b) {
    format->up_to_date = 0;
    format->fg_red = r;
    format->fg_green = g;
    format->fg_blue = b;
}

void set_background_color(struct format* format, unsigned char r, unsigned char g, unsigned char b) {
    format->up_to_date = 0;
    format->bg_red = r;
    format->bg_green = g;
    format->bg_blue = b;
}

void set_bold(struct format* format) {
    format->up_to_date = 0;
    format->bold = 1;
}

void clear_bold(struct format* format) {
    format->up_to_date = 0;
    format->bold = 0;
}

void set_italic(struct format* format) {
    format->up_to_date = 0;
    format->italic = 1;
}

void clear_italic(struct format* format) {
    format->up_to_date = 0;
    format->italic = 0;
}

void set_underline(struct format* format) {
    format->up_to_date = 0;
    format->underline = 1;
}

void clear_underline(struct format* format) {
    format->up_to_date = 0;
    format->underline = 0;
}

void set_strike(struct format* format) {
    format->up_to_date = 0;
    format->strike = 1;
}

void clear_strike(struct format* format) {
    format->up_to_date = 0;
    format->strike = 0;
}

