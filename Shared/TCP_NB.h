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

//Non blocking reading
//http://msdn.microsoft.com/ru-ru/library/windows/desktop/ms738573(v=vs.85).aspx

//Non blocking accept, select is non blocking by def.
//http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=%2Frzab6%2Frzab6xnonblock.htm

//Connect, non blocking
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms737625(v=vs.85).aspx

//compare linux and windows sockets
//http://habrahabr.ru/post/105918/

//How to get TCP/IP table
//http://msdn.microsoft.com/ru-ru/library/windows/desktop/aa366909(v=vs.85).aspx


//Nice shutdown
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms738547(v=vs.85).aspx

#pragma once

#include "PSocket.h"

enum eTCP_NB_Status
{
    ETCP_NB_STATUS_OK                    = 0,
    ETCP_NB_STATUS_CONNECTION_PENDING       ,
    ETCP_NB_STATUS_CONNECTION_ERROR         ,
    ETCP_NB_STATUS_NOT_READY                ,
    ETCP_NB_STATUS_WRONG_PARAMETERS         ,
    ETCP_NB_STATUS_NOT_INITIALIZED          ,
    ETCP_NB_STATUS_SOCKET_CREATE_ERROR      ,
    ETCP_NB_STATUS_BIND_ERROR               ,
    ETCP_NB_STATUS_LISTEN_ERROR             ,
    ETCP_NB_STATUS_SELECT_ERROR             ,
    ETCP_NB_STATUS_SEND_ERROR               ,
    ETCP_NB_STATUS_RECEIVE_ERROR            ,
};


////////////////////////////////////////////////////////////////////////////////
//CTCPNB_Base 
class CTCPNB_Base
{
protected:

    enum eFD_Type
    {
        FD_TYPE_WRITE,
        FD_TYPE_READ
    };

