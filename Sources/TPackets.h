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
// This header file describe all packets types which are used in communication /
// N.B.: In this file I remove all checks for valid pointers and packets       /
//       because this job is made outside by packet pool.                      /
////////////////////////////////////////////////////////////////////////////////
#ifndef TACKETS_H_AZH
#define TACKETS_H_AZH

//this value can store 32 bits but to avoid overflow we reduce it
#define TPACKET_MAX_ID                                              (0xFFFFFFF) 
#define TPACKET_PROCESS_NAME_MAX_LEN                                (96) 


////////////////////////////////////////////////////////////////////////////////
//Transport Packet types, we have only 4 bits for them (16 types max)
enum eTPacket_Type
{
    ETPT_CLIENT_HELLO             = 0x0,    //tH_Common + tH_Client_Initial
    ETPT_CLIENT_PING              = 0x1,    //tH_Common
    ETPT_CLIENT_DATA              = 0x2,
    ETPT_CLIENT_DATA_REPORT       = 0x3,
    ETPT_CLIENT_BYE               = 0x4,
    
    ETPT_ACKNOWLEDGMENT           = 0x9,
    ETPT_SERVER_DATA_REPORT       = 0xA,
    ETPT_SERVER_CHAINED           = 0xB,

    ETPACKET_TYPE_UNASSIGNED      = 0xF
};

#define TPACKET_MIN_SIZE                                                 (0x200) 
#define TPACKET_MAX_SIZE                                                (0xFF00) 

////////////////////////////////////////////////////////////////////////////////
//Transport Packet flags
#define TPACKET_FLAG_EXTRA_DATA                                         (0x0001)


////////////////////////////////////////////////////////////////////////////////
//Transport Packet structures


PRAGMA_PACK_ENTER(2) //alignment is now 2, MS only !

//This is common header for every P7 transport packet, it situated at the 
//beginning of the packet, then, depending on Type can be located other headers
struct sH_Common
{
    tUINT32  dwCRC32;
    tUINT32  dwID;
    tUINT16  wType:4;
    tUINT16  wFlags:12;
    //We are working through UDP, so the packet size is limited to 65k. 
    //The minimum guaranteed size of the packet 576 bytes, user size is 512 bytes.
    tUINT16  wSize; 
    tUINT16  wClient_ID;
} ATTR_PACK(2);

struct sH_Client_Hello
{
    tUINT16  wProtocol_Version;
    tUINT16  wData_Max_Size;
    //Process ID and process time are dedicated to matching processes on server
    //side
    tUINT32  dwProcess_ID;
    tUINT32  dwProcess_Start_Time_Hi;
    tUINT32  dwProcess_Start_Time_Lo;
    tWCHAR   pProcess_Name[TPACKET_PROCESS_NAME_MAX_LEN];
} ATTR_PACK(2);


struct sH_Data_Window
{
    tUINT32 dwFirst_ID;
    tUINT32 dwLast_ID;
} ATTR_PACK(2);


struct sH_Packet_Ack
{
    tUINT32  dwSource_ID;
    tUINT16  wResult; // 0 - NOK, 1 - OK
} ATTR_PACK(2);


struct sH_Data_Window_Report
{
    tUINT32 dwSource_ID;
    tUINT8  pID[1]; //Array of the ID, each byte contains 8 ID markers ... bitfield
} ATTR_PACK(2);


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                        User packet  definitions                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// This defines has been moved to P7_Client.h to be visible from outside the 
// library
// #define USER_PACKET_CHANNEL_ID_BITS_COUNT                                    (5)
// #define USER_PACKET_SIZE_BITS_COUNT       (32-USER_PACKET_CHANNEL_ID_BITS_COUNT)
// #define USER_PACKET_MAX_SIZE                  (1 << USER_PACKET_SIZE_BITS_COUNT)
// #define USER_PACKET_CHANNEL_ID_MAX_SIZE (1 << USER_PACKET_CHANNEL_ID_BITS_COUNT)

