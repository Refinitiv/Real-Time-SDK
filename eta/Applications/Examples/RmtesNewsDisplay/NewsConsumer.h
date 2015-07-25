/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright Thomson Reuters 2015. All rights reserved.
*/

#pragma once

#include "rtr/rsslDataPackage.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslTransport.h"
#include <winsock2.h>




namespace News
{
	using namespace System;
	using namespace System::Threading;

	public delegate void statusCallBack(System::String^);
	public delegate void dataCallBack(System::String^, System::String^, System::String^, wchar_t*, wchar_t*, System::String^);
	public delegate void connectionStatusCallback(bool connectionState);

	public ref class NewsConsumer
	{
	
	public:
	
	NewsConsumer();
	~NewsConsumer();
	
	void Connect(String^ host, String^ port, String^ service, String^ item);
	
	void consumerThread();
	
	void setStatusCallBackFn(statusCallBack^ fun);
	void setDataCallBackFn(dataCallBack^ fun);
	void setConnectionStatusCallbackFn(connectionStatusCallback^ fun);

	RsslRet sendRequest(const char* ricName, RsslInt32 streamId, RsslBool streaming);
	void disconnect();

protected:
	char* HostName;
	char* PortNum;
	char* ServiceName;
	char* ItemName;

	char* _pnacVal;
	char* _langVal;
	char* _timeVal;
	char* _nextLinkVal;

	wchar_t* _bcastWVal;
	wchar_t* _segWVal;

	fd_set* readfds;
	fd_set* wrtfds;
	fd_set* exceptfds;

	statusCallBack^		_pStatusFunc;
	dataCallBack^		_pDataFunc;
	connectionStatusCallback^ _pConnFunc;
	Thread^ consThread;
	RsslChannel* chnl;
	RsslUInt16			serviceId;
	RsslDataDictionary* dataDictionary;

	RsslBool active;
	RsslBool fieldDictionaryLoaded;
	RsslBool enumDictionaryLoaded;


	RsslRet processMsg(RsslBuffer* msgBuf);
	RsslRet sendLoginRequest();
	RsslRet sendDirectoryRequest();
	RsslRet sendDictionaryRequest();
	RsslBool verifyDictionary();

	RsslRet sendMessage(RsslBuffer* msgBuf);


	};
}