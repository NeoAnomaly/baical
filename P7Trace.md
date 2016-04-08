
---



---



# 1. About #

The library consists of 3 parts:
  * Network engine - `IP7_Client`. Deliver stream data from source to destination over network. Interface definition located in `[P7]\Headers\P7_Client.h`
  * Traces - `IP7_Trace`. Process your trace calls, collect all related information (thread, proc., file, ... etc), pack it and forward to network engine (`IP7_Client`). Interface definition located in `[P7]\Headers\P7_Trace.h`
  * Telemetry - `IP7_Telemetry`. Process your telemetry calls, pack it and forward to network engine (`IP7_Client`). Interface definition located in `[P7]\Headers\P7_Telemetry.h`

**Basic facts**:
  1. Language: C++
  1. Speed is priority, library designed to suit high load:
    * Intel i7-870, 50 000 traces per second - 0,5% CPU, max ~2.5 millions
    * Intel E8400 (Core 2 duo), 15 000 traces per second - 0,5% CPU, max ~ 750 000
    * ARM 926EJ (v5), 1 000 traces per second - 0,5% CPU, max ~20 000
  1. 400 ns for processing one trace message (sent(...) -> network -> Baical serve -> HDD)
  1. 300 ns for processing one telemetry sample (add(...) -> network -> Baical serve -> HDD)
  1. Thread safe
  1. Process variable arguments traces, like:
