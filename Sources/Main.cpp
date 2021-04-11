#include "GalaxyApplication.h"
#include "WinApplication.h"

constexpr auto kApplicationWidth = 1540u;
constexpr auto kApplicationHeight = 870u;

int main(int argc, char** argv)
{
    GalaxyApplication* obj = new GalaxyApplication();
    
    WinApplication application(*obj, kApplicationWidth, kApplicationHeight);
    return application.Run(argc, argv);
}
