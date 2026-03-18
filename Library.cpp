#include "pch.h"
#include "EncKeyDef.h"
#include "RemodelUtility.h"
#include "MemoryUtility.h"
#include "FilenameLists.h"
#include "Library.h"
#include <cstdio>
#include <cstring>
#include <map>
#include <fstream>
#include <ctime>
#include <iostream>
#include <Shlwapi.h>
#include "resource.h"
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")

//#define ENABLE_DEBUG_CONSOLE
#define ENABLE_ENCRYPTION
#define BUILD_VSRO
//#define BUILD_ISRO

#define PACK_FILE_NEW_NAME  "srdb.db"
#define SRO_CLIENT_NAME_LAUNCHER    "sro_client"
#define SRO_CLIENT_NAME     "sro_client.exe"
//#define SRO_REPLACER_NAME   ".\\setting\\Importer.exe"
#define SRO_REPLACER_NAME   "replacer.exe"
#define SRO_LAUNCHER_NAME   "silkroad.exe"
//#define SRO_REPLACER_NAME_WITH_FORMAT   "%s\\setting\\Importer.exe"
#define SRO_REPLACER_NAME_WITH_FORMAT   "replacer.exe"
#define SRO_PK2_KEY         "fgsaS<dafs%^$I^$@%MSFDGsdfmDFG654_!51a"

class IFileManager;

typedef HMODULE(__stdcall* FN_LOADLIBRARYA)(const char* szLibName);
typedef int(__stdcall* FN_GFXDLL_CREATE_OBJECT)(int mode, IFileManager** ppObject, int version);
typedef BOOL(__thiscall* FN_READ_FILE)(IFileManager* pSelf, HANDLE hFile, char* lpBuffer, int bytesToRead, unsigned long* bytesRead);
typedef BOOL(__thiscall* FN_WRITE_FILE)(IFileManager* pSelf, HANDLE hFile, const char* lpBuffer, int bytesToWrite, unsigned long* bytesWritten);
typedef int(__thiscall* FN_FILE_NAME_FROM_HANDLE)(IFileManager* pSelf, HANDLE hFile, char* dest, size_t count);
typedef void* (__thiscall* FN_OPEN_FM)(void* pSelf, const char* szFile, const char* szKey, int access);
typedef void* (__thiscall* FN_OPEN_FILE_10)(void* pSelf, void* pArchiveFm, char* szFileName, int nAccess, int nUnk);
typedef void* (__thiscall* FN_OPEN_FILE_11)(void* pSelf, char* szFileName, int nAccess, int nUnk);
typedef std::list<const char*> FileList;

FileList excludedFileNames;
FileList includedTxtFileNames;
FileList excludedFolderNames;

FN_LOADLIBRARYA pfnLoadLibraryA = NULL;
FN_READ_FILE pfnReadFile = NULL;
FN_WRITE_FILE pfnWriteFile = NULL;
FN_FILE_NAME_FROM_HANDLE pfnFileNameFromHandle = NULL;
FN_OPEN_FILE_10 pfnOpenFile_10 = NULL;
FN_OPEN_FILE_11 pfnOpenFile_11 = NULL;
FN_OPEN_FM pfnOpenFM = NULL;

std::map<void*, std::string> mapPtrFileName;
std::ofstream logFile;

void LogOperation(const char* fileName, bool isEncrypted, bool success)
{
#ifdef ENABLE_DEBUG_CONSOLE
    if (!logFile.is_open())
    {
    logFile.open("file_operations.txt", std::ios::app);
    }

    time_t now = time(0);
    char dt[64] = { 0 };
    ctime_s(dt, sizeof(dt), &now);
    dt[strlen(dt) - 1] = '\0'; // Remove newline

    logFile << dt << " | File: " << fileName << " | Encrypted: " << (isEncrypted ? "Yes" : "No") << std::endl;
    logFile.flush();
#endif
}