    tSOCKET           m_hSocket;
    sockaddr_storage  m_tAddress;
    tUINT32           m_dwAddress_Size;
    IJournal         *m_pLog;
    tINT32            m_iFamily;
    eTCP_NB_Status    m_eStatus;

public:
    CTCPNB_Base(IJournal *i_pLog,
                sockaddr *i_pAddress,
                tSOCKET   i_hSocket
               )
        : m_hSocket(i_hSocket)
        , m_dwAddress_Size(0)
        , m_pLog(i_pLog)
        , m_iFamily(AF_UNSPEC)
        , m_eStatus(ETCP_NB_STATUS_NOT_INITIALIZED)
    {
        eTCP_NB_Status l_eStatus = m_eStatus;
        if (m_pLog)
        {
            m_pLog->Add_Ref();
        }

        if (NULL == i_pAddress)
        {
            l_eStatus = ETCP_NB_STATUS_WRONG_PARAMETERS;
            JOURNAL_ERROR(m_pLog, TM("NULL == i_pAddress"));
            goto l_lblExit;
        }

        ////////////////////////////////////////////////////////////////////////
        //copy address
        memset(&m_tAddress, 0, sizeof(sockaddr_storage));

        m_iFamily = i_pAddress->sa_family;
        if (AF_INET == i_pAddress->sa_family)
        {
            m_dwAddress_Size = sizeof(sockaddr_in);
            memcpy(&m_tAddress, i_pAddress, m_dwAddress_Size);
        }
        else if (AF_INET6 == i_pAddress->sa_family)
        {
            m_dwAddress_Size = sizeof(sockaddr_in6);
            memcpy(&m_tAddress, i_pAddress, m_dwAddress_Size);
        }
        else
        {
            l_eStatus = ETCP_NB_STATUS_WRONG_PARAMETERS; 
            JOURNAL_ERROR(m_pLog, 
                          TM("Address family is wrong = %d"), 
                          (tUINT32)m_iFamily
                         );
            goto l_lblExit;
        }

        if (INVALID_SOCKET_VAL != m_hSocket)
        {
            l_eStatus = ETCP_NB_STATUS_OK; 
        }
        else
        {
            l_eStatus = ETCP_NB_STATUS_CONNECTION_PENDING; 
        }

    l_lblExit:
        m_eStatus = l_eStatus;

        Print();
    }

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Base::~CTCPNB_Base()
    ~CTCPNB_Base()
    {
        if (INVALID_SOCKET_VAL != m_hSocket)
        {
            tINT32 l_iFlag = 1;
            setsockopt(m_hSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&l_iFlag, sizeof(l_iFlag));

            if (SOCKET_ERROR == shutdown(m_hSocket, SD_BOTH))
            {
                JOURNAL_WARNING(m_pLog,
                                TM("Socket socket shutdown, unexpected result %d !"),
                                GET_SOCKET_ERROR()
                             );
            }

            closesocket(m_hSocket);

            m_hSocket = INVALID_SOCKET_VAL;
        }

        if (m_pLog)
        {
            m_pLog->Release();
            m_pLog = NULL;
        }
    }//CTCPNB_Base::~CTCPNB_Base();

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Base::Get_Staus
    virtual eTCP_NB_Status Get_Staus()                                      = 0;


    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Base::Get_Addr
    void Get_Addr(sockaddr_storage *o_pAddr)
    {
        memcpy_s(o_pAddr, sizeof(sockaddr_storage), &m_tAddress, sizeof(sockaddr_storage));
    }//CTCPNB_Base::Get_Addr

protected:
    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Base::Print
    void Print()
    {
        XCHAR    l_pIP[128] = TM("?");
        tUINT32  l_dwLength = sizeof(l_pIP) / sizeof(l_pIP[0]);
            
        if (Print_SAddr((sockaddr*)&m_tAddress, l_pIP, l_dwLength)) 
        {
            JOURNAL_INFO(m_pLog, 
                         TM("Socket: status = %d, Use address = %s"),
                         (tUINT32)m_eStatus,
                         l_pIP
                        );
        }
    }//CTCPNB_Base::Print

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Base::Create_Socket
    eTCP_NB_Status Create_Socket()
    {
        m_hSocket = socket(m_iFamily, SOCK_STREAM, IPPROTO_TCP);
        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            JOURNAL_ERROR(m_pLog,
                          TM("Socket creation failed, error=%d !"),
                          GET_SOCKET_ERROR()
                         );

            return ETCP_NB_STATUS_SOCKET_CREATE_ERROR;
        }

        tUINT32 l_dwMode = 1;    
        if (SOCKET_ERROR == ioctlsocket(m_hSocket, FIONBIO, (u_long*)&l_dwMode))
        {
            JOURNAL_ERROR(m_pLog,
                          TM("Socket unable to make non blockable, error=%d !"),
                          GET_SOCKET_ERROR()
                         );

            return ETCP_NB_STATUS_SOCKET_CREATE_ERROR;
        }

