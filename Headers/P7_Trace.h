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
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7.Tace                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_TRACE_H_AZH
#define P7_TRACE_H_AZH

////////////////////////////////////////////////////////////////////////////////
enum eP7Trace_Level
{
    EP7TRACE_LEVEL_TRACE        = 0,
    EP7TRACE_LEVEL_DEBUG           ,
    EP7TRACE_LEVEL_INFO            ,
    EP7TRACE_LEVEL_WARNING         ,
    EP7TRACE_LEVEL_ERROR           ,
    EP7TRACE_LEVEL_CRITICAL        ,

    EP7TRACE_LEVEL_COUNT           ,
};


////////////////////////////////////////////////////////////////////////////////
#define P7_DELIVER(i_wID, i_eLevel, i_dwModuleID, i_pFormat, ...) Trace(i_wID,\
                                                                        i_eLevel,\
                                                                        i_dwModuleID,\
                                                                        (tUINT16)__LINE__,\
                                                                        __FILE__,\
                                                                        __FUNCTION__,\
                                                                        i_pFormat,\
                                                                        __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QTRACE(i_wID, i_dwModuleID, i_pFormat, ...)    P7_DELIVER(i_wID,\
                                                                     EP7TRACE_LEVEL_TRACE,\
                                                                     i_dwModuleID,\
                                                                     i_pFormat,\
                                                                     __VA_ARGS__)
#define P7_TRACE(i_dwModuleID, i_pFormat, ...)            P7_QTRACE(0,\
                                                                    i_dwModuleID,\
                                                                    i_pFormat,\
                                                                    __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QDEBUG(i_wID, i_dwModuleID, i_pFormat, ...)    P7_DELIVER(i_wID,\
                                                                     EP7TRACE_LEVEL_DEBUG,\
                                                                     i_dwModuleID,\
                                                                     i_pFormat,\
                                                                     __VA_ARGS__)
#define P7_DEBUG(i_dwModuleID, i_pFormat, ...)            P7_QDEBUG(0,\
                                                                    i_dwModuleID,\
                                                                    i_pFormat,\
                                                                    __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QINFO(i_wID, i_dwModuleID, i_pFormat, ...)     P7_DELIVER(i_wID,\
                                                                     EP7TRACE_LEVEL_INFO,\
                                                                     i_dwModuleID,\
                                                                     i_pFormat,\
                                                                     __VA_ARGS__)
#define P7_INFO(i_dwModuleID, i_pFormat, ...)             P7_QINFO(0,\
                                                                   i_dwModuleID,\
                                                                   i_pFormat,\
                                                                   __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QWARNING(i_wID, i_dwModuleID, i_pFormat, ...)  P7_DELIVER(i_wID,\
                                                                     EP7TRACE_LEVEL_WARNING,\
                                                                     i_dwModuleID,\
                                                                     i_pFormat,\
                                                                     __VA_ARGS__)
#define P7_WARNING(i_dwModuleID, i_pFormat, ...)          P7_QWARNING(0,\
                                                                      i_dwModuleID,\
                                                                      i_pFormat,\
                                                                      __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QERROR(i_wID, i_dwModuleID, i_pFormat, ...)    P7_DELIVER(i_wID,\
                                                                     EP7TRACE_LEVEL_ERROR,\
                                                                     i_dwModuleID,\
                                                                     i_pFormat,\
                                                                     __VA_ARGS__)
#define P7_ERROR(i_dwModuleID, i_pFormat, ...)            P7_QERROR(0,\
                                                                    i_dwModuleID,\
                                                                    i_pFormat,\
                                                                    __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
#define P7_QCRITICAL(i_wID, i_dwModuleID, i_pFormat, ...) P7_DELIVER(i_wID,\
                                                                     EP7TRACE_LEVEL_CRITICAL,\
                                                                     i_dwModuleID,\
                                                                     i_pFormat,\
                                                                     __VA_ARGS__)
#define P7_CRITICAL(i_dwModuleID, i_pFormat, ...)         P7_QCRITICAL(0,\
                                                                      i_dwModuleID,\
                                                                      i_pFormat,\
                                                                      __VA_ARGS__)

