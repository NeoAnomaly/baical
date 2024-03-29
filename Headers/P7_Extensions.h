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
// P7 extensions - interfaces to some additional functionality                 /
//  - IP7_Trace : traces formatting and delivery to Baical server              /
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_EXTENSIONS_H_AZH
#define P7_EXTENSIONS_H_AZH


#define P7_EXTENSION_TYPE_BITS_COUNT                                         (5)
#define P7_EXTENSION_SUB_TYPE_BITS_COUNT                                     (5)
#define P7_EXTENSION_PACKET_SIZE_BITS_COUNT                                 (22)

#define P7_EXTENSION_MAX_TYPES               (1 << P7_EXTENSION_TYPE_BITS_COUNT)

#define P7TRACE_NAME_LENGTH                                                 (64)
#define P7TELEMETRY_NAME_LENGTH                                             (64)
#define P7TELEMETRY_COUNTER_NAME_LENGTH                                     (64)
#define P7TELEMETRY_COUNTERS_MAX_COUNT                    ((tUINT8)~((tUINT8)0))


#define P7TRACE_INFO_FLAG_BIG_ENDIAN                                    (0x0001)
#define P7TRACE_INFO_FLAG_UNSORTED                                      (0x0002)

#define P7TRACE_ARG_TYPE_CHAR                                             (0x01)
#define P7TRACE_ARG_TYPE_WCHAR                                            (0x02)
#define P7TRACE_ARG_TYPE_INT16                                            (0x03)
#define P7TRACE_ARG_TYPE_INT32                                            (0x04)
#define P7TRACE_ARG_TYPE_INT64                                            (0x05)
#define P7TRACE_ARG_TYPE_DOUBLE                                           (0x06)
#define P7TRACE_ARG_TYPE_PVOID                                            (0x07)
//unicode - UTF16 string
#define P7TRACE_ARG_TYPE_WSTR                                             (0x08)
//ASCII string
#define P7TRACE_ARG_TYPE_ASTR                                             (0x09)
//unicode - UTF8 string
#define P7TRACE_ARG_TYPE_USTR                                             (0x0A)


#if   defined(_M_X64)\
   || defined(__amd64__)\
   || defined(__amd64)\
   || defined(_WIN64)\
   || defined(__LP64__)\
   || defined(_LP64)\
   || defined(__x86_64__)\
   || defined(__ppc64__)

    #define SIZE_OF_ARG(t)   ( (sizeof(t) + 8 - 1) & ~(8 - 1) )

    typedef tUINT64 tKeyType;
#else
    #define SIZE_OF_ARG(t)   ( (sizeof(t) + 4 - 1) & ~(4 - 1) )

    typedef tUINT32 tKeyType;
#endif


////////////////////////////////////////////////////////////////////////////////
// Data types, we can transmit different packets types, here are the list 
// of all supported for the moment packet types
enum eP7User_Type
{
    EP7USER_TYPE_TRACE          =  0, 
    EP7USER_TYPE_TELEMETRY          , 

    EP7USER_TYPE_MAX            = P7_EXTENSION_MAX_TYPES 
};


////////////////////////////////////////////////////////////////////////////////
enum eP7Trace_Type
{
    EP7TRACE_TYPE_INFO          =  0, //OUT
    EP7TRACE_TYPE_DESC              , //OUT
    EP7TRACE_TYPE_DATA              , //OUT
    EP7TRACE_TYPE_VERB              , //IN
    EP7TRACE_TYPE_CLOSE             , //OUT

    EP7TRACE_TYPE_MAX           = 32 
};

////////////////////////////////////////////////////////////////////////////////
enum eP7Tel_Type
{
    EP7TEL_TYPE_INFO            =  0, //OUT
    EP7TEL_TYPE_COUNTER             , //OUT
    EP7TEL_TYPE_VALUE               , //OUT
    EP7TEL_TYPE_ENABLE              , //IN
    EP7TEL_TYPE_CLOSE               , //OUT

    EP7TEL_TYPE_MAX             = 32 
};


PRAGMA_PACK_ENTER(4) //alignment is now 4, MS Only//////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// common user header for extensions (like trace or telemetry)
struct sP7Ext_Header
{
   tUINT32 dwType   :P7_EXTENSION_TYPE_BITS_COUNT;     //eP7User_Type
   tUINT32 dwSubType:P7_EXTENSION_SUB_TYPE_BITS_COUNT;  
   //max 4 mb, value should include size of this header
   tUINT32 dwSize   :P7_EXTENSION_PACKET_SIZE_BITS_COUNT; 
   //At the end of structure we put serialized data
} ATTR_PACK(4);
//N.B.: extension packets can follow one by one in data packets.

