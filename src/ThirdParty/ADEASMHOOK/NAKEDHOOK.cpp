#include <windows.h>
#include "ADEASM.h"
#include "NAKEDHOOK.h"

#define NAKED __declspec(naked)
void NAKED Stub()
{
	__asm
	{
		pop eax
		push OLD_CODE
		push eax///如果使用其它的寄存器可能会失败，一般是在eax里面返回数据，所以这个可以随便用
		_emit 0xE9
			nop
			nop
			nop
			nop
			nop
OLD_CODE:
			_emit 0xCC
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			int 3
			ret
	}
}


BOOLEAN GetNeedRvaForHook(unsigned char * Fun,ULONG* JmpRva,ULONG* OldRva,ULONG*FuncLen)
{
	unsigned char* Old_code = 0,*Jmp_code = 0,*Point = 0;
	ULONG StubLen=GetFunctionLength((BYTE*)Fun);
	if(StubLen<5)
		return FALSE;
	*FuncLen=StubLen;
	Point=(BYTE*)Fun;
	for(ULONG i=0;i<StubLen;i++)
	{
		if(Point[i]==0xe9)
			Jmp_code=&Point[i];
		if(Point[i]==0xcc)
		{
			Old_code=&Point[i];
			break;
		}		
	}
	*JmpRva=(ULONG)Jmp_code-(ULONG)Fun;
	*OldRva=(ULONG)Old_code-(ULONG)Fun;
	return TRUE;

}

