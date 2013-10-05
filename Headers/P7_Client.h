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
#ifndef P7_CLIENT_H_AZH
#define P7_CLIENT_H_AZH

//Values: server address
//IPV4 address : XXX.XXX.XXX.XXX
//IPV6 address : not supported yet
//NetBios Name : any name
#define CLIENT_COMMAND_LINE_ADDRESS                            TM("/P7.Addr=")

//Values: server specified port, usually 9009
#define CLIENT_COMMAND_LINE_PORT                               TM("/P7.Port=")

//Values: size of the data packet, default 512
// Min: 512
// Max: 65535
// Recommended: your network MTU size
#define CLIENT_COMMAND_PACKET_SIZE                             TM("/P7.PSize=")

//Values:
// 0 : Debug
// 1 : Info
// 2 : Warnings
// 3 : Errors
// 4 : Critical
// If you do not specify this parameter = logging is off
#define CLIENT_COMMAND_LOG_VERBOSITY                           TM("/P7.Verb=")

//Values:
// 0 : Off
// 1 : On
#define CLIENT_COMMAND_LOG_ON                                  TM("/P7.On=")

//size of the internal buffers pool in kilobytes. Minimal 16(kb), max is not 
//specified, default value = 1mb
#define CLIENT_COMMAND_POOL_SIZE                               TM("/P7.Pool=")

//size of the transmission window in packets. Sometimes is useful to manage it
//if server aggressively loose incoming packets
//Min = 1
//max = (2 mb / packet size) or ((pool size / packet size) / 2)
#define CLIENT_COMMAND_WINDOW_SIZE                             TM("/P7.Window=")

//Values: No values
#define CLIENT_COMMAND_LOG_HELP                                TM("/P7.Help")


#define CLIENT_HELP_STRING  TM("P7 arguments list:\n")\
                            TM("   /P7.Addr   - Set server address (IPV4, IPV6, NetBios name)\n")\
                            TM("                Examples:\n")\
                            TM("                /P7.Addr=127.0.0.1\n")\
                            TM("                /P7.Addr=::1\n")\
                            TM("                /P7.Addr=MyPC\n")\
                            TM("                Default address is 127.0.0.1\n")\
                            TM("   /P7.Port   - Set server port, default port is 9009\n")\
                            TM("                Example: /P7.Port=9010\n")\
                            TM("   /P7.PSize  - Set packet size. Min value 512 bytes, Max - 65535, Default - 512\n")\
                            TM("                You should specify optimum packet size\n")\
                            TM("                for your network, usually it is MTU.\n")\
                            TM("                Example: /P7.PSize=1476\n")\
                            TM("   /P7.Pool   - Set size of the internal buffers pool in kilobytes. Minimal 16(kb)\n")\
                            TM("                maximal is limited by your OS and HW. Default value = 4mb\n")\
                            TM("                Example, 1 Mb allocation: /P7.Pool=1024\n")\
                            TM("   /P7.Verb   - Set logging verbosity level. This option allow to write\n")\
                            TM("                to text log file or stdout all internal engine messages.\n")\
                            TM("                Do not specify /P7.Verb parameter to switch off the logging\n")\
                            TM("                P7 internal logging has next verbosity levels:\n")\
                            TM("                0 : Info\n")\
                            TM("                1 : Debug\n")\
                            TM("                2 : Warnings\n")\
                            TM("                3 : Errors\n")\
                            TM("                4 : Critical\n")\
                            TM("                Example: /P7.Verb=4\n")\
                            TM("   /P7.On     - Enable/Disable P7 network engine, By default P7 is on\n")\
                            TM("                Examples:\n")\
                            TM("                /P7.On=1\n")\
                            TM("                /P7.On=0\n")\




enum eClient_Status
{
    //Regular statuses
    ECLIENT_STATUS_OK                   = 0,
    ECLIENT_STATUS_OFF                     ,
    ECLIENT_STATUS_INTERNAL_ERROR          ,

    //Temporary statuses             
    ECLIENT_STATUS_DISCONNECTED            ,
    ECLIENT_STATUS_NO_FREE_BUFFERS         ,
    ECLIENT_STATUS_NOT_ALLOWED             ,
    ECLIENT_STATUS_WRONG_PARAMETERS        ,
};

