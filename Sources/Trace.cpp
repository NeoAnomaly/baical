////////////////////////////////////////////////////////////////////////////////
// Copyright 2011 Zheltovskiy Andrey                                           /
//                                                                             /
//   Licensed under the Apache License, Version 2.0 (the "License");           /
//   you may not use this file except in compliance with the License.          /
//   You may obtain a copy of the License at                                   /
//                                                                             /
//       http://www.apache.org/licenses/LICENSE-2.0                            /
//                                                                             /
//  Unless  required  by  applicable  law  or  agreed  to in writing, software /
//  distributed  under the License is distributed on an "AS IS" BASIS, WITHOUT /
//  WARRANTIES  OR  CONDITIONS OF ANY KIND, either express or implied. See the /
//  License  for  the  specific language governing permissions and limitations /
//  under the License.                                                         /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////
// This file provide trace functionality                                       /
////////////////////////////////////////////////////////////////////////////////
//Printf Type Field Characters:                                                /
// http://msdn.microsoft.com/en-us/library/hf4y5e3w.aspx                       /
//Printf Type Field Characters:                                                /
// http://msdn.microsoft.com/en-us/library/hf4y5e3w(v=vs.100).aspx             /
//Size and Distance Specification                                              /
// http://msdn.microsoft.com/en-us/library/tcxf1dw6(v=vs.100).aspx             /
////////////////////////////////////////////////////////////////////////////////
#include "CommonClient.h"
#include "P7_Trace.h"
#include "P7_Extensions.h"
#include "Trace.h"

#define RESET_UNDEFINED                                           (0xFFFFFFFFUL)
#define RESET_FLAG_CHANNEL                                        (0x1)
#define RESET_FLAG_TRACE                                          (0x2)


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Client
IP7_Trace * __stdcall P7_Create_Trace(IP7_Client   *i_pClient, 
                                      const tXCHAR *i_pName
                                     )
{
    CP7Trace *l_pReturn = new CP7Trace(i_pClient, i_pName);

    //if not initialized - remove
    if (    (l_pReturn)
         &&  (TRUE != l_pReturn->Is_Initialized())
       )
    {
        l_pReturn->Release();
        l_pReturn = NULL;
    }

    return static_cast<IP7_Trace *>(l_pReturn);
}//P7_Create_Client


////////////////////////////////////////////////////////////////////////////////
//P7_Get_Shared_Trace
IP7_Trace * __stdcall P7_Get_Shared_Trace(const tXCHAR *i_pName)
{
    IP7_Trace *l_pReturn = NULL;

    if (Shared_Read(i_pName, (tUINT8*)&l_pReturn, sizeof(IP7_Trace*)))
    {
        if (l_pReturn)
        {
            l_pReturn->Add_Ref();
        }
    }
    else
    {
        l_pReturn = NULL;
    }

   return l_pReturn;
}//P7_Get_Shared_Trace


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
enum ePrefix_Type
{
    EPREFIX_TYPE_I64   = 0,
    EPREFIX_TYPE_I32      ,
    EPREFIX_TYPE_LL       ,
    EPREFIX_TYPE_L        ,
    EPREFIX_TYPE_H        ,
    EPREFIX_TYPE_I        ,
    EPREFIX_TYPE_W        ,
    EPREFIX_TYPE_UNKNOWN  ,
};

struct sPrefix_Desc
{
    const tXCHAR  *pPrefix;
    tUINT32        dwLen;
    ePrefix_Type   eType;
};



//the order of strings IS VERY important, because we will search for first match 
static const sPrefix_Desc g_pPrefixes[] = { {TM("I64"), 3, EPREFIX_TYPE_I64    }, 
                                            {TM("I32"), 3, EPREFIX_TYPE_I32    },
                                            {TM("ll"),  2, EPREFIX_TYPE_LL     },
                                            {TM("l"),   1, EPREFIX_TYPE_L      },
                                            {TM("h"),   1, EPREFIX_TYPE_H      },
                                            {TM("I"),   1, EPREFIX_TYPE_I      },
                                            {TM("w"),   1, EPREFIX_TYPE_W      },
                                            //{TM(""),    0, EPREFIX_TYPE_UNKNOWN}
                                          };

////////////////////////////////////////////////////////////////////////////////
// Get_Prefix                                       
const sPrefix_Desc *Get_Prefix(const tXCHAR *i_pFormat)
{
    const sPrefix_Desc *l_pPrefix = &g_pPrefixes[0];
    const sPrefix_Desc *l_pReturn = NULL;

    while (l_pPrefix->dwLen)
    {
        if (0 == PStrNCmp(i_pFormat, l_pPrefix->pPrefix, l_pPrefix->dwLen))
        {
            l_pReturn = l_pPrefix;
            break;
        }

        l_pPrefix ++;
    }

    return l_pReturn;
}// Get_Prefix



