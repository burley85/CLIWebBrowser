#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "logger.h"
#include "parse_html.h"

struct parser{
    struct istream* in;
    struct ostream* out;
    char** open_elements;
    unsigned int open_elements_count;
    unsigned int open_elements_capacity;
};

void parser_print_open_elements(struct parser* p) {
    for (unsigned int i = 0; i < p->open_elements_capacity; i++) {
        if(i == p->open_elements_count){
            debugf("---------------------------------\n");
        }

        if (p->open_elements[i]) {
            debugf("Open element: %s\n", p->open_elements[i]);
        }
        else {
           debugf("Open element: NULL\n");
        }
    }
}

struct parser* init_parser(struct stream* strm) {
    struct parser* p = malloc(sizeof(struct parser));
    if (!p) {
        errorf("Failed to allocate memory for parser\n");
        return NULL;
    }
    
    p->in = strm;
    p->out = init_ostream();
    if (!p->out) {
        errorf("Failed to initialize output stream\n");
        free(p);
        return NULL;
    }

    int starting_size = 1;

    p->open_elements = malloc(starting_size * sizeof(char*));
    if (!p->open_elements) {
        errorf("Failed to allocate memory for open elements\n");
        free(p);
        return NULL;
    }
    memset(p->open_elements, 0, starting_size * sizeof(char*));
    
    p->open_elements_count = 0;
    p->open_elements_capacity = starting_size;
    return p;
}

void delete_parser(struct parser* p) {
    if (!p) return;
    for (unsigned int i = 0; i < p->open_elements_count; i++) {
        free(p->open_elements[i]);
    }
    free(p->open_elements);
    free(p);
}

int parser_open_element(struct parser* p, char* element) {
    debugf("Opening element: %s\n", element);
    if(p->open_elements_count == p->open_elements_capacity) {
        char** new_elements = realloc(p->open_elements, p->open_elements_capacity * 2 * sizeof(char*));
        if (!new_elements) {
            errorf("Failed to reallocate memory for open elements\n");
            return 0;
        }
        p->open_elements = new_elements;

        memset(p->open_elements + p->open_elements_capacity, 0, p->open_elements_capacity * sizeof(char*));
        p->open_elements_capacity *= 2;
    }
    
    char* new_element = malloc(strlen(element) + 1);
    if(!new_element){
        errorf("Failed to allocate memory for new open element");
        return 0;
    }
    strcpy(new_element, element);
    new_element[strlen(element)] = '\0';

    p->open_elements[p->open_elements_count] = new_element;
    p->open_elements_count++;

    return 1;
}

int parser_element_is_open(struct parser* p, char* element) {
    for (int i = p->open_elements_count - 1; i >= 0; i--) {
        if (strcmp(p->open_elements[i], element) == 0) return 1;
    }
    return 0;
}

int parser_implicit_close_test(struct parser* p, char* element) {
    return 1;
}

int parser_close_element(struct parser* p, char* element) {
    debugf("Closing element: %s\n", element);
    if(!parser_element_is_open(p, element)) return 0;

    int count = p->open_elements_count;
    int i = count - 1;

    for (; i >= 0; i--) {
        if (strcmp(p->open_elements[i], element) == 0) {
            free(p->open_elements[i]);
            p->open_elements[i] = NULL;
            p->open_elements_count--;
            break;
        }
        else if (parser_implicit_close_test(p, p->open_elements[i])) {
            debugf("Implicitly closing element: %s\n", p->open_elements[i]);
            free(p->open_elements[i]);
            p->open_elements[i] = NULL;
        }
        p->open_elements_count--;
         
    }
    // Reopen elements that were not closed implicitly
    

    if(p->open_elements[p->open_elements_count]) p->open_elements_count++;
    
    for (int j = p->open_elements_count + 1; j < count; j++) {
        if(p->open_elements[j]) {
            p->open_elements[p->open_elements_count] = p->open_elements[j];
            p->open_elements_count++;
            p->open_elements[j] = NULL;
        }
    }
    return 1;    
}

char* parser_next_element(struct parser* p) {
    struct stream* s = p->in;
    char* element = NULL;

    if(!strm_skip_thru(s, "<")) return NULL;

    while(strm_peek(s) == '!'){
        if(!strm_match(s, "!--")){
            if(!strm_skip_thru(s, ">")) return NULL;
        }
        else {
            if(!strm_skip_thru(s, "-->")) return NULL;
        }

        if(!strm_skip_thru(s, "<")) return NULL;
    }

    if(strm_peek(s) == '/') {
        strm_next(s); // Skip '/'
        element = strm_get_word(s);
        if (!element) {
            errorf("Failed to read closing element\n");
            return NULL;
        }

        if(!strm_skip_thru(s, ">")){
            errorf("Failed to read closing element\n");
            free(element);
            return NULL;
        }
        
        if (!parser_close_element(p, element)) {
            errorf("Failed to close element: %s\n", element);
            free(element);
            return NULL;
        }
    }

    else {
        element = strm_get_word(s);
        if (!element) {
            errorf("Failed to read opening element\n");
            return NULL;
        }

        if(!strm_skip_thru(s, ">")){
            errorf("Failed to read opening element\n");
            free(element);
            return NULL;
        }

        if (!parser_open_element(p, element)) {
            errorf("Failed to open element: %s\n", element);
            free(element);
            return NULL;
        }
    }
    return element;
}

struct ostream* parse_html(struct stream* strm) {
    struct parser* p = init_parser(strm);
    if (!p) {
        errorf("Failed to initialize parser\n");
        return NULL;
    }

    while(parser_next_element(p)){
        int was_space = 1;
        int newline = 1;
        while(strm_peek(p->in) != '\0' && strm_peek(p->in) != '<'){
            char c = strm_next(p->in);
            if(isspace(c)){
                if (!was_space) {
                    ostrm_write(p->out, " ", 1);
                    was_space = 1;
                }
            }
            else {
                if (newline) {
                    ostrm_write(p->out, "\n", 1);
                    newline = 0;
                }
                ostrm_write(p->out, &c, 1);
                was_space = 0;
            }
        }
    }

    struct ostream* text = ostrm_copy(p->out);
    delete_parser(p);

    return text;
}