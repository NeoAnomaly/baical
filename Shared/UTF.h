//******************************************************************************
// Copyright 2012  Zheltovskiy Andrey                                          *
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
#ifndef UTF8_H_AZH
#define UTF8_H_AZH


///////////////////////////////////////////////////////////////////////////////
//Get_utf8_Length
static tINT32 Get_UTF8_Length(const char *i_pText)
{
    tINT32        l_dwLength = 0;
    unsigned char l_bCh      = 0;

    if (NULL == i_pText)
    {
        return 0;
    }

    while ( 0 != (*i_pText))
    {
        l_bCh = *i_pText;
        if (0x7F >= l_bCh) //1 char
        {
            i_pText += 1;
        }
        else if (0xE0 > l_bCh) //11100000b = 2 chars
        {
            i_pText += 2;
        }
        else if (0xF0 > l_bCh) //11110000b  = 3 chars
        {
            i_pText += 3;
        }
        else if (0xF8 > l_bCh) //11111000b  = 4 chars
        {
            i_pText += 4;
        }
        else if (0xFC > l_bCh) //11111100b  = 5 chars
        {
            i_pText += 5;
        }
        else //6 chars
        {
            i_pText += 6;
        }

        l_dwLength ++;
    }

    return l_dwLength;
}//Get_utf8_Length


////////////////////////////////////////////////////////////////////////////////
//Convert_UTF8_To_UTF16
//Warning: this function do not produce UTF-16 surrogate pairs - function replace
//surrogate pairs by '*' char. Surrogate pairs will be implemented in next life :-)
static tINT32 Convert_UTF8_To_UTF16(const char *i_pSrc, 
                                    tWCHAR     *o_pDst, 
                                    tUINT32     i_dwMax_Len
                                   )
{
    tINT32        l_iLength = 0;
    unsigned char l_bCh     = 0;
    
    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)  
         || (0    >= i_dwMax_Len)   
       )
    {
        return l_iLength;
    }

    i_dwMax_Len -= 1; //reserve space to trailing \0;
    
    while (    ( 0 != (*i_pSrc)) 
            && (i_dwMax_Len > (tUINT32)l_iLength)
          )
    {
        l_bCh = *i_pSrc;
        if (0x7F >= l_bCh) //1 char
        {
            *o_pDst = (tWCHAR)l_bCh;
        }
        else if (0xE0 > l_bCh) //11100000b = 2 chars
        {
            *o_pDst  = (tWCHAR)((l_bCh & 0x1F) << 6);
            *o_pDst |= (*(++i_pSrc) & 0x3F);
        }
        else if (0xF0 > l_bCh) //11110000b  = 3 chars
        {
            *o_pDst  = (tWCHAR)((l_bCh & 0xF) << 12);
            *o_pDst |= (*(++i_pSrc) & 0x3F) << 6;
            *o_pDst |= (*(++i_pSrc) & 0x3F);
        }
        else if (0xF8 > l_bCh) //11111000b  = 4 chars
        {
            *o_pDst = (tWCHAR)'*';
            i_pSrc += 3;
        }
        else if (0xFC > l_bCh) //11111100b  = 5 chars
        {
            *o_pDst = (tWCHAR)'*';
            i_pSrc += 4;
        }
        else //6 chars
        {
            *o_pDst = (tWCHAR)'*';
            i_pSrc += 5;
        }
        
        o_pDst ++;
        i_pSrc ++;
        l_iLength ++;
    }
    
    *o_pDst = 0;
    
    return l_iLength;
}//Convert_UTF8_To_UTF16

#endif //UTF8_H_AZH
