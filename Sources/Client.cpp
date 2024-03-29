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
// This header file provide client functionality                               /
////////////////////////////////////////////////////////////////////////////////

#define CLIENT_PROTOCOL_VERSION                                         (0x0007)

#include "CommonClient.h"
#include "Client.h"

////////////////////////////////////////////////////////////////////////////////
//P7_Create_Client
IP7_Client * __stdcall P7_Create_Client(const tXCHAR *i_pArgs)
{
    CClient *l_pReturn = new CClient(i_pArgs);

    //if not initialized - remove
    if (    (l_pReturn)
         &&  (ECLIENT_STATUS_OK != l_pReturn->Get_Status())
       )
    {
        l_pReturn->Release();
        l_pReturn = NULL;
    }

    return static_cast<IP7_Client *>(l_pReturn);
}//P7_Create_Client


////////////////////////////////////////////////////////////////////////////////
//P7_Get_Shared
IP7_Client * __stdcall P7_Get_Shared(const tXCHAR *i_pName)
{
    IP7_Client *l_pReturn = NULL;

    if (Shared_Read(i_pName, (tUINT8*)&l_pReturn, sizeof(IP7_Client*)))
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
}//P7_Get_Shared


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//CClient
CClient::CClient(const tXCHAR *i_pArgs)
    : m_lReference(1)

    , m_lReject_Mem(0)
    , m_lReject_Con(0)
    , m_lReject_Int(0)

    , m_bIs_Winsock(FALSE)
    , m_pSocket(NULL)
    , m_wClient_ID(0xFFFF)
    , m_pLog(NULL)
    , m_pBPool(NULL)
    , m_eStatus(ECLIENT_STATUS_OK)

    , m_pData_Wnd(NULL)
    , m_pData_Wnd_Cell(NULL)
    , m_dwData_Wnd_Max_Count(0)
    , m_dwData_Wnd_Size(0)
    , m_dwData_Wnd_TimeStamp(0)
    , m_dwData_Wnd_First_ID(TPACKET_MAX_ID + 1)
    , m_dwData_Wnd_Last_ID(TPACKET_MAX_ID + 1)
    , m_bData_Resending(FALSE)

    , m_dwDelivery_Fails(0)
    , m_bIs_Response_Waiting(FALSE)

    , m_pData_Queue_Out(NULL)
    , m_pData_Queue_In(NULL)

    , m_dwLast_Packet_ID(0)
    , m_dwServiceTimeStamp(0)
    , m_bConnected(TRUE)
    , m_dwConnection_Resets(0)

    , m_pPacket_Control(NULL)

    , m_bComm_Thread(FALSE)
    , m_hComm_Thread(0) //NULL
    , m_bChnl_Thread(FALSE)
    , m_hChnl_Thread(0) //NULL

    , m_pShared(NULL)
{
    memset(m_pChannels, 0, sizeof(IP7C_Channel*)*USER_PACKET_CHANNEL_ID_MAX_SIZE);

    memset(&m_hCS_Data_Out, 0, sizeof(m_hCS_Data_Out));
    memset(&m_hCS_User,     0, sizeof(m_hCS_User));
    memset(&m_hCS_Data_In,  0, sizeof(m_hCS_Data_In));
    memset(&m_hCS_Channels, 0, sizeof(m_hCS_Channels));
    
    //1. Parse command line from console and from source code,
    //   priority belongs to console
    int      l_iHC_ArgsC = 0;
    tXCHAR **l_pHC_Args  = i_pArgs ? CProc::Get_ArgV(i_pArgs, &l_iHC_ArgsC) : NULL;
    int      l_iCn_ArgsC = 0;
    tXCHAR **l_pCn_Args  = CProc::Get_ArgV(&l_iCn_ArgsC);
            

    //2. get ON/OFF option
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Base(l_pHC_Args, l_iHC_ArgsC, l_pCn_Args, l_iCn_ArgsC);
    }

    //3. initialize logging
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        Init_Log(l_pHC_Args, l_iHC_ArgsC, l_pCn_Args, l_iCn_ArgsC);
    }

    //4. initialize network layer and sockets
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Sockets(l_pHC_Args, l_iHC_ArgsC, l_pCn_Args, l_iCn_ArgsC);
    }

    //5. Initialize Pool
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Pool(l_pHC_Args, l_iHC_ArgsC, l_pCn_Args, l_iCn_ArgsC);
    }
   
    //6. Initialize variables
    if (ECLIENT_STATUS_OK == m_eStatus)
    {
        m_eStatus = Init_Members(l_pHC_Args, l_iHC_ArgsC, l_pCn_Args, l_iCn_ArgsC);
    }
   
    //7. cleanup
    if (l_pHC_Args)
    {
        CProc::Free_ArgV(l_pHC_Args);
        l_pHC_Args = NULL;
    }

    if (l_pCn_Args)
    {
        CProc::Free_ArgV(l_pCn_Args);
        l_pCn_Args = NULL;
    }
}//CClient


////////////////////////////////////////////////////////////////////////////////
//~CClient
CClient::~CClient()
{
    if (m_pShared)
    {
        Shared_Close(m_pShared);
        m_pShared = NULL;
    }

    m_cComm_Event.Set(THREAD_EXIT_SIGNAL);
    m_cChnl_Event.Set(THREAD_EXIT_SIGNAL);

    if (m_bComm_Thread)
    {
        if (TRUE == CThShell::Close(m_hComm_Thread, 60000))
        {
            m_hComm_Thread = 0;//NULL;
            m_bComm_Thread = FALSE;
        }
        else
        {
            JOURNAL_CRITICAL(m_pLog, TM("Can't close communication thread !"));
        }
    }

    if (m_bChnl_Thread)
    {
        if (TRUE == CThShell::Close(m_hChnl_Thread, 60000))
        {
            m_hChnl_Thread = 0;//NULL;
            m_bChnl_Thread = FALSE;
        }
        else
        {
            JOURNAL_CRITICAL(m_pLog, TM("Can't close channels thread !"));
        }
    }

    if (m_pData_Queue_Out)
    {
        pAList_Cell l_pElement = NULL;

        while ((l_pElement = m_pData_Queue_Out->Get_First()))
        {
            CTPacket *l_pPacket = m_pData_Queue_Out->Get_Data(l_pElement);
            m_pData_Queue_Out->Del(l_pElement, FALSE);
            m_pBPool->Push_Buffer(l_pPacket);
        }

        m_pData_Queue_Out->Clear(TRUE);
        delete m_pData_Queue_Out;
        m_pData_Queue_Out = NULL;
    }


    if (m_pData_Queue_In)
    {
        pAList_Cell l_pElement = NULL;

        while ((l_pElement = m_pData_Queue_In->Get_First()))
        {
            CTPacket *l_pPacket = m_pData_Queue_In->Get_Data(l_pElement);
            m_pData_Queue_In->Del(l_pElement, FALSE);
            m_pBPool->Push_Buffer(l_pPacket);
        }

        m_pData_Queue_In->Clear(TRUE);
        delete m_pData_Queue_In;
        m_pData_Queue_In = NULL;
    }


    if (m_pData_Wnd)
    {
        pAList_Cell l_pElement = NULL;

        while ((l_pElement = m_pData_Wnd->Get_First()))
        {
            CTPacket *l_pPacket = m_pData_Wnd->Get_Data(l_pElement);
            m_pData_Wnd->Del(l_pElement, FALSE);
            m_pBPool->Push_Buffer(l_pPacket);
        }

        m_pData_Wnd->Clear(TRUE);
        delete m_pData_Wnd;
        m_pData_Wnd = NULL;
    }

    //stop sockets
    if (m_bIs_Winsock)
    {
        WSA_UnInit();
    }

    if (m_pSocket)
    {
        delete m_pSocket;
        m_pSocket = NULL;
    }

    //clear pool
    if (m_pBPool)
    {
        delete m_pBPool;
        m_pBPool = NULL;
    }
    
    //remove log
    if (m_pLog)
    {
        m_pLog->Release();
        m_pLog = NULL;
    }

    LOCK_DESTROY(m_hCS_Data_Out);
    LOCK_DESTROY(m_hCS_User);

    LOCK_DESTROY(m_hCS_Data_In);
    LOCK_DESTROY(m_hCS_Channels);

} //~CClient


