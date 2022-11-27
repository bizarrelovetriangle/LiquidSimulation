#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#include "Sceen.h"

//#ifdef __cplusplus
//extern "C" {
//#endif
//#include <Windows.h>
//
//    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
//    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
//
//#ifdef __cplusplus
//}
//#endif

int main()
{
    Sceen sceen(false);
    sceen.Start();
    return 0;
}