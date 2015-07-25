/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#ifndef WL_STREAM_H
#define WL_STREAM_H

#include "rtr/wlBase.h"
#include "rtr/wlLogin.h"
#include "rtr/wlDirectory.h"
#include "rtr/wlItem.h"
#include "rtr/wlSymbolList.h"
#include "rtr/rsslRDMMsg.h"
#include "rtr/rsslHashTable.h"
#include "rtr/rsslQueue.h"
#include "rtr/rsslTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union WlStream WlStream;

typedef union WlRequest WlRequest;

union WlRequest
{
	WlRequestBase		base;
	WlLoginRequest		login;
	WlDirectoryRequest	directory;
	WlItemRequest		item;
	WlSymbolListRequest	symbolList;
};

union WlStream
{
	WlStreamBase		base;
	WlLoginStream		login;
	WlDirectoryStream	directory;
	WlItemStream		item;
};

#ifdef __cplusplus
}
#endif


#endif

