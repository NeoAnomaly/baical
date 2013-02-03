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
#ifndef CLIENT_H_AZH
#define CLIENT_H_AZH

#define COMMUNICATION_THREAD_IDLE_TIMEOUT                                 (5)
#define COMMUNICATION_THREAD_EXIT_TIMEOUT                                 (5000)
#define COMMUNICATION_RESPONSE_TIMEOUT                                    (500)
#define COMMUNICATION_DATA_SEGMENT_MAX_DURATION                           (750)
#define COMMUNICATION_IDLE_TIMEOUT                                        (1000)
#define COMMUNICATION_MAX_DELIVERY_FAILS                                  (10)
#define SOCKET_RECEIVE_RESPONSE_TIMEOUT                                   (10) 


#define  THREAD_EXIT_SIGNAL                                (MEVENT_SIGNAL_0)
#define  THREAD_DATA_SIGNAL                                (MEVENT_SIGNAL_0 + 1)


class CClient:
    public IP7_Client
{
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile      m_lReference;

    tINT32 volatile      m_lReject_Mem;
    tINT32 volatile      m_lReject_Con;
    tINT32 volatile      m_lReject_Int;

    tBOOL                m_bIs_Winsock;
    CUDP_Socket         *m_pSocket;
    tUINT16              m_wClient_ID;
    IJournal            *m_pLog;
    CBuffers_Pool       *m_pBPool;
    eClient_Status       m_eStatus;

    CBList<CTPacket*>   *m_pData_Wnd;
    pAList_Cell          m_pData_Wnd_Cell;
    tUINT32              m_dwData_Wnd_Max_Count;
    tUINT32              m_dwData_Wnd_Size;
    tUINT32              m_dwData_Wnd_TimeStamp;
    tUINT32              m_dwData_Wnd_First_ID;
    tUINT32              m_dwData_Wnd_Last_ID;
    tBOOL                m_bData_Resending;

    tUINT32              m_dwDelivery_Fails;
    tBOOL                m_bIs_Response_Waiting;

    CBList<CTPacket*>   *m_pData_Queue_Out;
    //need to synchronize updating m_pData_Queue by user and requests from internal threads
    tLOCK                m_hCS_Data_Out; 

    CBList<CTPacket*>   *m_pData_Queue_In;
    tLOCK                m_hCS_Data_In; 
    tLOCK                m_hCS_Channels; 

    tUINT32              m_dwLast_Packet_ID;
    //need to synchronize multiple calls PutLogPacket and GetLogPacket from different threads
    tLOCK                m_hCS_User; 
    tUINT32              m_dwServiceTimeStamp;
    tBOOL                m_bConnected;
    tUINT32              m_dwConnection_Resets;

    //permanent packets, they do not consume a lot of memory and more 
    //practical have them always instead of taking them from pool
    CTPacket            *m_pPacket_Control;
    CTPClient_Hello      m_cPacket_Hello;
    CTPData_Window       m_cPacket_Data_Report;
    CTPClient_Ping       m_cPacket_Alive;
    CTPClient_Bye        m_cPacket_Bye;

    IP7C_Channel        *m_pChannels[USER_PACKET_CHANNEL_ID_MAX_SIZE];

    CMEvent              m_cComm_Event;
    tBOOL                m_bComm_Thread;
    CThShell::tTHREAD    m_hComm_Thread;

    CMEvent              m_cChnl_Event;
    tBOOL                m_bChnl_Thread;
    CThShell::tTHREAD    m_hChnl_Thread;

public:
    CClient(tXCHAR *i_pArgs);
    ~CClient();

private:
    tXCHAR        *Get_Argument_Text_Value(tXCHAR **i_pArg, 
                                           tINT32   i_iCount,
                                           tXCHAR *i_pName
                                          );

    eClient_Status Init_Base(tXCHAR **i_pArg, tINT32 i_iCount);
    eClient_Status Init_Log(tXCHAR **i_pArg, tINT32 i_iCount);
    eClient_Status Init_Sockets(tXCHAR **i_pArg, tINT32 i_iCount);
    eClient_Status Init_Pool(tXCHAR **i_pArg, tINT32 i_iCount);
    eClient_Status Init_Members(tXCHAR **i_pArg, tINT32 i_iCount);
    eClient_Status Init_Threads(tXCHAR **i_pArg, tINT32 i_iCount);

    void           Inc_Packet_ID(tUINT32 * o_pPacketID);

    tBOOL          Process_Incoming_Packet(CTPacket *i_pPacket);
    CTPacket      *Get_Delivered_Packet();
    tBOOL          Is_Ready_To_Exit();

    void           Set_Connected(tBOOL i_bConnected);

    void           Reset_Connetion();
    CTPacket      *Create_Data_Wnd_Report();
    CTPacket      *Pull_Firt_Data_Packet();
    CTPacket      *Pull_Last_Data_Packet();
    tBOOL          Push_Last_Data_Packet(CTPacket *i_pPacket);

    void           Comm_Routine();
    void           Chnl_Routine();

public:
    eClient_Status Get_Status()
    {
        return m_eStatus;
    }

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

    eClient_Status Register_Channel(IP7C_Channel *i_pChannel);
    eClient_Status Unregister_Channel(tUINT32 i_dwID);

    eClient_Status Sent(tUINT32            i_dwChannel_ID,
                        sP7C_Data_Chunk   *i_pChunks, 
                        tUINT32            i_dwCount,
                        tUINT32            i_dwSize
                       );

    tBOOL Get_Status(sP7C_Status *o_pStatus);
    tBOOL Get_Info(sP7C_Info *o_pInfo);

private:
    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Comm_Routine(void *i_pContext)
    {
        CClient *l_pRoutine = static_cast<CClient *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->Comm_Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 

    static THSHELL_RET_TYPE THSHELL_CALL_TYPE Static_Chnl_Routine(void *i_pContext)
    {
        CClient *l_pRoutine = static_cast<CClient *>(i_pContext);
        if (l_pRoutine)
        {
            l_pRoutine->Chnl_Routine();
        }

        CThShell::Cleanup();
        return THSHELL_RET_OK;
    } 
};

#endif //CLIENT_H_AZH