        return ETCP_NB_STATUS_OK;
    }//CTCPNB_Base::Create_Socket


    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Base::Is_Ready
    eTCP_NB_Status Is_Ready(eFD_Type i_eType, tUINT32 i_dwTimeOut)
    {
        eTCP_NB_Status l_eReturn = ETCP_NB_STATUS_NOT_READY;

        if (INVALID_SOCKET_VAL == m_hSocket)
        {
            return ETCP_NB_STATUS_NOT_INITIALIZED;
        }

        tINT32  l_iSelect = SOCKET_ERROR;
        fd_set  l_pFDS;
        timeval l_tTimeOut;

        l_tTimeOut.tv_sec  = 0;
        l_tTimeOut.tv_usec = i_dwTimeOut * 1000; 

        FD_ZERO(&l_pFDS);
        FD_SET((tUINT32)m_hSocket, &l_pFDS);
    
        if (FD_TYPE_READ == i_eType)
        {
            l_iSelect = select(((tUINT32)m_hSocket) + 1,
                               &l_pFDS,
                               NULL, 
                               NULL,
                               &l_tTimeOut
                              );
        }
        else
        {
            l_iSelect = select(((tUINT32)m_hSocket) + 1, 
                               NULL, 
                               &l_pFDS, 
                               NULL, 
                               &l_tTimeOut
                              );
        }

        if (SOCKET_ERROR == l_iSelect)
        {
            JOURNAL_ERROR(m_pLog, 
                          TM("Select fail, error=%d !"), 
                          GET_SOCKET_ERROR()
                         );

            l_eReturn = ETCP_NB_STATUS_SELECT_ERROR;
        }
        else if (    (l_iSelect > 0) 
                  && (FD_ISSET(m_hSocket, &l_pFDS))
                )
        {
            l_eReturn = ETCP_NB_STATUS_OK;
        }

        return l_eReturn;
    }//CTCPNB_Base::Is_Ready

};//CTCPNB_Base ////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//CTCPNB_Data
class CTCPNB_Data
    : public CTCPNB_Base
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Data::CTCPNB_Data
    CTCPNB_Data(IJournal *i_pLog,
                sockaddr *i_pAddress,
                tSOCKET   i_hSocket = INVALID_SOCKET_VAL
               )
        : CTCPNB_Base(i_pLog, i_pAddress, i_hSocket)
    {
        if (INVALID_SOCKET_VAL != m_hSocket)
        {
            return;
        }

        if (ETCP_NB_STATUS_OK != (m_eStatus = Create_Socket()))
        {
            return;
        }

        if (0 == connect(m_hSocket, (struct sockaddr *)&m_tAddress, m_dwAddress_Size))
        {
            m_eStatus = ETCP_NB_STATUS_OK;
        }
        else
        {
            if (CONNECTION_IN_PROGRESS == GET_SOCKET_ERROR())
            {
                m_eStatus = ETCP_NB_STATUS_CONNECTION_PENDING;
            }
            else
            {
                m_eStatus = ETCP_NB_STATUS_CONNECTION_ERROR;
            }
        }
    }//CTCPNB_Data::CTCPNB_Data

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Data::Send
    eTCP_NB_Status Send(tINT8 *i_pBuffer, tUINT32 i_dwLength, tUINT32 *o_pSent)
    {
        if (    (NULL == i_pBuffer)
             || (NULL == o_pSent)
           )
        {
            return ETCP_NB_STATUS_WRONG_PARAMETERS;
        }

        *o_pSent = 0;

        if (ETCP_NB_STATUS_OK != m_eStatus)
        {
            return m_eStatus;
        }

        if (ETCP_NB_STATUS_OK != Is_Ready(FD_TYPE_WRITE, 0))
        {
            return ETCP_NB_STATUS_NOT_READY;
        }

        tINT32 l_iResult = send(m_hSocket, i_pBuffer, i_dwLength, 0);

        if (SOCKET_ERROR == l_iResult)
        {
            return ETCP_NB_STATUS_SEND_ERROR;
        }

        *o_pSent = l_iResult; 
        return ETCP_NB_STATUS_OK;
    }//CTCPNB_Data::Send

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Data::Receive
    eTCP_NB_Status Receive(tINT8 *o_pBuffer, tUINT32 i_dwLength, tUINT32 *o_pReceive)
    {
        if (    (NULL == o_pBuffer)
             || (NULL == o_pReceive)
           )
        {
            return ETCP_NB_STATUS_WRONG_PARAMETERS;
        }

        *o_pReceive = 0;

        if (ETCP_NB_STATUS_OK != m_eStatus)
        {
            return m_eStatus;
        }

        if (ETCP_NB_STATUS_OK != Is_Ready(FD_TYPE_READ, 0))
        {
            return ETCP_NB_STATUS_NOT_READY;
        }

        tINT32 l_iResult = recv(m_hSocket, o_pBuffer, i_dwLength, 0);

        if (SOCKET_ERROR == l_iResult)
        {
            return ETCP_NB_STATUS_RECEIVE_ERROR;
        }

        *o_pReceive = l_iResult; 
        return ETCP_NB_STATUS_OK;
    }//CTCPNB_Data::Receive

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Data::Get_Staus
    virtual eTCP_NB_Status Get_Staus()                              
    {
        if (ETCP_NB_STATUS_CONNECTION_PENDING == m_eStatus)
        {
            if (ETCP_NB_STATUS_OK == Is_Ready(FD_TYPE_WRITE, 0))
            {
                m_eStatus = ETCP_NB_STATUS_OK;
            }
        }

        return m_eStatus;
    }//CTCPNB_Data::Get_Staus