////////////////////////////////////////////////////////////////////////////////
//Get_Argument_Text_Value
tXCHAR *CClient::Get_Argument_Text_Value(tXCHAR **i_pHC_Args,
                                         tINT32   i_iHC_Count,
                                         tXCHAR **i_pCn_Args,
                                         tINT32   i_iCn_Count,
                                         tXCHAR  *i_pName
                                        )
{
    tXCHAR *l_pReturn = NULL;

    if (NULL == i_pName)
    {
        return l_pReturn;
    }

    tUINT32 l_dwName_Lenght = PStrLen(i_pName);
    tUINT32 i_dwArg_Lenght  = 0; 

    //Scan first console arguments, console has a proirity
    if (    (NULL != i_pCn_Args) 
         && (0 < i_iCn_Count) 
       )
    {
        for(tINT32 l_iIDX = 0; l_iIDX < i_iCn_Count; l_iIDX++)
        {
            i_dwArg_Lenght = PStrLen(i_pCn_Args[l_iIDX]); 

            if (    (l_dwName_Lenght <= i_dwArg_Lenght)
                 && (0 == PStrNCmp(i_pCn_Args[l_iIDX], i_pName, l_dwName_Lenght))
               )
            {
                l_pReturn = i_pCn_Args[l_iIDX] + l_dwName_Lenght;
                break;
            }
        }
    }

    //if console arguments hasn't a value trying to find in hardcoded arguments
    if (    (NULL != i_pHC_Args) 
         && (0 < i_iHC_Count) 
         && (NULL == l_pReturn)
       )
    {
        for(tINT32 l_iIDX = 0; l_iIDX < i_iHC_Count; l_iIDX++)
        {
            i_dwArg_Lenght = PStrLen(i_pHC_Args[l_iIDX]); 

            if (    (l_dwName_Lenght <= i_dwArg_Lenght)
                 && (0 == PStrNCmp(i_pHC_Args[l_iIDX], i_pName, l_dwName_Lenght))
               )
            {
                l_pReturn = i_pHC_Args[l_iIDX] + l_dwName_Lenght;
                break;
            }
        }
    }


    return l_pReturn;
}//Get_Argument_Text_Value


////////////////////////////////////////////////////////////////////////////////
//Init_Base
eClient_Status CClient::Init_Base(tXCHAR **i_pHC_Args,
                                  tINT32   i_iHC_Count,
                                  tXCHAR **i_pCn_Args,
                                  tINT32   i_iCn_Count
                                 )
{
    eClient_Status  l_eReturn    = ECLIENT_STATUS_OK;
    tXCHAR         *l_pArg_Value = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                                           i_pCn_Args, i_iCn_Count,
                                                           (tXCHAR*)CLIENT_COMMAND_LOG_ON);
    if (    (NULL != l_pArg_Value)
         && (TM('0') == l_pArg_Value[0])
       )
    {
        l_eReturn = ECLIENT_STATUS_OFF;
        //JOURNAL_WARNING(m_pLog, L"Service is OFF by console command");
    }

    if ( Get_Argument_Text_Value(NULL,
                                 0, 
                                 i_pCn_Args, 
                                 i_iCn_Count, 
                                 (tXCHAR*)CLIENT_COMMAND_LOG_HELP
                                ) 
       )
    {
        PRINTF((tXCHAR*)CLIENT_HELP_STRING);
    }

    return l_eReturn;
}//Init_Base


////////////////////////////////////////////////////////////////////////////////
//Init_Log
eClient_Status CClient::Init_Log(tXCHAR **i_pHC_Args,
                                 tINT32   i_iHC_Count,
                                 tXCHAR **i_pCn_Args,
                                 tINT32   i_iCn_Count
                                )
{
    tXCHAR        *l_pArg_Value   = NULL;
    eFJournal_Type l_eVerbosity   = EFJOIRNAL_TYPE_CRITICAL;

    l_pArg_Value = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                           i_pCn_Args, i_iCn_Count,
                                           (tXCHAR*)CLIENT_COMMAND_LOG_VERBOSITY);
    if (NULL == l_pArg_Value)
    {
        goto l_lClean_Up;
    }

    l_eVerbosity = (eFJournal_Type)PStrToInt(l_pArg_Value);

    if (EFJOIRNAL_TYPES_COUNT <= l_eVerbosity)
    {
        goto l_lClean_Up;
    }


    m_pLog = new CJournal();

    if (m_pLog)
    {
        if (FALSE == m_pLog->Initialize(TM("P7.Logs\\")))
        {
            m_pLog->Release();
            m_pLog = NULL;
        }
        else
        {
            m_pLog->Set_Verbosity(l_eVerbosity);
        }
    }

l_lClean_Up:

    return ECLIENT_STATUS_OK;
}//Init_Log


////////////////////////////////////////////////////////////////////////////////
//Init_Sockets
eClient_Status CClient::Init_Sockets(tXCHAR **i_pHC_Args,
                                     tINT32   i_iHC_Count,
                                     tXCHAR **i_pCn_Args,
                                     tINT32   i_iCn_Count
                                    )
{
    eClient_Status l_eReturn = ECLIENT_STATUS_OK;

    if (FALSE == WSA_Init())
    {
        JOURNAL_CRITICAL(m_pLog, 
                         TM("Windows Socket initialization fail !"));

        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
    }
    else
    {
        m_bIs_Winsock = TRUE;
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        tXCHAR     *l_pAddr = NULL;
        tXCHAR     *l_pPort = NULL;

        tADDR_INFO *l_pInfo = NULL;
        tADDR_INFO *l_pNext = NULL;
        tADDR_INFO  l_tHint;

        memset(&l_tHint, 0, sizeof(l_tHint));

        l_pAddr = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                          i_pCn_Args, i_iCn_Count,
                                          (tXCHAR*)CLIENT_COMMAND_LINE_ADDRESS);
        if (NULL == l_pAddr)
        {
            l_pAddr = (tXCHAR*)TM("127.0.0.1");
        }

        l_pPort = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                          i_pCn_Args, i_iCn_Count,
                                          (tXCHAR*)CLIENT_COMMAND_LINE_PORT);

        if (NULL == l_pPort)
        {
            l_pPort = (tXCHAR*)TM("9009");
        }

        l_tHint.ai_family   = AF_UNSPEC; //AF_INET;
        l_tHint.ai_socktype = SOCK_DGRAM;
        l_tHint.ai_protocol = IPPROTO_UDP;
        
        l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;

        //http://msdn.microsoft.com/en-us/library/windows/desktop/ms738520(v=vs.85).aspx
        //http://linux.die.net/man/3/getaddrinfo
        if (0 == GET_ADDR_INFO(l_pAddr, l_pPort, &l_tHint, &l_pInfo))
        {
             for(l_pNext = l_pInfo; l_pNext != NULL ;l_pNext=l_pNext->ai_next) 
             {
                 if (    (    (AF_INET  == l_pNext->ai_family)
                           || (AF_INET6 == l_pNext->ai_family)
                         )
                      && (SOCK_DGRAM  == l_pNext->ai_socktype)
                      && (IPPROTO_UDP == l_pNext->ai_protocol)
                    )
                 {
                     m_pSocket = new CUDP_Socket(m_pLog, 
                                                (sockaddr*) l_pNext->ai_addr,
                                                FALSE);
                     if (m_pSocket)
                     {
                         if (m_pSocket->Initialized())
                         {
                             l_eReturn = ECLIENT_STATUS_OK;
                             break;
                         }
                         else
                         {
                             delete m_pSocket;
                             m_pSocket = NULL;
                         }
                     }
                 }
             }
        } //if (0 == GetAddrInfoW

        if (l_pInfo)
        {
            FREE_ADDR_INFO(l_pInfo);
            l_pInfo = NULL;
        }
    }//if (ECLIENT_STATUS_OK == l_eReturn)

    return l_eReturn;
}//Init_Sockets