BOOL InstallInlineByAddress(PVOID FuncAddr,PVOID  NewAddr,PHOOK_INFO Info)
{
	DWORD OldPro;
	ULONG JmpRva=0,OldRva=0,StubLen=0;
	int NeedLen;
	unsigned char OPCode[6]={0};
	__try
	{
	PVOID Buffer=VirtualAlloc(NULL,StubLen+8,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if(Buffer==0)
		return  FALSE;
	if(!FuncAddr||!NewAddr)
		return FALSE;
	if(!GetNeedRvaForHook((unsigned char *)Stub,&JmpRva,&OldRva,&StubLen))
		return FALSE;
	if(Info)
	{
		Info->OldAddress=FuncAddr;
		Info->StubAddress=Buffer;
		RtlCopyMemory(Info->SavedCode,FuncAddr,16);
	}
	RtlCopyMemory(Buffer,Stub,StubLen);//将Stub放到内存里面,为了支持多个函数 hook
	OPCode[0]=0x58;
	OPCode[1]=0x68;
	*(DWORD*)&OPCode[2]=(ULONG)Buffer+OldRva;
	RtlCopyMemory(Buffer,OPCode,6);
	OPCode[0]=0xe9;
	NeedLen=GetProbLength((BYTE*)FuncAddr,5);
	RtlCopyMemory((char*)Buffer+OldRva,FuncAddr,NeedLen);//复制将要被覆盖的指令
	*(DWORD*)&OPCode[1]=(ULONG)FuncAddr+NeedLen-((ULONG)Buffer+OldRva+NeedLen+5);
	RtlCopyMemory((char*)Buffer+OldRva+NeedLen,OPCode,5);//写上跳回去执行的指令
	*(DWORD*)&OPCode[1]=(ULONG)NewAddr-((ULONG)Buffer+JmpRva+5);
	RtlCopyMemory((char*)Buffer+JmpRva,OPCode,5);//跳转到新函数的指令
	*(DWORD*)&OPCode[1]=(ULONG)Buffer-((ULONG)FuncAddr+5);
	if (!VirtualProtect(FuncAddr,NeedLen,PAGE_EXECUTE_READWRITE,&OldPro))
	{
		VirtualFree(Buffer,0,MEM_RELEASE);
		return FALSE;
	}
	if(Info)
	{
		RtlCopyMemory(Info->SavedOpCode,OPCode,5);
	}
	RtlCopyMemory(FuncAddr,OPCode,5);//修改原始函数入口点
	VirtualProtect(FuncAddr,NeedLen,OldPro,&OldPro);
	return TRUE;
	}
	__except(1)
	{
		return FALSE;
	}
}



BOOL InstallInlineByName(char *ModuleName,char *FuncName,PVOID NewAddr,PHOOK_INFO Info)
{
	PVOID FuncAddr;
	HMODULE hModule=GetModuleHandleA(ModuleName);
	if(hModule==NULL)
		hModule=LoadLibraryA(ModuleName);
	if(hModule==0)
		return FALSE;
	FuncAddr=(PVOID)GetProcAddress(hModule,FuncName);
	if(FuncAddr==0)
		return FALSE;
	return InstallInlineByAddress(FuncAddr,NewAddr,Info);
}
VOID UnInstallInline(PHOOK_INFO Info)
{
	DWORD OldPro;
	if(!Info)
		return;
	if(VirtualProtect(Info->OldAddress,16,PAGE_EXECUTE_READWRITE,&OldPro))
	{
		RtlCopyMemory(Info->OldAddress,Info->SavedCode,16);
		VirtualFree(Info->StubAddress,0,MEM_RELEASE);
		VirtualProtect(Info->StubAddress,16,OldPro,&OldPro);
	}

}

#ifdef SUPORTFASTCALL
void NAKED FSStub()
{
	__asm
	{
		pop eax
		push edx//压入第二个参数
		push eax//将原始函数地址放下去
		mov edx,ecx//设置第个参数
		mov ecx,Addr//设置第一个参数
		_emit 0xE9
		nop
		nop
		nop
		nop
		nop
Addr:
		_emit 0xCC
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		int 3
		ret;

	}
}

void NAKED FCStdStub()
{
	__asm
	{
		pop eax
			push edx//压入第二个参数
			push ecx//压入第一个参数
			push OLD_CODE//压入原始地址
			push eax//压入返回地址
			_emit 0xE9
			nop
			nop
			nop
			nop
			nop
OLD_CODE:
		_emit 0xCC
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			int 3
			ret

	}
}

BOOLEAN InstallInlineForFastCall(unsigned char * OldAddr,unsigned char* NewAddr,PHOOK_INFO Info,
				 BOOLEAN CovertToStd)
{
	/*
	最后一个参数指明是否转成stdcall
	*/
	DWORD OldPro;
	ULONG JmpRva,OldRva,StubLen;
	int NeedLen;
	unsigned char OPCode[6]={0};
	unsigned char * Func=(unsigned char*)FSStub;
	if(CovertToStd)
		Func=(unsigned char *)FCStdStub;
	if(!GetNeedRvaForHook(Func,&JmpRva,&OldRva,&StubLen))
	{
		return FALSE;
	}
	if(!OldAddr||!NewAddr)
		return FALSE;
	__try
	{
		PVOID Buffer=VirtualAlloc(NULL,StubLen+8,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
		if(Buffer==0)
			return  FALSE;
		if(Info)
		{
			Info->OldAddress=OldAddr;
			Info->StubAddress=Buffer;
			RtlCopyMemory(Info->SavedCode,OldAddr,16);
		}
		RtlCopyMemory(Buffer,Func,StubLen);//将Stub放到内存里面,为了支持多个函数 hook
		if(CovertToStd)
		{//转成stdcall
			*(DWORD*)((char*)Buffer+4)=(ULONG)Buffer+OldRva;
		}
		else
		{//使用fastcall
			*(DWORD*)((char*)Buffer+6)=(ULONG)Buffer+OldRva;
		}
		OPCode[0]=0xe9;
		NeedLen=GetProbLength((BYTE*)OldAddr,5);
		RtlCopyMemory((char*)Buffer+OldRva,OldAddr,NeedLen);//复制将要被覆盖的指令
		*(DWORD*)&OPCode[1]=(ULONG)OldAddr+NeedLen-((ULONG)Buffer+OldRva+NeedLen+5);
		RtlCopyMemory((char*)Buffer+OldRva+NeedLen,OPCode,5);//写上跳回去执行的指令
		*(DWORD*)&OPCode[1]=(ULONG)NewAddr-((ULONG)Buffer+JmpRva+5);//跳转地址
		RtlCopyMemory((char*)Buffer+JmpRva,OPCode,5);//跳转到新函数的指令
		*(DWORD*)&OPCode[1]=(ULONG)Buffer-((ULONG)OldAddr+5);
		if (!VirtualProtect(OldAddr,NeedLen,PAGE_EXECUTE_READWRITE,&OldPro))
		{
			VirtualFree(Buffer,0,MEM_RELEASE);
			return FALSE;
		}
		if(Info)
		{
			RtlCopyMemory(Info->SavedOpCode,OPCode,5);
		}
		RtlCopyMemory(OldAddr,OPCode,5);//修改原始函数入口点
		VirtualProtect(OldAddr,NeedLen,OldPro,&OldPro);
		return TRUE;
	}
	__except(1)
	{
		return FALSE;
	}
}



#endif