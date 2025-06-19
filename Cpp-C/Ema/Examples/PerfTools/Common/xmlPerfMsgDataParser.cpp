/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "xmlPerfMsgDataParser.h"

// MessageData static objects
MessageData* MessageData::instance = NULL;
perftool::common::Mutex MessageData::mutex;

MessageData::MessageData(const char* msgDataFileName)
{
	if (xmlMsgDataInit(msgDataFileName) != RSSL_RET_SUCCESS)
		exit(-1);
}

MessageData::~MessageData()
{
	xmlMsgDataCleanup();
}

MessageData* MessageData::getInstance(const char* msgDataFileName)
{
	if (instance == NULL)
	{
		mutex.lock();
		if (instance == NULL)
		{
			instance = new MessageData(msgDataFileName);
		}
		mutex.unlock();
	}
	return instance;
}