////////////////////////////////////////////////////////////////////////////////
// CP7Trace_Item                                       
CP7Trace_Desc::CP7Trace_Desc(tUINT16        i_wID,
                             tUINT16        i_wLine, 
                             tUINT16        i_wModuleID,
                             const char    *i_pFile,
                             const char    *i_pFunction,
                             const tXCHAR  *i_pFormat
                            )
    : m_wID(i_wID)
    , m_dwResets(RESET_UNDEFINED)
    , m_dwSize(0)
    , m_pBuffer(NULL)
    , m_pArgs(NULL)
    , m_dwArgs_Len(0)
    , m_dwArgs_Max(0)

    , m_pBlocks(NULL)
    , m_dwBlocks_Count(0)
   
    , m_bInitialized(TRUE)
{
     tUINT32 l_dwFile_Size = 0;
     tUINT32 l_dwFunc_Size = 0;
     tUINT32 l_dwForm_Size = 0;
     
     m_bInitialized = (NULL != i_pFormat);

     m_pKey[0] = (tKeyType)i_pFunction;
     m_pKey[1] = (tKeyType)i_pFormat;

     if (m_bInitialized)
     {
         ePrefix_Type  l_ePrefix   = EPREFIX_TYPE_UNKNOWN;
         const tXCHAR *l_pIterator = i_pFormat;
         tBOOL         l_bPercent  = FALSE;

         while (    (*l_pIterator)
                 && (m_bInitialized)
               )
         {
             if (FALSE == l_bPercent)
             {
                 if (TM('%') == (*l_pIterator))
                 {
                     //we can get "%%" in this case we should ignore "%"
                     l_bPercent = ! l_bPercent;//TRUE;
                     l_ePrefix  = EPREFIX_TYPE_UNKNOWN;
                 }
             }
             else
             {
                 switch (*l_pIterator)
                 {
                     case TM('I'):
                     case TM('l'):
                     case TM('h'):
                     case TM('w'):
                     {
                         const sPrefix_Desc *l_pPrefix = Get_Prefix(l_pIterator);
                         if (l_pPrefix)
                         {
                             l_ePrefix = l_pPrefix->eType;

                             if (1 < l_pPrefix->dwLen)
                             {
                                 l_pIterator += (l_pPrefix->dwLen - 1);
                             }
                         }
                         break;
                     }
                     case TM('c'):
                     case TM('C'):
                     {

                         if (EPREFIX_TYPE_H == l_ePrefix)
                         {
                             //1 bytes character
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_CHAR, 
                                                           SIZE_OF_ARG(char)
                                                          );
                         }
                         else if (EPREFIX_TYPE_L == l_ePrefix)
                         {
                             //2 byte character
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_WCHAR, 
                                                           SIZE_OF_ARG(tXCHAR)
                                                          );
                         }
                         else if (TM('c') == (*l_pIterator))
                         {
                             //2 bytes character
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_WCHAR, 
                                                           SIZE_OF_ARG(tXCHAR)
                                                          );
                         }
                         else
                         {
                             //1 bytes character
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_CHAR, 
                                                           SIZE_OF_ARG(char)
                                                          );
                         }

                         l_bPercent = FALSE;
                         break;
                     }
                     case TM('d'):
                     case TM('i'):
                     case TM('o'):
                     case TM('u'):
                     case TM('x'):
                     case TM('X'):
                     {
                         if (EPREFIX_TYPE_H == l_ePrefix)
                         {
                             //2 bytes integer
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_INT16, 
                                                           SIZE_OF_ARG(tUINT16)
                                                          );
                         }
                         else if (    (EPREFIX_TYPE_L   == l_ePrefix)
                                   || (EPREFIX_TYPE_I32 == l_ePrefix)
                                 )
                         {
                             //4 bytes integer
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_INT32, 
                                                           SIZE_OF_ARG(tUINT32)
                                                          );
                         }
                         else if (    (EPREFIX_TYPE_LL  == l_ePrefix)
                                   || (EPREFIX_TYPE_I64 == l_ePrefix)
                                 )
                         {
                             //8 bytes integer
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_INT64, 
                                                           SIZE_OF_ARG(tUINT64)
                                                          );
                         }
                         else if (EPREFIX_TYPE_I  == l_ePrefix)
                         {
                             #ifdef _WIN64
                                 m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_INT64, 
                                                               SIZE_OF_ARG(UINT64)
                                                              );
                             #else
                                 m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_INT32, 
                                                               SIZE_OF_ARG(tUINT32)
                                                              );
                             #endif
                         }
                         else //by default
                         {
                             //4 bytes integer
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_INT32, 
                                                           SIZE_OF_ARG(tUINT32)
                                                          );
                         }

                         l_bPercent = FALSE;
                         break;
                     }
                     case TM('e'):
                     case TM('E'):
                     case TM('f'):
                     case TM('g'):
                     case TM('G'):
                     case TM('a'):
                     case TM('A'):
                     {
                         //8 bytes double
                         m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_DOUBLE, 
                                                       SIZE_OF_ARG(tDOUBLE)
                                                      );
                         l_bPercent = FALSE;
                         break;
                     }
                     case TM('n'):
                     case TM('p'):
                     {
                         m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_PVOID, 
                                                       SIZE_OF_ARG(void*)
                                                      );
                         l_bPercent = FALSE;
                         break;
                     }
                     case TM('s'):
                     case TM('S'):
                     {
                         if (EPREFIX_TYPE_H == l_ePrefix)
                         {
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_ASTR, 0); //SIZE_OF_ARG(char*)    
                         }
                         else if (    (EPREFIX_TYPE_L == l_ePrefix)
                                   || (TM('s') == (*l_pIterator))
                                 )
                         {
#ifdef UTF8_ENCODING
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_USTR, 0); //SIZE_OF_ARG(char*)     
#else
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_WSTR, 0); //SIZE_OF_ARG(wchar_t*)     
#endif                             
                         }
                         else if (TM('S') == (*l_pIterator))
                         {
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_ASTR, 0); //SIZE_OF_ARG(char*)
                         }
                         else
                         {
#ifdef UTF8_ENCODING
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_USTR, 0); //SIZE_OF_ARG(char*)     
#else
                             m_bInitialized = Add_Argument(P7TRACE_ARG_TYPE_WSTR, 0); //SIZE_OF_ARG(wchar_t*)     