////////////////////////////////////////////////////////////////////////////////
//Init_Pool
eClient_Status CClient::Init_Pool(tXCHAR **i_pHC_Args,
                                  tINT32   i_iHC_Count,
                                  tXCHAR **i_pCn_Args,
                                  tINT32   i_iCn_Count
                                 )
{
    //eClient_Status l_eReturn       = ECLIENT_STATUS_OK;
    tXCHAR        *l_pArg_Value    = NULL;
    tUINT32        l_dwMax_Memory  = 0x100000; //1mb by default
    tUINT32        l_dwPacket_Size = TPACKET_MIN_SIZE;

    ////////////////////////////////////////////////////////////////////////////
    //packet size
    l_pArg_Value = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                           i_pCn_Args, i_iCn_Count,
                                           (tXCHAR*)CLIENT_COMMAND_PACKET_SIZE);
    if (l_pArg_Value)
    {
        l_dwPacket_Size = (tUINT32)PStrToInt(l_pArg_Value);

        if (TPACKET_MIN_SIZE > l_dwPacket_Size)
        {
            l_dwPacket_Size = TPACKET_MIN_SIZE;
        }
        else if (TPACKET_MAX_SIZE < l_dwPacket_Size)
        {
            l_dwPacket_Size = TPACKET_MAX_SIZE;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //pool size
    l_pArg_Value = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                           i_pCn_Args, i_iCn_Count,
                                           (tXCHAR*)CLIENT_COMMAND_POOL_SIZE);
    if (l_pArg_Value)
    {
        l_dwMax_Memory = 1024 * (tUINT32)PStrToInt(l_pArg_Value);

        if (16384 > l_dwMax_Memory)
        {
            l_dwMax_Memory = 16384;
        }
    }

    //check that memory is enough to store 20 packets
    if (l_dwMax_Memory < (l_dwPacket_Size * 20))
    {
        l_dwMax_Memory = l_dwPacket_Size * 20;

        JOURNAL_WARNING(m_pLog, 
                        TM("Pool size is not enought to store 20 packets, enlarge up to %d"),
                        l_dwMax_Memory
                        );
    }


    ////////////////////////////////////////////////////////////////////////////
    //window size

    //here is possible maximal window length
    m_dwData_Wnd_Max_Count = (SERVER_REPORT_SIZE - SERVER_REPORT_HEADER_SIZE) * 8;

    //if window size is more than half of all memory we resize the window length
    if ( ((tUINT64)m_dwData_Wnd_Max_Count * (tUINT64)l_dwPacket_Size) > (l_dwMax_Memory / 2) )
    {
        m_dwData_Wnd_Max_Count = (l_dwMax_Memory / l_dwPacket_Size) / 2;
    }
    
    //if window size is more than 2Mb, truncate to 2Mb
    if (((tUINT64)m_dwData_Wnd_Max_Count * (tUINT64)l_dwPacket_Size) > 0x200000)
    {
        m_dwData_Wnd_Max_Count = 0x200000 / l_dwPacket_Size;
    }

    
    l_pArg_Value = Get_Argument_Text_Value(i_pHC_Args, i_iHC_Count, 
                                           i_pCn_Args, i_iCn_Count,
                                           (tXCHAR*)CLIENT_COMMAND_WINDOW_SIZE);
    if (l_pArg_Value)
    {
        tUINT32 l_dwCount = (tUINT32)PStrToInt(l_pArg_Value);

        if (1 > l_dwCount)
        {
            l_dwCount = 1;
        }
        
        if (l_dwCount > m_dwData_Wnd_Max_Count)
        {
            l_dwCount = m_dwData_Wnd_Max_Count;
        }
        
        m_dwData_Wnd_Max_Count = l_dwCount;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //initialize pool
    m_pBPool = new CBuffers_Pool(m_pLog, 
                                 l_dwMax_Memory / 10,   //10% 
                                 l_dwMax_Memory,        //100%
                                 l_dwPacket_Size, 
                                 0
                                );

    if (    (NULL  != m_pBPool)
         && (FALSE == m_pBPool->Get_Initialized())
       )
    {
        JOURNAL_CRITICAL(m_pLog, TM("Pool initialization failed"));

        delete m_pBPool;
        m_pBPool = NULL;
    }

    JOURNAL_INFO(m_pLog, 
                 TM("Pool: Max memory = %d, Packet size = %d, Window length = %d"),
                 l_dwMax_Memory,
                 l_dwPacket_Size,
                 m_dwData_Wnd_Max_Count
                );

    return (NULL != m_pBPool) ? ECLIENT_STATUS_OK : ECLIENT_STATUS_INTERNAL_ERROR;
}//Init_Pool


////////////////////////////////////////////////////////////////////////////////
//Init_Members
eClient_Status CClient::Init_Members(tXCHAR **i_pHC_Args,
                                     tINT32   i_iHC_Count,
                                     tXCHAR **i_pCn_Args,
                                     tINT32   i_iCn_Count
                                    )
{
    eClient_Status l_eReturn = ECLIENT_STATUS_OK;

    UNUSED_ARG(i_pHC_Args);
    UNUSED_ARG(i_iHC_Count);
    UNUSED_ARG(i_pCn_Args);
    UNUSED_ARG(i_iCn_Count);

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        LOCK_CREATE(m_hCS_Data_Out);
        LOCK_CREATE(m_hCS_User);
        LOCK_CREATE(m_hCS_Data_In);
        LOCK_CREATE(m_hCS_Channels);
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        m_pData_Queue_Out = new CBList<CTPacket*>();
        if (NULL == m_pData_Queue_Out)
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_CRITICAL(m_pLog, TM("Data queue [out] initialization failed"));
        }
    }


    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        m_pData_Queue_In = new CBList<CTPacket*>();
        if (NULL == m_pData_Queue_In)
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_CRITICAL(m_pLog, TM("Data queue [in] initialization failed"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        m_pData_Wnd = new CBList<CTPacket*>();
        if (NULL == m_pData_Wnd)
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_CRITICAL(m_pLog, TM("Data lagoon initialization failed"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        tUINT32 l_dwHTime = 0;
        tUINT32 l_dwLTime = 0;
        tWCHAR  l_pProc_Name[128];
        
        l_pProc_Name[0] = 0;
        
        CProc::Get_Process_Time(&l_dwHTime, &l_dwLTime);
        CProc::Get_Process_Name(l_pProc_Name, LENGTH(l_pProc_Name));

        m_cPacket_Hello.Fill(CLIENT_PROTOCOL_VERSION,
                            (tUINT16)m_pBPool->Get_Buffer_Size(),
                            CProc::Get_Process_ID(),
                            l_dwHTime,
                            l_dwLTime,
                            l_pProc_Name
                           );

        Inc_Packet_ID(&m_dwLast_Packet_ID);
        m_cPacket_Hello.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

        m_pPacket_Control = static_cast<CTPacket*>(&m_cPacket_Hello);
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cComm_Event.Init(1, EMEVENT_SINGLE_AUTO))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == CThShell::Create(&Static_Comm_Routine,
                                      this,
                                      &m_hComm_Thread,
                                      TM("P7:Comm") 
                                     )
           )
        {
            l_eReturn      = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Communication thread wasn't created !"));
        }
        else
        {
            m_bComm_Thread = TRUE;
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == m_cChnl_Event.Init(2, EMEVENT_SINGLE_AUTO, EMEVENT_MULTI))
        {
            l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Exit event wasn't created !"));
        }
    }

    if (ECLIENT_STATUS_OK == l_eReturn)
    {
        if (FALSE == CThShell::Create(&Static_Chnl_Routine, 
                                      this,
                                      &m_hChnl_Thread,
                                      TM("P7:Channel")               
                                     )
           )
        {
            l_eReturn      = ECLIENT_STATUS_INTERNAL_ERROR;
            JOURNAL_ERROR(m_pLog, TM("Channel thread wasn't created !"));
        }
        else
        {
            m_bChnl_Thread = TRUE;
        }
    }


    return l_eReturn;
}//Init_Members