```
P7.Trace("DMA (%d) is complete\n", szCount); 
P7.Error("DMA (%d) error %s\n", szCount, pError); 
```
  1. Process your telemetry samples (buffers usage, CPU consuming, handles count, ... everything what you would like [visualize](http://baical.googlecode.com/svn/wiki/Images/P7Telemetry_Algo.png))
  1. The library pack your traces & telemetry and deliver them to [server](http://code.google.com/p/baical/wiki/Baical) over network
  1. The library use reliable streaming protocol over UDP.
  1. Trace messages strings: UTF-8 (Linux) UTF-16 (Windows)
  1. If connection with a server is not established or has been lost - all new traces/telemetry samples will be dropped. Thus your application will work without additional load.
  1. Verbosity level can be set from server to reduce load
  1. Telemetry counters are managed (on/off) through Baical server
  1. The library works for
    * Windows 32/64 bits (Visual Studio)
    * Linux 32/64 bits (G++)
  1. Every trace message contains detailed information:
    * Text message
    * Level (error, warning, .. etc)
    * Time with 100ns granularity
    * Current thread ID
    * Current module ID
    * Current processor number
    * Function name
    * File name
    * File line number
    * Sequence number

# 2. Speed test #
You can run your own speed tests to estimate productivity of your hardware, to do that you need:
  * [Download](http://code.google.com/p/baical/wiki/Download) latest Baical server, unpack the archive and run `GBaical.exe`;
  * Setup your firewall - you need to enable UDP port 9009 for Baical server;
  * [Checkout](http://code.google.com/p/baical/source/checkout) latest P7 library source code;
  * Compile it, how to do it you can read in chapter "compilation"
  * Run the speed test (located in `[P7]\Binaries\`), it allows to compare different way of information printing (to buffer, to console and to Baical server). Command line arguments (Replace IP 192.168.0.1 by your Baical IP):
```
Speed /P7.Addr=192.168.0.1 /P7.PSize=1472 /P7.Pool=8192
```

You can build them in Windows and Linux, build instructions are located at the bottom of the page.

# 3 P7 Client #
Interface is repsonsable for delivering data from source to destination over IP network. Interface definition located in `[P7]\Headers\P7_Client.h` and name is `IP7_Client`<br />
Here is list of interface and helper functions.

## 3.1 P7\_Create\_Client() ##
_Description_: create an instance of IP7\_Client interface<br />
_Prototype_:
```
IP7_Client * P7_Create_Client(tXCHAR *i_pArgs);
```

_Arguments_:
  * i\_pArgs {in} - configuration string. The string can contains next parameters in any combinations:
    * /P7.Addr - Set server address (IPV4, IPV6, NetBios name), default value is 127.0.0.1. Examples:
      * /P7.Addr=127.0.0.1
      * /P7.Addr=192.168.0.1
      * /P7.Addr=::1
      * /P7.Addr=MyPC
    * /P7.Port - Set server port, default port is 9009
    * /P7.PSize - Set packet size. Min value 512 bytes, Max - 65535, Default - 512. You should specify optimum packet size for your network, usually it is MTU.
    * /P7.Pool - Set size of the internal buffers pool in kilobytes. Minimal 16(kb), maximal is limited by your OS and HW. Default value = 4mb
    * /P7.Verb - Set logging verbosity level. This option allow to write to text log file or stdout all internal engine messages. Do not specify /P7.Verb parameter to switch off the logging. P7 internal logging has next verbosity levels:
      * 0 : Info
      * 1 : Debug
      * 2 : Warnings
      * 3 : Errors
      * 4 : Critical
    * /P7.On - Enable (1) / Disable (0) P7 network engine, By default P7 is on

**N.B.**: if `i_pArgs` is NULL the library will use process command line arguments to search parameters.


_Return_: If function fails it will return NULL. Details will be located inside log file (Windows) or printed to console (Linux).

## 3.2 Add\_Ref() ##
_Description_: Increase object reference counter, initial object value is 1<br />

_Prototype_:
```
tINT32 IP7_Client::Add_Ref();
```

_Arguments_: No<br />
_Return_: New reference counter value<br />



## 3.3 Release() ##
_Description_: Decrease object reference counter, 0 value is equal to self-destruction<br />

_Prototype_:
```
tINT32 IP7_Client::Release();
```

_Arguments_: No<br />
_Return_: New reference counter value<br />

## 3.4 Get\_Status() ##
_Description_: Provide information about connection status<br />

_Prototype_:
```
tBOOL IP7_Client::Get_Status(sP7C_Status *o_pStatus);
```

_Arguments_:
  * o\_pStatus {out} - structure contains next fields filled by P7\_Client object:
    * bConnected - bool value, TRUE - connection with server is established, FALSE - otherwise
    * dwResets - count of connection drops

_Return_: TRUE on success, FALSE - otherwise

## 3.5 Get\_Info() ##
_Description_: Provide information about internal state<br />

_Prototype_:
```
tBOOL IP7_Client::Get_Info(sP7C_Info *o_pInfo);
```

_Arguments_:
  * o\_pInfo {out} - structure contains next fileds filled by P7\_Client object:
    * dwMem\_Used - amount of memory used by object for delivery buffers
    * dwMem\_Free - amount of free memory for delivery buffers
    * dwReject\_Mem - count of rejected call IP7\_Client::Sent(). Reason - no memory.
    * dwReject\_Con - count of rejected call IP7\_Client::Sent(). Reason - no connection
    * dwReject\_Int - count of rejected call IP7\_Client::Sent(). Reason - internal errors

_Return_: TRUE on success, FALSE - otherwise


## 3.6 Register\_Channel() ##
_Description_: Channel registration. You should register your channel to be able to start working. There are 32 channels max.<br />


_Prototype_:
```
tBOOL IP7_Client::Register_Channel(IP7C_Channel *i_pChannel);
```

_Arguments_:
  * i\_pChannel {in} - pointer to the object which implement IP7C\_Channel interface. It used to obtain Channel ID and receive incoming channel data, here is simple example:
```
   class CMyChannel:
        public IP7C_Channel
   {
       tUINT32 m_dwChannel_ID;
       
   public: 
       CMyChannel()
           : m_dwChannel_ID(0)
       {
       }

       void Init(sP7C_Channel_Info *i_pInfo)
       {
           if (i_pInfo)
           {
               m_dwChannel_ID = i_pInfo->dwID;
           }
       }

       void On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize)
       {
           //processing incoming data ....  
       }

       tUINT32 Get_ID() 
       {
           return m_dwChannel_ID; 
       }
   };

   /////////////////////////////////////////////////////////////////////
   void Main()
   {
       IP7_Client *l_pClient = P7_Create_Client(NULL);
       CMyChannel  l_cChannel;

       // ......
       // ......
       // ......

       l_pClient->Register_Channel(&l_cChannel);

       // ......

       l_pClient->Unregister_Channel(l_cChannel.Get_ID());

       // ......
   }


```

_Return_:
  * ECLIENT\_STATUS\_OK - success
  * ECLIENT\_STATUS\_WRONG\_PARAMETERS - input parameter is wrong
  * ECLIENT\_STATUS\_INTERNAL\_ERROR - no more free channels


## 3.7 Unregister\_Channel() ##
_Description_: Unregister previously registered channel<br />

_Prototype_:
```
tBOOL IP7_Client::Unregister_Channel(tUINT32 i_dwID);
```

_Arguments_:
  * i\_dwID {in} - channel ID

_Return_:
  * ECLIENT\_STATUS\_OK - success
  * ECLIENT\_STATUS\_WRONG\_PARAMETERS - input parameter is wrong, channel is not used.


## 3.8 Sent() ##
_Description_: Send data to server<br />

_Prototype_:
```
eClient_Status IP7_Client::Sent(tUINT32          i_dwChannel_ID,
                                sP7C_Data_Chunk *i_pChunks,
                                tUINT32          i_dwCount,
                                tUINT32          i_dwSize);
```

_Arguments_:
  * i\_dwChannel\_ID {in} - channel ID, you can obtain in by calling !IP7\_Client::Register\_Channel()
  * i\_pChunks {in} - pointer to the array of data chunks, every chunk has next fields:
    * pData - data pointer
    * dwSize - data size (in bytes)
  * i\_dwCount {in} - chunks count
  * i\_dwSize {in} - total memory size (in bytes) accumulated by chunks array (i\_pChunks). It is very important to specify this value, in release build there is no internal check for correct value.

_Return_:
  * ECLIENT\_STATUS\_OK - success
  * ECLIENT\_STATUS\_OFF - connection is not established, data is dropped
  * ECLIENT\_STATUS\_NO\_FREE\_BUFFERS - no free memory to process request
  * ECLIENT\_STATUS\_WRONG\_PARAMETERS - parameters are wrong

## 3.9 Share() ##
_Description_: share current P7 object instance in address space of the current process, to retrieve previously shared P7 instance use function `P7_Get_Shared(tXCHAR *i_pName);`<br />

_Prototype_:
`tBOOL Share(const tXCHAR *i_pName);`

_Arguments_:
  * i\_pName {in} - shared object name, should be unique for current process

_Return_:
  * TRUE - object instance was shared successfully
  * FALSE - provided name is not unique for current process

## 3.10 P7\_Get\_Shared() ##
_Description_: retrieve previously shared P7 instance. Doesn't work for Linux yet<br />
_Prototype_:
```
IP7_Client *P7_Get_Shared(const tXCHAR *i_pName);
```

_Arguments_:
  * i\_pName {in} - shared object name, should be unique for current process

_Return_: If function fails it will return NULL. Details will be located inside log file (Windows) or printed to console (Linux).


# 4 P7 Trace interface #
Interface is repsonsable for processing trace messages. Interface definition located in `[P7]\Headers\P7_Trace.h` and name is `IP7_Trace`<br />
Here is list of interface and helper functions.

## 4.1 P7\_Create\_Trace ##
_Description_: create an instance of IP7\_Trace interface. You can create up to 32 IP7\_Trace objects per one IP7\_Client object<br />
_Prototype_:
```
IP7_Trace * P7_Create_Trace(IP7_Client   *i_pClient,
                            const tXCHAR *i_pName
                           );
```

_Arguments_:
  * i\_pClient {in} - IP7\_Client object previously created
  * i\_pName {in} - Trace channel name. UTF-16 or UTF-8 (depending on OS) string.

_Return_: If function fails it will return NULL. Details will be located inside log file (Windows) or printed to console (Linux).


## 4.2 Add\_Ref() ##
_Description_: Increase object reference counterm, initial object value is 1<br />

_Prototype_:
```
tINT32 IP7_Trace::Add_Ref();
```

_Arguments_: No<br />
_Return_: New reference counter value<br />



## 4.3 Release() ##
_Description_: Decrease object reference counter, 0 value is equal to self-destruction<br />

_Prototype_:
```
tINT32 IP7_Trace::Release();
```

_Arguments_: No<br />
_Return_: New reference counter value<br />


## 4.4 Set\_Verbosity() ##
_Description_: Set minimal verbosity level. Traces with less level will be dropped<br />

_Prototype_:
```
void IP7_Trace::Set_Verbosity(eP7Trace_Level i_eVerbosity);
```

_Arguments_:
  * i\_eVerbosity {in} - minimal verbosity level. Next values are acceptable:
    * EP7TRACE\_LEVEL\_TRACE
    * EP7TRACE\_LEVEL\_DEBUG
    * EP7TRACE\_LEVEL\_INFO
    * EP7TRACE\_LEVEL\_WARNING
    * EP7TRACE\_LEVEL\_ERROR
    * EP7TRACE\_LEVEL\_CRITICAL

_Return_: no<br />

## 4.5 Trace() ##
_Description_: Post trace message to delivery queue, function is not blocking<br />

_Prototype_:
```
tBOOL IP7_Trace::Trace(tUINT16         i_wTrace_ID,   
                       eP7Trace_Level  i_eLevel, 
                       tUINT16         i_wModule_ID,
                       tUINT16         i_wLine,
                       const char     *i_pFile,
                       const char     *i_pFunction,
                       const tXCHAR   *i_pFormat,
                       ...
                      );
```

_Arguments_:
  * i\_wTrace\_ID {in} - HARDCODED trace ID, possible range is (0 .. 1023). This ID is used to match trace data and trace format string on server side. You can specify this parameter in range [1..1024] if you want to send a trace as quickly as possible. Otherwise you can put 0 - and this function will work a little bit slowly.
  * i\_eLevel {in} - trace level (error, warning, debug, etc) see list of the possible values inside enum eP7Trace\_Level
  * i\_wModule\_ID {in} - used specified value, you can use it on server side for filtering, searching, highlighting.
  * i\_wLine {in} - source file line number from where  your trace  is  called. Usually macro LINE is used
  * i\_pFile {in} - source file name from where your  trace  is called. Usually macro FILE is used
  * i\_pFunction {in} - function name from where  your trace  is called. Usually macro FUNCTION is used
  * i\_pFormat {in} - format  string (like L"Value = %d, %08x"). UTF-16 or UTF-8 (depending on OS) string.

Library support next type format fields characters:  I, l, h, w, c, C, d, i, o, u, x, X, p, n, S, s, e, E, f, g, G, a, A<br />
And next prefixes and format-type specifiers: I64, I32, ll, l, h, I, w<br />
Full documentation about format string you can find [here](http://msdn.microsoft.com/en-us/library/56e442dc.aspx).<br />
**N.B.: DO NOT USE VARIABLES for format string! You have to always use constant text like "My Format %d, %s"**<br />

_Return_: Function return only 2 possible values:
  * TRUE  - if trace has been posted in delivery queue
  * FALSE - can be returned by the next reasons (in descending priority order):
    * There is no connection with Baical server
    * Current trace level (i\_eLevel) has  less  priority  than current verbosity - see function Set\_Verbosity
    * P7 network engine did not have enough  time  to  deliver all your traces. This can happen when  you  are  sending events faster than Baical server can process  them. But it is difficult to reach this limit. Engine normally process 500 000 traces per second on Intel Core 2 Duo.
    * IP7\_Trace object is not initialized  properly (internal)
    * Memory allocation problems(internal)
    * P7 network engine fails (internal)



## 4.6 Trace\_Embedded() ##
_Description_: Function is equal to IP7\_Trace::Trace() with one exception: Trace\_Embedded() can be used inside another function with variable arguments<br />

_Prototype_:
```
tBOOL IP7_Trace::Trace_Embedded(tUINT16        i_wTrace_ID,   
                                eP7Trace_Level i_eLevel, 
                                tUINT16        i_wModule_ID,
                                tUINT16        i_wLine,
                                const char    *i_pFile,
                                const char    *i_pFunction,
                                const tXCHAR **i_ppFormat
                               );


```

_Arguments_: see IP7\_Trace::Trace() description<br />
_Return_: see IP7\_Trace::Trace() description<br />


## 4.7 Share() ##
_Description_: share current P7 trace object instance in address space of the current process, to retrieve previously shared P7 trace instance use function `P7_Get_Shared_Trace(tXCHAR *i_pName);`<br />

_Prototype_:
`tBOOL Share(const tXCHAR *i_pName);`

_Arguments_:
  * i\_pName {in} - shared object name, should be unique for current process

_Return_:
  * TRUE - object instance was shared successfully
  * FALSE - provided name is not unique for current process

## 4.8 P7\_Get\_Shared\_Trace() ##
_Description_: retrieve previously shared P7 instance. Doesn't work for Linux yet<br />
_Prototype_:
```
IP7_Trace *P7_Get_Shared_Trace(const tXCHAR *i_pName);
```

_Arguments_:
  * i\_pName {in} - shared object name, should be unique for current process

_Return_: If function fails it will return NULL. Details will be located inside log file (Windows) or printed to console (Linux).

# 5. P7 Telemetry interface #
Interface is repsonsable for processing telemetry samples. Interface definition located in `[P7]\Headers\P7_Telemetry.h` and name is `IP7_Telemetry`<br />
Here is list of interface and helper functions.

## 5.1 P7\_Create\_Trace ##
_Description_: create an instance of IP7\_Telemetry interface. You can create up to 32 IP7\_Telemetry objects per one IP7\_Client object<br />
_Prototype_:
```
IP7_Telemetry *P7_Create_Telemetry(IP7_Client   *i_pClient,
                                  const tXCHAR *i_pName
                                 );
```

_Arguments_:
  * i\_pClient {in} - IP7\_Client object previously created
  * i\_pName {in} - Telemetry channel name. UTF-16 or UTF-8 (depending on OS) string.

_Return_: If function fails it will return NULL. Details will be located inside log file (Windows) or printed to console (Linux).


## 5.2 Add\_Ref() ##
_Description_: Increase object reference counterm, initial object value is 1<br />

_Prototype_:
```
tINT32 IP7_Trace::Add_Ref();
```

_Arguments_: No<br />
_Return_: New reference counter value<br />

## 5.3 Release() ##
_Description_: Decrease object reference counter, 0 value is equal to self-destruction<br />

_Prototype_:
```
tINT32 IP7_Trace::Release();
```

_Arguments_: No<br />
_Return_: New reference counter value<br />

## 5.4 Create() ##
_Description_: Create additional named counter, you can create 256 counters max per session<br />

_Prototype_:
```
tBOOL Create(const tXCHAR  *i_pName, 
             tINT64         i_llMin,
             tINT64         i_llMax,
             tINT64         i_llAlarm,
             tUINT8         i_bOn,
             tUINT8        *o_pID 
            );
```

_Arguments_:<br />
  * i\_pName {in} - Telemetry counter name (case sensitive), should be **unique** for current session. UTF-16 or UTF-8 (depending on OS) string.
  * i\_llMin {in} - min value of counter.
  * i\_llMax {in} - max value of counter
  * i\_llAlarm {in} - warning counter value, value should be in range i\_llMin .. i\_llMax.
  * i\_bOn {in} - counter state after creation, TRUE = ON, FALSE = OFF. If counter is OFF you can enable it later by Baical
  * o\_pID {out} - ID of the newly created counter, this value will be used later in Add() function.

_Return_:<br />
  * TRUE - counter was added successfully
  * FALSE - fail, name is not unique or all 256 counters slots are busy

## 5.5 Add() ##
_Description_: Add sample value to the counter<br />

_Prototype_:
```
tBOOL Add(tUINT8 i_bID, 
          tINT64 i_llValue 
         );
```

_Arguments_:<br />
  * i\_bID {in} - counter's ID.
  * i\_llValue {in} - sample value.

_Return_:<br />
  * TRUE - success
  * FALSE - fail, usually when connection with Baical is not established

## 5.6 Find() ##
_Description_: Find counter's ID by name<br />

_Prototype_:
```
tBOOL Find(const tXCHAR *i_pName, 
           tUINT8       *o_pID
          );
```

_Arguments_:<br />
  * i\_pName {in} - Telemetry counter name (case sensitive). UTF-16 or UTF-8 (depending on OS) string.
  * o\_pID {out} - ID of the counter.

_Return_:<br />
  * TRUE - success
  * FALSE - fail, no counter with such name


## 5.7 Share() ##
_Description_: share current P7 telemetry object instance in address space of the current process, to retrieve previously shared P7 telemetry instance use function `P7_Get_Shared_Telemetry(tXCHAR *i_pName);`<br />

_Prototype_:
`tBOOL Share(const tXCHAR *i_pName);`

_Arguments_:
  * i\_pName {in} - shared object name, should be unique for current process

_Return_:
  * TRUE - object instance was shared successfully
  * FALSE - provided name is not unique for current process

## 5.8 P7\_Get\_Shared\_Telemetry() ##
_Description_: retrieve previously shared P7 telemetry instance. Doesn't work for Linux yet<br />
_Prototype_:
```
IP7_Telemetry *P7_Get_Shared_Telemetry(const tXCHAR *i_pName);
```

_Arguments_:
  * i\_pName {in} - shared object name, should be unique for current process

_Return_: If function fails it will return NULL. Details will be located inside log file (Windows) or printed to console (Linux).

# 6. Example #
The example demonstrates how to:
  * Create P7 client
  * Create 2 trace channels
  * Create telemetry channels
  * Send few traces
  * Send 1 000 000 telemetry samples
  * Cleanup all objects
[Source code](http://baical.googlecode.com/svn/trunk/Tests/Example/) is also available and you are able to build it under Windows or Linux. Build instructions are located at the bottom of the page.

```
#include <stdlib.h>

#include "GTypes.h"

#include "P7_Client.h"
#include "P7_Trace.h"
#include "P7_Telemetry.h"


int main(int i_iArgC, char* i_pArgV[])
{
    IP7_Client    *l_pClient    = NULL;
    IP7_Trace     *l_pTrace_1   = NULL;
    IP7_Trace     *l_pTrace_2   = NULL;
    tUINT32        l_dwIdx      = 0;
    IP7_Telemetry *l_pTelemetry = NULL;
    tUINT8         l_pTID       = 0;

    //create P7 client object
    l_pClient = P7_Create_Client(0);

    if (NULL == l_pClient)
    {
        goto l_lblExit;
    }

    //create P7 telemetry object
    l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Telemetry channel 1"));
    if (NULL == l_pTelemetry)
    {
        goto l_lblExit;
    }

    //register telemetry counter, it has values in range 0 ... 1023
    if (FALSE == l_pTelemetry->Create(TM("Test counter"), 0, 1023, 1000, 1, &l_pTID))
    {
        goto l_lblExit;
    }

    //create P7 trace object 1
    l_pTrace_1 = P7_Create_Trace(l_pClient, TM("Trace channel 1"));
    if (NULL == l_pTrace_1)
    {
        goto l_lblExit;
    }

    //create P7 trace object 2
    l_pTrace_2 = P7_Create_Trace(l_pClient, TM("Trace channel 2"));
    if (NULL == l_pTrace_2)
    {
        goto l_lblExit;
    }

    //send few trace messages
    l_pTrace_1->P7_TRACE(0, TM("Test trace message #%d"), l_dwIdx ++);
    l_pTrace_1->P7_INFO(0, TM("Test info message #%d"), l_dwIdx ++);
    l_pTrace_1->P7_DEBUG(0, TM("Test debug message #%d"), l_dwIdx ++);
    l_pTrace_1->P7_WARNING(0, TM("Test warning message #%d"), l_dwIdx ++);
    l_pTrace_1->P7_ERROR(0, TM("Test error message #%d"), l_dwIdx ++);
    l_pTrace_1->P7_CRITICAL(0, TM("Test critical message #%d"), l_dwIdx ++);
    
    l_pTrace_2->P7_QTRACE(1, 0, TM("Test quick trace on channel %d"), 2);

    //send 1 million telemetry samples
    for (tUINT64 l_qwI = 0ULL; l_qwI < 1000000ULL; l_qwI ++)
    {
        l_pTelemetry->Add(l_pTID, (l_qwI & 0x3FF));
    }
l_lblExit:
    if (l_pTelemetry)
    {
        l_pTelemetry->Release();
        l_pTelemetry = NULL;
    }

    if (l_pTrace_1)
    {
        l_pTrace_1->Release();
        l_pTrace_1 = NULL;
    }

    if (l_pTrace_2)
    {
        l_pTrace_2->Release();
        l_pTrace_2 = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}
```

To run the test use this command line example, just need to replace IP address if you run Baical on different PC:
```
P7_Example /P7.Addr=127.0.0.1
```

N.B.: Do not forget to setup your firewall (UDP, port: 9009), it can block traffic.

# 7. Integration #
Integration process is simple:
  1. Build the [library](http://baical.googlecode.com/svn/trunk/)
  1. Include into your project next headers
    * _GTypes.h_: {P7}\Headers\
    * _P7\_Client.h_: {P7}\Headers\
    * _P7\_Trace.h_: {P7}\Headers\
    * _P7\_Telemetry.h_: {P7}\Headers\
  1. Link from {P7}\Binaries\
    * Windows: _Ws2\_32.lib_ and _P7Client\_XXX.lib_ libraries where XXX is postfix 32, 32\_d, 64, 64\_d
    * Linux : _libP7.a_ library

# 8. Compilation #
## 8.1. Requirements ##
  * **Windows**
    * Windows XP or later
    * MS Visual studio 2010 (SP1) or later. I've not created project for this VS 2008 yet.
    * [SetX.exe](http://technet.microsoft.com/en-us/library/cc755104(v=ws.10).aspx) tool. Place it in "<Windows folder>\System32" folder.
  * **Linux**
    * GNU Compiler (C++), I've not yet figured out the minimum version, let me know if you do.

## 8.2. Windows ##
  1. [Checkout](http://code.google.com/p/baical/source/checkout) the project's source code. For example you checkout it to "C:\P7"
  1. Open "C:\P7\P7.sln" solution in Visual studio
  1. Select "Release" solution configuration for Win32 or X64 platform
  1. Rebuild the solution. Command from menu: "Build\Rebuild solution"
  1. Folder "C:\P7\Binaries" will contain all necessary files

## 8.3. Linux ##
Before building you need to [checkout](http://code.google.com/p/baical/source/checkout) the project's source code. For example you checkout it to "/home/Guest/Projects/P7"

Building library:
  1. Open terminal and change current directory to "/home/Guest/Projects/P7/Sources/"
  1. execute _make_ command
  1. If build is successful the library _libP7.a_ will be located in "/home/Guest/Projects/P7/Binaries"

Building example:
  1. Open terminal and change current directory to "/home/Guest/Projects/P7/Tests/Example/"
  1. execute _make_ command
  1. If build is successful the executable file will be located in "/home/Guest/Projects/P7/Binaries"

Building speed test:
  1. Open terminal and change current directory to "/home/Guest/Projects/P7/Tests/Speed/"
  1. execute _make_ command
  1. If build is successful the executable file will be located in "/home/Guest/Projects/P7/Binaries"

Building complex tests:
  1. Open terminal and change current directory to "/home/Guest/Projects/P7/Tests/Trace/"
  1. execute _make_ command
  1. If build is successful the executable file will be located in "/home/Guest/Projects/P7/Binaries"