#endif                             
                         }

                         l_bPercent = FALSE;
                         break;
                     }
                     case TM('%'):
                     {
                         l_bPercent = FALSE;
                         break;
                     }
                 } //switch (*l_pIterator)
             }

             l_pIterator ++;
         } //while (    (*l_pIterator)

         //if (FALSE == m_bInitialized)
         //{
         //    m_dwArgs_Len = 0;
         //    i_pFormat    = TM("WRONG TRACE ARGUMENTS !");
         //}
     } //if (m_bInitialized)

     if (m_bInitialized)
     {
         m_dwSize = sizeof(sP7Trace_Format);
                    
         if (m_pArgs)
         {
             m_dwSize += sizeof(sP7Trace_Arg) * m_dwArgs_Len;
         }

         if (i_pFile)
         {
             l_dwFile_Size += (tUINT32)strlen(i_pFile);
         }
         l_dwFile_Size ++; //last 0

         if (i_pFunction)
         {
             l_dwFunc_Size += (tUINT32)strlen(i_pFunction);
         }
         l_dwFunc_Size ++; //last 0

         l_dwForm_Size = (PUStrLen(i_pFormat) + 1)  * sizeof(tWCHAR);
         m_dwSize     += l_dwForm_Size + l_dwFunc_Size + l_dwFile_Size;

         m_pBuffer = new tUINT8[m_dwSize];
         if (NULL == m_pBuffer)
         {
             m_bInitialized = FALSE;
         }
     }

     if (m_bInitialized)
     {
         tUINT32 l_dwOffset = 0;

         sP7Trace_Format *l_pHeader_Desc = (sP7Trace_Format *)(m_pBuffer + l_dwOffset);

         l_pHeader_Desc->sCommon.dwSize     = m_dwSize;
         l_pHeader_Desc->sCommon.dwType     = EP7USER_TYPE_TRACE;
         l_pHeader_Desc->sCommon.dwSubType  = EP7TRACE_TYPE_DESC;

         l_pHeader_Desc->wArgs_Len          = (tUINT16)m_dwArgs_Len;
         l_pHeader_Desc->wID                = i_wID;
         l_pHeader_Desc->wLine              = i_wLine;
         l_pHeader_Desc->wModuleID          = i_wModuleID;
         l_dwOffset                        += sizeof(sP7Trace_Format);


         if (m_pArgs)
         {
             memcpy(m_pBuffer + l_dwOffset, 
                    m_pArgs,
                    sizeof(sP7Trace_Arg) * m_dwArgs_Len
                   );
             l_dwOffset += sizeof(sP7Trace_Arg) * m_dwArgs_Len;
         }

         //we do not verify address of i_pFormat because we do it before
         PUStrCpy((tWCHAR*)(m_pBuffer + l_dwOffset),
                  (m_dwSize - l_dwOffset) / sizeof(tWCHAR),
                  i_pFormat
                 );
         
         l_dwOffset += l_dwForm_Size;

         if (i_pFile)
         {
             memcpy(m_pBuffer + l_dwOffset, i_pFile, l_dwFile_Size);
             l_dwOffset += l_dwFile_Size;
         }
         else
         {
             m_pBuffer[l_dwOffset ++] = 0; 
         }

         if (i_pFunction)
         {
             memcpy(m_pBuffer + l_dwOffset, i_pFunction, l_dwFunc_Size);
             l_dwOffset += l_dwFunc_Size;
         }
         else
         {
             m_pBuffer[l_dwOffset ++] = 0; 
         }
     }


     if (    (m_bInitialized)
          && (m_pArgs)
        )
     {
         //calculate blocks count ...
         m_dwBlocks_Count = 0;

         tBOOL l_bCumulative = FALSE;
         for (tUINT32 l_dwI = 0; l_dwI < m_dwArgs_Len; l_dwI++)
         {
             if (    (P7TRACE_ARG_TYPE_WSTR == m_pArgs[l_dwI].bType)
                  || (P7TRACE_ARG_TYPE_ASTR == m_pArgs[l_dwI].bType)
                  || (P7TRACE_ARG_TYPE_USTR == m_pArgs[l_dwI].bType)
                )
             {
                 m_dwBlocks_Count ++;   
                 l_bCumulative = FALSE;
             }
             else
             {
                 if (FALSE == l_bCumulative)
                 {
                     m_dwBlocks_Count ++;   
                 }

                 l_bCumulative = TRUE;
             }
         }

         m_pBlocks = new tINT32[m_dwBlocks_Count];
         
         if (m_pBlocks) //fill the blocks
         {
             tUINT32 l_dwNum = 0;
 
             memset(m_pBlocks, 0, sizeof(tINT32) * m_dwBlocks_Count);
             l_bCumulative = FALSE;

             for (tUINT32 l_dwI = 0; l_dwI < m_dwArgs_Len; l_dwI++)
             {
                 if (P7TRACE_ARG_TYPE_WSTR == m_pArgs[l_dwI].bType)
                 {
                     l_dwNum ++;
                     m_pBlocks[l_dwNum - 1] = P7TRACE_ITEM_BLOCK_WSTRING;
                     l_bCumulative = FALSE;
                 }
                 else if (    (P7TRACE_ARG_TYPE_ASTR == m_pArgs[l_dwI].bType)
                           || (P7TRACE_ARG_TYPE_USTR == m_pArgs[l_dwI].bType)
                         )
                 {
                     l_dwNum ++;
                     //P7TRACE_ITEM_BLOCK_USTRING will be processed as ASCII
                     m_pBlocks[l_dwNum - 1] = P7TRACE_ITEM_BLOCK_ASTRING;
                     l_bCumulative = FALSE;
                 }
                 else
                 {
                     if (FALSE == l_bCumulative)
                     {
                         l_dwNum ++;   
                         l_bCumulative = TRUE;
                     }

                     m_pBlocks[l_dwNum - 1] += m_pArgs[l_dwI].bSize;
                 }
             }
         }
         else
         {
             m_bInitialized = FALSE;
         }
     }

    if (m_pArgs)
    {
        delete [] m_pArgs;
        m_pArgs = NULL;
    }

    m_dwArgs_Len = 0;
    m_dwArgs_Max = 0;

} // CP7Trace_Item   