////////////////////////////////////////////////////////////////////////////////
//Inc_Packet_ID
void CClient::Inc_Packet_ID(tUINT32 * o_pPacketID)
{
    ++(*o_pPacketID);
    if ( TPACKET_MAX_ID < (*o_pPacketID) )
    {
        (*o_pPacketID) = 1;
    }
}//Inc_Packet_ID


////////////////////////////////////////////////////////////////////////////////
//Process_Incoming_Packet
tBOOL CClient::Process_Incoming_Packet(CTPacket *i_pPacket)
{
    tBOOL l_bResult = TRUE;

    if (NULL == i_pPacket)
    {
        l_bResult = FALSE;
    }

    //check packet integrity
    if (    (l_bResult) 
         && (TRUE == i_pPacket->Is_Damaged())
       )
    {
        JOURNAL_ERROR(m_pLog,
                      TM("Packet is corrupted (CRC32) Packet ID = %d"), 
                      i_pPacket->Get_ID()
                     );

        m_bIs_Response_Waiting = FALSE;
        m_dwDelivery_Fails ++;
        l_bResult = FALSE;
    }

    if (FALSE == m_bIs_Response_Waiting)
    {
        l_bResult = FALSE;
        JOURNAL_WARNING(m_pLog,
                        TM("Drop packet, ID = %d"), 
                        i_pPacket->Get_ID()
                       );
    }

    if (l_bResult)
    {
        m_wClient_ID = i_pPacket->Get_Client_ID();
    }

    if (l_bResult)
    {
        eTPacket_Type l_eType = i_pPacket->Get_Type();
        if (ETPT_ACKNOWLEDGMENT == l_eType) 
        {
            CTPAcknowledgment l_cServerResponse(i_pPacket);

            //verify that response source ID is equal to control ID
            if (    (m_pPacket_Control) 
                 && (l_cServerResponse.Get_Source_ID() == m_pPacket_Control->Get_ID()) 
               )
            {
                m_bIs_Response_Waiting = FALSE;
                m_pData_Wnd_Cell       = NULL;
                m_dwDelivery_Fails     = 0;

                if (l_cServerResponse.Get_Result())
                {
                    //if this is response to data report packet ... 
                    if (ETPT_CLIENT_DATA_REPORT == m_pPacket_Control->Get_Type())
                    {
                        pAList_Cell l_pElement = NULL;
                        while ((l_pElement = m_pData_Wnd->Get_First()) != NULL)
                        {
                            CTPacket * l_pPacket = m_pData_Wnd->Get_Data(l_pElement);
                            m_pData_Wnd->Del(l_pElement, FALSE);
                            m_pBPool->Push_Buffer(l_pPacket);
                        }

                        m_dwData_Wnd_First_ID = TPACKET_MAX_ID + 1;
                        m_dwData_Wnd_Last_ID  = TPACKET_MAX_ID + 1;
                        m_dwData_Wnd_Size     = 0;
                        m_bData_Resending     = FALSE;
                    }

                    //control packet is local class packet
                    //it is not necessary to return it to pool
                    m_pPacket_Control = NULL; 
                }
                else //server set this flag to 0 when he not recognize this client
                {
                    Reset_Connetion();
                }

                if (TPACKET_FLAG_EXTRA_DATA & i_pPacket->Get_Flags())
                {
                    LOCK_ENTER(m_hCS_Data_In); 
                    m_pData_Queue_In->Add_After(m_pData_Queue_In->Get_Last(), i_pPacket);
                    LOCK_EXIT(m_hCS_Data_In); 

                    m_cChnl_Event.Set(THREAD_DATA_SIGNAL);

                    i_pPacket = NULL;
                }
            }
        }
        else if (ETPT_SERVER_DATA_REPORT == l_eType)
        {
            CTPData_Window_Report l_cServerReport(i_pPacket);

            if (l_cServerReport.Get_Source_ID() == m_pPacket_Control->Get_ID() )
            {
                tUINT32 l_dwTotal      = m_pData_Wnd->Count();
                m_bIs_Response_Waiting = FALSE;
                m_pData_Wnd_Cell       = NULL;
                m_bData_Resending      = TRUE;

                pAList_Cell l_pEl      = m_pData_Wnd->Get_First();
                pAList_Cell l_pEl_Exch = NULL;
                while (l_pEl)
                {
                    CTPacket *l_pPacket = m_pData_Wnd->Get_Data(l_pEl);

                    l_pEl_Exch = m_pData_Wnd->Get_Next(l_pEl);

                    if (FALSE == l_cServerReport.Is_ID(m_cPacket_Data_Report.Get_First_ID(), 
                                                       l_pPacket->Get_ID()
                                                      )
                       )
                    {
                        m_pData_Wnd->Del(l_pEl, FALSE);
                        m_pBPool->Push_Buffer(l_pPacket);
                    }

                    l_pEl = l_pEl_Exch;
                }

                if (m_pData_Wnd->Count())
                {
                    JOURNAL_ERROR(m_pLog,
                                  TM("Server report: count of lost packets: %d, total: %d"), 
                                  m_pData_Wnd->Count(),
                                  l_dwTotal
                                 );
                }
                else
                {
                    m_bData_Resending = FALSE;
                    JOURNAL_ERROR(m_pLog,
                                  TM("Server report: empty report is received !")
                                 );
                }

                m_pPacket_Control = NULL;
            }
        } // else if (ETPT_SERVER_DATA_REPORT == l_eType)
    }//if (l_bResult)

    if (i_pPacket)
    {
        m_pBPool->Push_Buffer(i_pPacket);
    }

    return l_bResult;
}//Process_Incoming_Packet


