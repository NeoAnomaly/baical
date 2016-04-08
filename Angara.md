
---



---


# 1. About #
Many years ago Microsoft presents their new technology: [Event Tracing for Windows (ETW)](http://msdn.microsoft.com/en-us/library/windows/desktop/bb968803(v=vs.85).aspx). It was great improvement for a lot of driver's developers. Now they can send thousands traces from theirs drivers without any performance issues. MS did great job in optimization, process of traces integration is a little bit complex but after that you get real power.<br />
Microsoft also provide [TraceView](http://msdn.microsoft.com/en-us/library/windows/hardware/ff553892(v=vs.85).aspx) tool for managing & viewing trace sessions.  Unfortunately the tool has problems and fixing the problems for MS isn't priority, corporation is focused on business applications developers. Here is list of the problems which I would like to solve:
  * Support of large trace files  (more than 10 Mb :-) ), TraceView can write them but opening speed and processing are not good enough.
  * Often TraceView produce damaged trace files and hours of test go to garbage.
  * Extremely slow
  * View window can show only 65536 traces
  * Navigation, highlighting, searching are absent or useless
  * Trace file is recorded on the same PC where driver is executed; often after BSD you lose your trace data.

Here is video which shows what is Angara is and how it works:<br />
<a href='http://www.youtube.com/watch?feature=player_embedded&v=o1sQ0u4dmEY' target='_blank'><img src='http://img.youtube.com/vi/o1sQ0u4dmEY/0.jpg' width='425' height=344 /></a>

# 2. Requirements #
  * PC with Windows XP or later. Or you can use 2 PC connected by network: on one your driver will execute on another Baical server will receive traces.
  * Baical server
  * Angara

# 3. How it works #
  * Your driver using ETW send messages
  * Angara (running on the same PC with a driver) receive that messages, decode them using TMF file and send messages to Baical server over network (global, local or loopback)

# 4. Usage #
## 4.1. TMF generation ##
Driver instrumentation with traces is [covered](http://msdn.microsoft.com/en-us/library/windows/hardware/ff556204(v=vs.85).aspx) by Microsoft, so I will focus only on Angara setup<br />
First of all: you need to upgrade post-build step for your driver and add generation of TMF file from PDB file after every successful build. The command is very simple:
```
tracepdb.exe -o Result.tmf -f MyDriver.pdb
```
Let's disassemble the command:
  * "tracepdb.exe" - this tool is usually located inside your Windows DDK, in my case the path is "C:\WinDDK\7600.16385.1\tools\tracing\i386\tracepdb.exe"
  * Key "-o" ask the tool to generate single TMF file instead unique TMF file for every driver's source code file.
  * Key "-f" ask the tool to use as a source your driver's PDB file, this file is generated automatically after every successful build.
If command is executed successfully you will get "Result.tmf" - this file contains text description for all trace messages which your driver can send. Also it will be used by Angara to decode incoming messages<br />

## 4.2. Configure Angara ##
The second step is Angara setup, all configuration parameters are located in "Angara.xml" (this file has UTF-16 LE encoding). Configuration file has 3 main sections:
  1. Provider section. It used to specify trace session parameters.
  1. P7Trace section. It used to specify network parameters.
  1. Levels section. It maps driver trace levels to understandable levels for Baikal.
### 4.2.1. Provider section ###
XML node example:
```
<Provider TMF="C:\MyDriver\TMF\"
          GUID="{63423ADB-D156-48d5-B0BC-8ACAEDDBD810}"
          Level="5"
          Flags="0xFFFFFFFFFFFFFFFF"
          Filter=""
          Platform="X32"
/>
```
Parameters description:
  * TMF - directory where are located generated TMF files. If you change some TMF files while Angara is running you should restart it.
  * GUID - Event trace session  GUID, it is hardcoded for every driver. You can find it inside your driver source code - marco "WPP\_DEFINE\_CONTROL\_GUID"
  * Level - You can specify minimum trace level, here is list of values from Evntrace.h:
    * 0 - NONE       : Tracing is not on
    * 1 - CRITICAL   : Abnormal exit or termination
    * 2 - ERROR      : Severe errors that need logging
    * 3 - WARNING    : Warnings such as allocation failure
    * 4 - INFORMATION: Includes non-error cases(e.g.,Entry-Exit)
    * 5 - VERBOSE    : Detailed traces from intermediate steps
  * Flags - Hex bitmask of keywords that determine the category of events that you want the provider to write. The provider writes the event  if  any of the event's keyword bits match any of the bits set in this mask. 64 bits value max, example Flags="0xFFFFFFFFFFFFFFFF" 64 bits flag available only for Vista, Windows 7 and future OS. For old OS will be used only first 32 bits.
  * Filter - The provider uses to filter data to prevent events that  match  the filter criteria from being written to  the  session;  the  provider determines the layout of the data and how it applies the filter  to the event's data. You should specify you binary filter data in hex mode, for  example Filter="0x10BB56CAB2", Angara will deliver this 5 bytes to provider. And your provider should decide  how  it  will use  this  data  for filtering. Angara use custom filter type equal to 0xAB1DE.
  * Platform : provider platform (X32 or X64), this flag take effect on pointers printing.

### 4.2.2. P7Trace ###
XML node example:
```
<P7Trace Address="localhost"
         Port="9009"        
         Packet="1472"      
         Name="MyChannel"   
         Verbosity="1"      
/>                          
```

Parameters description:
  * Address - Baical server address (IPv4/IPv6/Name)
  * Port - Baical server port (usually 9009)
  * Packet - Minimum transfer unit. I recommend to use values 512..1472 for all network interfaces, except loopback (127.0.0.1, ::1), for loopback you can use up to 65535, it will increase delivery speed. Wrong value can cause delivery problem. Value 512 will work all the time, even with very old hardware, 1472 will work on MOST of all modern networks. Greater values than 1472 usually will work only on loopback interfaces
  * Verbosity - If you specify this parameter - Angara will dump all internal messages to log file. If you do not specify this parameter = internal logging is off. Here are possible values:
    * 0 - Debug
    * 1 - Info
    * 2 - Warnings
    * 3 - Errors
    * 4 - Critical
  * Name - trace channel name, max length is 64 characters. You will see this name in Baical server channels list.

### 4.2.3. Levels section ###
XML node example:
```
<Levels>                                     
    <Trace>                                  
        <WPP Text="TRACE_LEVEL_VERBOSE"/>    
    </Trace>                                 
    <Debug>                                  
        <WPP Text="TRACE_LEVEL_NONE"/>       
    </Debug>                                 
    <Info>                                   
        <WPP Text="TRACE_LEVEL_INFORMATION"/>
    </Info>                                  
    <Warning>                                
        <WPP Text="TRACE_LEVEL_WARNING"/>    
    </Warning>                               
    <Error>                                  
        <WPP Text="TRACE_LEVEL_ERROR"/>      
    </Error>                                 
    <Critical>                               
        <WPP Text="TRACE_LEVEL_FATAL"/>      
        <WPP Text="TRACE_LEVEL_CRITICAL"/>   
    </Critical>                              
</Levels>                                    
```

ETW level names are  located inside "km-default.tpl" file
```
// Define anything which is needs but missing from
// older versions of the DDK.
#include <evntrace.h>
#include <stddef.h>
#include <stdarg.h>
#include <wmistr.h>

#if !defined(TRACE_LEVEL_NONE)
  #define TRACE_LEVEL_NONE        0
  #define TRACE_LEVEL_CRITICAL    1
  #define TRACE_LEVEL_FATAL       1
  #define TRACE_LEVEL_ERROR       2
  #define TRACE_LEVEL_WARNING     3
  #define TRACE_LEVEL_INFORMATION 4
  #define TRACE_LEVEL_VERBOSE     5
  #define TRACE_LEVEL_RESERVED6   6
  #define TRACE_LEVEL_RESERVED7   7
  #define TRACE_LEVEL_RESERVED8   8
  #define TRACE_LEVEL_RESERVED9   9
#endif
```

Usually developers do not override then, but if you do you should specify new levels names in Levels section. <br />
New levels example:
```
// Define new debug levels
#define     NONE     0 // Tracing is not on
#define     FATAL    1 // Abnormal exit or termination
#define     ERROR    2 // Severe errors that need logging
#define     WARNING  3 // Warnings such as allocation failure
#define     INFO     4 // Includes non-error cases such as Entry-Exit
#define     TRACE    5 // Detailed traces from intermediate steps
#define     LOUD     6 // Detailed trace from every step
```

Corresponding XML modification:
```
<Levels>                                     
    <Trace>                                  
        <WPP Text="LOUD"/>    
    </Trace>                                 
    <Debug>                                  
        <WPP Text="TRACE"/>    
    </Debug>                                 
    <Info>                                   
        <WPP Text="INFO"/>
    </Info>                                  
    <Warning>                                
        <WPP Text="WARNING"/>    
    </Warning>                               
    <Error>                                  
        <WPP Text="ERROR"/>      
    </Error>                                 
    <Critical>                               
        <WPP Text="FATAL"/>      
    </Critical>                              
</Levels>                                    
```


# 4.3. Starting #
To start Angara you should:
  1. Generate TMF file
  1. Prepare XML configuration file. List of the obligatory parameters is very short
    * Provider\TMF
    * Provider\GUID
    * P7Trace\Address
    * Levels, only if you override trace levels names in your driver
  1. Make sure that Baical is running, it is not mandatory, you can run it later.
  1. Make sure that firewall will not block Angara traffic

If all steps are done you can execute command:
```
Angara_32.exe Angara.xml
```


# 4.4. Application main screen #
It is very simple and minimalist:<br />
![http://baical.googlecode.com/svn/wiki/Images/Angara.png](http://baical.googlecode.com/svn/wiki/Images/Angara.png)

Angara provides you 12 counters in 3 groups. They were actively used only during application debug, but I save them for monitoring purposes, here they are:
  * Angara. Main statistics:
    * Send � total amount of trace messages were sent
    * Rejected - total amount of rejected trace messages (all reasons)
    * Unknown � total amount of unknown trace messages. Usually this counter grows when Angara can't recognize incoming trace message. There are 2 possible reasons:
      * Corresponding TMF file missing
      * Corresponding TMF file is out of date, you need to regenerate it.
    * TPS - traces per second
  * P7. Network statistics:
    * Free mem. - percentage of free memory
    * Rej. No connect. - amount of rejected trace messages. Reason - connection with Baical is not established.
    * Rej. No Memory - amount of rejected trace messages. Reason - not enough memory.
    * Rej. Internal - amount of rejected trace messages. Reason - internal problems.
  * ETW statistics:
    * Traces lost - amount of lost trace messages by ETW
    * Buffers lost - amount of lost trace message buffers by ETW
    * RealT. Buf. lost - amount of lost real time trace message buffers by ETW
    * Broken Seq. - amount of skipped sequence numbers