////////////////////////////////////////////////////////////////////////////////
// ~CP7Trace_Item                                       
CP7Trace_Desc::~CP7Trace_Desc()
{
    if (m_pBuffer)
    {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }

    if (m_pArgs)
    {
        delete [] m_pArgs;
        m_pArgs = NULL;
    }

    if (m_pBlocks)
    {
        delete [] m_pBlocks;
        m_pBlocks = NULL;
    }
}// ~CP7Trace_Item                                       


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                       
tBOOL CP7Trace_Desc::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Get_Buffer                                       
tUINT8 *CP7Trace_Desc::Get_Buffer(tUINT32 *o_pSize)
{
    if (NULL != o_pSize)
    {
        *o_pSize = m_dwSize;
    }
    return m_pBuffer;
}// Get_Buffer                                       


////////////////////////////////////////////////////////////////////////////////
// Get_Blocks                                       
tINT32 *CP7Trace_Desc::Get_Blocks(tUINT32 *o_pCount)
{
    if (NULL == o_pCount)
    {
        return NULL;
    }

    *o_pCount = m_dwBlocks_Count;

    return m_pBlocks;
}// Get_Blocks


////////////////////////////////////////////////////////////////////////////////
// Set_Resets 
void CP7Trace_Desc::Set_Resets(tUINT32 i_dwResets)
{
    m_dwResets = i_dwResets;
}// Set_Resets                                       