////////////////////////////////////////////////////////////////////////////////
//Get_Delivered_Packet
CTPacket * CClient::Get_Delivered_Packet()
{
    CTPacket *l_pReturn = NULL;
    tBOOL     l_bFilled = FALSE;

    if (TRUE == m_bIs_Response_Waiting)
    {
        //if timeout is exceed we flush flag of waiting and resend the same control packet 
        if (CTicks::Difference(GetTickCount(), m_dwServiceTimeStamp) >= COMMUNICATION_RESPONSE_TIMEOUT)
        {
            JOURNAL_ERROR(m_pLog,
                          TM("Server Response waiting timeout. Resending ...")
                         );

            m_bIs_Response_Waiting = FALSE;
            m_dwDelivery_Fails ++;
        }
        else
        {
            l_bFilled = TRUE;
        }
    }

    //Do we have control packet
    if (    (FALSE == l_bFilled) 
         && (m_pPacket_Control)
       )
    {
        m_dwServiceTimeStamp   = GetTickCount();
        l_bFilled              = TRUE;
        m_bIs_Response_Waiting = TRUE;
        l_pReturn              = m_pPacket_Control;
    }

    //if we sending data packets and some limits are exceed .... 
    //we send report packet
    if (    (FALSE == l_bFilled)
         && (FALSE == m_bData_Resending) 
         && (m_pData_Wnd->Count()) 
       )
    {
        if (    (CTicks::Difference(GetTickCount(), m_dwData_Wnd_TimeStamp) >= COMMUNICATION_DATA_SEGMENT_MAX_DURATION) 
             || (m_dwData_Wnd_Max_Count <= m_pData_Wnd->Count())  
           )
        {
            l_pReturn = Create_Data_Wnd_Report();
            l_bFilled = TRUE;
        }
    } //if ((FALSE == l_bFilled) && (0 == m_dwDelivery_Fails))


    if (FALSE == l_bFilled)
    {
        if (FALSE == m_bData_Resending)
        {
            l_pReturn = Pull_Firt_Data_Packet();
            if (l_pReturn)
            {
                //if data window is empty - this is first data packet
                //we need to check ID for overflow, to be sure that all ID 
                //inside data window will be sequential
                if ( (0 == m_pData_Wnd->Count()) &&  
                     ((m_dwLast_Packet_ID + m_dwData_Wnd_Max_Count + 128) >= TPACKET_MAX_ID)
                   )
                {
                    m_dwLast_Packet_ID = 1;
                }
                else
                {
                    Inc_Packet_ID(&m_dwLast_Packet_ID);
                }

                //we store time when we start sending data
                if (0 == m_pData_Wnd->Count())
                {
                    m_dwData_Wnd_TimeStamp = GetTickCount();

                    //JOURNAL_DEBUG(m_pLog,
                    //              L"Start sending data");
                }

                l_pReturn->Finalize(m_dwLast_Packet_ID, m_wClient_ID);
                m_dwData_Wnd_Size   += l_pReturn->Get_Size();
                m_pData_Wnd->Add_After(m_pData_Wnd->Get_Last(), l_pReturn);
                m_dwServiceTimeStamp = GetTickCount();
                l_bFilled            = TRUE;
            }
            else if (m_pData_Wnd->Count())
            {
                l_pReturn = Create_Data_Wnd_Report();
                l_bFilled = TRUE;
            }
        }//if (FALSE == m_bData_Resending)
        else if (m_pData_Wnd->Count()) 
        {
             //if redeliver all packets from data window
             if (m_pData_Wnd_Cell == m_pData_Wnd->Get_Last()) 
             {
                 if (    (TPACKET_MAX_ID > m_dwData_Wnd_First_ID) 
                      && (TPACKET_MAX_ID > m_dwData_Wnd_Last_ID)
                    )
                 {
                     l_pReturn = &m_cPacket_Data_Report;
                     m_pPacket_Control = &m_cPacket_Data_Report;

                     Inc_Packet_ID(&m_dwLast_Packet_ID);

                     m_cPacket_Data_Report.Fill(m_dwData_Wnd_First_ID, m_dwData_Wnd_Last_ID);
                     m_cPacket_Data_Report.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

                     m_bIs_Response_Waiting = TRUE;
                     m_dwServiceTimeStamp   = GetTickCount();
                     m_pData_Wnd_Cell       = NULL;
                     l_bFilled              = TRUE;
                 }
             }
             else
             {
                 m_pData_Wnd_Cell     = m_pData_Wnd->Get_Next(m_pData_Wnd_Cell);
                 l_pReturn            = m_pData_Wnd->Get_Data(m_pData_Wnd_Cell);
                 m_dwServiceTimeStamp = GetTickCount();
                 l_bFilled            = TRUE;
             }
        }//else if (m_pData_Wnd->Count()) 

    } //if (FALSE == l_bFilled)



    //ehhh, nothing to send ... it is sooooo sad.
    //let's sent alive packet.
    if (    (FALSE == l_bFilled)
         && (0 == m_pData_Wnd->Count()) 
         && (CTicks::Difference(GetTickCount(), m_dwServiceTimeStamp) >= COMMUNICATION_IDLE_TIMEOUT)
       )
    {

        l_pReturn = &m_cPacket_Alive;
        Inc_Packet_ID(&m_dwLast_Packet_ID);
        l_pReturn->Finalize(m_dwLast_Packet_ID, m_wClient_ID);
        m_bIs_Response_Waiting = TRUE;
        m_dwServiceTimeStamp   = GetTickCount();
        l_bFilled              = TRUE;
        m_pPacket_Control      = l_pReturn;
    }

    return l_pReturn;
}//Get_Delivered_Packet


////////////////////////////////////////////////////////////////////////////////
//Is_Ready_To_Exit
tBOOL CClient::Is_Ready_To_Exit()
{
    tBOOL l_bResult = TRUE;
    if (FALSE == m_bConnected)
    {
        return TRUE;
    }

    LOCK_ENTER(m_hCS_Data_Out);

    l_bResult = ( (0 == m_pData_Queue_Out->Count()) && (0 == m_pData_Wnd->Count()) );

    LOCK_EXIT(m_hCS_Data_Out);

    return l_bResult;
}//Is_Ready_To_Exit


////////////////////////////////////////////////////////////////////////////////
//Set_Connected
void CClient::Set_Connected(tBOOL i_bConnected)
{
    if (i_bConnected == m_bConnected)
    {
        return;
    }

    LOCK_ENTER(m_hCS_User); 

    m_bConnected  = i_bConnected;

    if (i_bConnected)
    {
        m_dwConnection_Resets ++;
    }

    LOCK_EXIT(m_hCS_User); 

    JOURNAL_ERROR(m_pLog,
                  TM("Set connection : %s"), 
                  (m_bConnected) ? TM("ON") : TM("OFF")
                 );
}//Set_Connection_Status


