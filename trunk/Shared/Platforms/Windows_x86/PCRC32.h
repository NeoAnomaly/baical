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
// This header file provide CRC32 functions                                    *
//******************************************************************************
#pragma once

tUINT32 Get_CRC32(tUINT8 *i_pData, tUINT32 i_dwCount)
{
    tUINT32 l_dwResult = 0xFFFFFFFF;

    if (    (NULL == i_pData)
         || (0  >= i_dwCount) 
       )
    {
        return l_dwResult;
    }


#if defined(_M_X64) || defined(__amd64__) || defined(__amd64) || defined(_WIN64) || defined(__linux__)

    for (tUINT32 l_dwIDX = 0;  l_dwIDX < i_dwCount;  l_dwIDX ++)
    {
        l_dwResult = g_pCRC32_Table[(l_dwResult  ^ i_pData[l_dwIDX]) & 0xFF ] ^ (l_dwResult >> 8);
    }

#else

    // Register use:
    //  eax - CRC32 value
    //  ebx - a lot of things
    //  ecx - CRC32 value
    //  edx - address of end of buffer
    //  esi - address of start of buffer
    //  edi - CRC32 table
    __asm
    {
            // Save the esi and edi registers
            push esi
            push edi

            mov ecx, l_dwResult                 // load dwCrc32

            mov edi, g_ppCRC32_Table            // Load the CRC32 table

            mov esi, i_pData                    // Load buffer
            mov ebx, i_dwCount                  // Load dwBytesRead
            lea edx, [esi + ebx]                // Calculate the end of the buffer

    crc32loop:
            xor eax, eax                        // Clear the eax register
            mov bl, byte ptr [esi]              // Load the current source byte

            mov al, cl                          // Copy crc value into eax
            inc esi                             // Advance the source pointer

            xor al, bl                          // Create the index into the CRC32 table
            shr ecx, 8

            mov ebx, [edi + eax * 4]            // Get the value out of the table
            xor ecx, ebx                        // xor with the current byte

            cmp edx, esi                        // Have we reached the end of the buffer?
            jne crc32loop

            // Restore the edi and esi registers
            pop edi
            pop esi

            lea eax, l_dwResult                 // Load the pointer to dwCrc32
            mov [eax], ecx                      // Write the result
    }

#endif

    return l_dwResult;
}


// #if  defined(_WIN32) || defined(_WIN64)
// tUINT32 Get_CRC32_SSE4(tUINT8 *i_pData, tUINT32 i_dwCount)
// {
//     tUINT32 l_dwCount  = i_dwCount >> 2; //count of DWORDs
//     tUINT32 l_dwResult = 0x0;
//     
//     while (l_dwCount--)
//     {
//         l_dwResult = _mm_crc32_u32(l_dwResult, *(DWORD*)i_pData);
//         i_pData += 4;
//     }
// 
//     l_dwCount = i_dwCount & 0x3; //bytes count at the tail
// 
//     while (l_dwCount--)
//     {
//         l_dwResult = _mm_crc32_u8(l_dwResult, *i_pData);
//         i_pData ++;
//     }
// 
//     return l_dwResult;
// } 
// #endif 