////////////////////////////////////////////////////////////////////////////////
// Get_Resets                                       
tUINT32 CP7Trace_Desc::Get_Resets()
{
    return m_dwResets;
}// Get_Resets                                       


////////////////////////////////////////////////////////////////////////////////
// Is_Equal                  
P7TRACE_ASSERT(P7TRACE_KEY_LENGTH == 2);
tBOOL CP7Trace_Desc::Is_Equal(tKeyType *i_pKey)
{
    return ((i_pKey[0] == m_pKey[0]) && (i_pKey[1] == m_pKey[1]));
}// Is_Equal


////////////////////////////////////////////////////////////////////////////////
// Is_Greater (m_pKey > i_pKey) == TRUE
P7TRACE_ASSERT(P7TRACE_KEY_LENGTH == 2);
tBOOL CP7Trace_Desc::Is_Greater(tKeyType *i_pKey)
{
    if (i_pKey[0] < m_pKey[0])
    {
        return TRUE;
    }

    if ((i_pKey[0] == m_pKey[0]) && (i_pKey[1] < m_pKey[1]))
    {
        return TRUE;
    }

    return FALSE;
}// Is_Greater


////////////////////////////////////////////////////////////////////////////////
// Get_ID                                       
tUINT16 CP7Trace_Desc::Get_ID()
{
    return m_wID;
} //Get_ID


////////////////////////////////////////////////////////////////////////////////
// Add_Argument                                       
tBOOL CP7Trace_Desc::Add_Argument(tUINT8 i_bType, tUINT8 i_bSize)
{
    if ( (m_dwArgs_Len + 1) >= m_dwArgs_Max )
    {
        tUINT32       l_dwMax = m_dwArgs_Len + 16;
        sP7Trace_Arg *l_pTMP  = new sP7Trace_Arg[l_dwMax];

        if (l_pTMP)
        {
            if (m_pArgs)
            {
                memcpy(l_pTMP, m_pArgs, sizeof(sP7Trace_Arg) * m_dwArgs_Len);
                delete [ ] m_pArgs;
            }

            m_pArgs      = l_pTMP;
            m_dwArgs_Max = l_dwMax;
        }
    }


    if (m_pArgs)
    {
        m_pArgs[m_dwArgs_Len].bType = i_bType;
        m_pArgs[m_dwArgs_Len].bSize = i_bSize;
        m_dwArgs_Len ++;
        return TRUE;
    }

    return FALSE;
}// Add_Argument



////////////////////////////////////////////////////////////////////////////////
// CP7Trace                                       
CP7Trace::CP7Trace(IP7_Client *i_pClient, const tXCHAR *i_pName)
    : m_lReference(1)
    , m_dwSequence(0)
    , m_pClient(i_pClient)
    , m_dwChannel_ID(0)
    , m_wDesc_Tree_ID(P7_TRACE_DESC_HARDCODED_COUNT)
    , m_dwLast_ID(0)
    , m_bInitialized(TRUE)
    , m_eVerbosity(EP7TRACE_LEVEL_TRACE)
    , m_dwResets(RESET_UNDEFINED)
    , m_pChunks(NULL)
    , m_dwChunks_Max_Count(0)
    , m_bIs_Channel(FALSE)
    , m_pShared(NULL)
{
     memset(&m_sCS, 0, sizeof(m_sCS));
    
     LOCK_CREATE(m_sCS);

     memset(&m_sHeader_Info, 0, sizeof(m_sHeader_Info));
     memset(&m_sHeader_Data, 0, sizeof(m_sHeader_Data));

     if (NULL == m_pClient)
     {
         m_bInitialized = FALSE;
     }
     else
     {
         m_pClient->Add_Ref();
     }

     if (m_bInitialized)
     {
         memset(m_pDesc_Array, 0, sizeof(m_pDesc_Array));
         m_bInitialized = Inc_Chunks(32);
     }

     if (m_bInitialized)
     {
         m_sHeader_Info.sCommon.dwSize    = sizeof(sP7Trace_Info);
         m_sHeader_Info.sCommon.dwType    = EP7USER_TYPE_TRACE;
         m_sHeader_Info.sCommon.dwSubType = EP7TRACE_TYPE_INFO;

         m_sHeader_Data.sCommon.dwType    = EP7USER_TYPE_TRACE;
         m_sHeader_Data.sCommon.dwSubType = EP7TRACE_TYPE_DATA;

         if (i_pName)
         {
             PUStrCpy(m_sHeader_Info.pName, P7TRACE_NAME_LENGTH, i_pName);
         }
         else
         {
             PUStrCpy(m_sHeader_Info.pName, P7TRACE_NAME_LENGTH, TM("Unknown"));
         }

         m_sHeader_Info.qwTimer_Frequency = GetPerformanceFrequency();
         m_sHeader_Info.qwTimer_Value     = GetPerformanceCounter();

         GetEpochTime(&m_sHeader_Info.dwTime_Hi, &m_sHeader_Info.dwTime_Lo);

         m_sHeader_Info.qwFlags   = 0;
     }

     if (m_bInitialized)
     {
         m_bIs_Channel  = (ECLIENT_STATUS_OK == m_pClient->Register_Channel(this));
         m_bInitialized = m_bIs_Channel;
     }
}


