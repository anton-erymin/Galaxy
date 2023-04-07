#include "GalaxyEngine.h"

#define NUCLEUS_DEFINE_ENTRY_POINT(engine)\
int main()\
{\
    cout << #engine;\
    return 0;\
}

NUCLEUS_DEFINE_ENTRY_POINT(GalaxyEngine)