////////////////////////////////////////////////////////////////////////////////
//Reset_Connetion
void CClient::Reset_Connetion()
{
    CTPacket * l_pPacket = NULL;

    JOURNAL_WARNING(m_pLog, TM("Reset Connection"));

    if (m_pPacket_Control)
    {
        m_pPacket_Control = NULL;
    }

    //m_dwDelivery_Fails = 0;
    m_bIs_Response_Waiting  = FALSE;
    m_pData_Wnd_Cell        = NULL;
    m_dwData_Wnd_First_ID   = TPACKET_MAX_ID + 1;
    m_dwData_Wnd_Last_ID    = TPACKET_MAX_ID + 1;
    m_dwData_Wnd_Size       = 0;
    m_bData_Resending       = FALSE;


    pAList_Cell l_pElement = NULL;
    while ((l_pElement = m_pData_Wnd->Get_First()) != NULL)
    {
        l_pPacket = m_pData_Wnd->Get_Data(l_pElement);
        m_pData_Wnd->Del(l_pElement, FALSE);
        m_pBPool->Push_Buffer(l_pPacket);
    }

    //protect list against user calls
    LOCK_ENTER(m_hCS_User);
    while ((l_pPacket = Pull_Firt_Data_Packet()) != NULL)
    {
        m_pBPool->Push_Buffer(l_pPacket);
    }

    m_dwConnection_Resets ++;

    LOCK_EXIT(m_hCS_User);

    Inc_Packet_ID(&m_dwLast_Packet_ID);
    m_cPacket_Hello.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

    m_pPacket_Control = static_cast<CTPacket*>(&m_cPacket_Hello);
}//Reset_Connetion


////////////////////////////////////////////////////////////////////////////////
//RoutineCreate_Data_Wnd_Report
CTPacket * CClient::Create_Data_Wnd_Report()
{
    CTPacket *l_pResult = NULL;

    m_dwData_Wnd_First_ID = TPACKET_MAX_ID + 1;
    m_dwData_Wnd_Last_ID = TPACKET_MAX_ID + 1;

    CTPacket *l_pLagoonPacket = m_pData_Wnd->Get_Data(m_pData_Wnd->Get_First());
    if (l_pLagoonPacket)
    {
        m_dwData_Wnd_First_ID = l_pLagoonPacket->Get_ID();
    }

    l_pLagoonPacket = m_pData_Wnd->Get_Data(m_pData_Wnd->Get_Last());
    if (l_pLagoonPacket)
    {
        m_dwData_Wnd_Last_ID = l_pLagoonPacket->Get_ID();
    }

    if (    (TPACKET_MAX_ID > m_dwData_Wnd_First_ID) 
         && (TPACKET_MAX_ID > m_dwData_Wnd_Last_ID)
       )
    {
        Inc_Packet_ID(&m_dwLast_Packet_ID);
        m_cPacket_Data_Report.Fill(m_dwData_Wnd_First_ID, m_dwData_Wnd_Last_ID);
        m_cPacket_Data_Report.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

        l_pResult              = &m_cPacket_Data_Report;
        m_bIs_Response_Waiting = TRUE;
        m_dwServiceTimeStamp   = GetTickCount();
        m_pPacket_Control      = l_pResult;
    }

    return l_pResult;
}//RoutineCreate_Data_Wnd_Report


////////////////////////////////////////////////////////////////////////////////
//Pull_Firt_Data_Packet
CTPacket *CClient::Pull_Firt_Data_Packet()
{
    CTPacket * l_pResult = NULL;
    
    LOCK_ENTER(m_hCS_Data_Out); 

    pAList_Cell l_pElement = m_pData_Queue_Out->Get_First();
    if (l_pElement)
    {
        l_pResult = m_pData_Queue_Out->Get_Data(l_pElement);
        m_pData_Queue_Out->Del(l_pElement, FALSE);
    }

    LOCK_EXIT(m_hCS_Data_Out); 

    return l_pResult;
}//Pull_Firt_Data_Packet


////////////////////////////////////////////////////////////////////////////////
//Pull_Last_Data_Packet
CTPacket *CClient::Pull_Last_Data_Packet()
{
    CTPacket * l_pResult = NULL;
    
    LOCK_ENTER(m_hCS_Data_Out); 

    pAList_Cell l_pElement = m_pData_Queue_Out->Get_Last();
    if (l_pElement)
    {
        l_pResult = m_pData_Queue_Out->Get_Data(l_pElement);
        m_pData_Queue_Out->Del(l_pElement, FALSE);
    }

    LOCK_EXIT(m_hCS_Data_Out); 

    return l_pResult;
}//Pull_Last_Data_Packet


////////////////////////////////////////////////////////////////////////////////
//Push_Last_Data_Packet
tBOOL CClient::Push_Last_Data_Packet(CTPacket *i_pPacket)
{
    if (NULL == i_pPacket)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS_Data_Out); 

    m_pData_Queue_Out->Add_After(m_pData_Queue_Out->Get_Last(), i_pPacket);

    LOCK_EXIT(m_hCS_Data_Out); 

    return TRUE;
}//Push_Last_Data_Packet


////////////////////////////////////////////////////////////////////////////////
//Comm_Routine
void CClient::Comm_Routine()
{
    tUINT32           l_dwExit_Signal_Time = 0;
    tBOOL             l_bExit_Signal       = FALSE;
    tBOOL             l_bExit              = FALSE;
    tUINT32           l_dwSignal           = MEVENT_TIME_OUT;
    tUINT32           l_dwWait_TimeOut     = COMMUNICATION_THREAD_IDLE_TIMEOUT;
    CTPacket         *l_pReceived_Packet   = NULL;
    CTPacket         *l_pSent_Packet       = NULL;
    tUINT32           l_dwReceived         = 0;
    tBOOL             l_bConnected_Cur     = FALSE;
    sockaddr_storage  l_tAddress;

    memset(&l_tAddress, 0, sizeof(l_tAddress));

    while (FALSE == l_bExit)
    {
        l_dwSignal = m_cComm_Event.Wait(l_dwWait_TimeOut);

        if (THREAD_EXIT_SIGNAL == l_dwSignal)
        {
            l_dwExit_Signal_Time = GetTickCount();
            l_bExit_Signal       = TRUE;
        }

        //have to close thread ....
        if ( l_bExit_Signal )
        {
            if (Is_Ready_To_Exit()) 
            {
                Inc_Packet_ID(&m_dwLast_Packet_ID);
                m_cPacket_Bye.Finalize(m_dwLast_Packet_ID, m_wClient_ID);

                if (UDP_SOCKET_OK != m_pSocket->Send((char*)m_cPacket_Bye.Get_Buffer(), 
                                                      m_cPacket_Bye.Get_Size())
                   )
                {
                    JOURNAL_ERROR(m_pLog, TM("<<< m_pSocket->Send Failed"));
                }

                l_bExit = TRUE;
            }
            else if (CTicks::Difference(GetTickCount(), l_dwExit_Signal_Time) > COMMUNICATION_THREAD_EXIT_TIMEOUT)
            {
                JOURNAL_ERROR(m_pLog, 
                              TM("Exit imeout is exceed, not of all packets were sent ! "));
                l_bExit = TRUE;
            }
        }

        if (    (MEVENT_TIME_OUT == l_dwSignal)
             && (FALSE == l_bExit) 
           )
        {
            //initiate timeout value, if we receive something or will be ready to send we will put 0 as value
            l_dwWait_TimeOut = COMMUNICATION_THREAD_IDLE_TIMEOUT;

            if (NULL == l_pReceived_Packet)
            {
                l_pReceived_Packet = m_pBPool->Pull_Buffer();
            }

            if (l_pReceived_Packet)
            {
                l_dwReceived = 0;
                if (UDP_SOCKET_OK == m_pSocket->Recv(&l_tAddress, 
                                                      (char*)l_pReceived_Packet->Get_Buffer(), 
                                                      l_pReceived_Packet->Get_Buffer_Size(), 
                                                      &l_dwReceived,
                                                      m_bIs_Response_Waiting ? SOCKET_RECEIVE_RESPONSE_TIMEOUT : 0
                                                     )
                   )
                {
                    if (    (l_dwReceived) 
                         && (FALSE == m_pSocket->Is_Server_Address(&l_tAddress)) 
                       )
                    {
                        l_dwReceived = 0;
                        JOURNAL_ERROR(m_pLog, 
                                      TM("<<<: Packet from wrong adress !")); 
                    }

                    if (l_dwReceived)
                    {
                        PACKET_PRINT(m_pLog, TM("[INP]"), l_pReceived_Packet);

                        //l_pPacket will be freed inside function or stored ...
                        Process_Incoming_Packet(l_pReceived_Packet);

                        ////////////////////////////////////////////////////////
                        // We pass packet to the stream, now packet it is head 
                        // pain of the stream, we can forget about it
                        ////////////////////////////////////////////////////////
                        l_pReceived_Packet = NULL;
                        l_dwWait_TimeOut = 0;
                    }
                }
            }

            while ((l_pSent_Packet = Get_Delivered_Packet()) != NULL)
            {
                l_dwWait_TimeOut = 0;
                PACKET_PRINT(m_pLog, TM("[OUT]"), l_pSent_Packet);

                if (UDP_SOCKET_OK != m_pSocket->Send((char*)l_pSent_Packet->Get_Buffer(), 
                                                      l_pSent_Packet->Get_Size())
                   )
                {
                    //nothing to do ... continue sending ....
                    //error message will be printed inside socket object
                }
            }

            l_bConnected_Cur = m_bConnected;

            if (COMMUNICATION_MAX_DELIVERY_FAILS <= m_dwDelivery_Fails)
            {
                l_bConnected_Cur = FALSE;
            }
            else
            {
                l_bConnected_Cur = TRUE;
            }

            if (l_bConnected_Cur != m_bConnected)
            {
                if (FALSE == l_bConnected_Cur)
                {
                    Set_Connected(FALSE);
                    Reset_Connetion();
                    JOURNAL_INFO(m_pLog, TM("------->Drop Connection<-------"));
                }
                else
                {
                    Set_Connected(TRUE);
                    JOURNAL_INFO(m_pLog, TM("<-------Establish Connection------->"));
                }
            }
        } // if (WAIT_TIMEOUT == l_dwSignal)
    }//while (FALSE == l_bExit)


    if (l_pReceived_Packet)
    {
        m_pBPool->Push_Buffer(l_pReceived_Packet);
        l_pReceived_Packet = NULL;
    }
}//Comm_Routine


