#include "disklist.h"
#include "diskinfo.h"
#include "diskget.h"
#include "diskput.h"
#include "helperfunctions.h"


int main(int argc, char* argv[]) {

    #if defined(PART1)
        diskinfo(argc, argv);
    #elif defined(PART2)
        disklist(argc, argv);
    #elif defined(PART3)
        diskget(argc, argv);
    #elif defined(PART4)
        diskput(argc, argv);
    // #else
    // #   error "PART[1234] must be defined"
    #endif

    return 0;

}