////////////////////////////////////////////////////////////////////////////////
// ~CP7Trace                                       
CP7Trace::~CP7Trace()
{
    if (m_pShared)
    {
        Shared_Close(m_pShared);
        m_pShared = NULL;
    }

    if (m_bIs_Channel)
    {
        //inform server about channel closing, I didn't check status, just
        //sending data
        if (m_bInitialized)
        {
            if (RESET_UNDEFINED != m_dwResets)
            {
                sP7Ext_Header   l_sHeader = { EP7USER_TYPE_TRACE, 
                                              EP7TRACE_TYPE_CLOSE,
                                              sizeof(sP7Ext_Header)
                                            };

                sP7C_Data_Chunk l_sChunk = {&l_sHeader, l_sHeader.dwSize};

                m_pClient->Sent(m_dwChannel_ID, &l_sChunk, 1, l_sChunk.dwSize);
            }
        }

        m_pClient->Unregister_Channel(m_dwChannel_ID);    
    }

    for (tUINT32 l_dwI = 0; l_dwI < P7_TRACE_DESC_HARDCODED_COUNT; l_dwI++)
    {
        if (m_pDesc_Array[l_dwI])
        {
            delete m_pDesc_Array[l_dwI];
            m_pDesc_Array[l_dwI] = NULL;
        }
    }

    if (m_pClient)
    {
        m_pClient->Release();
        m_pClient = NULL;
    }

    if (m_pChunks)
    {
        delete [] m_pChunks;
        m_pChunks = NULL;
    }

    LOCK_DESTROY(m_sCS);
}// ~CP7Trace                                      


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                      
tBOOL CP7Trace::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Init                                      
void CP7Trace::Init(sP7C_Channel_Info *i_pInfo)
{
    if (i_pInfo)
    {
        m_dwChannel_ID = i_pInfo->dwID;
    }
}// Init


////////////////////////////////////////////////////////////////////////////////
// On_Receive                                      
void CP7Trace::On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize)
{
    LOCK_ENTER(m_sCS);

    sP7Ext_Header *l_pHeader = (sP7Ext_Header *)i_pBuffer;

    if (    (l_pHeader)
         && (i_dwSize > sizeof(sP7Ext_Header))
       )
    {
        //i_pBuffer += sizeof(sP7Ext_Header);

        if (    (EP7USER_TYPE_TRACE == l_pHeader->dwType)
             && (EP7TRACE_TYPE_VERB == l_pHeader->dwSubType)
           )
        {
            m_eVerbosity = ((sP7Trace_Verb*)i_pBuffer)->eVerbosity;
        }
    }

    LOCK_EXIT(m_sCS);
}// On_Receive 


////////////////////////////////////////////////////////////////////////////////
// Set_Verbosity                                      
void CP7Trace::Set_Verbosity(eP7Trace_Level i_eVerbosity)
{
    LOCK_ENTER(m_sCS);
    m_eVerbosity = i_eVerbosity;
    LOCK_EXIT(m_sCS);
}// Set_Verbosity


////////////////////////////////////////////////////////////////////////////////
// Trace                                      
tBOOL CP7Trace::Trace(tUINT16       i_wTrace_ID,
                     eP7Trace_Level i_eLevel, 
                     tUINT16        i_wModule_ID,
                     tUINT16        i_wLine,
                     const char    *i_pFile,
                     const char    *i_pFunction,
                     const tXCHAR  *i_pFormat,
                     ...
                    )
{
    return Trace_Raw(i_wTrace_ID, 
                     i_eLevel, 
                     i_wModule_ID, 
                     i_wLine, 
                     i_pFile, 
                     i_pFunction, 
                     (tKeyType*)&i_pFunction,
                     &i_pFormat
                    );
}// Trace                                      