struct sH_User_Data //user data header
{
    tUINT32 dwSize       :USER_PACKET_SIZE_BITS_COUNT; 
    tUINT32 dwChannel_ID :USER_PACKET_CHANNEL_ID_BITS_COUNT;
} ATTR_PACK(2);

PRAGMA_PACK_EXIT()




////////////////////////////////////////////////////////////////////////////////
// CTPacket for providing basic functionality over tH_Common packet's header
class CTPacket
{
protected:
    tUINT8     *m_pBuffer;  
    tUINT32     m_dwBuffer_Size;
    tBOOL       m_bInitialized;
    tUINT32     m_dwPool_ID;
    tBOOL       m_bShell;
    sH_Common  *m_pHeader;  //point to m_pBuffer    
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::CTPacket
    CTPacket()
       : m_pBuffer(NULL)
       , m_dwBuffer_Size(0)
       , m_bInitialized(TRUE)
       , m_dwPool_ID(0)
       , m_bShell(TRUE)
       , m_pHeader(NULL)
    {
    }//CTPacket::CTPacket


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::CTPacket
    CTPacket(tUINT8 *i_pBuffer,
             tUINT32 i_dwBuffer_Size,
             tUINT32 i_dwPool_ID
            )
       : m_pBuffer(i_pBuffer)
       , m_dwBuffer_Size(i_dwBuffer_Size)
       , m_bInitialized(TRUE)
       , m_dwPool_ID(i_dwPool_ID)
       , m_bShell(TRUE)
       , m_pHeader(NULL)
    {
        if (m_pBuffer)
        {
            m_bInitialized  = TRUE;
            memset(m_pBuffer, 0, m_dwBuffer_Size);
            m_pHeader = (sH_Common*)m_pBuffer;
        }
        else
        {
            m_bInitialized = FALSE;
        }
    }//CTPacket::CTPacket


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::CTPacket
    CTPacket(tUINT32 i_dwBuffer_Size, tUINT32 i_dwPool_ID)
       : m_pBuffer(NULL)  
       , m_dwBuffer_Size(0)
       , m_bInitialized(FALSE)
       , m_dwPool_ID(i_dwPool_ID)
       , m_bShell(FALSE)
       , m_pHeader(NULL)
    {
        m_pBuffer = new tUINT8[i_dwBuffer_Size];

        if (m_pBuffer)
        {
            m_dwBuffer_Size = i_dwBuffer_Size;
            m_bInitialized  = TRUE;
            memset(m_pBuffer, 0, m_dwBuffer_Size);
            m_pHeader = (sH_Common*)m_pBuffer;
        }
    }//CTPacket::CTPacket


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::~CTPacket
    ~CTPacket()
    {
        if (    (FALSE == m_bShell)
             && (NULL != m_pBuffer)
           )
        {
            delete [] m_pBuffer;
            m_pBuffer = NULL;
        }

        m_dwBuffer_Size = 0;
        m_bInitialized  = FALSE;
        m_dwPool_ID     = 0;
        m_pHeader       = NULL;
    } //CTPacket::~CTPacket

    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Initialized
    tBOOL Get_Initialized()
    {
        return m_bInitialized;
    } //CTPacket::Get_Initialized


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Finalize
    virtual void Finalize(tUINT32 i_dwPacket_ID, tUINT16 i_wClient_ID)
    {
        m_pHeader->dwID       = i_dwPacket_ID;
        m_pHeader->wClient_ID = i_wClient_ID;
        m_pHeader->dwCRC32    = Calculate_CRC();
    }//CTPacket::Finalize


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Size
    tUINT16 Get_Size()
    {
        return m_pHeader->wSize;
    }//CTPacket::Get_Size

    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Client_ID
    tUINT16 Get_Client_ID()
    {
        return m_pHeader->wClient_ID;
    }//CTPacket::Get_Client_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Type
    eTPacket_Type Get_Type()
    {
        return (eTPacket_Type)m_pHeader->wType;
    }//CTPacket::Get_Type


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_ID
    tUINT32 Get_ID()
    {
        return m_pHeader->dwID;
    }//CTPacket::Get_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Is_Damaged
    tBOOL Is_Damaged()
    {
        return (Calculate_CRC() != m_pHeader->dwCRC32);
    }//CTPacket::Is_Damaged


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Flags
    tUINT16 Get_Flags()
    {
        return m_pHeader->wFlags; 
    }//CTPacket::Get_Flags


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Buffer
    tUINT8 *Get_Buffer()
    {
        return m_pBuffer;
    }//CTPacket::Get_Buffer

    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Buffer_Size
    tUINT32 Get_Buffer_Size()
    {
        return m_dwBuffer_Size;
    }//CTPacket::Get_Buffer_Size


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Get_Pool_ID
    tUINT32 Get_Pool_ID()
    {
        return m_dwPool_ID;
    }//CTPacket::Get_Pool_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Attach
    virtual tBOOL Attach(CTPacket *i_pPacket)
    {
        if (    (NULL  == i_pPacket)
             || (FALSE == m_bShell)
           )
        {
            return FALSE;
        }

        m_pBuffer       = i_pPacket->Get_Buffer();
        m_dwBuffer_Size = i_pPacket->Get_Buffer_Size();
        m_pHeader       = (sH_Common*)m_pBuffer;
        m_dwPool_ID     = i_pPacket->Get_Pool_ID();

        return (NULL != m_pBuffer);
    }//CTPacket::Attach


    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Detach
    tBOOL Detach()
    {
        if (FALSE == m_bShell)
        {
            return FALSE;
        }

        m_pBuffer       = NULL;
        m_dwBuffer_Size = 0;
        m_pHeader       = NULL;
        m_dwPool_ID     = 0;

        return TRUE;
    }//CTPacket::Detach

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CTPacket::Calculate_CRC
    tUINT32 Calculate_CRC()
    {
        tUINT32 l_dwOffset = offsetof(sH_Common, dwCRC32) + sizeof(m_pHeader->dwCRC32);
        tUINT32 l_dwLength = m_pHeader->wSize;

        //this is absolute minimum size of the packet
        if (sizeof(sH_Common) > l_dwLength)
        {
            l_dwLength = sizeof(sH_Common); 
        }

        return Get_CRC32(m_pBuffer + l_dwOffset, l_dwLength - l_dwOffset);
    }//CTPacket::Calculate_CRC
};//CTPacket