////////////////////////////////////////////////////////////////////////////////
//Chnl_Routine
void CClient::Chnl_Routine()
{
    tBOOL              l_bExit         = FALSE;
    tUINT32            l_dwSignal      = 0;
    pAList_Cell        l_pElement      = NULL;
    CTPacket          *l_pPacket       = NULL;
    tUINT8            *l_pBuffer       = NULL;
    tUINT8            *l_pStop         = NULL;
    sH_User_Data      *l_pHeader       = NULL;
    IP7C_Channel      *l_pChannel      = NULL;

    while (FALSE == l_bExit)
    {
        l_dwSignal = m_cChnl_Event.Wait(INFINITE);

        if (THREAD_EXIT_SIGNAL == l_dwSignal)
        {
            l_bExit = TRUE;
        }
        else if (THREAD_DATA_SIGNAL == l_dwSignal)
        {
            l_pPacket = NULL;

            LOCK_ENTER(m_hCS_Data_In); 
            l_pElement = m_pData_Queue_In->Get_First();
            if (l_pElement)
            {
                l_pPacket = m_pData_Queue_In->Get_Data(l_pElement);
                m_pData_Queue_In->Del(l_pElement, FALSE);
            }

            LOCK_EXIT(m_hCS_Data_In); 

            if (l_pPacket)
            {
                l_pBuffer = l_pPacket->Get_Buffer() + ACKNOWLEDGMENT_SIZE;
                l_pStop   = l_pPacket->Get_Buffer() + l_pPacket->Get_Size();

                LOCK_ENTER(m_hCS_Channels);

                while (l_pBuffer < l_pStop)
                {
                    l_pHeader  = (sH_User_Data*)(l_pBuffer);

                    if (USER_PACKET_CHANNEL_ID_MAX_SIZE > l_pHeader->dwChannel_ID)
                    {
                        l_pChannel = m_pChannels[l_pHeader->dwChannel_ID];
                    }

                    if (l_pChannel)
                    {
                        l_pChannel->On_Receive(l_pHeader->dwChannel_ID,
                                               l_pBuffer + sizeof(sH_User_Data), 
                                               l_pHeader->dwSize - sizeof(sH_User_Data)
                                              );
                    }
                    else
                    {
                        JOURNAL_ERROR(m_pLog, 
                                      TM("Channel %d is not registered!"), 
                                      (tUINT32)l_pHeader->dwChannel_ID
                                     );
                    }

                    l_pBuffer += l_pHeader->dwSize;
                }

                LOCK_EXIT(m_hCS_Channels);

                m_pBPool->Push_Buffer(l_pPacket);
                l_pPacket = NULL;
            }
            else
            {
                JOURNAL_ERROR(m_pLog, 
                              TM("Get event [Data In], but no buffers was found!")
                             );
            }
        } //else if ( (WAIT_OBJECT_0 + 1) == l_dwSignal)
    }
}//Chnl_Routine


////////////////////////////////////////////////////////////////////////////////
//Get_Status
tBOOL CClient::Get_Status(sP7C_Status *o_pStatus)
{
    if (NULL == o_pStatus)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS_User); 

    o_pStatus->bConnected = m_bConnected;
    o_pStatus->dwResets   = m_dwConnection_Resets;

    LOCK_EXIT(m_hCS_User); 

    return TRUE;
}//Get_Status


////////////////////////////////////////////////////////////////////////////////
//Get_Info
tBOOL CClient::Get_Info(sP7C_Info *o_pInfo)
{
    if (NULL == o_pInfo)
    {
        return FALSE;
    }

    LOCK_ENTER(m_hCS_User); 

    //memcpy(&(o_pInfo->sServer), m_pSocket->Get_Address(), sizeof(sockaddr_storage));

    m_pBPool->Get_Memory_Info((tUINT32*)&o_pInfo->dwMem_Used, (tUINT32*)&o_pInfo->dwMem_Free);

    o_pInfo->dwReject_Mem = m_lReject_Mem;
    o_pInfo->dwReject_Con = m_lReject_Con;
    o_pInfo->dwReject_Int = m_lReject_Int;

    LOCK_EXIT(m_hCS_User); 

    return TRUE;
}//Get_Info


////////////////////////////////////////////////////////////////////////////////
// Share                                      
tBOOL CClient::Share(const tXCHAR *i_pName)
{
    if (NULL != m_pShared)
    {
        return FALSE;
    }

    void *l_pPointer = static_cast<IP7_Client*>(this);

    m_pShared = Shared_Create(i_pName, (tUINT8*)&l_pPointer, sizeof(l_pPointer));

    return (NULL != m_pShared);
}// Share