protected:
};//CTCPNB_Data ////////////////////////////////////////////////////////////////


             
////////////////////////////////////////////////////////////////////////////////
//CTCPNB_Acceptor
class CTCPNB_Acceptor
    : public CTCPNB_Base
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Acceptor::CTCPNB_Acceptor
    CTCPNB_Acceptor(IJournal *i_pLog,
                    sockaddr *i_pAddress
                   )
        : CTCPNB_Base(i_pLog, i_pAddress, INVALID_SOCKET_VAL)
    {
        if (ETCP_NB_STATUS_OK != (m_eStatus = Create_Socket()))
        {
            return;
        }

        tUINT32 l_dwOn = 1;
        setsockopt(m_hSocket, 
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   (char *)&l_dwOn, 
                   sizeof(l_dwOn)
                  );

        if (SOCKET_ERROR == bind(m_hSocket, (struct sockaddr *)&m_tAddress, m_dwAddress_Size))
        {
            JOURNAL_ERROR(m_pLog, 
                          TM("Bind failed, error=%d !"), 
                          (tUINT32)GET_SOCKET_ERROR()
                         );

            m_eStatus = ETCP_NB_STATUS_BIND_ERROR;
            return;
        }

        if (SOCKET_ERROR == listen(m_hSocket, 64))
        {
            JOURNAL_ERROR(m_pLog, 
                          TM("listen failed, error=%d !"), 
                          (tUINT32)GET_SOCKET_ERROR()
                         );

            m_eStatus = ETCP_NB_STATUS_LISTEN_ERROR;
            return;
        }

        m_eStatus = ETCP_NB_STATUS_OK;
    }//CTCPNB_Acceptor::CTCPNB_Acceptor

    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Acceptor::Get_Staus
    virtual eTCP_NB_Status Get_Staus()                              
    {
        return m_eStatus;
    }//CTCPNB_Acceptor::Get_Staus


    ////////////////////////////////////////////////////////////////////////////
    //CTCPNB_Acceptor::Get_Connection
    CTCPNB_Data *Get_Connection()                              
    {
        CTCPNB_Data *l_pResult = NULL;

        if (ETCP_NB_STATUS_OK != m_eStatus)
        {
            return NULL;
        }

        if (ETCP_NB_STATUS_OK == Is_Ready(FD_TYPE_READ, 0))
        {
            sockaddr_storage  l_sAddress;
            tINT32            l_dwSize  = sizeof(sockaddr_in6);
            tSOCKET           l_hSocket = INVALID_SOCKET_VAL; 

            memset(&l_sAddress, 0, sizeof(sockaddr_storage));

            l_hSocket = accept(m_hSocket, 
                               (struct sockaddr*)&l_sAddress,
                               &l_dwSize
                              );

            if (INVALID_SOCKET_VAL != l_hSocket)
            {
                l_pResult = new CTCPNB_Data(m_pLog, (sockaddr*)&l_sAddress, l_hSocket);
            }
        }

        return l_pResult;
    }//CTCPNB_Acceptor::Get_Connection

};//CTCPNB_Acceptor //////////////////////////////////////////////////////////// 


