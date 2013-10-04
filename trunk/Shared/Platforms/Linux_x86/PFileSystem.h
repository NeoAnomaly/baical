//******************************************************************************
// Copyright 2011 Zheltovskiy Andrey                                           *
//                                                                             *
//   Licensed under the Apache License, Version 2.0 (the "License");           *
//   you may not use this file except in compliance with the License.          *
//   You may obtain a copy of the License at                                   *
//                                                                             *
//       http://www.apache.org/licenses/LICENSE-2.0                            *
//                                                                             *
//  Unless  required  by  applicable  law  or  agreed  to in writing, software *
//  distributed  under the License is distributed on an "AS IS" BASIS, WITHOUT *
//  WARRANTIES  OR  CONDITIONS OF ANY KIND, either express or implied. See the *
//  License  for  the  specific language governing permissions and limitations *
//  under the License.                                                         *
//                                                                             *
//******************************************************************************
// This header file provide base file system functions                         *
//******************************************************************************
#ifndef PFILESYSTEM_H_AZH
#define PFILESYSTEM_H_AZH

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


class CFSYS
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Directory_Exists
    static tBOOL Directory_Exists(tACHAR *i_pPath)
    {
        struct stat l_sStat;
        l_sStat.st_mode = 0;
        if (0 == stat(i_pPath, &l_sStat))
        {
            if (0 != (l_sStat.st_mode & S_IFDIR))
            {
                return TRUE;
            }
        }

        return FALSE;
    }//Directory_Exists


    ////////////////////////////////////////////////////////////////////////////
    //Directory_Create
    static tBOOL Directory_Create(const tACHAR *i_pDir)
    {
        if (NULL == i_pDir)
        {
            return FALSE;
        }

        char *l_pDir  = (char *)malloc(strlen(i_pDir) + 1);
        char *l_pIter = l_pDir;

        if (NULL == l_pDir)
        {
            return FALSE;
        }

        strcpy(l_pDir, i_pDir);
        while (*l_pIter)
        {
            if ('/' == (*l_pIter))
            {
                *l_pIter = 0;

                if (    (FALSE == CFSYS::Directory_Exists(l_pDir))
                     && (l_pIter != l_pDir)
                   )
                {
                    mkdir(l_pDir, S_IRWXU | S_IRWXG | S_IROTH);
                }

                *l_pIter = '/';
            }

            l_pIter++;
        }

        free(l_pDir);
        l_pDir = NULL;

        mkdir(i_pDir, S_IRWXU | S_IRWXG | S_IROTH);

        return Directory_Exists((tACHAR*)i_pDir);
    }//Directory_Create


    ////////////////////////////////////////////////////////////////////////////
    //File_Exists
    static tBOOL File_Exists(tACHAR * i_pFileName)
    {
        struct stat l_sStat;
        l_sStat.st_mode = 0;
        if (0 == stat(i_pFileName, &l_sStat))
        {
            if (0 != (l_sStat.st_mode & S_IFREG))
            {
                return TRUE;
            }
        }

        return FALSE;
    }//File_Exists


//    ////////////////////////////////////////////////////////////////////////////
//    // Enumerate_Files
//    static void Enumerate_Files(CBList<CWString*> *i_pDll_List,
//                                CWString          *i_pDirectory,
//                                wchar_t           *i_pMask, //for example L"*.dll"
//                                DWORD              i_dwDepth = 0xFFFFFF
//                               )
//    {
//        WIN32_FIND_DATAW l_sFind_Info = {0};
//        HANDLE           l_hFind      = INVALID_HANDLE_VALUE;
//        DWORD            l_dwLength   = i_pDirectory->Length();
//        CWString         l_cSearch_Path;


//        //**************************************************************************
//        // Enumerate all files in current directory
//        l_cSearch_Path.Set(i_pDirectory->Get());
//        l_cSearch_Path.Append(2, L"\\", i_pMask);

//        l_hFind = FindFirstFileW(l_cSearch_Path.Get(), &l_sFind_Info);

//        while(INVALID_HANDLE_VALUE != l_hFind)
//        {
//            if (0 == (l_sFind_Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//            {
//                CWString *i_pFile = new CWString();
//                if (i_pFile)
//                {
//                    i_pFile->Set(i_pDirectory->Get());
//                    i_pFile->Append(2, L"\\", l_sFind_Info.cFileName);
//                    i_pDll_List->Add_After(i_pDll_List->Get_Last(), i_pFile);
//                }
//            }

//            if (! FindNextFileW(l_hFind, &l_sFind_Info))
//            {
//                FindClose(l_hFind);
//                l_hFind = INVALID_HANDLE_VALUE;
//            }
//        } //while(INVALID_HANDLE_VALUE != hFind)