PRAGMA_PACK_EXIT()//4///////////////////////////////////////////////////////////



PRAGMA_PACK_ENTER(2) //alignment is now 2, MS Only//////////////////////////////


//trace info header
struct sP7Trace_Info
{
    sP7Ext_Header sCommon;
    //Contains a 64-bit value representing the number of 100-nanosecond intervals 
    //since January 1, 1601 (UTC). In windows we use FILETIME structure for 
    //representing
    tUINT32       dwTime_Hi;
    tUINT32       dwTime_Lo;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    tUINT64       qwTimer_Value;
    //timer's count heartbeats in second
    tUINT64       qwTimer_Frequency;
    tUINT64       qwFlags; 
    tWCHAR        pName[P7TRACE_NAME_LENGTH];
} ATTR_PACK(2);


//this structure describe each argument inside variable arguments list
//all arguments are serialized data block 
struct sP7Trace_Arg
{
    //argument's type - one of P7TRACE_ARG_TYPE_XXX
    tUINT8 bType; 
    //Size - how many bytes is used by argument inside block, this value is not
    //       directly depend on type, usually it depend on processor architecture
    //       for example "char" on WIN32 this is 4 bytes, but for EventTrace 
    //       engine this is only 1 byte.
    //       N.B.: All strings has 0 size.
    tUINT8 bSize; 
} ATTR_PACK(2);

//trace description header
struct sP7Trace_Format
{
    sP7Ext_Header sCommon;
    tUINT16       wID;
    tUINT16       wLine;
    tUINT16       wModuleID;  //Module ID, who send trace
    tUINT16       wArgs_Len;  //arguments count
    //At the end of structure we put serialized data:
    //sP7Trace_Arg [dwArgs_Len]   - array of arguments
    //wchar_t      Format[]       - null terminated string
    //char         FileName[]     - null terminated string
    //char         FunctionName[] - null terminated string
} ATTR_PACK(2);

//trace data header
struct sP7Trace_Data
{
    sP7Ext_Header sCommon;
    tUINT16       wID;          //trace ID
    tUINT8        bLevel;       //eP7Trace_Level
    tUINT8        bProcessor;   //Processor number
    tUINT32       dwThreadID;   //Thread ID
    tUINT32       dwSequence;   //sequence number
    tUINT64       qwTimer;      //High resolution timer value
    //At the end of structure we put serialized data:
    // - trace variable arguments values
} ATTR_PACK(2);

//trace verbosity header
struct sP7Trace_Verb
{
    sP7Ext_Header    sCommon;
    eP7Trace_Level   eVerbosity;
} ATTR_PACK(2);


//telemetry info header
struct sP7Tel_Info
{
    sP7Ext_Header sCommon;
    //Contains a 64-bit value representing the number of 100-nanosecond intervals 
    //since January 1, 1601 (UTC). In windows we use FILETIME structure for 
    //representing
    tUINT32       dwTime_Hi;
    tUINT32       dwTime_Lo;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    tUINT64       qwTimer_Value;
    //timer's count heartbeats in second
    tUINT64       qwTimer_Frequency;
    tUINT64       qwFlags; 
    tWCHAR        pName[P7TELEMETRY_NAME_LENGTH];
} ATTR_PACK(2);


//Telemetry counter description
struct sP7Tel_Counter
{
    sP7Ext_Header sCommon;
    tUINT8        bID;
    tUINT8        bOn;
    tINT64        llMin;
    tINT64        llMax;
    tINT64        llAlarm;
    tWCHAR        pName[P7TELEMETRY_COUNTER_NAME_LENGTH];
} ATTR_PACK(2);

//telemetry counter On/Off verbosity header
struct sP7Tel_Enable
{
    sP7Ext_Header    sCommon;
    tUINT8           bID;
    tUINT8           bOn;
} ATTR_PACK(2);


//Telemetry counter value
struct sP7Tel_Value
{
    sP7Ext_Header sCommon;
    tUINT8        bID;
    tUINT8        bReserved;    //[1 byte hole - to be used later]
    tUINT64       qwTimer;      //High resolution timer value
    tINT64        llValue;
} ATTR_PACK(2);



PRAGMA_PACK_EXIT()//2///////////////////////////////////////////////////////////


#endif //P7_EXTENSIONS_H_AZH
