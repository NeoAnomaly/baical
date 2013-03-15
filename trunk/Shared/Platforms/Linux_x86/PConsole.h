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
#ifndef PCONSOLE_H_AZH
#define PCONSOLE_H_AZH

#include <termios.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////
//Is_Key_Hit
static tBOOL Is_Key_Hit(void)
{
//  struct timeval tv;
//  fd_set rdfs;
// 
//  tv.tv_sec = 0;
//  tv.tv_usec = 0;
// 
//  FD_ZERO(&rdfs);
//  FD_SET (STDIN_FILENO, &rdfs);
// 
//  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
//  return FD_ISSET(STDIN_FILENO, &rdfs);    
    
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return TRUE;
  }
 
  return FALSE;
}//Is_Key_Hit


////////////////////////////////////////////////////////////////////////////////
//Get_Char
static tXCHAR Get_Char()
{
    return getchar();
}//Get_Char

#endif //PCONSOLE_H_AZH