//__declspec(novtable)
class IP7_Trace
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Add_Ref - increase object's reference count
    //
    //Output:
    // tINT32 - new reference count value
    virtual tINT32 Add_Ref()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Release - decrease object's reference count. If reference count less or
    //          equal to 0 - object will be destroyed
    //
    //Output:
    // tINT32 - new reference count value
    virtual tINT32 Release()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Set_Verbosity - function to set trace verbosity, all traces with less  
    //                priority will be rejected
    //Input:
    // i_eVerbosity - verbosity to set. see list of the possible values inside
    //                enum eP7Trace_Level
    virtual void Set_Verbosity(eP7Trace_Level i_eVerbosity)                 = 0;


    ////////////////////////////////////////////////////////////////////////////
    //Trace - send trace message to Baical server.
    //
    //Inputs:
    // i_wTrace_ID - HARDCODED trace ID, possible range is [0 .. 1023]. This  ID
    //               is used to match trace data and  trace  format   string  on 
    //               server side. You  can  specify  this   parameter  in  range 
    //               [1..1023] if you  want  to  send  a  trace  as  quickly  as 
    //               possible. Otherwise  you can put 0 - and this function will
    //               work a little bit slowly.
    // i_eLevel    - trace level (error, warning, debug, etc)  see  list of  the 
    //               possible values inside enum eP7Trace_Level
    // i_wLine     - source file line number from where  your trace  is  called. 
    //               Usually we are using macro __LINE__
    // i_pFile     - source file name from where your  trace  is called. Usually 
    //               we are using macro __FILE__
    // i_pFunction - function name from where  your trace  is called. Usually we 
    //               are using macro __FUNCTION__
    // i_pFormat   - format  string (like L"Value = %d, %08x"). We support next                  
    //               type field characters: 
    //                 I, l, h, w, c, C, d, i, o, u, x, X, p, n, S, s, e, E, f, 
    //                 g, G, a, A
    //               And next prefixes and format-Type specifiers:
    //                 I64, I32, ll, l, h, I, w
    //               Full documentation about format string you can find here:
    //               http://msdn.microsoft.com/en-us/library/56e442dc.aspx
    //               N.B.: DO  NOT USE VARIABLE for format  string!  You  should
    //                     always use constant text like L"My Format %d, %s"
    // ...         - arguments variable list
    //Output:
    // tBOOL       - Function return only 2 possible values:
    //              - TRUE  - if trace has been posted in delivery queue
    //              - FALSE - can be returned by the next reasons (in descending 
    //                        priority order):
    //                - There is no connection with Baical server 
    //                - Current trace level (i_eLevel) has  less  priority  than
    //                  current verbosity - see function Set_Verbosity
    //                - P7 network engine did not have enough  time  to  deliver
    //                  all your traces. This can happen when  you  are  sending 
    //                  events more faster than Baical server can process  them. 
    //                  But it is difficult to reach this limit. Engine normally 
    //                  process 500 000 traces per second on Intel Core 2 Duo.
    //                - IP7_Trace object is not initialized  properly (internal)
    //                - Memory allocation problems(internal)
    //                - P7 network engine fails (internal)
    //Limitations: - If you are using UTF-8  as  format  string  you  have  next
    //               restrictions:
    //                - Format string contains "%S" can be processed  on  Baical 
    //                  side with errors in some cases. Use "%s" instead
    //                - Format string contains "%c" or "%C" process  only  ASCII 
    //                  characters
    //                  
    //
    //
    //
    //
    //
    //
    virtual tBOOL Trace(tUINT16         i_wTrace_ID,   
                        eP7Trace_Level  i_eLevel, 
                        tUINT16         i_wModule_ID,
                        tUINT16         i_wLine,
                        const char     *i_pFile,
                        const char     *i_pFunction,
                        const tXCHAR   *i_pFormat,
                        ...
                       )                                                    = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Trace - send trace message to Baical server.
    //this function is a copy of Trace function, but it dedicated to embedded 
    //usage (you already have function with variable arguments, but body of you
    //function you can replace by eTrace).
    virtual tBOOL Trace_Embedded(tUINT16       i_wTrace_ID,   
                                eP7Trace_Level i_eLevel, 
                                tUINT16        i_wModule_ID,
                                tUINT16        i_wLine,
                                const char    *i_pFile,
                                const char    *i_pFunction,
                                const tXCHAR **i_ppFormat
                               )                                            = 0;
};


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Trace - function create new instance of IP7_Trace object
//
//Inputs:
// i_pClient - P7 network engine. This engine will be used to deliver trace to
//             Baical server
// i_pName   - Name of the session, you can specify any name. It will be used
//             by Baical to show you trace session
//Output:
// IP7_Trace - object for traces posting. If NULL - internal errors during
//             initialization.
extern IP7_Trace * __stdcall P7_Create_Trace(IP7_Client   *i_pClient,
                                             const tXCHAR *i_pName
                                            );



////////////////////////////////////////////////////////////////////////////////
//This functions allow you to get trace/client instance if it was created by 
//someone other inside current process. If no instance was registered inside
//current process - function will return NULL. Do not forget to call Release
//on IP7_Trace/IP7_Client when you finish your work
//N.B.: Call of this functions can be slow, do not call this functions often, 
//      the best choice is to call it once per module (DLL, LIB, ...) and then
//      redistribute pointer inside module by other way
//extern IP7_Trace  * __stdcall P7_Get_Trace();
//extern IP7_Client * __stdcall P7_Get_Client();


#endif //P7_TRACE_H_AZH