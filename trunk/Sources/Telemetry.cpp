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
// This file provide telemetry functionality                                   /
////////////////////////////////////////////////////////////////////////////////
#include "CommonClient.h"
#include "P7_Telemetry.h"
#include "P7_Trace.h"
#include "P7_Extensions.h"
#include "Telemetry.h"


#define RESET_UNDEFINED                                           (0xFFFFFFFFUL)
#define RESET_FLAG_CHANNEL                                        (0x1)
#define RESET_FLAG_COUNTER                                        (0x2)


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Telemetry
IP7_Telemetry * __stdcall P7_Create_Telemetry(IP7_Client   *i_pClient, 
                                              const tXCHAR *i_pName
                                             )
{
    CP7Telemetry *l_pReturn = new CP7Telemetry(i_pClient, i_pName);

    //if not initialized - remove
    if (    (l_pReturn)
         &&  (TRUE != l_pReturn->Is_Initialized())
       )
    {
        l_pReturn->Release();
        l_pReturn = NULL;
    }

    return static_cast<IP7_Telemetry *>(l_pReturn);
}//P7_Create_Telemetry


////////////////////////////////////////////////////////////////////////////////
//P7_Get_Shared_Trace
IP7_Telemetry * __stdcall P7_Get_Shared_Telemetry(const tXCHAR *i_pName)
{
    IP7_Telemetry *l_pReturn = NULL;

    if (Shared_Read(i_pName, (tUINT8*)&l_pReturn, sizeof(IP7_Telemetry*)))
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


////////////////////////////////////////////////////////////////////////////////
// CP7Tel_Counter                                       
CP7Tel_Counter::CP7Tel_Counter(tUINT8        i_bID,
                               tUINT8        i_bOn,
                               tINT64        i_llMin,
                               tINT64        i_llMax,
                               tINT64        i_llAlarm,
                               const tXCHAR *i_pName
                              )
    : m_bInitialized(TRUE)
{
    memset(&m_sCounter, 0, sizeof(sP7Tel_Counter));

    m_sCounter.sCommon.dwSize     = sizeof(sP7Tel_Counter);
    m_sCounter.sCommon.dwType     = EP7USER_TYPE_TELEMETRY;
    m_sCounter.sCommon.dwSubType  = EP7TEL_TYPE_COUNTER;

    m_sCounter.bID     = i_bID;
    m_sCounter.bOn     = i_bOn;
    m_sCounter.llAlarm = i_llAlarm;
    m_sCounter.llMax   = i_llMax;
    m_sCounter.llMin   = i_llMin;
    
    PUStrCpy(m_sCounter.pName,
             P7TELEMETRY_NAME_LENGTH,
             i_pName
            );
         
} // CP7Tel_Counter   


////////////////////////////////////////////////////////////////////////////////
// ~CP7Tel_Counter                                       
CP7Tel_Counter::~CP7Tel_Counter()
{
}// ~CP7Trace_Item                                       


////////////////////////////////////////////////////////////////////////////////
// Has_Name                                       
tBOOL CP7Tel_Counter::Has_Name(const tWCHAR *i_pName)
{
    tBOOL   l_bResult = TRUE;
    tWCHAR *l_pName   = m_sCounter.pName;
    tUINT32 l_dwLen   = 0;


    if (    (FALSE == m_bInitialized)
         || (NULL == i_pName)
       )
    {
        return FALSE;
    }

    while (    (*i_pName)
            || (*l_pName)
          )
    {
        if ((*i_pName) != (*l_pName))
        {
            l_bResult = FALSE;
            break;
        }
        else
        {
            i_pName ++;
            l_pName ++;
        }

        l_dwLen ++;

        if (P7TELEMETRY_COUNTER_NAME_LENGTH <= l_dwLen)
        {
            break;
        }
    }

    return l_bResult;
}// Has_Name


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                       
tBOOL CP7Tel_Counter::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Enable 
void CP7Tel_Counter::Enable(tUINT8 i_bOn)
{
    m_sCounter.bOn = i_bOn;
}// Enable



////////////////////////////////////////////////////////////////////////////////
// CP7Telemetry                                       
CP7Telemetry::CP7Telemetry(IP7_Client *i_pClient, const tXCHAR *i_pName)
    : m_lReference(1)
    , m_pClient(i_pClient)
    , m_dwChannel_ID(0)
    , m_bInitialized(TRUE)
    , m_dwUsed(0)
    , m_dwResets_Channel(RESET_UNDEFINED)
    , m_dwResets_Counters(RESET_UNDEFINED)
    , m_pChunks(NULL)
    , m_dwChunks_Max_Count(0)
    , m_bIs_Channel(FALSE)
    , m_pShared(NULL)
{
     memset(&m_sCS, 0, sizeof(m_sCS));
    
     LOCK_CREATE(m_sCS);

     memset(&m_sHeader_Info, 0, sizeof(m_sHeader_Info));
     memset(&m_sValue, 0, sizeof(m_sValue));

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
         memset(m_pCounters, 0, sizeof(m_pCounters));
         m_bInitialized = Inc_Chunks(P7TELEMETRY_COUNTERS_MAX_COUNT + 8);
     }

     if (m_bInitialized)
     {
         m_sHeader_Info.sCommon.dwSize    = sizeof(sP7Trace_Info);
         m_sHeader_Info.sCommon.dwType    = EP7USER_TYPE_TELEMETRY;
         m_sHeader_Info.sCommon.dwSubType = EP7TEL_TYPE_INFO;

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

         m_sValue.sCommon.dwSize         = sizeof(sP7Tel_Value); 
         m_sValue.sCommon.dwType         = EP7USER_TYPE_TELEMETRY;
         m_sValue.sCommon.dwSubType      = EP7TEL_TYPE_VALUE;
     }

     if (m_bInitialized)
     {
         m_bIs_Channel  = (ECLIENT_STATUS_OK == m_pClient->Register_Channel(this));
         m_bInitialized = m_bIs_Channel;
     }
}// CP7Telemetry


