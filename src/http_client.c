#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wspiapi.h>

#include "logger.h"
#include "stream.h"
#include "http_client.h"

int init_http_client() {
    static int initialized = 0;
    if (initialized) return 0;

    WSADATA wsaData;
    int rval = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (rval != 0) {
        errorf("WSAStartup failed with error: %d\n", rval);
        return 1;
    }

    return 0;
}

struct addrinfo* get_server_address(char *url) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *result = NULL;

    char url_copy[MAX_URL_LENGTH + 1] = {0};
    if (strlen(url) > MAX_URL_LENGTH) {
        errorf("URL exceeds maximum length\n");
        return NULL;
    }
    strncpy(url_copy, url, MAX_URL_LENGTH);
    url = url_copy;

    char* path = strchr(url, '/');
    if (path) *path = '\0';

    int rval = getaddrinfo(url, "80", &hints, &result);

    if (path) *path = '/';

    if (rval != 0) {
        errorf("getaddrinfo failed with error: %d\n", rval);
        return NULL;
    }
    return result;
}

SOCKET connect_to_server(struct addrinfo *result) {
    SOCKET s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (s == INVALID_SOCKET) {
        errorf("socket failed with error: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    if (connect(s, result->ai_addr, (int)result->ai_addrlen)) {
        errorf("connect failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        return INVALID_SOCKET;
    }
    return s;
}

unsigned int hex_char_to_int(char hex) {
    switch(hex) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A':
        case 'a': return 10;
        case 'B':
        case 'b': return 11;
        case 'C':
        case 'c': return 12;
        case 'D':
        case 'd': return 13;
        case 'E':
        case 'e': return 14;
        case 'F':
        case 'f': return 15;
    }
    return 0; // Invalid hex character
}

unsigned int hex_str_to_int(char* hex, unsigned int length) {
    if(length == 0) return 0;

    unsigned int value = hex_str_to_int(hex, length - 1);
    return  value * 16 + hex_char_to_int(hex[length - 1]);

}


char* buffer_response(SOCKET s, int* buffer_size) {
    int max_size = 8192;
    char* buffer = malloc(max_size);
    if (!buffer) {
        errorf("Failed to allocate memory for response buffer\n");
        return NULL;
    }

    *buffer_size = 0;
    int rval = recv(s, buffer, max_size, 0);
    while(rval > 0) {
        *buffer_size += rval;
        if(*buffer_size == max_size) {
            max_size *= 2;
            char* new_buffer = realloc(buffer, max_size);
            if (!new_buffer) {
                errorf("Failed to reallocate memory for response buffer\n");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        rval = recv(s, buffer + *buffer_size, max_size - *buffer_size, 0);
    }
    if (rval < 0) {
        errorf("recv failed with error: %d\n", WSAGetLastError());
        free(buffer);
        return NULL;
    }

    char* new_buffer = realloc(buffer, *buffer_size);
    if (!new_buffer) {
        errorf("Failed to reallocate memory for response buffer\n");
        free(buffer);
        return NULL;
    }

    return new_buffer;
}
    

//Skip to chunk length
//Parse chunk size
//Read chunk data
//Skip to next chunk
//Repeat until chunk size is 0
//Return file data
char* chunked_transfer(SOCKET s, char* response, int buffer_size, unsigned int* file_size) {
    struct stream* strm = init_stream(response);
    if (!strm) {
        errorf("Failed to initialize stream\n");
        return NULL;
    }

    char* file_data = malloc(buffer_size);
    *file_size = 0;
    if (!file_data) {
        errorf("Failed to allocate memory for file data\n");
        delete_stream(strm);
        return NULL;
    }

    while(1){
        strm_skip_thru(strm, "\r\n\r\n");
        if(strm_length(strm) <= 0) break;


        char* chunk_size_str = strm_get_word(strm);

        if (!chunk_size_str) {
            errorf("Failed to read chunk size\n");
            free(file_data);
            delete_stream(strm);
            return NULL;
        }
        unsigned int chunk_size = hex_str_to_int(chunk_size_str, strlen(chunk_size_str));
        free(chunk_size_str);

        if (chunk_size == 0) break;

        strm_skip_thru(strm, "\r\n");

        int bytes_read = strm_copy(strm, file_data + *file_size, chunk_size);
        if (bytes_read != chunk_size) {
            errorf("Failed to read complete chunk, expected %u bytes, got %d bytes\n", chunk_size, bytes_read);
            free(file_data);
            delete_stream(strm);
            return NULL;
        }
        *file_size += bytes_read;
    }
    strm_skip_thru(strm, "\r\n");
    
    if (strm_length(strm) > 0) {
        errorf("Unexpected data (%d bytes) after last chunk: %s\n", strm_length(strm), strm_remaining(strm));
        free(file_data);
        delete_stream(strm);
        return NULL;
    }

    char* final_data = realloc(file_data, *file_size);
    if (!final_data) {
        errorf("Failed to reallocate memory for final file data\n");
        free(file_data);
        delete_stream(strm);
        return NULL;
    }

    delete_stream(strm);
    return final_data;
}


char* download_file(char* url) {
    debugf("Downloading file from %s\n", url);
    int rval = init_http_client();
    if(rval) return NULL;

    struct addrinfo *result = get_server_address(url);
    if(result == NULL) {
        WSACleanup();
        return NULL;
    }
    debugf("Connecting to server...\n");

    SOCKET s = connect_to_server(result);
    freeaddrinfo(result);
    if(s == INVALID_SOCKET) {
        WSACleanup();
        return NULL;
    }
    debugf("Connected to server\n");

    if(strstr(url, "://"))
        url = strstr(url, "://") + 3;
    
    char* path = strchr(url, '/');
    unsigned int hostname_length;
    if(path) {
        hostname_length = path - url;
    } else {
        hostname_length = strlen(url);
        path = "/";
    }

    char *sendbuf_format = "GET %s HTTP/1.1\r\nHost: %.*s\r\nConnection: close\r\nAccept: text/html\r\n\r\n";
    char* sendbuf = malloc(strlen(sendbuf_format) + strlen(url) + 20);
    if (!sendbuf) {
        errorf("Failed to allocate memory for send buffer\n");
        closesocket(s);
        WSACleanup();
        return NULL;
    }

    snprintf(sendbuf, strlen(sendbuf_format) + strlen(url) + 20, sendbuf_format, path, hostname_length, url);

    rval = send(s, sendbuf, (int)strlen(sendbuf), 0);
    if (rval == SOCKET_ERROR) {
        errorf("send failed with error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return NULL;
    }
    free(sendbuf);


    int buffer_size = 0;
    char* response = buffer_response(s, &buffer_size);
    if (!response) {
        closesocket(s);
        WSACleanup();
        return NULL;
    }

    char* file_data = NULL;
    unsigned int file_size = 0;
    file_data = chunked_transfer(s, response, buffer_size, &file_size);
    free(response);

    closesocket(s);
    WSACleanup();
    return file_data;
}