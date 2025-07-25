#include "logger.h"


int main(int argc, char *argv[]) {
    init_logger("debug.log", "warning.log", "error.log");
    return 0;
}