////////////////////////////////////////////////////////////////////////////////
// ~CP7Telemetry                                       
CP7Telemetry::~CP7Telemetry()
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
            if (RESET_UNDEFINED != m_dwResets_Counters)
            {
                sP7Ext_Header   l_sHeader = { EP7USER_TYPE_TELEMETRY, 
                                              EP7TEL_TYPE_CLOSE,
                                              sizeof(sP7Ext_Header)
                                            };

                sP7C_Data_Chunk l_sChunk = {&l_sHeader, l_sHeader.dwSize};

                m_pClient->Sent(m_dwChannel_ID, &l_sChunk, 1, l_sChunk.dwSize);
            }
        }

        m_pClient->Unregister_Channel(m_dwChannel_ID);    
    }

    for (tUINT32 l_dwI = 0; l_dwI < P7TELEMETRY_COUNTERS_MAX_COUNT; l_dwI++)
    {
        if (m_pCounters[l_dwI])
        {
            delete m_pCounters[l_dwI];
            m_pCounters[l_dwI] = NULL;
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
}// ~CP7Telemetry                                      


////////////////////////////////////////////////////////////////////////////////
// Is_Initialized                                      
tBOOL CP7Telemetry::Is_Initialized()
{
    return m_bInitialized;
}// Is_Initialized


////////////////////////////////////////////////////////////////////////////////
// Init                                      
void CP7Telemetry::Init(sP7C_Channel_Info *i_pInfo)
{
    if (i_pInfo)
    {
        m_dwChannel_ID = i_pInfo->dwID;
    }
}// Init


////////////////////////////////////////////////////////////////////////////////
// On_Receive                                      
void CP7Telemetry::On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize)
{
    LOCK_ENTER(m_sCS);

    sP7Ext_Header *l_pHeader = (sP7Ext_Header *)i_pBuffer;

    UNUSED_ARG(i_dwChannel);

    if (    (l_pHeader)
         && (i_dwSize > sizeof(sP7Ext_Header))
       )
    {
        //i_pBuffer += sizeof(sP7Ext_Header);

        if (    (EP7USER_TYPE_TELEMETRY == l_pHeader->dwType)
             && (EP7TEL_TYPE_ENABLE == l_pHeader->dwSubType)
           )
        {
            sP7Tel_Enable *l_pEnable = (sP7Tel_Enable*)i_pBuffer;

            if (m_pCounters[l_pEnable->bID])
            {
                m_pCounters[l_pEnable->bID]->Enable(l_pEnable->bOn);
                //printf("Enable %d, %d\n", (int)l_pEnable->bID, (int)l_pEnable->bOn);
            }
        }
    }

    LOCK_EXIT(m_sCS);
}// On_Receive 



