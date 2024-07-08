/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's
 * LICENSE.md for details.
 * Copyright (C) 2022 LSEG. All rights reserved.
*/

#ifndef __JSONHANDLER_H
#define __JSONHANDLER_H

#include "rtr/rsslRDMMsg.h"
#include <cjson/cJSON.h>

RsslDomainTypes jsonGetDomainType(cJSON* json);
RsslRDMLoginMsgType jsonGetRDMLoginMsgType(cJSON* json);
RsslRDMDirectoryMsgType jsonGetRDMDirectoryMsgType(cJSON* json);
const char* jsonGetRefreshStateText(cJSON* json);
RsslStreamStates jsonGetStreamState(cJSON* json);
int jsonGetSupportOMMPost(cJSON* json);
const char* jsonGetLoginRefreshApplicationName(cJSON* json);
RsslInt32 jsonGetStreamId(cJSON* json);
RsslBool jsonIsFinalState(cJSON* json);
const char* jsonGetMsgKeyName(cJSON* json);
RsslBool jsonCheckPostUserInfo(cJSON* json);
RsslBool jsonIsRefreshComplete(cJSON* json);
RsslBool jsonRefreshMsgDataStateOK(cJSON* json);

#define RSSL_MC_JSON_PING		253
#define RSSL_MC_JSON_PONG		254
#define RSSL_MC_JSON_UNKNOWN	255

RsslMsgClasses jsonGetMsgClass(cJSON* json);

#endif
