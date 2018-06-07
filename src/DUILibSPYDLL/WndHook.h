#pragma once


#ifndef DUILIBSPYDLL_EXPORTS
#define DLL_EXPORT __declspec(dllimport)
#pragma comment(lib, "DUILibSPYDLL.lib")
#else
#define DLL_EXPORT __declspec(dllexport)
#endif // !DUILIBSPYDLL_EXPORTS

DLL_EXPORT BOOL SetHook();
DLL_EXPORT BOOL UnSetHook();