////////////////////////////////////////////////////////////////////////////////
// Create, i_pName is case sensitive and should be unique 
tBOOL CP7Telemetry::Create(const tXCHAR  *i_pName, 
                           tINT64         i_llMin,
                           tINT64         i_llMax,
                           tINT64         i_llAlarm,
                           tUINT8         i_bOn,
                           tUINT8        *o_pID 
                          )
{
    tWCHAR  l_pName[P7TELEMETRY_COUNTER_NAME_LENGTH];
    tBOOL   l_bReturn = TRUE;

    if (    (FALSE == m_bInitialized)
         || (NULL == i_pName)
         || (NULL == o_pID)
       )
    {
        return FALSE;
    }

    LOCK_ENTER(m_sCS);

    ////////////////////////////////////////////////////////////////////////////
    //find by name already existing counter

    l_pName[0] = 0;
    PUStrCpy(l_pName, P7TRACE_NAME_LENGTH, i_pName);

    for (tUINT32 l_dwI = 0; l_dwI < m_dwUsed; l_dwI++)
    {
        if (m_pCounters[l_dwI]->Has_Name(l_pName))
        {
            l_bReturn = FALSE;
            goto l_lblExit;
        }
    }


    if (P7TELEMETRY_COUNTERS_MAX_COUNT <= m_dwUsed)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    m_pCounters[m_dwUsed] = new CP7Tel_Counter((tUINT8)m_dwUsed,
                                               i_bOn,
                                               i_llMin,
                                               i_llMax,
                                               i_llAlarm,
                                               i_pName
                                              );
    if (    (NULL == m_pCounters[m_dwUsed])
         || (FALSE == m_pCounters[m_dwUsed]->Is_Initialized())
       )
    {
        if (m_pCounters[m_dwUsed])
        {
            delete m_pCounters[m_dwUsed];
            m_pCounters[m_dwUsed] = NULL;
        }

        l_bReturn = FALSE;
        goto l_lblExit;
    }

l_lblExit:
    if (l_bReturn)
    {
        m_dwResets_Counters = RESET_UNDEFINED;
        *o_pID              = (tUINT8)m_dwUsed;
        m_dwUsed ++;
    }

    LOCK_EXIT(m_sCS);

    return l_bReturn;
} //Create