void* __fastcall MyOpenFile_10(void* pSelf, void* /* dummy edx */, void* pArchiveFm, char* szFileName, int nAccess, int nUnk)
{
   // printf("OpenFile %s\n", szFileName);

    std::map<void*, std::string>::iterator it = mapPtrFileName.find(pSelf);
    char buffer[2048] = { 0 };

    if (it != mapPtrFileName.end())
    {
        if (strcmp(it->second.c_str(), "Particles.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "particles", szFileName);
            return pfnOpenFile_10(pSelf, pArchiveFm, buffer, nAccess, nUnk);
        }
        else if (strcmp(it->second.c_str(), "Music.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "music", szFileName);
            return pfnOpenFile_10(pSelf, pArchiveFm, buffer, nAccess, nUnk);
        }
        else if (strcmp(it->second.c_str(), "Data.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "data", szFileName);
            return pfnOpenFile_10(pSelf, pArchiveFm, buffer, nAccess, nUnk);
        }
        else if (strcmp(it->second.c_str(), "Media.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "media", szFileName);
            return pfnOpenFile_10(pSelf, pArchiveFm, buffer, nAccess, nUnk);
        }
        else if (strcmp(it->second.c_str(), "Map.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "map", szFileName);
            return pfnOpenFile_10(pSelf, pArchiveFm, buffer, nAccess, nUnk);
        }
    }

    return pfnOpenFile_10(pSelf, pArchiveFm, szFileName, nAccess, nUnk);
}

void* __fastcall MyOpenFile_11(void* pSelf, void* /* dummy edx */, char* szFileName, int nAccess, int unk)
{

    //printf("OpenFile %s\n", szFileName);

    std::map<void*, std::string>::iterator it = mapPtrFileName.find(pSelf);
    char buffer[2048] = { 0 };

    if (it != mapPtrFileName.end())
    {
        if (strcmp(it->second.c_str(), "Particles.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "particles", szFileName);
            return pfnOpenFile_11(pSelf, buffer, nAccess, unk);
        }
        else if (strcmp(it->second.c_str(), "Music.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "music", szFileName);
            return pfnOpenFile_11(pSelf, buffer, nAccess, unk);
        }
        else if (strcmp(it->second.c_str(), "Data.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "data", szFileName);
            return pfnOpenFile_11(pSelf, buffer, nAccess, unk);
        }
        else if (strcmp(it->second.c_str(), "Media.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "media", szFileName);
           // printf("supposed to open %s\n", buffer);
            
            if (strcmp(buffer, "media\\SV.T") == 0)
                sprintf(buffer, "SV.T"); //but why cant go to media folder ?? find out... !

            if (strcmp(buffer, "media\\sv.t") == 0)
            {
                sprintf(buffer, "sv.t"); //but why cant go to media folder ?? find out... !
               // printf("PATCH VERSION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            void* pRes = pfnOpenFile_11(pSelf, buffer, nAccess, unk);

           // printf("pRes = 0x%x\n", pRes);
            return pRes;
        }
        else if (strcmp(it->second.c_str(), "Map.pk2") == 0)
        {
            sprintf(buffer, "%s\\%s", "map", szFileName);
            return pfnOpenFile_11(pSelf, buffer, nAccess, unk);
        }
    }

    return pfnOpenFile_11(pSelf, szFileName, nAccess, unk);
}

void GetDirectoryPath(const char* fullPath, char* directoryPath, size_t maxLen)
{
    const char* lastSlash = strrchr(fullPath, '\\');
    if (lastSlash)
    {
        size_t length = lastSlash - fullPath + 1;
        if (length < maxLen)
        {
            strncpy(directoryPath, fullPath, length);
            directoryPath[length] = '\0';
        }
        else
        {
            directoryPath[0] = '\0';
        }
    }
    else
    {
        directoryPath[0] = '\0';
    }
}

namespace Encryption
{
    void Decrypt(char* buffer, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            unsigned char temp = DECRYPTION_TABLE[static_cast<unsigned char>(buffer[i])];
            buffer[i] = static_cast<char>(temp ^ ENC_DEC_XOR_KEY);
        }
    }

    void Encrypt(const char* src, char* dest, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            unsigned char temp = static_cast<unsigned char>(src[i] ^ ENC_DEC_XOR_KEY);
            dest[i] = ENCRYPTION_TABLE[temp];
        }
    }
}

void DeleteExtraMaxiFiles();

bool IsFileEncrypted(IFileManager* fileManager, HANDLE fileHandle)
{

    char fileNameBuffer[512] = { 0 };
    bool success = true;
    bool isEncrypted = true;

    try
    {
        pfnFileNameFromHandle(fileManager, fileHandle, fileNameBuffer, sizeof(fileNameBuffer));
        std::string fullPath(fileNameBuffer);
        //printf("%s -> from 0x%x\n", fullPath.c_str(), REMODEL_GET_RETURN_ADDRESS());
       // if ((DWORD)REMODEL_GET_RETURN_ADDRESS() == 0x5db85f45)
        //    return false;
#ifndef ENABLE_ENCRYPTION
        return false;
#endif


        for (FileList::iterator it = g_lstNonEncFolderNames.begin(); it != g_lstNonEncFolderNames.end(); ++it)
        {
            if (fullPath.rfind(*it, 0) == 0)
            {
                isEncrypted = false;
                break;
            }
        }

        if (isEncrypted)
        {
            for (FileList::iterator it = g_lstNonEncFileNames.begin(); it != g_lstNonEncFileNames.end(); ++it)
            {
               // printf("%s\n", fileNameBuffer);
                if (strcmp(*it, fileNameBuffer) == 0)
                {
                    isEncrypted = false;
                    break;
                }
            }
        }

        const char* extension = PathFindExtensionA(fileNameBuffer);
        if (extension && (strcmp(extension, ".txt") == 0 || strcmp(extension, ".TXT") == 0))
        {
            isEncrypted = false;
            for (FileList::iterator it = g_lstEncTxtFileNames.begin(); it != g_lstEncTxtFileNames.end(); ++it)
            {
                if (strcmp(*it, fileNameBuffer) == 0)
                {
                    isEncrypted = true;
                    break;
                }
            }
        }

#ifdef ENABLE_DEBUG_CONSOLE
        //printf("FileName %s, encrypted = %s\n", fileNameBuffer, isEncrypted ? "yes" : "no");
      //  LogOperation(fileNameBuffer, isEncrypted, success);

       // if(strcmp(fileNameBuffer, "map\\tile2d\\c_grass_fld_10.ddj") == 0)
       //     DeleteExtraMaxiFiles();
#endif
        return isEncrypted;
    }
    catch (...)
    {
#ifdef ENABLE_DEBUG_CONSOLE
       //printf("FileName %s, encrypted = %s (ERROR)\n", fileNameBuffer, isEncrypted ? "yes" : "no");
       // LogOperation(fileNameBuffer, isEncrypted, false);
#endif
        throw;
    }
}

BOOL __fastcall MyReadFile(IFileManager* self, void* /* dummy */, HANDLE file, char* buffer, int bytesToRead, unsigned long* bytesRead)
{
    BOOL result = pfnReadFile(self, file, buffer, bytesToRead, bytesRead);
    if (IsFileEncrypted(self, file))
    {
        Encryption::Decrypt(buffer, *bytesRead);
    }
    return result;
}

BOOL __fastcall MyWriteFile(IFileManager* self, void* /* dummy */, HANDLE file, const char* buffer, int bytesToWrite, unsigned long* bytesWritten)
{
    //printf("Write...\n");

    char fileNameBuffer[512] = { 0 };
    bool success = true;
    bool isEncrypted = true;

        pfnFileNameFromHandle(self, file, fileNameBuffer, sizeof(fileNameBuffer));
      // MessageBoxA(0, fileNameBuffer, "write", MB_OK);
    if (IsFileEncrypted(self, file))
    {
        char* encryptedBuffer = new char[bytesToWrite];
        Encryption::Encrypt(buffer, encryptedBuffer, bytesToWrite);

        BOOL result = pfnWriteFile(self, file, encryptedBuffer, bytesToWrite, bytesWritten);
        delete[] encryptedBuffer;
        return result;
    }
    return pfnWriteFile(self, file, buffer, bytesToWrite, bytesWritten);
}

void* __fastcall MyOpenFileMgr(void* pSelf, void* /* dummy edx */, const char* szFileName, const char* szKey, int mode)
{
  
    const char* targetFiles[] = {
        "Music.pk2",
        "Particles.pk2",
        "Media.pk2",
        "Data.pk2",
        "Map.pk2"
    };

    char directoryPath[512] = { 0 };
    GetDirectoryPath(szFileName, directoryPath, sizeof(directoryPath));

    const char* fileName = strrchr(szFileName, '\\');
    fileName = fileName ? fileName + 1 : szFileName;

    mapPtrFileName.insert(std::make_pair(pSelf, fileName));

    for (int i = 0; i < sizeof(targetFiles) / sizeof(targetFiles[0]); ++i)
    {
        if (strcmp(fileName, targetFiles[i]) == 0)
        {
            strcat(directoryPath, PACK_FILE_NEW_NAME);
            szFileName = directoryPath;
            printf("SPOOF %s -> %s !!!!\n", fileName, szFileName);
            break;
        }
    }

    for (std::map<void*, std::string>::iterator it = mapPtrFileName.begin(); it != mapPtrFileName.end(); it++)
    {
        if (strcmp(fileName, it->second.c_str()) == 0)
        {
           // printf("Spoofed FM ptr !\n");
            //return it->first;
        }
    }

    void* pRes = pfnOpenFM(pSelf, szFileName, szKey, mode);
    //printf("Open FM -> %s, res = 0x%x\n", szFileName, pRes);

    return pfnOpenFM(pSelf, szFileName, szKey, mode);
}

typedef int(__thiscall* FN_PK2_CREATE)(void* pSelf, const char* szFileName, int nSize);
FN_PK2_CREATE pfnPk2Create = NULL;

int __fastcall MyPk2_Create(void* pSelf, void* /* dummy edx */, const char* szFileName, int nSize)
{
#if 1 == 0
    if (strcmp(szFileName, "SV.T") == 0)
    {
        MessageBoxA(0, "Create", "Create sv.t", MB_OK);
        int res = pfnPk2Create(pSelf, "media\\SV.T", nSize);
        printf("spoofed create() res 0x%x \n", res);
        return res;
    }
#endif

    int res = pfnPk2Create(pSelf, szFileName, nSize);
   // MessageBoxA(0, szFileName, "Create", MB_OK);

    printf("%s, file %s size %d res 0x%x\n", __FUNCTION__, szFileName, nSize, res);
    return res;
}
HMODULE __stdcall MyLoadLibraryA(const char* libraryName)
{
    HMODULE module = pfnLoadLibraryA(libraryName);

    VMProtectBeginUltra("__LOAD_LIBRARY_BODY__");

    if (strstr(libraryName, "GFXFileManager.dll"))
    {
        FN_GFXDLL_CREATE_OBJECT createObject = reinterpret_cast<FN_GFXDLL_CREATE_OBJECT>(GetProcAddress(module, "GFXDllCreateObject"));

        IFileManager** dummyManager = new IFileManager * [2];

        createObject(1, dummyManager, 0x1007);
        pfnOpenFM = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_OPEN_FM, 4);

        pfnReadFile = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_READ_FILE, 18);
        pfnWriteFile = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_WRITE_FILE, 19);
        pfnFileNameFromHandle = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_FILE_NAME_FROM_HANDLE, 34);
        pfnOpenFile_10 = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_OPEN_FILE_10, 10);
        pfnOpenFile_11 = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_OPEN_FILE_11, 11);
        pfnPk2Create = REMODEL_GET_VIRTUAL_FN_BY_IDX(*dummyManager, FN_PK2_CREATE, 15);
        DetourTransactionBegin();
        DetourAttach(&(PVOID&)pfnReadFile, MyReadFile);
        DetourAttach(&(PVOID&)pfnWriteFile, MyWriteFile);

        //single-file packing
        //DetourAttach(&(PVOID&)pfnOpenFM, MyOpenFileMgr);
        // DetourAttach(&(PVOID&)pfnOpenFile_10, MyOpenFile_10);
        //DetourAttach(&(PVOID&)pfnOpenFile_11, MyOpenFile_11);
        //DetourAttach(&(PVOID&)pfnPk2Create, MyPk2_Create);

        DetourTransactionCommit();
    }

    VMProtectEnd();

    return module;
}


void DropMediaResourceAsFile()
{
    // Get the handle to the current DLL (the one where resources are embedded)
    HMODULE hModule = GetModuleHandleA("ClientEncLib.dll");
    if (hModule == NULL) {
        printf("Failed to get module handle. Error: %d\n", GetLastError());
        return;
    }

    // Find the resource inside the DLL using the ID and RT_RCDATA
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_RES_TYPE_FOR_PK22), RT_RCDATA);
    if (hResource == NULL) {
        printf("Failed to find resource. Error: %d\n", GetLastError());
        return;
    }

    // Load the resource data
    HGLOBAL hLoadedRes = LoadResource(hModule, hResource);
    if (hLoadedRes == NULL) {
        printf("Failed to load resource. Error: %d\n", GetLastError());
        return;
    }

    // Lock the resource and get the data pointer
    LPVOID pData = LockResource(hLoadedRes);
    DWORD dwSize = SizeofResource(hModule, hResource);
    if (dwSize == 0) {
        printf("Resource is empty or not found\n");
        return;
    }

    printf("Size of resource: %d bytes\n", dwSize);

    // Get current directory
    char currentDir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, currentDir) == 0) {
        printf("Failed to get current directory. Error: %d\n", GetLastError());
        return;
    }

    // Define the path for the output file
    char filePath[MAX_PATH];
    snprintf(filePath, MAX_PATH, "%s\\Media.pk2", currentDir);

    // Open the file for writing (binary mode)
    HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to create file. Error: %d\n", GetLastError());
        return;
    }

    // Write the resource data to the file
    DWORD bytesWritten;
    BOOL bResult = WriteFile(hFile, pData, dwSize, &bytesWritten, NULL);
    if (!bResult || bytesWritten != dwSize) {
        printf("Failed to write to file. Error: %d\n", GetLastError());
    }
    else {
        printf("Resource successfully written to: %s\n", filePath);
    }

    // Clean up
    CloseHandle(hFile);
}