////////////////////////////////////////////////////////////////////////////***
// Class for providing basic functionality over tH_Client_Initial packet's 
// header
#define CLIENT_INITIAL_SIZE    (sizeof(sH_Client_Hello) + sizeof(sH_Common))

class CTPClient_Hello: public CTPacket
{
    sH_Client_Hello *m_pInitial;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::CTPClient_Hello
    //stationary packet, it belong to stream
    CTPClient_Hello():
        CTPacket(CLIENT_INITIAL_SIZE, 0)
       ,m_pInitial(NULL)
    {
        if (m_pHeader)
        {
            m_pHeader->wSize = CLIENT_INITIAL_SIZE;
            m_pHeader->wType  = ETPT_CLIENT_HELLO;

            m_pInitial = (sH_Client_Hello*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPClient_Hello::CTPClient_Hello


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::CTPClient_Hello
    //dynamic packet, it belong to Pool
    CTPClient_Hello(CTPacket *i_pPacket):
        CTPacket()
       ,m_pInitial(NULL)
    {
        m_bInitialized = Attach(i_pPacket);
        if (m_bInitialized)
        {
            m_pInitial = (sH_Client_Hello*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPClient_Hello::CTPClient_Hello
        

    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Fill
    tBOOL Fill(tUINT16   i_wProtocol_Version,
               tUINT16   i_wData_Max_Size,
               tUINT32   i_dwProcess_ID,
               tUINT32   i_dwProcess_Start_Time_Hi,
               tUINT32   i_dwProcess_Start_Time_Lo,
               tWCHAR   *i_pProcess_Name //[TPACKET_PROCESS_NAME_MAX_LEN]
             ) 
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
             || (NULL  == i_pProcess_Name)
           )
        {
            return FALSE;
        }

        m_pInitial->wProtocol_Version       = i_wProtocol_Version;
        m_pInitial->wData_Max_Size          = i_wData_Max_Size;
        m_pInitial->dwProcess_ID            = i_dwProcess_ID;
        m_pInitial->dwProcess_Start_Time_Hi = i_dwProcess_Start_Time_Hi;
        m_pInitial->dwProcess_Start_Time_Lo = i_dwProcess_Start_Time_Lo;
        
        tWCHAR *l_pSrc = i_pProcess_Name;
        tWCHAR *l_pDst = m_pInitial->pProcess_Name;
        tUINT16 l_wRst = TPACKET_PROCESS_NAME_MAX_LEN - 1;
        
        //wcsncpy_s(m_pInitial->pProcess_Name,
        //          TPACKET_PROCESS_NAME_MAX_LEN,
        //          i_pProcess_Name,
        //          TPACKET_PROCESS_NAME_MAX_LEN - 1
        //         );
        while (    (l_wRst)
                && (*l_pSrc)
              )
        {
            *l_pDst = *l_pSrc;
            l_pDst ++;
            l_pSrc ++;
            l_wRst --;
        }
        
        *l_pDst = 0;
        
        return TRUE;
    } //CTPClient_Hello::Fill


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Fill
    tUINT16 Get_Protocol_Version()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pInitial->wProtocol_Version;
    }//CTPClient_Hello::Fill


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Get_Data_Max_Size
    tUINT16 Get_Data_Max_Size()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pInitial->wData_Max_Size;
    }//CTPClient_Hello::Get_Data_Max_Size


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Get_Process_ID
    tUINT32 Get_Process_ID()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pInitial->dwProcess_ID;
    }//CTPClient_Hello::Get_Process_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Get_Process_Time_Hi
    tUINT32 Get_Process_Time_Hi()
    {
        if (FALSE == m_bInitialized)
           
        {
            return 0;
        }

        return m_pInitial->dwProcess_Start_Time_Hi;
    }//CTPClient_Hello::Get_Process_Times

    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Get_Process_Time_Low
    tUINT32 Get_Process_Time_Low()
    {
        if (FALSE == m_bInitialized)
           
        {
            return 0;
        }

        return m_pInitial->dwProcess_Start_Time_Lo;
    }//CTPClient_Hello::Get_Process_Times


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Hello::Get_Process_Name
    tWCHAR *Get_Process_Name()
    {
        if (FALSE == m_bInitialized)
        {
            return NULL;
        }

        return m_pInitial->pProcess_Name;
    }//CTPClient_Hello::Get_Process_Name
}; //CTPClient_Hello


////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over client data packets
#define CLIENT_DATA_HEADER_SIZE                                sizeof(sH_Common)

class CTPData: public CTPacket
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::CTPClient_Data
    CTPData(CTPacket *i_pPacket):
        CTPacket()
    {
        m_bInitialized = Attach(i_pPacket);
    }//CTPClient_Data::CTPClient_Data

    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::CTPClient_Data
    CTPData():
        CTPacket()
    {
    }//CTPClient_Data::CTPClient_Data


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Attach
    tBOOL Attach(CTPacket *i_pPacket)
    {
        m_bInitialized = CTPacket::Attach(i_pPacket);
        return m_bInitialized;
    }//CTPClient_Data::Attach


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Initialize
    void Initialize()
    {
        if (m_pHeader)
        {
            m_pHeader->wSize = sizeof(sH_Common);
            m_pHeader->wType  = ETPT_CLIENT_DATA;
        }
    }//CTPClient_Data::Initialize

    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Get_Data
    tUINT8 *Get_Data()
    {
        if (    (NULL == m_pBuffer)
             || (m_dwBuffer_Size <= sizeof(sH_Common))
           )
        {
            return NULL;
        }

        return m_pBuffer + sizeof(sH_Common);
    }//CTPClient_Data::Get_Data


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Get_Data_Size
    tUINT16 Get_Data_Size()
    {
        if (NULL == m_pHeader)
        {
            return 0;
        }

        return m_pHeader->wSize - sizeof(sH_Common);
    }//CTPClient_Data::Get_Data_Size


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Get_Tail
    tUINT8 *Get_Tail()
    {
        if (    (NULL == m_pBuffer)
             || (m_pHeader->wSize >= m_dwBuffer_Size)
           )
        {
            return NULL;
        }

        return m_pBuffer + m_pHeader->wSize;
    }//CTPClient_Data::Get_Tail


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Get_Tail_Size
    tUINT16 Get_Tail_Size()
    {
        if (NULL == m_pHeader)
        {
            return 0;
        }

        return (tUINT16)(m_dwBuffer_Size - m_pHeader->wSize);
    }//CTPClient_Data::Get_Tail_Size


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Append_Size
    tBOOL Append_Size(tUINT16 i_wSize)
    {
        if (    (NULL == m_pHeader) 
             || ( i_wSize > Get_Tail_Size() )
           )
        {
            return FALSE;
        }

        m_pHeader->wSize += i_wSize;
        return TRUE;
    }//CTPClient_Data::Append_Size


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data::Finalize
    void Finalize(tUINT32 i_dwPacket_ID, tUINT16 i_wClient_ID)
    {
        if (m_pHeader)
        {
            m_pHeader->wType = ETPT_CLIENT_DATA;
            CTPacket::Finalize(i_dwPacket_ID, i_wClient_ID);
        }
    }//CTPClient_Data::Finalize
};//CTPClient_Data



