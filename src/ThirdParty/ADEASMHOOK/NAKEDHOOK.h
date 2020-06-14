#ifndef HOOK_LIB_H_
#define HOOK_LIB_H_
#define SUPORTFASTCALL 1
typedef struct _HOOKINFO
{
	PVOID OldAddress;
	PVOID StubAddress;
	unsigned char  SavedCode[16];
	unsigned char SavedOpCode[10];//呵呵，用于猥琐的进行重复检查
}HOOK_INFO,*PHOOK_INFO;

BOOL  HookOneIAT(char*ModuleName,PVOID FunAddr,PVOID NewFunAddr,PVOID Base);
BOOL HookAModule(char* ModuleName,PVOID NewFunc,PVOID OldFunAddr);

BOOL InstallInlineByAddress(PVOID FuncAddr,PVOID  NewAddr,PHOOK_INFO Info);

BOOL InstallInlineByName(char *ModuleName,char *FuncName,PVOID NewAddr,PHOOK_INFO Info);

#ifdef SUPORTFASTCALL
BOOLEAN InstallInlineForFastCall(unsigned char * OldAddr,unsigned char* NewAddr,PHOOK_INFO Info,
				 BOOLEAN CovertToStd);
#endif

VOID UnInstallInline(PHOOK_INFO Info);
#ifdef NEEDDEBUG
#define PrintA(a) OutputDebugStringA(a)
#else
#define PrintA(a)
#endif 

#endif
