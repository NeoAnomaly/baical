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
#ifndef TRACE_H_AZH
#define TRACE_H_AZH

#define P7TRACE_ITEM_BLOCK_ASTRING                                          (-1)
#define P7TRACE_ITEM_BLOCK_WSTRING                                          (-2)
#define P7TRACE_ITEM_BLOCK_USTRING                                          (-3)

#define P7TRACE_KEY_LENGTH                                                   (2)

#define P7TRACE_MAP_FILE_NAME                                  L"Local\\P7Trace"
#define P7TRACE_MAP_FILE_SIZE          (sizeof(CP7Trace*) + sizeof(IP7_Client*))


#define P7TRACE_ASSERT(cond) typedef int assert_type[(cond) ? 1 : -1]


////////////////////////////////////////////////////////////////////////////////
tBOOL P7_Get_Map_Instance(IP7_Client **o_pClient, IP7_Trace **o_pTrace);
void P7_Get_Map_Name(wchar_t *o_pBuffer, tUINT32 i_dwLength);


////////////////////////////////////////////////////////////////////////////////
class CP7Trace_Desc
{
    tUINT16           m_wID;
    //count of connections drops... see sP7C_Status. If this value and 
    //value from IP7_Client are different - it mean we loose connection
    tUINT32          m_dwResets; 
    tUINT32          m_dwSize;
    tUINT8          *m_pBuffer;//Buffer will contain sP7T_Header_Desc
    sP7Trace_Arg    *m_pArgs;  
    tUINT32          m_dwArgs_Len;
    tUINT32          m_dwArgs_Max;

    tINT32          *m_pBlocks;
    tINT32           m_dwBlocks_Count;

    tKeyType         m_pKey[P7TRACE_KEY_LENGTH]; 

    tBOOL            m_bInitialized;
public:
    CP7Trace_Desc(tUINT16        i_wID,
                  tUINT16        i_wLine, 
                  tUINT16        i_wModuleID,
                  const char    *i_pFile,
                  const char    *i_pFunction,
                  const tXCHAR  *i_pFormat
                 );
    ~CP7Trace_Desc();

    tBOOL    Is_Initialized();
    tUINT8  *Get_Buffer(tUINT32 *o_pSize);
    tINT32  *Get_Blocks(tUINT32 *o_pCount);
    void     Set_Resets(tUINT32 i_dwDrops);
    tUINT32  Get_Resets();

    tBOOL    Is_Equal(tKeyType *i_pKey);
    tBOOL    Is_Greater(tKeyType *i_pKey);

    tUINT16  Get_ID();

private:
    tBOOL    Add_Argument(tUINT8 i_bType, tUINT8 i_bSize);
};


////////////////////////////////////////////////////////////////////////////////
class CDesc_Tree:
    public CRBTree<CP7Trace_Desc*, tKeyType*>
{
protected:
    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey < i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Key_Less(tKeyType *i_pKey, CP7Trace_Desc *i_pData) 
    {
        return i_pData->Is_Greater(i_pKey);
    }

    //////////////////////////////////////////////////////////////////////////// 
    //Return
    //TRUE  - if (i_pKey == i_pData::key)
    //FALSE - otherwise
    tBOOL Is_Qual(tKeyType *i_pKey, CP7Trace_Desc *i_pData) 
    {
        return i_pData->Is_Equal(i_pKey);
    }
};


////////////////////////////////////////////////////////////////////////////////
#define P7_TRACE_DESC_HARDCODED_COUNT                                     (1024)


class CP7Trace:
    public IP7C_Channel
   ,public IP7_Trace
{
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile         m_lReference;
    tUINT32                 m_dwSequence;

    IP7_Client             *m_pClient; 
    tUINT32                 m_dwChannel_ID;
    CP7Trace_Desc          *m_pDesc_Array[P7_TRACE_DESC_HARDCODED_COUNT];
    CDesc_Tree              m_cDesc_Tree;
    tUINT16                 m_wDesc_Tree_ID;

    tLOCK                   m_sCS; 
    tUINT32                 m_dwLast_ID;
    tBOOL                   m_bInitialized;
    eP7Trace_Level          m_eVerbosity;

    sP7Trace_Info           m_sHeader_Info;
    sP7Trace_Data           m_sHeader_Data;

    tUINT32                 m_dwResets; 
    sP7C_Status             m_sStatus;

    sP7C_Data_Chunk        *m_pChunks;
    tUINT32                 m_dwChunks_Max_Count;

    tBOOL                   m_bIs_Channel;

    hShared                *m_pShared;
public:
    CP7Trace(IP7_Client *i_pClient, const tXCHAR *i_pName); //for arguments description see P7_Client.h
    ~CP7Trace();

    tBOOL Is_Initialized();

    void Init(sP7C_Channel_Info *i_pInfo);
    void On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize);

    void Set_Verbosity(eP7Trace_Level i_eVerbosity);

    tBOOL Trace(tUINT16       i_wTrace_ID,   
               eP7Trace_Level i_eLevel, 
               tUINT16        i_wModule_ID,
               tUINT16        i_wLine,
               const char    *i_pFile,
               const char    *i_pFunction,
               const tXCHAR  *i_pFormat,
               ...
              );

    tBOOL Trace_Embedded(tUINT16        i_wTrace_ID,   
                         eP7Trace_Level i_eLevel, 
                         tUINT16        i_wModule_ID,
                         tUINT16        i_wLine,
                         const char    *i_pFile,
                         const char    *i_pFunction,
                         const tXCHAR **i_ppFormat
                        );

    virtual tINT32 Add_Ref()
    {
        return ATOMIC_INC(&m_lReference);
    }

    virtual tINT32 Release()
    {
        tINT32 l_lResult = ATOMIC_DEC(&m_lReference);
        if ( 0 >= l_lResult )
        {
            delete this;
        }

        return l_lResult;
    }

    tBOOL Share(const tXCHAR *i_pName);

private:
    tBOOL   Trace_Raw(tUINT16        i_wTrace_ID,   
                      eP7Trace_Level i_eLevel, 
                      tUINT16        i_wModule_ID,
                      tUINT16        i_wLine,
                      const char    *i_pFile,
                      const char    *i_pFunction,
                      tKeyType      *i_pKey,
                      const tXCHAR **i_ppFormat
                  );

    tBOOL   Inc_Chunks(tUINT32 i_dwInc);
};

#endif //TRACE_H_AZH