////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over client data report packet
#define CLIENT_DATA_REPORT_SIZE    (sizeof(sH_Data_Window) + sizeof(sH_Common))
class CTPData_Window: public CTPacket
{
    sH_Data_Window *m_pReport;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data_Report::CTPClient_Data_Report
    //stationary packet, it belong to stream
    CTPData_Window():
        CTPacket(CLIENT_DATA_REPORT_SIZE, 0)
       ,m_pReport(NULL)
    {
        if (m_bInitialized)
        {
            m_pHeader->wSize = CLIENT_DATA_REPORT_SIZE;
            m_pHeader->wType  = ETPT_CLIENT_DATA_REPORT;

            m_pReport = (sH_Data_Window*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPClient_Data_Report::CTPClient_Data_Report


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data_Report::CTPClient_Data_Report
    //dynamic packet, it belong to Pool
    CTPData_Window(CTPacket *i_pPacket):
        CTPacket()
       ,m_pReport(NULL)
    {
        m_bInitialized = Attach(i_pPacket);
        if (m_bInitialized)
        {
            m_pReport = (sH_Data_Window*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPClient_Data_Report::CTPClient_Data_Report


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data_Report::Fill
    tBOOL Fill(tUINT32 i_dwData_First_ID, tUINT32 i_dwData_Last_ID)
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
           )
        {
            return FALSE;
        }

        m_pReport->dwFirst_ID  = i_dwData_First_ID;
        m_pReport->dwLast_ID = i_dwData_Last_ID;

        return TRUE;
    }


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data_Report::Get_First_ID
    tUINT32 Get_First_ID()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pReport->dwFirst_ID;
    }//CTPClient_Data_Report::Get_First_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Data_Report::Get_Last_ID
    tUINT32 Get_Last_ID()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pReport->dwLast_ID;
    }//CTPClient_Data_Report::Get_Last_ID
};//CTPClient_Data_Report


////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over client alive packet
#define CLIENT_ALIVE_SIZE                                    (sizeof(sH_Common))
class CTPClient_Ping: public CTPacket
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Ping::CTPClient_Ping
    //stationary packet, it belong to stream
    CTPClient_Ping():
        CTPacket(CLIENT_ALIVE_SIZE, 0)
    {
        if (m_bInitialized)
        {
            m_pHeader->wSize = CLIENT_ALIVE_SIZE;
            m_pHeader->wType  = ETPT_CLIENT_PING;
        }
    }//CTPClient_Ping::CTPClient_Ping
};




////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over client bye packet
#define CLIENT_BYE_SIZE                                    (sizeof(sH_Common))
class CTPClient_Bye: public CTPacket
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPClient_Ping::CTPClient_Ping
    //stationary packet, it belong to stream
    CTPClient_Bye():
        CTPacket(CLIENT_BYE_SIZE, 0)
    {
        if (m_bInitialized)
        {
            m_pHeader->wSize = CLIENT_BYE_SIZE;
            m_pHeader->wType  = ETPT_CLIENT_BYE;
        }
    }//CTPClient_Ping::CTPClient_Ping
};//CTPClient_Ping



