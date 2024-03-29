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
#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Mswsock.h>

//256k
#define CLIENT_RECV_BUFFER_SIZE                                        (0x40000) 
#define CLIENT_SEND_BUFFER_SIZE                                        (0x40000) 

//2m
#define SERVER_RECV_BUFFER_SIZE                                       (0x200000) 
//256k
#define SERVER_SEND_BUFFER_SIZE                                        (0x40000) 


typedef SOCKET      tSOCKET; 

typedef int         tADDR_LEN; 

typedef addrinfoW   tADDR_INFO;


#define INVALID_SOCKET_VAL                                      (INVALID_SOCKET)
#define CONNECTION_IN_PROGRESS                                  (WSAEWOULDBLOCK)

#define CLOSE_SOCKET(i_Socket)                            closesocket(m_hSocket)


#define GET_SOCKET_ERROR()                                     WSAGetLastError()

//#define SOCKET_ERROR                                                      (-1)

#define GET_ADDR_INFO(i_Node, i_Service, i_Hints, o_Res) GetAddrInfoW(i_Node,\
                                                                      i_Service,\
                                                                      i_Hints,\
                                                                      o_Res)                                  

#define FREE_ADDR_INFO(i_Info) FreeAddrInfoW(i_Info)    


////////////////////////////////////////////////////////////////////////////////
//WSA_Init
static tBOOL WSA_Init()
{
    WSADATA l_tWSA;
    
    if (0 != WSAStartup(MAKEWORD(1,1), &l_tWSA))
    {
        return FALSE;
    }
    return TRUE;
}//WSA_Init


////////////////////////////////////////////////////////////////////////////////
//WSA_UnInit
static void WSA_UnInit()
{
    WSACleanup();
}//WSA_UnInit


////////////////////////////////////////////////////////////////////////////////
//Print_SAddr
static tBOOL Print_SAddr(sockaddr *i_pAddress, XCHAR *o_pIP, tUINT32 i_dwLen)
{
    tBOOL   l_bReturn = FALSE;
    tUINT32 l_dwSize  = 0;

    if (NULL == i_pAddress)
    {
        return l_bReturn;
    }

    if (AF_INET6 == i_pAddress->sa_family)
    {
        l_dwSize = sizeof(sockaddr_in6);
    }
    else if (AF_INET == i_pAddress->sa_family)
    {
        l_dwSize = sizeof(sockaddr_in);
    }

    if (0 == WSAAddressToString(i_pAddress, 
                                l_dwSize, 
                                NULL, 
                                o_pIP, 
                                (LPDWORD)&i_dwLen
                               )
       )
    {
        l_bReturn = TRUE;
    }
    
    return l_bReturn;
}//Print_SAddr


////////////////////////////////////////////////////////////////////////////////
//Disable_PortUnreachable_ICMP
static tBOOL Disable_PortUnreachable_ICMP(tSOCKET i_hSocket)
{
    tUINT32  l_dwBytesReturned = 0;
    tBOOL    l_bNewBehavior    = FALSE;
    tUINT32  l_dwStatus        = 0;
    
    
    l_dwStatus = WSAIoctl(i_hSocket, 
                          SIO_UDP_CONNRESET,
                          &l_bNewBehavior, 
                          sizeof(l_bNewBehavior),
                          NULL, 
                          0,
                          (LPDWORD)&l_dwBytesReturned,
                          NULL,
                          NULL
                         );
    return (SOCKET_ERROR == l_dwStatus) ? FALSE : TRUE;
}//Disable_PortUnreachable_ICMP
