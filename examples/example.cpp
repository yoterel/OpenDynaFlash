#include "OpenDynaFlash.h"

int main(int argc, char *argv[])
{
    DynaFlashProjector projector(false, false, 20, true); // create projector
    projector.init();                                     // init projector
    projector.show();                                     // projector will show a white image
    return 0;
}