////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over server response packet
#define ACKNOWLEDGMENT_SIZE     (sizeof(sH_Packet_Ack) + sizeof(sH_Common))
class CTPAcknowledgment: 
      public CTPacket
{
    sH_Packet_Ack *m_pResponse;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::CTPServer_Response
    //stationary packet, it belong to stream
    CTPAcknowledgment():
        CTPacket(TPACKET_MIN_SIZE, 0)
       ,m_pResponse(NULL)
    {
        if (m_bInitialized)
        {
            m_pHeader->wSize  = ACKNOWLEDGMENT_SIZE;
            m_pHeader->wType  = ETPT_ACKNOWLEDGMENT;

            m_pResponse = (sH_Packet_Ack*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPServer_Response::CTPServer_Response


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::CTPServer_Response
    //dynamic packet, it belong to Pool
    CTPAcknowledgment(CTPacket *i_pPacket):
        CTPacket()
       ,m_pResponse(NULL)
    {
        m_bInitialized = Attach(i_pPacket);
        if (m_bInitialized)
        {
            m_pResponse = (sH_Packet_Ack*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPServer_Response::CTPServer_Response


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::Fill
    tBOOL Fill(tUINT32 i_dwSource_ID, tUINT16 i_wResult)
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
           )
        {
            return FALSE;
        }

        m_pResponse->dwSource_ID = i_dwSource_ID;
        m_pResponse->wResult     = i_wResult;

        return TRUE;
    }


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::Get_Source_ID
    tUINT32 Get_Source_ID()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pResponse->dwSource_ID;
    }//CTPClient_Data_Report::Get_Source_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::Get_Result
    tUINT16 Get_Result()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pResponse->wResult;
    }//CTPClient_Data_Report::Get_Result


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::Set_Extra
    void Set_Extra(tUINT8 *i_pData, tUINT32 i_dwSize)
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
             || (NULL  == i_pData)
             || (0     >= i_dwSize)
             || ((m_dwBuffer_Size - ACKNOWLEDGMENT_SIZE ) < i_dwSize)
           )
        {
            return;
        }

        memcpy(m_pBuffer + ACKNOWLEDGMENT_SIZE,
               //m_dwBuffer_Size - ACKNOWLEDGMENT_SIZE,
               i_pData, 
               i_dwSize
              );

        m_pHeader->wSize = (tUINT16)(ACKNOWLEDGMENT_SIZE + i_dwSize);

        m_pHeader->wFlags = m_pHeader->wFlags | TPACKET_FLAG_EXTRA_DATA;
    }//CTPClient_Data_Report::Set_Extra


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Response::Clr_Extra
    void Clr_Extra()
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
           )
        {
            return;
        }

        m_pHeader->wSize = ACKNOWLEDGMENT_SIZE;
        m_pHeader->wFlags = m_pHeader->wFlags & (~TPACKET_FLAG_EXTRA_DATA);
    }//CTPClient_Data_Report::Clr_Extra
};//CTPServer_Response



