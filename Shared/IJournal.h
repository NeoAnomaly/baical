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
// Simple logger to file/stdout                                                /
// Each start it create new file and delete old files if count of files is more/
// than allowed                                                                /
// It works in different thread                                                /
////////////////////////////////////////////////////////////////////////////////
#ifndef IJOURNAL_H_AZH
#define IJOURNAL_H_AZH

#define JOURNAL_DEBUG(i_pLogger,    ...)   if (i_pLogger) (i_pLogger)->Log(EFJOIRNAL_TYPE_DEBUG,    __FUNCTION__, __LINE__, __VA_ARGS__);
#define JOURNAL_INFO(i_pLogger,     ...)   if (i_pLogger) (i_pLogger)->Log(EFJOIRNAL_TYPE_INFO,     __FUNCTION__, __LINE__, __VA_ARGS__);
#define JOURNAL_WARNING(i_pLogger,  ...)   if (i_pLogger) (i_pLogger)->Log(EFJOIRNAL_TYPE_WARNING,  __FUNCTION__, __LINE__, __VA_ARGS__);
#define JOURNAL_ERROR(i_pLogger,    ...)   if (i_pLogger) (i_pLogger)->Log(EFJOIRNAL_TYPE_ERROR,    __FUNCTION__, __LINE__, __VA_ARGS__);
#define JOURNAL_CRITICAL(i_pLogger, ...)   if (i_pLogger) (i_pLogger)->Log(EFJOIRNAL_TYPE_CRITICAL, __FUNCTION__, __LINE__, __VA_ARGS__);


////////////////////////////////////////////////////////////////////////////////
enum eFJournal_Type
{
    EFJOIRNAL_TYPE_DEBUG     = 0,
    EFJOIRNAL_TYPE_INFO         , 
    EFJOIRNAL_TYPE_WARNING      ,
    EFJOIRNAL_TYPE_ERROR        ,
    EFJOIRNAL_TYPE_CRITICAL     ,

    EFJOIRNAL_TYPES_COUNT       ,
};


////////////////////////////////////////////////////////////////////////////////
class IJournal
{
public:
    virtual tBOOL          Initialize(const tXCHAR *i_pName)                = 0;
    virtual void           Set_Verbosity(eFJournal_Type i_eVerbosity)       = 0;
    virtual eFJournal_Type Get_Verbosity()                                  = 0;
    virtual tBOOL          Log(eFJournal_Type i_eType, 
                               const char    *i_pFunction, 
                               tUINT32        i_dwLine,
                               const tXCHAR  *i_pFormat, 
                               ...
                              )                                             = 0;

    virtual tUINT64        Get_Count()                                      = 0;
    virtual tINT32         Add_Ref()                                        = 0;
    virtual tINT32         Release()                                        = 0;
};

#endif //IJOURNAL_H_AZH