PRAGMA_PACK_ENTER(4) //alignment is now 4, MS Only//////////////////////////////

struct sP7C_Status
{
    tBOOL  bConnected;
    //count of the connection drops, when connection was reinitialized
    tUINT32 dwResets;
}ATTR_PACK(4);


struct sP7C_Data_Chunk
{
    void    *pData;
    tUINT32  dwSize;
}ATTR_PACK(4);


struct sP7C_Channel_Info
{
    tUINT32 dwID;
}ATTR_PACK(4);

struct sP7C_Info
{
    //sockaddr_storage sServer;
    tUINT32            dwMem_Used;
    tUINT32            dwMem_Free;
    tUINT32            dwReject_Mem; //chunks rejected counter - no memory
    tUINT32            dwReject_Con; //chunks rejected counter - no connection
    tUINT32            dwReject_Int; //chunks rejected counter - internal errors
}ATTR_PACK(4);

PRAGMA_PACK_EXIT()//4///////////////////////////////////////////////////////////


class /*__declspec(novtable)*/ IP7C_Channel
{
public:
    virtual void Init(sP7C_Channel_Info *i_pInfo)                           = 0;
    virtual void On_Receive(tUINT32 i_dwChannel, 
                            tUINT8 *i_pBuffer, 
                            tUINT32 i_dwSize)                               = 0;
};


//Define the max channels count, and max packet size for IP7_Client::Sent(...)
#define USER_PACKET_CHANNEL_ID_BITS_COUNT                                    (5)
#define USER_PACKET_SIZE_BITS_COUNT       (32-USER_PACKET_CHANNEL_ID_BITS_COUNT)
#define USER_PACKET_MAX_SIZE                  (1 << USER_PACKET_SIZE_BITS_COUNT)
#define USER_PACKET_CHANNEL_ID_MAX_SIZE (1 << USER_PACKET_CHANNEL_ID_BITS_COUNT)


class /*__declspec(novtable)*/ IP7_Client
{
public:
    virtual tINT32         Add_Ref()                                        = 0;
    virtual tINT32         Release()                                        = 0;

    virtual tBOOL          Get_Status(sP7C_Status *o_pStatus)               = 0;
    virtual tBOOL          Get_Info(sP7C_Info *o_pInfo)                     = 0;
    //N.B. If you use Baikal server to receive data - please do not mix
    //     different packets formats (Trace + Telemetry for example) on the same
    //     channel, in this case one data format will be dropped by server
    virtual eClient_Status Register_Channel(IP7C_Channel *i_pChannel)       = 0;
    virtual eClient_Status Unregister_Channel(tUINT32 i_dwID)               = 0;

    virtual eClient_Status Sent(tUINT32          i_dwChannel_ID,
                                sP7C_Data_Chunk *i_pChunks, 
                                tUINT32          i_dwCount,
                                tUINT32          i_dwSize)                  = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Share  - function to share current P7 object in address space of
    //         the current process, to get shared instance use function
    //         P7_Get_Shared(tXCHAR *i_pName)
    //Input:
    // i_pName - name of the shared object, should be unique for process.
    //Return:
    // tBOOL - TRUE on success, FALSE - otherwise (system error or object with
    //         such name (i_pName) is already exists
    virtual tBOOL           Share(const tXCHAR *i_pName)                    = 0;


};


//l_pArgs : list of args like "/P7.Addr=127.0.0.1 /P7.Port=9008"
//list of all possible arguments you can find in this header at the top
//If you do not specify arguments - the function will try to analyze process 
//arguments
extern IP7_Client * __stdcall P7_Create_Client(tXCHAR *i_pArgs);


////////////////////////////////////////////////////////////////////////////////
//This functions allow you to get P7 instance if it was created by 
//someone other inside current process. If no instance was registered inside
//current process - function will return NULL. Do not forget to call Release
//on interface when you finish your work
//N.B.: Call of this functions can be slow, do not call this functions often, 
//      the best choice is to call it once per module (DLL, LIB, SO ..) and then
//      redistribute pointer inside module by other way
extern IP7_Client  * __stdcall P7_Get_Shared(const tXCHAR *i_pName);


#endif //P7_CLIENT_H_AZH