////////////////////////////////////////////////////////////////////////////////
// Class for providing basic functionality over server report packet
#define SERVER_REPORT_SIZE                  (TPACKET_MIN_SIZE)
#define SERVER_REPORT_HEADER_SIZE           (sizeof(sH_Common) +\
                                             offsetof(sH_Data_Window_Report, pID))

class CTPData_Window_Report: 
      public CTPacket
{
    sH_Data_Window_Report *m_pReport;
    tUINT32                m_dwCount;
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::CTPServer_Report
    //stationary packet, it belong to stream
    CTPData_Window_Report():
        CTPacket(SERVER_REPORT_SIZE, 0)
       ,m_pReport(NULL)
       ,m_dwCount(0)
    {
        if (m_bInitialized)
        {
            m_pHeader->wSize  = SERVER_REPORT_SIZE;
            m_pHeader->wType  = ETPT_SERVER_DATA_REPORT;
            m_dwCount         = m_pHeader->wSize - SERVER_REPORT_HEADER_SIZE;

            m_pReport = (sH_Data_Window_Report*)(m_pBuffer + sizeof(sH_Common));
        }
    }//CTPServer_Report::CTPServer_Report


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::CTPServer_Report
    //dynamic packet, it belong to Pool
    CTPData_Window_Report(CTPacket *i_pPacket):
        CTPacket()
       ,m_pReport(NULL)
       ,m_dwCount(0)
    {
        m_bInitialized = Attach(i_pPacket);
        if (m_bInitialized)
        {
            m_pReport = (sH_Data_Window_Report*)(m_pBuffer + sizeof(sH_Common));
            m_dwCount = m_pHeader->wSize - SERVER_REPORT_HEADER_SIZE;
        }
    }//CTPServer_Report::CTPServer_Report


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::Is_ID
    tBOOL Is_ID(tUINT32 i_dwFirst_ID, tUINT32 i_dwCurrent_ID)
    {
        tUINT32 l_dwOffset = ID_Difference(i_dwFirst_ID, i_dwCurrent_ID);

        if (    (NULL == m_pReport)
             || (l_dwOffset >= (m_dwCount << 3)) 
           )
        {
            return FALSE;
        }

        return ( 0 != (m_pReport->pID[l_dwOffset >> 3] & (1 << (l_dwOffset % 8))) );
    }//CTPServer_Report::Is_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::Clr_IDs
    tBOOL Clr_IDs()
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
           )
        {
            return FALSE;
        }

        memset(m_pReport->pID, 0, m_dwCount);

        return TRUE;
    } //Clr_IDs

    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::Set_ID
    tBOOL Set_ID(tUINT32 i_dwFirst_ID, tUINT32 i_dwCurrent_ID)
    {
        tUINT32 l_dwOffset = ID_Difference(i_dwFirst_ID, i_dwCurrent_ID);

        if (    (NULL == m_pReport)
             || (l_dwOffset >= (m_dwCount << 3)) 
           )
        {
            return FALSE;
        }

        m_pReport->pID[l_dwOffset >> 3] |= (1 << (l_dwOffset % 8));
        return TRUE;
    }//CTPServer_Report::Set_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::Get_Source_ID
    tUINT32 Get_Source_ID()
    {
        if (FALSE == m_bInitialized)
        {
            return 0;
        }

        return m_pReport->dwSource_ID;
    }//CTPServer_Report::Get_Source_ID


    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::Fill
    tBOOL Fill(tUINT32 i_dw_Source_ID)
    {
        if (    (FALSE == m_bInitialized)
             || (TRUE  == m_bShell)
           )
        {
            return FALSE;
        }

        m_pReport->dwSource_ID = i_dw_Source_ID;
        return TRUE;
    }//CTPServer_Report::Fill