//        //**************************************************************************
//        // Enumerate all sub directories in current directory ....
//        if (i_dwDepth)
//        {
//            l_cSearch_Path.Set(i_pDirectory->Get());
//            l_cSearch_Path.Append(1, L"\\*.*");

//            l_hFind = FindFirstFileW(l_cSearch_Path.Get(), &l_sFind_Info);

//            while(INVALID_HANDLE_VALUE != l_hFind)
//            {
//                if (    (0 != (l_sFind_Info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
//                     && (0 != wcscmp(l_sFind_Info.cFileName, L"."))
//                     && (0 != wcscmp(l_sFind_Info.cFileName, L".."))
//                   )
//                {
//                    i_pDirectory->Append(2, L"\\", l_sFind_Info.cFileName);
//                    Enumerate_Files(i_pDll_List, i_pDirectory, i_pMask, i_dwDepth - 1);
//                    i_pDirectory->Trim(l_dwLength);
//                }

//                if (! FindNextFileW(l_hFind, &l_sFind_Info))
//                {
//                    FindClose(l_hFind);
//                    l_hFind = INVALID_HANDLE_VALUE;
//                }
//            } //while(INVALID_HANDLE_VALUE != hFind)
//        }
//    }// Enumerate_Files


//    ////////////////////////////////////////////////////////////////////////////
//    //Get_Version
//    static UINT64 Get_Version(wchar_t *i_pFile)
//    {
//        DWORD  l_dwUnknown      = 0;
//        DWORD  l_dwFileInfoSize = GetFileVersionInfoSizeW(i_pFile, &l_dwUnknown);
//        UINT64 l_qwReturn       = 0;

//        if (l_dwFileInfoSize)
//        {
//            BYTE * l_pFileInfo = new BYTE[l_dwFileInfoSize];
//            if (l_pFileInfo)
//            {
//                memset(l_pFileInfo, 0, l_dwFileInfoSize);

//                if (GetFileVersionInfoW(i_pFile, 0, l_dwFileInfoSize, l_pFileInfo) )
//                {
//                    VS_FIXEDFILEINFO * l_tVersion = NULL;
//                    UINT l_dwSize = 0;
//                    if (VerQueryValueW(l_pFileInfo, L"\\", (LPVOID *)&l_tVersion, &l_dwSize))
//                    {
//                        l_qwReturn = (((UINT64)l_tVersion->dwProductVersionMS) << 32) +
//                                    l_tVersion->dwProductVersionLS;
//                    }
//                }

//                if (l_pFileInfo)
//                {
//                    delete [ ] l_pFileInfo;
//                    l_pFileInfo = NULL;
//                }
//            } //if (l_pFileInfo)
//        } //if (l_dwFileInfoSize)

//        return l_qwReturn;
//    }//Get_Version


//    ////////////////////////////////////////////////////////////////////////////
//    //Get_Process_Directory
//    static BOOL Get_Process_Directory(CWString *o_pDirectory)
//    {
//        BOOL  l_bReturn  = FALSE;
//        DWORD l_dwLength = 4096;

//        if (    (NULL == o_pDirectory)
//             || (FALSE == o_pDirectory->Realloc(l_dwLength))
//           )
//        {
//            return l_bReturn;
//        }

//        if (0 != GetModuleFileNameW(GetModuleHandleW(NULL),
//                                    o_pDirectory->Get(),
//                                    o_pDirectory->Max_Length())
//           )
//        {
//            wchar_t *l_pExe_Name = NULL;

//            if (    (l_pExe_Name = wcsrchr(o_pDirectory->Get(), L'\\'))
//                 || (l_pExe_Name = wcsrchr(o_pDirectory->Get(), L'/'))
//               )
//            {
//                o_pDirectory->Trim((DWORD)(l_pExe_Name - o_pDirectory->Get()));
//                l_bReturn = TRUE;
//            }
//        }

//        return l_bReturn;
//    }//Get_Process_Directory

//    ////////////////////////////////////////////////////////////////////////////
//    //Delete_File
//    static BOOL Delete_File(wchar_t *i_pFile_Name)
//    {
//        if (NULL == i_pFile_Name)
//        {
//            return FALSE;
//        }

//        DWORD l_dwFAttr = GetFileAttributesW(i_pFile_Name);
//        l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_ARCHIVE);
//        l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_READONLY);
//        l_dwFAttr = l_dwFAttr & (~FILE_ATTRIBUTE_SYSTEM);
//        SetFileAttributesW(i_pFile_Name, l_dwFAttr);
//        return DeleteFileW(i_pFile_Name);
//    }//Delete_File
};


#endif //PFILESYSTEM_H_AZH
