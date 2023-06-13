#include "errors.h"

#include <stdio.h>
#include <stdlib.h>

void termError(char* msg) {
    perror(msg);
    exit(1);
}