private:
    ////////////////////////////////////////////////////////////////////////////
    //CTPServer_Report::ID_Difference
    tUINT32 ID_Difference(tUINT32 i_dwFirst_ID, tUINT32 i_dwCurrent_ID)
    {
        if (i_dwCurrent_ID < i_dwFirst_ID)
        {
            return ((TPACKET_MAX_ID - i_dwFirst_ID) + i_dwCurrent_ID);
        }

        return (i_dwCurrent_ID - i_dwFirst_ID);
    }//CTPServer_Report::ID_Difference

};//CTPServer_Report



#if  defined(_DEBUG) || defined(DEBUG) 
    #define PACKET_PRINT(i_Log, i_Prefix, i_Packet) Packet_Print(i_Log,i_Prefix, i_Packet); 
#else
    #define PACKET_PRINT(i_Log, i_Prefix, i_Packet)  
#endif

inline void Packet_Print(IJournal     *i_pLog, 
                         const tXCHAR *l_pPrefix, 
                         CTPacket     *i_pPacket)
{
    if (    (NULL == i_pLog)
         || (EFJOIRNAL_TYPE_DEBUG < i_pLog->Get_Verbosity())
         || (NULL == i_pPacket)
       )
    {
        return;
    }

    eTPacket_Type l_eType = i_pPacket->Get_Type();

    switch (l_eType)
    {
        case ETPT_CLIENT_HELLO:
        {
            CTPClient_Hello l_cHello(i_pPacket);

            JOURNAL_DEBUG(i_pLog, 
                          TM("%s [Initial] ID=%d, Size=%d, Protocol=%d, MaxSize=%d Process[%d/%s]"),
                          l_pPrefix,
                          (tUINT32)l_cHello.Get_ID(),
                          (tUINT32)l_cHello.Get_Size(),
                          (tUINT32)l_cHello.Get_Protocol_Version(),
                          (tUINT32)l_cHello.Get_Data_Max_Size(),
                          (tUINT32)l_cHello.Get_Process_ID(),
                          l_cHello.Get_Process_Name()
                          );
            break;
        }
        case ETPT_CLIENT_PING:
        {
            JOURNAL_DEBUG(i_pLog, 
                          TM("%s [Alive] ID=%d, Size=%d"),
                          l_pPrefix,
                          (tUINT32)i_pPacket->Get_ID(),
                          (tUINT32)i_pPacket->Get_Size()
                          );
            break;
        }
        case ETPT_CLIENT_DATA:       
        {
            break;
        }
        case ETPT_CLIENT_DATA_REPORT:
        {
            CTPData_Window l_cReport(i_pPacket);
            JOURNAL_DEBUG(i_pLog, 
                          TM("%s [Data report] ID=%d, Size=%d, window ID [%d .. %d]"),
                          l_pPrefix,
                          (tUINT32)l_cReport.Get_ID(),
                          (tUINT32)l_cReport.Get_Size(),
                          (tUINT32)l_cReport.Get_First_ID(),
                          (tUINT32)l_cReport.Get_Last_ID()
                          );
            break;
        }
        case ETPT_CLIENT_BYE:        
        {
            JOURNAL_DEBUG(i_pLog, 
                         TM("%s [Bye] ID=%d, Size=%d"),
                          l_pPrefix,
                          (tUINT32)i_pPacket->Get_ID(),
                          (tUINT32)i_pPacket->Get_Size()
                          );
            break;
        }
        case ETPT_ACKNOWLEDGMENT:   
        {
            CTPAcknowledgment l_cResponse(i_pPacket);
            JOURNAL_DEBUG(i_pLog, 
                          TM("%s [Response] ID=%d, Size=%d, Source ID=%d Result=%d"),
                          l_pPrefix,
                          (tUINT32)l_cResponse.Get_ID(),
                          (tUINT32)l_cResponse.Get_Size(),
                          (tUINT32)l_cResponse.Get_Source_ID(),
                          (tUINT32)l_cResponse.Get_Result()
                          );
            break;
        }
        case ETPT_SERVER_DATA_REPORT:
        {
            CTPData_Window_Report l_cReport(i_pPacket);
            JOURNAL_DEBUG(i_pLog, 
                          TM("%s [Data report] ID=%d, Size=%d, Source ID=%d"),
                          l_pPrefix,
                          (tUINT32)l_cReport.Get_ID(),
                          (tUINT32)l_cReport.Get_Size(),
                          (tUINT32)l_cReport.Get_Source_ID()
                          );
            break;
        }
        case ETPT_SERVER_CHAINED:    
        {
            break;
        }
        default:
        {
            JOURNAL_DEBUG(i_pLog, 
                          TM("%s [Unknown] ID=%d, Size=%d"),
                          l_pPrefix,
                          (tUINT32)i_pPacket->Get_ID(),
                          (tUINT32)i_pPacket->Get_Size()
                          );
            break;
        }
    }
}

#endif //TACKETS_H_AZH