////////////////////////////////////////////////////////////////////////////////
//Register_Channel
eClient_Status CClient::Register_Channel(IP7C_Channel *i_pChannel)
{
    eClient_Status  l_eReturn = ECLIENT_STATUS_INTERNAL_ERROR;

    if (NULL == i_pChannel)
    {
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }

    LOCK_ENTER(m_hCS_Channels);

    for (tUINT32 l_dwI = 0; l_dwI < USER_PACKET_CHANNEL_ID_MAX_SIZE; l_dwI++)
    {
        if (NULL == m_pChannels[l_dwI])
        {
            sP7C_Channel_Info l_sInfo = {l_dwI};

            m_pChannels[l_dwI] = i_pChannel;
            i_pChannel->Init(&l_sInfo);

            l_eReturn = ECLIENT_STATUS_OK;

            break;
        }
    }

    LOCK_EXIT(m_hCS_Channels);

    return l_eReturn;
}//Register_Channel


////////////////////////////////////////////////////////////////////////////////
//Unregister_Channel
eClient_Status CClient::Unregister_Channel(tUINT32 i_dwID)
{
    eClient_Status  l_eReturn = ECLIENT_STATUS_OK;

    LOCK_ENTER(m_hCS_Channels);

    if (    (i_dwID >= USER_PACKET_CHANNEL_ID_MAX_SIZE)
         || (NULL   == m_pChannels[i_dwID])
       )
    {
        l_eReturn = ECLIENT_STATUS_WRONG_PARAMETERS;
    }
    else
    {
        m_pChannels[i_dwID] = NULL;
    }

    LOCK_EXIT(m_hCS_Channels);

    return l_eReturn;
}//Unregister_Channel


////////////////////////////////////////////////////////////////////////////////
//Sent
eClient_Status CClient::Sent(tUINT32          i_dwChannel_ID,
                             sP7C_Data_Chunk *i_pChunks, 
                             tUINT32          i_dwCount,
                             tUINT32          i_dwSize)
{
    eClient_Status   l_eReturn       = ECLIENT_STATUS_OK;
    CTPacket        *l_pPacket       = NULL;
    sH_User_Data     l_sHeader       = {i_dwSize + sizeof(l_sHeader), i_dwChannel_ID};
    sP7C_Data_Chunk  l_sHeader_Chunk = {&l_sHeader, sizeof(l_sHeader)};
    tUINT32          l_dwPacket_Size = 0;
    //Wanr: variables without default value!
    tBOOL            l_bExit;
    sP7C_Data_Chunk *l_pChunk;
    tUINT32          l_dwChunk_Offs;
    CTPData          l_cData;

    if (ECLIENT_STATUS_OK != m_eStatus)
    {
        ATOMIC_INC(&m_lReject_Int);
        return m_eStatus;
    }

    if (    (NULL                            == i_pChunks)
         || (0                               >= i_dwCount)
         || (USER_PACKET_MAX_SIZE            <= l_sHeader.dwSize) 
         || (USER_PACKET_CHANNEL_ID_MAX_SIZE <= i_dwChannel_ID)
       )
    {
        //InterlockedIncrement(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }

    //N.B. We do not check i_dwSize and real size of all chunks in release mode!!!
#ifdef _DEBUG
    tUINT32 l_dwReal_Size = 0;
    for (tUINT32 l_dwI = 0; l_dwI < i_dwCount; l_dwI ++)
    {
        if (i_pChunks[l_dwI].pData)
        {
            l_dwReal_Size += i_pChunks[l_dwI].dwSize;
        }
        else
        {
            break;
        }
    }

    if (l_dwReal_Size != i_dwSize)
    {
        ATOMIC_INC(&m_lReject_Int);
        return ECLIENT_STATUS_WRONG_PARAMETERS;
    }
#endif


    LOCK_ENTER(m_hCS_User);

    if (FALSE == m_bConnected)
    {
        ATOMIC_INC(&m_lReject_Con);
        l_eReturn = ECLIENT_STATUS_OFF;
        goto l_lExit;
    }

    l_dwPacket_Size = m_pBPool->Get_Buffer_Size() - CLIENT_DATA_HEADER_SIZE;

    //if size is more than available ...
    //If we will use all buffers from Pool for data - we can cause DOS.
    //no packets in Pool for incoming data from server, communication is broken
    if (   (l_sHeader.dwSize + (l_dwPacket_Size * 3) )
         > (l_dwPacket_Size * m_pBPool->Get_Free_Count())
       )
    {
        ATOMIC_INC(&m_lReject_Mem);
        l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
        goto l_lExit;
    }

    //because we add header as chunk
    i_dwCount ++;

    //extract last packet form data queue, maybe it contain free space ?! :-)
    l_pPacket = Pull_Last_Data_Packet();

    if (l_pPacket)
    {
        l_cData.Attach(l_pPacket);
        if (0 >= l_cData.Get_Tail_Size())
        {
            l_cData.Detach();
            Push_Last_Data_Packet(l_pPacket);
            l_pPacket = NULL;
        }
    }

    l_bExit        = FALSE;
    l_pChunk       = &l_sHeader_Chunk; //&i_pChunks[0];
    l_dwChunk_Offs = 0;
    
    while (FALSE == l_bExit)
    {
        //if packet is null we need to extract another one 
        if (NULL == l_pPacket)
        {
            l_pPacket = m_pBPool->Pull_Buffer();
            if (l_pPacket)
            {
                l_cData.Attach(l_pPacket);
                l_cData.Initialize();
            }
            else
            {
                l_bExit   = TRUE;
                ATOMIC_INC(&m_lReject_Mem);
                l_eReturn = ECLIENT_STATUS_NO_FREE_BUFFERS;
            }
        }

        while (    (l_pPacket)
                && (i_dwCount)
              )
        {
            //if packet free size is larger or equal to chunk size
            if ( l_cData.Get_Tail_Size() >= l_pChunk->dwSize )
            {
                memcpy(l_cData.Get_Tail(), 
                       ((tUINT8*)l_pChunk->pData) + l_dwChunk_Offs,
                       l_pChunk->dwSize
                      );

                l_cData.Append_Size((tUINT16)l_pChunk->dwSize );

                //current chunk was moved, we reduce chunks amount 
                --i_dwCount;

                //if there is no more chunks - move packet to queue
                if (0 >= i_dwCount)
                {
                    Push_Last_Data_Packet(l_pPacket);
                    l_cData.Detach();
                    l_pPacket = NULL;
                    l_bExit   = TRUE;
                }
                else
                {
                    //we are finish with that chunk
                    //l_pChunk->dwSize = 0; 
                    l_dwChunk_Offs = 0;

                    if (l_pChunk == &l_sHeader_Chunk)
                    {
                        l_pChunk = &i_pChunks[0];
                    }
                    else
                    {
                        //go to next chunk
                        l_pChunk ++;
                    }

                    //if packet is filled - put it to data queue
                    if (0 >= l_cData.Get_Tail_Size())
                    {
                        Push_Last_Data_Packet(l_pPacket);
                        l_cData.Detach();
                        l_pPacket = NULL;
                    }
                }
            }
            else //if chunk data is greater than packet free space
            {
                memcpy(l_cData.Get_Tail(), 
                       ((tUINT8*)l_pChunk->pData) + l_dwChunk_Offs,
                       l_cData.Get_Tail_Size()
                      );
                l_dwChunk_Offs   += l_cData.Get_Tail_Size();
                l_pChunk->dwSize -= l_cData.Get_Tail_Size();

                l_cData.Append_Size(l_cData.Get_Tail_Size());

                Push_Last_Data_Packet(l_pPacket);
                l_cData.Detach();
                l_pPacket = NULL;
            }
        } //while ( (l_pPacket) && (i_dwCount) )
    } //while (FALSE == l_bExit)


l_lExit:

    LOCK_EXIT(m_hCS_User);

    return l_eReturn;

}//Sent
