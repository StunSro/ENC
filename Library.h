#pragma once
#include "pch.h"

//TODO: Check if calls to ReadFile() are from GFXFileManager / see ret addrs

class CLibrary
{
private:
	/*
	typedef int(__thiscall* FN_IFileManager_Read)(HANDLE hFile, char* lpBuffer, int nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead);

	static FN_IFileManager_Read s_pfnCPFileManager_Read;
	*/
	
public:
	static void Initialize();

};