void DropDataResourceAsFile()
{
    // Get the handle to the current DLL (the one where resources are embedded)
    HMODULE hModule = GetModuleHandleA("ClientEncLib.dll");
    if (hModule == NULL) {
        printf("Failed to get module handle. Error: %d\n", GetLastError());
        return;
    }

    // Find the resource inside the DLL using the ID and RT_RCDATA
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_RES_TYPE_FOR_PK21), RT_RCDATA);
    if (hResource == NULL) {
        printf("Failed to find resource. Error: %d\n", GetLastError());
        return;
    }

    // Load the resource data
    HGLOBAL hLoadedRes = LoadResource(hModule, hResource);
    if (hLoadedRes == NULL) {
        printf("Failed to load resource. Error: %d\n", GetLastError());
        return;
    }

    // Lock the resource and get the data pointer
    LPVOID pData = LockResource(hLoadedRes);
    DWORD dwSize = SizeofResource(hModule, hResource);
    if (dwSize == 0) {
        printf("Resource is empty or not found\n");
        return;
    }

    printf("Size of resource: %d bytes\n", dwSize);

    // Get current directory
    char currentDir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, currentDir) == 0) {
        printf("Failed to get current directory. Error: %d\n", GetLastError());
        return;
    }

    // Define the path for the output file
    char filePath[MAX_PATH];
    snprintf(filePath, MAX_PATH, "%s\\Data.pk2", currentDir);

    // Open the file for writing (binary mode)
    HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Failed to create file. Error: %d\n", GetLastError());
        return;
    }

    // Write the resource data to the file
    DWORD bytesWritten;
    BOOL bResult = WriteFile(hFile, pData, dwSize, &bytesWritten, NULL);
    if (!bResult || bytesWritten != dwSize) {
        printf("Failed to write to file. Error: %d\n", GetLastError());
    }
    else {
        printf("Resource successfully written to: %s\n", filePath);
    }

    // Clean up
    CloseHandle(hFile);
}
int CountProcessesWithName(const char* processName)
{
    if (processName == nullptr || strlen(processName) == 0)
    {
        return -1; // Invalid process name
    }

    DWORD processes[1024], needed, count = 0;

    // Enumerate all processes
    if (!EnumProcesses(processes, sizeof(processes), &needed))
    {
        return -1; // Failed to enumerate processes
    }

    // Iterate through each process ID
    for (unsigned int i = 0; i < needed / sizeof(DWORD); i++)
    {
        DWORD pid = processes[i];

        if (pid == 0) // Skip the idle process
            continue;

        // Open the process to get its executable path
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess == NULL)
        {
            continue; // Skip if we cannot open the process
        }

        char buffer[MAX_PATH];
        // Get the executable path of the process
        if (GetModuleFileNameExA(hProcess, NULL, buffer, MAX_PATH) > 0)
        {
            // Extract just the process name from the path
            char* fileName = strrchr(buffer, '\\');
            if (fileName != nullptr)
            {
                fileName++; // Skip the backslash
            }
            else
            {
                fileName = buffer; // No backslash, use the whole string
            }

            // Compare the process name with the given name
            if (strcmp(processName, fileName) == 0)
            {
                count++;
            }
        }

        CloseHandle(hProcess);
    }

    return count;
}
void DeleteExtraMaxiFiles()
{
    if (CountProcessesWithName(SRO_CLIENT_NAME) > 1)
        return;

    // Define file paths for Media.pk2 and Data.pk2
    char currentDir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, currentDir) == 0) {
        printf("Failed to get current directory. Error: %d\n", GetLastError());
        return;
    }

    // Construct full paths for the files to delete
    char filePath1[MAX_PATH];
    snprintf(filePath1, MAX_PATH, "%s\\Media.pk2", currentDir);

    char filePath2[MAX_PATH];
    snprintf(filePath2, MAX_PATH, "%s\\Data.pk2", currentDir);

    // Delete the files
    if (!DeleteFileA(filePath1)) {
        printf("Failed to delete file: %s. Error: %d\n", filePath1, GetLastError());
    }
    else {
        printf("Successfully deleted file: %s\n", filePath1);
    }

    if (!DeleteFileA(filePath2)) {
        printf("Failed to delete file: %s. Error: %d\n", filePath2, GetLastError());
    }
    else {
        printf("Successfully deleted file: %s\n", filePath2);
    }
}