////////////////////////////////////////////////////////////////////////////////
// Share                                      
tBOOL CP7Trace::Share(const tXCHAR *i_pName)
{
    if (NULL != m_pShared)
    {
        return FALSE;
    }

    void *l_pTrace = static_cast<IP7_Trace*>(this);

    m_pShared = Shared_Create(i_pName, (tUINT8*)&l_pTrace, sizeof(l_pTrace));

    return (NULL != m_pShared);
}// Share


////////////////////////////////////////////////////////////////////////////////
// Trace_Embedded                                      
tBOOL CP7Trace::Trace_Embedded(tUINT16       i_wTrace_ID,   
                              eP7Trace_Level i_eLevel, 
                              tUINT16        i_wModule_ID,
                              tUINT16        i_wLine,
                              const char    *i_pFile,
                              const char    *i_pFunction,
                              const tXCHAR **i_ppFormat
                             )
{
    tKeyType l_pKey[P7TRACE_KEY_LENGTH] = {(tKeyType)i_pFunction, (tKeyType)*i_ppFormat};

    return Trace_Raw(i_wTrace_ID, 
                     i_eLevel, 
                     i_wModule_ID, 
                     i_wLine, 
                     i_pFile, 
                     i_pFunction, 
                     l_pKey,
                     i_ppFormat
                    );

}// Trace_Embedded


