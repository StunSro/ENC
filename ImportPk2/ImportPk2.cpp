// ImportPk2.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "ImportPk2.h"
#include <vector>
#include <string>
#include <list>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

#include "common/PK2Writer.h"
#include "EncKeyDef.h"

#define SRO_TARGET_PK2_FILE_NAME "Solus.Game"
#define SRO_PK2_KEY              "169841"
#define SRO_REPLACER_FILENAME    "SolusReplacer.exe"
#define SRO_LAUNCHER_EXE_NAME    "SolusLauncher.exe"


typedef std::list<const char*> FileList;
//fn4
FileList excludedFileNames;
FileList includedTxtFileNames;
FileList excludedFolderNames;


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

void GetFilesInFolder(const std::string& directory, const std::string& relativePath, std::vector<std::string>& files) 
{
    WIN32_FIND_DATAA findFileData;

    HANDLE hFind = FindFirstFileA((directory + "\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) 
        return;

    do 
    {
        const std::string fileOrDirName = findFileData.cFileName;
        if (fileOrDirName == "." || fileOrDirName == "..") {
            continue; 
        }

        std::string fullPath = directory + "\\" + fileOrDirName;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
            GetFilesInFolder(fullPath, relativePath + "\\" + fileOrDirName, files);
        }
        else 
        {
            files.push_back(relativePath + "\\" + fileOrDirName);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
}




bool IsFileEncrypted(const char* szFileName)
{
  //  MessageBoxA(0, szFileName, "SilkEncLib IsFileEncrypted path", MB_OK);
    std::string fullPath(szFileName);
    // printf("path = %s\n", fullPath.c_str());
#ifndef ENABLE_ENCRYPTION
    return false;
#endif

    for (FileList::iterator it = excludedFolderNames.begin(); it != excludedFolderNames.end(); ++it)
    {
        if (fullPath.rfind(*it, 0) == 0)
        {
            return false;
        }
    }

    for (FileList::iterator it = excludedFileNames.begin(); it != excludedFileNames.end(); ++it)
    {
        if (strcmp(*it, fullPath.c_str()) == 0)
        {
            return false;
        }
    }

    const char* extension = PathFindExtensionA(fullPath.c_str());
    if (extension && (strcmp(extension, ".txt") == 0 || strcmp(extension, ".TXT") == 0))
    {
        for (FileList::iterator it = includedTxtFileNames.begin(); it != includedTxtFileNames.end(); ++it)
        {
            if (strcmp(*it, fullPath.c_str()) == 0)
            {
                return true;
            }
        }
        return false;
    }

    return true;
}

void DeleteDirectoryRecursive(const std::string& dir)
{
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((dir + "\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do {
        const std::string fileOrDir = findFileData.cFileName;

        if (fileOrDir == "." || fileOrDir == "..")
            continue;

        const std::string fullPath = dir + "\\" + fileOrDir;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            DeleteDirectoryRecursive(fullPath);
            RemoveDirectoryA(fullPath.c_str());
        }
        else
        {
            DeleteFileA(fullPath.c_str());
        }

    } while (FindNextFileA(hFind, &findFileData));

    FindClose(hFind);
}

void ImportPackFileFromFolder(std::string srcFolder, std::string packFileName, std::string key)
{
    PK2Writer writer;

    bool bInitialized = writer.Initialize("GFXFileManager.dll");
    if (!bInitialized)
        return;

    bool bOpen = writer.Open(SRO_TARGET_PK2_FILE_NAME, (void*)SRO_PK2_KEY, (unsigned char)strlen(SRO_PK2_KEY));
    if (!bOpen)
        return;

    std::vector<std::string> files;
    GetFilesInFolder(srcFolder.c_str(), "", files);
   
    for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); it++)
    {
       // MessageBoxA(0, it->c_str(), 0, MB_OK);

        std::string sourceFile = srcFolder + "\\" + *it;
        std::string targetFile = srcFolder + "\\" + *it;

        //===========================================================
        //read tmp
        FILE* fp = nullptr;
        errno_t err = fopen_s(&fp, sourceFile.c_str(), "rb");
        if (err != 0 || !fp) {
            // handle error
        }

        fseek(fp, 0, SEEK_END);
        long len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char* pBuffer = new char[len];
        memset(pBuffer, 0x00, len);
        fread(pBuffer, len, 1, fp);
        fclose(fp);

        //===========================================================

        if (IsFileEncrypted(sourceFile.c_str()))
        {
            char* pEncryptedBuf = new char[len];
            Encrypt(pBuffer, pEncryptedBuf, len);

            //===========================================================
            //write encrypted dummy file
            errno_t err = fopen_s(&fp, "dummy.bin", "wb");
            if (err != 0 || !fp) {
                // معالجة الخطأ
            }
            fwrite(pEncryptedBuf, len, 1, fp);
            fflush(fp);
            fclose(fp);
            //===========================================================

            writer.ImportFile(targetFile.c_str(), "dummy.bin");
            DeleteFileA("dummy.bin");
        }
        else
        {
            //not encrypted
            writer.ImportFile(targetFile.c_str(), sourceFile.c_str());
        }

        //MessageBoxA(0, targetFile.c_str(), NULL, MB_OK);

        //===========================================================

        DeleteFileA(sourceFile.c_str());
    }


    writer.Close();

    DeleteDirectoryRecursive(srcFolder);
    RemoveDirectoryA(srcFolder.c_str()); //ensure parent shit
}

void RegisterEncFileLists()
{
    //=========================================================================================
    includedTxtFileNames.push_back("server_dep\\silkroad\\textdata\\gameworlddata.txt");
    includedTxtFileNames.push_back("server_dep\\silkroad\\textdata\\gameworldconfigdata.txt");
    includedTxtFileNames.push_back("server_dep\\silkroad\\textdata\\refqusetreward.txt");
    includedTxtFileNames.push_back("server_dep\\silkroad\\textdata\\refquestrewarditems.txt");
    includedTxtFileNames.push_back("server_dep\\silkroad\\textdata\\skilleffect.txt");
    //=========================================================================================
    /*
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textdataname.txt");

    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_5000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_10000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_15000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_20000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_25000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_30000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_35000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_40000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_45000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\characterdata_50000.txt");
    */
    //=========================================================================================

    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldataenc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_5000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_10000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_15000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_20000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_25000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_30000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_35000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_40000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_45000enc.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_50000enc.txt");

    //=========================================================================================

    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_5000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_10000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_15000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_20000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_25000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_30000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_35000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_40000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_45000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\itemdata_50000.txt");

    //=========================================================================================

    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\magicoption.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textuisystem.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refshopgroup.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refmappingshopgroup.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refmappingshopwithtab.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refshoptab.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refscrapofpackageitem.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\npcpos.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\leveldata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\teleportbuilding.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\teleportlink.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\teleportdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refskillbyitemoptleveldata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refeventrewarditems.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refshopgoods.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refabilitybyitemoptleveldata.txt");

    //=========================================================================================

    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_5000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_10000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_15000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_20000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_25000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_30000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_35000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_40000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_45000.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_50000.txt");

    //=========================================================================================

    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refextraabilitybyequipitemoptlevel.txt");

    //[MBot, only names missing from list above]
    excludedFileNames.push_back("DIVISIONINFO.TXT");
    excludedFileNames.push_back("divisioninfo.txt");
    excludedFileNames.push_back("GATEPORT.TXT");
    excludedFileNames.push_back("gateport.txt");
    excludedFileNames.push_back("SV.T");
    excludedFileNames.push_back("sv.t");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refsetitemgroup.txt");

    //=========================================================================================

    //[phBot, not yet complete list (coz data & other stuff)]
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\gamedataconfig.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\tradeconflict_joblevel.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textdata_object.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textdata_equip&skill.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textquest.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textquest_otherstring.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textquest_speech&name.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textquest_queststring.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\questcontentsdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\questdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refquestreward.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refquestrefwarditems.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\worldmapguidedata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\quest.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textevent.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refoptionalteleport.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refeventrewarditems.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\shopdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\stoptabdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\shopitemdata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\textzonename.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\regioninfo.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\refregion.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\gameworlddatastabil.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skillmasterydata.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skillgroup.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skilldata_r.txt");
    excludedFileNames.push_back("server_dep\\silkroad\\textdata\\skillgroup.txt");
    //=========================================================================================
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{


#if 1 == 0
    //dir
    char currentDir[MAX_PATH];
    if (!GetCurrentDirectoryA(MAX_PATH, currentDir)) {
        return 1;
    }

    std::string workingDir(currentDir);
    size_t pos2 = workingDir.find_last_of("\\/");
    if (pos2 != std::string::npos) {
        workingDir = workingDir.substr(0, pos2);
    }
    else {
        return 1;
    }

    
    //SetCurrentDirectoryA(workingDir.c_str());
#endif

    //MessageBoxW(0, lpCmdLine, lpCmdLine, MB_OK);
    RegisterEncFileLists();

    ImportPackFileFromFolder("Particles", SRO_TARGET_PK2_FILE_NAME, SRO_PK2_KEY);
    ImportPackFileFromFolder("Music", SRO_TARGET_PK2_FILE_NAME, SRO_PK2_KEY);
    ImportPackFileFromFolder("Data", SRO_TARGET_PK2_FILE_NAME, SRO_PK2_KEY);
    ImportPackFileFromFolder("Map", SRO_TARGET_PK2_FILE_NAME, SRO_PK2_KEY);
    ImportPackFileFromFolder("Media", SRO_TARGET_PK2_FILE_NAME, SRO_PK2_KEY);

    //Now run the actual replacer
    std::wstring strCmdLineW(lpCmdLine);
    std::string strCmdLineA(strCmdLineW.begin(), strCmdLineW.end());

    std::string strOldExeName = "Silkroad.exe";
    size_t pos = strCmdLineA.find(strOldExeName.c_str());

    if (pos != std::string::npos)
        strCmdLineA.replace(pos, strOldExeName.length(), SRO_LAUNCHER_EXE_NAME);

    char szCmd[256] = { 0 };
    sprintf_s(szCmd, sizeof(szCmd), "%s %s", SRO_REPLACER_FILENAME, strCmdLineA.c_str());



    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi = { 0 };
   

    char fullPath[MAX_PATH];
    GetFullPathNameA(SRO_REPLACER_FILENAME, MAX_PATH, fullPath, NULL);


    bool bSuccess = CreateProcessA(
        fullPath, szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

   // DWORD err = GetLastError();
    //char szBuf[6] = { 0 };

   // MessageBoxA(0, bSuccess ? "proc created" : "failed to create proc", fullPath, MB_OK);
    
    //if(!bSuccess) msgbox (cannot start replacer)
    //we dont need to wait fo rit to exit, so .. 
    return 0;
}