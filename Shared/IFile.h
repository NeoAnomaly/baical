//******************************************************************************
// Copyright 2013  Zheltovskiy Andrey                                          *
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
#ifndef IFILE_H_AZH
#define IFILE_H_AZH



////////////////////////////////////////////////////////////////////////////////
class IFile
{
public:
    enum eFlags
    {
        EOPEN                                                    = 0x00000001UL,
        ECREATE                                                  = 0x00000002UL,
        ESHARE_WRITE                                             = 0x00000004UL,
        ESHARE_READ                                              = 0x00000008UL
    };

public:
    virtual tBOOL          Open(const tXCHAR *i_pName, tUINT32 i_dwFlags)   = 0;
    virtual tBOOL          Close(tBOOL i_bFlush)                            = 0;
    virtual tBOOL          Set_Position(tUINT64 i_qwOffset)                 = 0;
    virtual tUINT64        Get_Position()                                   = 0;
    virtual tUINT64        Get_Size()                                       = 0;
    virtual tUINT64        Write(const tUINT8 *i_pBuffer,
                                 size_t        i_szBuffer,
                                 tBOOL         i_bFlush
                                )                                           = 0;
    virtual tUINT64        Read(tUINT8 *o_pBuffer, size_t i_szBuffer)       = 0;

    virtual tINT32         Add_Ref()                                        = 0;
    virtual tINT32         Release()                                        = 0;
};


#endif //IFILE_H_AZH