////////////////////////////////////////////////////////////////////////////////
// Trace_Raw  
__forceinline tBOOL CP7Trace::Trace_Raw(tUINT16        i_wTrace_ID,   
                                        eP7Trace_Level i_eLevel, 
                                        tUINT16        i_wModule_ID,
                                        tUINT16        i_wLine,
                                        const char    *i_pFile,
                                        const char    *i_pFunction,
                                        tKeyType      *i_pKey,
                                        const tXCHAR **i_ppFormat
                                       )
{
    tBOOL            l_bReturn  = TRUE;
    tUINT32          l_dwSize   = 0;
    CP7Trace_Desc   *l_pDesc    = NULL; 
    tINT32          *l_pBlocks  = NULL;
    tUINT32          l_dwBCount = 0;
    tUINT8          *l_pVArgs   = (tUINT8*)(i_ppFormat) + sizeof(tXCHAR*);
    tUINT8           l_bDrops   = 0;
    sP7C_Data_Chunk *l_pChunk; //we do not initialize it here, we do it later

    if (FALSE == m_bInitialized)
    {
        return FALSE;
    }

    m_pClient->Get_Status(&m_sStatus);

    if (FALSE == m_sStatus.bConnected)
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    if (i_eLevel < m_eVerbosity)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    m_dwSequence ++;

    ////////////////////////////////////////////////////////////////////////////
    //if user specify direct Trace ID
    if (    (1 <= i_wTrace_ID)
         && (P7_TRACE_DESC_HARDCODED_COUNT > i_wTrace_ID)
       )
    {
        l_pDesc = m_pDesc_Array[i_wTrace_ID];
        if (NULL == l_pDesc)
        {
            l_pDesc = new CP7Trace_Desc(i_wTrace_ID, 
                                        i_wLine,
                                        i_wModule_ID,
                                        i_pFile, 
                                        i_pFunction, 
                                       *i_ppFormat
                                       );
            m_pDesc_Array[i_wTrace_ID] = l_pDesc;
        }
    }
    else //if we should use map to find it
    {
        //This moment should be clarified:
        //Here we made search through RB tree by key. Key consist of 2 values
        // - address of function
        // - address of format string
        //They located in stack [i_pFunction][*i_ppFormat]
        //Function eTrace work with stack of upper function like Trace(...)
        //and we should operate by stack of that function. This is why here
        //we get i_ppFormat, convert it to pointer (4 or 8 bytes depending on OS)
        //and subtract one element to get pointer to the function name.
        l_pDesc = m_cDesc_Tree.Find(i_pKey);
        if (    (NULL == l_pDesc)
             && (0xFFFF > m_wDesc_Tree_ID)
           )
        {
            m_wDesc_Tree_ID ++;
            l_pDesc = new CP7Trace_Desc(m_wDesc_Tree_ID, 
                                        i_wLine, 
                                        i_wModule_ID,
                                        i_pFile, 
                                        i_pFunction, 
                                       *i_ppFormat
                                       );
            //in stack we have pointer of the function and then format, 2 values
            m_cDesc_Tree.Push(l_pDesc, i_pKey);
        }
    }

    if (    (NULL  == l_pDesc)
         || (FALSE == l_pDesc->Is_Initialized())
         //|| (i_pFormat != l_pDesc->Get_Key())
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_pBlocks = l_pDesc->Get_Blocks(&l_dwBCount);
    //increase chunks count if it is necessary
    if (    (l_dwBCount)
         && ((l_dwBCount + 4) >= m_dwChunks_Max_Count)
       )
    {
        if (FALSE == Inc_Chunks(l_dwBCount + 4 - m_dwChunks_Max_Count))
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }

    l_pChunk = m_pChunks;

    //connection was lost, we need to resend initial data 
    if (m_dwResets != m_sStatus.dwResets)
    {
        m_dwResets       = m_sStatus.dwResets;
        l_bDrops        |= RESET_FLAG_CHANNEL;
        l_pChunk->dwSize = sizeof(m_sHeader_Info);
        l_pChunk->pData  = &m_sHeader_Info;
        l_dwSize        += l_pChunk->dwSize;

        l_pChunk ++;
    }

    //trace description have to send again
    if (m_dwResets != l_pDesc->Get_Resets())
    {
        l_pDesc->Set_Resets(m_dwResets);

        l_bDrops       |= RESET_FLAG_TRACE;
        l_pChunk->pData = l_pDesc->Get_Buffer(&l_pChunk->dwSize);
        l_dwSize       += l_pChunk->dwSize;
        l_pChunk ++;
    }

    //we should also add all variable parameters length ... later
    m_sHeader_Data.sCommon.dwSize = sizeof(m_sHeader_Data); 

    m_sHeader_Data.bLevel         = (tUINT8)i_eLevel;
    m_sHeader_Data.bProcessor     = (tUINT8)CProc::Get_Processor();
    m_sHeader_Data.dwThreadID     = CProc::Get_Thread_Id();
    m_sHeader_Data.wID            = l_pDesc->Get_ID();
    m_sHeader_Data.dwSequence     = m_dwSequence;
    m_sHeader_Data.qwTimer        = GetPerformanceCounter();

    l_pChunk->dwSize              = sizeof(m_sHeader_Data);
    l_pChunk->pData               = &m_sHeader_Data;

    l_pChunk ++;

    while (l_dwBCount --)
    {
        if (0 <= (*l_pBlocks))
        {
            l_pChunk->dwSize = (*l_pBlocks);
            l_pChunk->pData  = l_pVArgs;
            l_pVArgs        += (*l_pBlocks);
        }
        else if (P7TRACE_ITEM_BLOCK_ASTRING == (*l_pBlocks))
        {
            l_pChunk->dwSize = (tUINT32)strlen(*(char**)l_pVArgs) + 1;
            l_pChunk->pData  = *(char**)l_pVArgs;
            l_pVArgs        += sizeof(char*);
        }
#ifndef UTF8_ENCODING
        else if (P7TRACE_ITEM_BLOCK_WSTRING == (*l_pBlocks))
        {
            l_pChunk->dwSize = (tUINT32)((wcslen(*(wchar_t**)l_pVArgs) + 1) * sizeof(wchar_t));
            l_pChunk->pData  = *(wchar_t**)l_pVArgs;
            l_pVArgs        += sizeof(wchar_t*);
        }
#endif                             

        m_sHeader_Data.sCommon.dwSize += l_pChunk->dwSize;

        l_pChunk ++;
        l_pBlocks++;
    }

    l_dwSize += m_sHeader_Data.sCommon.dwSize;

    if (ECLIENT_STATUS_OK != m_pClient->Sent(m_dwChannel_ID,
                                             m_pChunks,
                                             (tUINT32)(l_pChunk - m_pChunks),
                                             l_dwSize
                                            )
       )
    {
        //if delivery was failed and we try to deliver also trace description
        //we set marker that is should be redelivered next time
        if (l_bDrops & RESET_FLAG_TRACE)
        {
            l_pDesc->Set_Resets(RESET_UNDEFINED);
        }

        if (l_bDrops & RESET_FLAG_CHANNEL)
        {
            m_dwResets = RESET_UNDEFINED;
        }

        l_bReturn = FALSE;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Trace_Raw


////////////////////////////////////////////////////////////////////////////////
// Inc_Chunks                                      
__forceinline tBOOL CP7Trace::Inc_Chunks(tUINT32 i_dwInc)
{
    sP7C_Data_Chunk  *l_pChunks = new sP7C_Data_Chunk[m_dwChunks_Max_Count + i_dwInc];  
    if (NULL == l_pChunks)
    {
        return FALSE;
    }

    if (m_pChunks)
    {
        memcpy(l_pChunks, m_pChunks, m_dwChunks_Max_Count * sizeof(sP7C_Data_Chunk));
        delete [] m_pChunks;
    }

    m_pChunks             = l_pChunks;
    m_dwChunks_Max_Count += i_dwInc;

    return TRUE;
}// Inc_Chunks
