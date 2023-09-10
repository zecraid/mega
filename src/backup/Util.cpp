#include "Util.h"
#include "../log/Log.h"
#include <stdio.h>
#include <stdlib.h>

void Util::errif(bool condition, const char *errmsg){
    if(condition){
        LOG_ERROR(errmsg);
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}

void Util::welcome() {
    printf("======================================================== \n");
    printf("    /\\_____/\\         __  ___    ______   ______    ___ \n");
    printf("   /  o   o  \\       /  |/  /   / ____/  / ____/   /   |\n");
    printf("  ( ==  ^  == )     / /|_/ /   / __/    / / __    / /| |\n");
    printf("   )         (     / /  / /   / /___   / /_/ /   / ___ |\n");
    printf("  (           )   /_/  /_/   /_____/   \\____/   /_/  |_|\n");
    printf(" ( (  )   (  ) )    Author: labcz      Date: 2023-08-27\n");
    printf("======================================================== \n");
}