void SetFilesHidden()
{
    const char* files[] = { ".\\Media.pk2", ".\\Data.pk2" };

    for (int i = 0; i < 2; i++) {
        if (SetFileAttributesA(files[i], FILE_ATTRIBUTE_HIDDEN)) {
            printf("File %s set to hidden.\n", files[i]);
        }
        else {
            printf("Failed to hide file %s. Error: %lu\n", files[i], GetLastError());
        }
    }
}


void ExtraMediaForMaxiguard()
{
    DropMediaResourceAsFile();
    DropDataResourceAsFile();
    SetFilesHidden();
   printf("Load maxi now\n");
    LoadLibraryA("MaxiGuard.dll");
  
   // DeleteExtraMaxiFiles();
}

typedef BOOL(__stdcall* FN_ON_DEBUG_ASSERT_1)();
typedef bool(__stdcall* FN_ON_DEBUG_ASSERT_2)(int a1, int a2, char* format, ...);
typedef LONG(__stdcall* FN_ON_TOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS* pEx);
typedef void(__stdcall* FN_EXIT_PROCESS)(UINT exitCode);


FN_ON_DEBUG_ASSERT_1 pfnOnDebugAssert1;
FN_ON_DEBUG_ASSERT_2 pfnOnDebugAssert2;
FN_ON_TOP_LEVEL_EXCEPTION_FILTER pfnOnTopLvlExceptionFilter;
FN_EXIT_PROCESS pfnExitProcess;

