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
#ifndef COMMON_H_AZH
#define COMMON_H_AZH

#define CLIENT_PROTOCOL_VERSION                                         (0x0007)

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include 
// WinSDKVer.h and set the _WIN32_WINNT macro to the platform you wish to support 
// before including SDKDDKVer.h.

#pragma warning(disable : 4267)

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include "PTypes.h"
#include "GTypes.h"

#include "Length.h"

#include "PAtomic.h"
#include "PLock.h"
#include "PString.h"
#include "PTime.h"
#include "Ticks.h"
#include "IMEvent.h"
#include "PMEvent.h"
#include "PThreadShell.h"
#include "CRC32.h"
#include "RBTree.h"
#include "AList.h"
#include "PShared.h"

#include "PWString.h"
#include "PFileSystem.h"

#include "PProcess.h"
#include "IJournal.h"
#include "PJournal.h"

#endif //COMMON_H_AZH
