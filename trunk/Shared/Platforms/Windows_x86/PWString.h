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
// This header file provide simple string functionality                        *
//******************************************************************************
#pragma once

class CWString
{
    wchar_t *m_pBuffer;
    DWORD    m_dwBuffer_Length;
public:
    CWString():
        m_pBuffer(NULL)
       ,m_dwBuffer_Length(0)
    {
    }

    CWString(wchar_t *i_pValue):
        m_pBuffer(NULL)
       ,m_dwBuffer_Length(0)
    {
        Set(i_pValue);
    }

    ~CWString()
    {
        Remove();
    }

    BOOL Realloc(DWORD i_dwLength)
    {
        BOOL l_bResult = TRUE;

        if ( i_dwLength <= m_dwBuffer_Length )
        {
            return l_bResult;
        }

        wchar_t *l_pBuffer = new wchar_t[i_dwLength];
        if (l_pBuffer)
        {
            if (m_pBuffer)
            {
                wcscpy_s(l_pBuffer, m_dwBuffer_Length, m_pBuffer);
                Remove();
            }
            else
            {
                l_pBuffer[0] = 0;
            }

            m_dwBuffer_Length = i_dwLength;
            m_pBuffer         = l_pBuffer;
        }
        else
        {
            l_bResult = FALSE;
        }

        return l_bResult;
    }

    BOOL Set(wchar_t *i_pValue)
    {
        BOOL l_bResult = TRUE;

        if (NULL != i_pValue)
        {
            DWORD l_dwInput_Value_Length = (DWORD)wcslen(i_pValue);
            if ( l_dwInput_Value_Length >= m_dwBuffer_Length )
            {
                Remove();
                m_dwBuffer_Length = l_dwInput_Value_Length + 1;
                m_pBuffer = new wchar_t[m_dwBuffer_Length];
            }

            if (m_pBuffer)
            {
                wcscpy_s(m_pBuffer, m_dwBuffer_Length, i_pValue);
            }
            else
            {
                l_bResult = FALSE;
                Remove();
            }
        }
        else
        {
            Remove();
        }

        return l_bResult;
    }

    // Example Append(3, L"Text1", L"Text2", L"Text3")
    BOOL Append(DWORD i_dwCount, ...)
    {
        BOOL         l_bResult         = TRUE;
        DWORD        l_dwAppend_Length = 0;
        DWORD        l_dwString_Length = m_pBuffer ? (DWORD)wcslen(m_pBuffer) : 0;


        //Parameters verification
        va_list      l_pVar_Args       = NULL;
        wchar_t     *l_pItem           = NULL;
        DWORD        l_dwIDX           = 0;

        if (l_bResult)
        {
            va_start(l_pVar_Args, i_dwCount);

            while (l_dwIDX < i_dwCount)
            {
                l_pItem = va_arg(l_pVar_Args, wchar_t*);
                if (l_pItem)
                {
                    l_dwAppend_Length += (DWORD)wcslen(l_pItem);
                }

                l_dwIDX ++;
            }

            va_end(l_pVar_Args);
        }

        if ( (l_bResult) && ( (l_dwAppend_Length + l_dwString_Length) >= m_dwBuffer_Length ) )
        {
            m_dwBuffer_Length = l_dwAppend_Length + l_dwString_Length + 1;
            wchar_t *l_pTMP_Buffer = new wchar_t[m_dwBuffer_Length];

            if ( (l_pTMP_Buffer) && (m_pBuffer) && (l_dwString_Length) )
            {
                wcscpy_s(l_pTMP_Buffer, m_dwBuffer_Length, m_pBuffer);
            }

            if (m_pBuffer)
            {
                delete [] m_pBuffer;
            }

            m_pBuffer = l_pTMP_Buffer;

            if (NULL == m_pBuffer)
            {
                Remove();
                l_bResult = FALSE;
            }
            else if (0 >= l_dwString_Length)
            {
                m_pBuffer[0] = 0;
            }
        }

        if (l_bResult)
        {
            l_pVar_Args = NULL;
            l_pItem     = NULL;
            l_dwIDX     = 0;

            va_start(l_pVar_Args, i_dwCount);

            while (l_dwIDX < i_dwCount)
            {
                l_pItem = va_arg(l_pVar_Args, wchar_t*);
                if (l_pItem)
                {
                    wcscpy_s(m_pBuffer + l_dwString_Length, m_dwBuffer_Length - l_dwString_Length, l_pItem);
                    l_dwString_Length += (DWORD)wcslen(l_pItem);
                }

                l_dwIDX ++;
            }

            va_end(l_pVar_Args);
        }

        return l_bResult;
    }

    wchar_t * Get()
    { 
        return m_pBuffer; 
    }


    DWORD Length() 
    { 
        return m_pBuffer ? (DWORD)wcslen(m_pBuffer) : 0; 
    }

    DWORD Max_Length() 
    { 
        return m_dwBuffer_Length; 
    }

    void Trim(DWORD i_dwLenght)
    {
        DWORD l_dwString_Length = m_pBuffer ? (DWORD)wcslen(m_pBuffer) : 0;

        if (i_dwLenght >= l_dwString_Length)
        {
            return;
        }

        m_pBuffer[i_dwLenght] = 0;
    }

    int Find(wchar_t *i_pSub, BOOL i_bCase_Sens = FALSE)
    {
        if (    (NULL == i_pSub)
             || (NULL == m_pBuffer)
           )
        {
            return -1;
        }

        size_t l_dwSub_Len = wcslen(i_pSub);
        size_t l_dwStr_Len = wcslen(m_pBuffer);

        if (FALSE == i_bCase_Sens)
        {
            for (size_t l_dwI = 0; l_dwI <= (l_dwStr_Len - l_dwSub_Len); l_dwI++)
            {
                if (0 == _wcsnicmp(m_pBuffer + l_dwI, i_pSub, l_dwSub_Len))
                {
                    return (int)l_dwI;
                }
            }
        }
        else
        {
            for (size_t l_dwI = 0; l_dwI <= (l_dwStr_Len - l_dwSub_Len); l_dwI++)
            {
                if (0 == wcsncmp(m_pBuffer + l_dwI, i_pSub, l_dwSub_Len))
                {
                    return (int)l_dwI;
                }
            }
        }

        return -1;
    }

private:
    void Remove()
    {
        if (m_pBuffer)
        {
            delete [] m_pBuffer;
            m_pBuffer = NULL;           
        }

        m_dwBuffer_Length = 0;
    }

};
