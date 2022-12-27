#include "Scene.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <Windows.h>

    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

int main()
{
    Scene scene;
    scene.Start();
    return 0;
}