//******************************************************************************
// Copyright 2013  Zheltovskiy Andrey                                          *
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
#ifndef PFILE_H_AZH
#define PFILE_H_AZH


//UTF-8 text file header - {0xEF, 0xBB, 0xBF}

////////////////////////////////////////////////////////////////////////////////
class CPFile
    : public IFile //virtual
{
    volatile tINT32    m_iRCnt;
    HANDLE             m_hFile;
public:
    ////////////////////////////////////////////////////////////////////////////
    CPFile()
        : m_iRCnt(1)
        , m_hFile(NULL)
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    ~CPFile()
    {
        Close(FALSE);
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Open(const tXCHAR *i_pName, tUINT32 i_dwFlags)
    {
        DWORD l_dwDesiredAccess = GENERIC_READ;

        if (ESHARE_WRITE & i_dwFlags)
        {
            l_dwDesiredAccess |= GENERIC_WRITE;
        }

        Close(TRUE);

        m_hFile = CreateFileW(i_pName, 
                              l_dwDesiredAccess, 
                              FILE_SHARE_READ, 
                              NULL, 
                              (i_dwFlags & EOPEN) ? OPEN_EXISTING : CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL
                             );

        if (    (NULL == m_hFile)
             || (INVALID_HANDLE_VALUE == m_hFile)
           )
        {
            m_hFile = NULL;
        }

        return (NULL != m_hFile);
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Close(tBOOL i_bFlush)
    {
        tBOOL  l_bReturn = TRUE;
        HANDLE l_hTemp   = NULL;

        if (NULL == m_hFile)
        {
            return TRUE;
        }

        if (i_bFlush)
        {
            if (FALSE == FlushFileBuffers(m_hFile))
            {
                l_bReturn = FALSE;
                goto l_lExit;
            }
        }

        l_hTemp = m_hFile;
        m_hFile = NULL;

        if (FALSE == CloseHandle(l_hTemp))
        {
            l_bReturn = FALSE;
            goto l_lExit;
        }

    l_lExit:
        return l_bReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    tBOOL Set_Position(tUINT64 i_qwOffset)
    {
        if (NULL == m_hFile)
        {
            return FALSE;
        }

        LONG l_lOffsetHi = (LONG)(i_qwOffset >> 32);

        if (INVALID_SET_FILE_POINTER == SetFilePointer(m_hFile, 
                                                       (LONG)i_qwOffset,  
                                                       &l_lOffsetHi,
                                                       FILE_BEGIN
                                                      )
           )
        {
            return FALSE;
        }

        return TRUE;
    }

    ////////////////////////////////////////////////////////////////////////////
    tUINT64 Get_Position()
    {
        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            return 0ULL;
        }

        if (NULL == m_hFile)
        {
            return FALSE;
        }

        LONG  l_lOffsetHi   = 0;
        DWORD l_dwOffsetLow = SetFilePointer(m_hFile, 
                                             0,  
                                             &l_lOffsetHi,
                                             FILE_CURRENT
                                            );

        if (    (INVALID_SET_FILE_POINTER != l_dwOffsetLow)
             || (NO_ERROR == GetLastError())
           )
        {
            return (((tUINT64)l_lOffsetHi) << 32) + (tUINT64)l_dwOffsetLow;
        }

        return 0ULL;
    }

    ////////////////////////////////////////////////////////////////////////////
    tUINT64 Get_Size()
    {
        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            return 0ULL;
        }

        LARGE_INTEGER l_qwSize;

        if (FALSE ==  GetFileSizeEx(m_hFile, &l_qwSize))
        {
            l_qwSize.QuadPart = 0LL;
        }

        return (tUINT64)l_qwSize.QuadPart;
    }

    ////////////////////////////////////////////////////////////////////////////
    size_t Write(const tUINT8 *i_pBuffer, size_t i_szBuffer, tBOOL i_bFlush)
    {
        if (    (INVALID_HANDLE_VALUE == m_hFile)
             || (NULL == i_pBuffer)
             || (0 == i_szBuffer)
           )
        {
            return 0ULL;
        }

        size_t  l_szReturn   = 0;
        DWORD   l_dwWritten = 0;
        int     l_iError    = 0;

        while (l_szReturn < i_szBuffer)
        {
            if (WriteFile(m_hFile, 
                          i_pBuffer + l_szReturn, 
                          i_szBuffer - l_szReturn, 
                          &l_dwWritten, 
                          NULL
                         )
               )
            {
                l_szReturn += (size_t)l_dwWritten;
            }
            else
            {
                break;
            }
        }

        if (    (i_bFlush)
             && (l_szReturn)
           )
        {
            FlushFileBuffers(m_hFile);
        }

        return l_szReturn;
    }

    ////////////////////////////////////////////////////////////////////////////
    size_t Read(tUINT8 *o_pBuffer, size_t i_szBuffer)
    {
        if (    (INVALID_HANDLE_VALUE == m_hFile)
             || (NULL == o_pBuffer)
             || (0 == i_szBuffer)
           )
        {
            return 0ULL;
        }

        size_t l_szReturn = 0;
        DWORD  l_dwRead  = 0;

        while (l_szReturn < i_szBuffer)
        {
            if (ReadFile(m_hFile, 
                         o_pBuffer + l_szReturn, 
                         i_szBuffer - l_szReturn,
                         &l_dwRead,
                         NULL
                        )
               )
            {
                l_szReturn += (size_t)l_dwRead;
            }
            else
            {
                break;
            }

            if (0 >= l_dwRead)
            {
                break;
            }
        }

        return l_szReturn;
    }


    ////////////////////////////////////////////////////////////////////////////
    tINT32  Add_Ref()
    {
        return ATOMIC_INC(&m_iRCnt);
    }


    ////////////////////////////////////////////////////////////////////////////
    tINT32  Release()
    {
        volatile tINT32 l_iReturn = ATOMIC_DEC(&m_iRCnt);

        if (0 >= l_iReturn)
        {
            delete this;
        }

        return l_iReturn;
    }
};

#endif //PFILE_H_AZH



/*
    CPFile       l_cFile;
    const tUINT8 l_pHeader[] = {0xEF, 0xBB, 0xBF};
    const char   l_pText[]   = "Some Text For me !\xD\xA Another Text\xD\xA Third Text";
    tUINT64      l_qwCount   = 0;
    tUINT8       l_pBuffer[256] = {0};

    l_cFile.Open("./ABC.txt", IFile::ECREATE | IFile::ESHARE_WRITE);

    l_qwCount = l_cFile.Write(l_pHeader, LENGTH(l_pHeader), FALSE);
    l_qwCount = l_cFile.Write((tUINT8*)l_pText, LENGTH(l_pText) - 1, TRUE);

    l_qwCount = l_cFile.Get_Size();

    l_cFile.Set_Position(0);

    l_qwCount = l_cFile.Read(l_pBuffer, LENGTH(l_pBuffer));

    l_cFile.Close(TRUE);
*/