////////////////////////////////////////////////////////////////////////////////
// Find, i_pName is case sensitive
tBOOL CP7Telemetry::Find(const tXCHAR *i_pName, tUINT8 *o_pID)
{
   tWCHAR           l_pName[P7TELEMETRY_COUNTER_NAME_LENGTH];
   tBOOL            l_bReturn  = FALSE;

    if (    (FALSE == m_bInitialized)
         || (NULL == i_pName)
         || (NULL == o_pID)
       )
    {
        return FALSE;
    }

    *o_pID = 0;

    LOCK_ENTER(m_sCS);

    l_pName[0] = 0;
    PUStrCpy(l_pName, P7TRACE_NAME_LENGTH, i_pName);

    for (tUINT32 l_dwI = 0; l_dwI < m_dwUsed; l_dwI++)
    {
        if (m_pCounters[l_dwI]->Has_Name(l_pName))
        {
            sP7Tel_Counter *l_pCnt = &(m_pCounters[l_dwI]->m_sCounter);

            if (l_pCnt)
            {
                *o_pID    = l_pCnt->bID;
                l_bReturn = TRUE;
            }

            break;
        }
    }

    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Find


////////////////////////////////////////////////////////////////////////////////
// Add  
tBOOL CP7Telemetry::Add(tUINT8 i_bID, tINT64 i_llValue)
{
    tBOOL            l_bReturn  = TRUE;
    tUINT32          l_dwSize   = 0;
    tUINT8           l_bReset   = 0;

    //we do not initialize it here, we do it later
    sP7C_Data_Chunk *l_pChunk; 

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


    ////////////////////////////////////////////////////////////////////////////
    //check ID
    if (    (P7TELEMETRY_COUNTERS_MAX_COUNT <= i_bID)
         || (NULL == m_pCounters[i_bID])
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_pChunk  = m_pChunks;

    //connection was lost, we need to resend initial data 
    if (m_dwResets_Channel != m_sStatus.dwResets)
    {
        m_dwResets_Channel = m_sStatus.dwResets;

        l_bReset          |= RESET_FLAG_CHANNEL;
        l_pChunk->dwSize   = sizeof(m_sHeader_Info);
        l_pChunk->pData    = &m_sHeader_Info;
        l_dwSize          += l_pChunk->dwSize;
        l_pChunk ++;
    }

    //counters descriptions have to send again
    if (m_dwResets_Counters != m_sStatus.dwResets)
    {
        CP7Tel_Counter **l_pIter = m_pCounters;

        while (*l_pIter)
        {
            l_pChunk->pData  = &((*l_pIter)->m_sCounter);
            l_pChunk->dwSize = (*l_pIter)->m_sCounter.sCommon.dwSize;
            l_dwSize        += l_pChunk->dwSize;
            l_pChunk ++;
            l_pIter++;
        }

        m_dwResets_Counters = m_sStatus.dwResets;
        l_bReset          |= RESET_FLAG_COUNTER;
    }

    if (m_pCounters[i_bID]->m_sCounter.bOn)
    {
        m_sValue.bID     = i_bID;
        m_sValue.llValue = i_llValue;
        m_sValue.qwTimer = GetPerformanceCounter();


        l_pChunk->pData  = &m_sValue;
        l_pChunk->dwSize = m_sValue.sCommon.dwSize;
        l_dwSize        += l_pChunk->dwSize;

        l_pChunk ++;
    }
    else if (0 >= l_dwSize)
    {
        goto l_lblExit;
    }


    if (ECLIENT_STATUS_OK != m_pClient->Sent(m_dwChannel_ID,
                                             m_pChunks,
                                             (tUINT32)(l_pChunk - m_pChunks),
                                             l_dwSize
                                            )
       )
    {
        //if delivery was failed and we try to deliver also trace description
        //we set marker that is should be redelivered next time
        if (l_bReset & RESET_FLAG_CHANNEL)
        {
            m_dwResets_Channel = RESET_UNDEFINED;
        }
        if (l_bReset & RESET_FLAG_COUNTER)
        {
            m_dwResets_Counters = RESET_UNDEFINED;
        }

        l_bReturn = FALSE;
    }

l_lblExit:
    LOCK_EXIT(m_sCS);

    return l_bReturn;
}// Add  


////////////////////////////////////////////////////////////////////////////////
// Share                                      
tBOOL CP7Telemetry::Share(const tXCHAR *i_pName)
{
    if (NULL != m_pShared)
    {
        return FALSE;
    }

    void *l_pPointer = static_cast<IP7_Telemetry*>(this);

    m_pShared = Shared_Create(i_pName, (tUINT8*)&l_pPointer, sizeof(l_pPointer));

    return (NULL != m_pShared);
}// Share


////////////////////////////////////////////////////////////////////////////////
// Inc_Chunks                                      
__forceinline tBOOL CP7Telemetry::Inc_Chunks(tUINT32 i_dwInc)
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


