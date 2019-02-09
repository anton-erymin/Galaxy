#include "Application.h"

int main(int argc, char* argv[])
{
    return Application::GetInstance().Run(argc, argv);
}