BOOL __stdcall MyOnDebugAssert1()
{
    DeleteExtraMaxiFiles();
    //MessageBoxA(0, "Debug1", "Debug1", MB_OK);
    return pfnOnDebugAssert1();
}

bool __stdcall MyOnDebugAssert2(int a1, int a2, char* format, ...)
{
    DeleteExtraMaxiFiles();


    // Forward variadic arguments to the original function
    va_list args;
    va_start(args, format);

    bool result = pfnOnDebugAssert2(a1, a2, format, args);

    va_end(args);

   //MessageBoxA(0, "Debug2", "Debug2", MB_OK);
    return result;
}


LONG __stdcall MyOnTopLevelExFilter(struct _EXCEPTION_POINTERS* pEx)
{
    DeleteExtraMaxiFiles();
   // MessageBoxA(0, "TLEF", "TLEF", MB_OK);
    return pfnOnTopLvlExceptionFilter(pEx);
}

void __stdcall MyExitProcess(UINT nExitCode)
{
    DeleteExtraMaxiFiles();
  //  MessageBoxA(0, "ExitProcess", "ExitProcess", MB_OK);
    return pfnExitProcess(nExitCode);
}


void CLibrary::Initialize()
{
    unsigned char bytes[16] = { 0x04, 0x0D, 0x15, 0x18, 0x0F, 0x06, 0x02, 0x0A,
                             0x03, 0x17, 0x00, 0x12, 0x1E, 0x0B, 0x01, 0x00 // zero terminator haha 
    };

    char* szBlowfishKey = (char*)bytes;
    char processName[MAX_PATH];
    std::string strFileName;
    if (GetModuleFileNameA(NULL, processName, MAX_PATH))
    {
        std::string filename = processName;
        size_t pos = filename.find_last_of("\\/");
        if (pos != std::string::npos)
        {
            strFileName = filename.substr(pos + 1);
        }
    }

#ifdef BUILD_ISRO
    if (strcmp(strFileName.c_str(), SRO_CLIENT_NAME) == 0)
    {
        MEMUTIL_WRITE_VALUE(WORD, 0x00B52950, 0xC390);

        MEMUTIL_WRITE_VALUE(const char*, 0x0044675C + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00463428 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00463635 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x009391BD + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00939221 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0093927D + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x009392D9 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00939335 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x009397D9 + 1, SRO_PK2_KEY);
    }

    if (strcmp(strFileName.c_str(), SRO_LAUNCHER_NAME) == 0)
    {
        //launcher
        MEMUTIL_WRITE_VALUE(const char*, 0x00434601 + 1, SRO_CLIENT_NAME_LAUNCHER);
        MEMUTIL_WRITE_VALUE(const char*, 0x0040941C + 1, SRO_REPLACER_NAME);
        MEMUTIL_WRITE_VALUE(const char*, 0x0040A4E5 + 1, SRO_REPLACER_NAME);
        MEMUTIL_WRITE_VALUE(const char*, 0x0043487C + 1, SRO_REPLACER_NAME_WITH_FORMAT);


        MEMUTIL_WRITE_VALUE(const char*, 0x00407183 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x004073BF + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00432131 + 1, SRO_PK2_KEY);
    }
#endif

#ifdef BUILD_VSRO
    printf("FileName = %s\n", strFileName.c_str());
    if (strcmp(strFileName.c_str(), SRO_CLIENT_NAME) == 0)
    {
        printf("SRO_CLIENT DETECTED\n");
        //MessageBoxA(0, "sro_client", "sro_client", MB_OK);
        MEMUTIL_WRITE_VALUE(const char*, 0x004978B8 + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x004CCC4A + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x004CCE1F + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0083A84D + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0083A8B1 + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0083A90D + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0083A969 + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0083A9C5 + 1,SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x0083AE69 + 1,SRO_PK2_KEY);

        pfnOnDebugAssert1 = reinterpret_cast<FN_ON_DEBUG_ASSERT_1>(0x49E890);
        pfnOnDebugAssert2 = reinterpret_cast<FN_ON_DEBUG_ASSERT_2>(0x49E490);
        pfnOnTopLvlExceptionFilter = reinterpret_cast<FN_ON_TOP_LEVEL_EXCEPTION_FILTER>(0x00832110);
        pfnExitProcess = (FN_EXIT_PROCESS)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "ExitProcess");

        DetourTransactionBegin();
        DetourAttach(&(PVOID&)pfnOnDebugAssert1, MyOnDebugAssert1);
        DetourAttach(&(PVOID&)pfnOnDebugAssert2, MyOnDebugAssert2);
        DetourAttach(&(PVOID&)pfnOnTopLvlExceptionFilter, MyOnTopLevelExFilter);
        DetourAttach(&(PVOID&)pfnExitProcess, MyExitProcess);
        DetourTransactionCommit();

        MEMUTIL_WRITE_VALUE(WORD, 0x009EE3E0, 0xC390);

      

        ExtraMediaForMaxiguard();
    }

    if (strcmp(strFileName.c_str(), SRO_LAUNCHER_NAME) == 0)
    {
        if (CountProcessesWithName(SRO_CLIENT_NAME) == 0)
        {
            DeleteFileA("Media.pk2");
            DeleteFileA("Data.pk2");
        }
        MEMUTIL_WRITE_VALUE(const char*, 0x00433181 + 1, SRO_CLIENT_NAME_LAUNCHER);
        MEMUTIL_WRITE_VALUE(const char*, 0x0040ACAC + 1, SRO_REPLACER_NAME);
        MEMUTIL_WRITE_VALUE(const char*, 0x0040B811 + 1, SRO_REPLACER_NAME);
        MEMUTIL_WRITE_VALUE(const char*, 0x00433368 + 1, SRO_REPLACER_NAME_WITH_FORMAT);

        MEMUTIL_WRITE_VALUE(const char*, 0x00408A17 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00408C15 + 1, SRO_PK2_KEY);
        MEMUTIL_WRITE_VALUE(const char*, 0x00430FD3 + 1, SRO_PK2_KEY);
    }
#endif

    VMProtectBeginUltra("__INIT_LIBRARY__");

#ifdef ENABLE_DEBUG_CONSOLE
    logFile.open("file_operations.txt", std::ios::app);
    logFile << "\n=== New Session Started ===\n";
    logFile << "Date: " << __DATE__ << " Time: " << __TIME__ << "\n\n";

    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    std::cout << "======================================" << std::endl;
    std::cout << "======================================" << std::endl;
    printf("Compile time: %s | %s\n", __DATE__, __TIME__);
    std::cout << "======================================" << std::endl;
    printf("Encryption Initializing... \n");
#endif

    InitializeEncryptedFiles();
    InitializeNonEncryptedFiles();

 

    HMODULE kernel32 = GetModuleHandleA("Kernel32.dll");
    pfnLoadLibraryA = reinterpret_cast<FN_LOADLIBRARYA>(GetProcAddress(kernel32, "LoadLibraryA"));
    DetourTransactionBegin();
    DetourAttach(&(PVOID&)pfnLoadLibraryA, MyLoadLibraryA);
    DetourTransactionCommit();

    VMProtectEnd();
}