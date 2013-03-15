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
#ifndef COMMONCLIENT_H_AZH
#define COMMONCLIENT_H_AZH

// Linux:
//#include <unistd.h>   
//#include <iostream>
//#include <pthread.h>  
//#include <time.h>
//#include <sys/time.h> 
//#include <errno.h>


////////////////////////////////////////////////////////////////////////////////
//Independent
#include "Common.h"

#include "Socket.h"

#include "P7_Client.h"
//because USER_PACKET_CHANNEL_ID_MAX_SIZE defined in "P7_Client.h".
#include "TPackets.h"
#include "PacketsPool.h"


#endif //COMMONCLIENT_H_AZH
