#include <bareio.h>
#include <barelib.h>

byte builtin_welcome(){
    printf("\033[95m-----------------------------\n");
    printf("\033[96mWelcome to Matei Shell \033[91m(Mash)\n");
    printf("\033[95m-----------------------------\033[39m\n\n");
 
    return 0;
}