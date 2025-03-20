#pragma once

#include <windows.h>
#include <intrin.h>
#include <stdio.h>
#ifdef _WIN64 
#define NATIVE_VALUE DWORD64  
#endif

extern "C" NATIVE_VALUE Global_OrigCreateFileReturnAddr;
extern "C" NATIVE_VALUE Global_OrigReferenceAddrOne;
extern "C" NATIVE_VALUE Global_OrigReferenceAddrTwo;
extern "C" NATIVE_VALUE MainAddr;
extern "C" NATIVE_VALUE HeapAddress;

extern "C" DWORD index;

extern "C" VOID labelStubOne();
extern "C" VOID labelStubTwo();
extern "C" VOID HookStub();
extern "C" NATIVE_VALUE FinalStackVal;
extern "C" VOID GetStackOffset();
extern "C" VOID FindStackPtr();
extern "C" NATIVE_VALUE SizeOfStack;

#define STACK_OFFSET_ADJUST 0x20
#define MAX_STACK_SEARCH    1024