#include <time.h>

#include "NewsConsumer.h"
#include "rtr/rsslDataPackage.h"
#include "rtr/rsslRmtes.h"
#include "rtr/rsslMessagePackage.h"
#include "rtr/rsslTransport.h"
#include "rtr/rsslRDMMsg.h"
#include "winsock2.h"

#define LOGIN_STREAMID				1
#define DIRECTORY_STREAMID			2
#define FIELD_DICTIONARY_STREAMID	3
#define ENUM_DICTIONARY_STREAMID	4
#define HEADLINE_STREAMID			5

#define PNAC		235
#define BCAST_TEXT	264
#define LANG_IND	752
#define STORY_TIME	1024
#define SEG_TEXT	258
#define NEXT_LR		238



using namespace News;
using namespace System;
using namespace System::Runtime::InteropServices;

NewsConsumer::NewsConsumer()
{
	active = RSSL_FALSE;
	chnl = NULL;
	dataDictionary = (RsslDataDictionary*)malloc(sizeof(RsslDataDictionary));

	rsslClearDataDictionary(dataDictionary);
	fieldDictionaryLoaded = RSSL_FALSE;
	enumDictionaryLoaded = RSSL_FALSE;

	_pnacVal = (char*)malloc(sizeof(char)*128);
	_langVal = (char*)malloc(sizeof(char)*128);
	_timeVal = (char*)malloc(sizeof(char)*128);
	_nextLinkVal = (char*)malloc(sizeof(char)*128);

	_bcastWVal = (wchar_t*)malloc(sizeof(wchar_t)*1000);
	_segWVal = (wchar_t*)malloc(sizeof(wchar_t)*1000);
}

NewsConsumer::~NewsConsumer()
{
	if(active == RSSL_TRUE)
		disconnect();

	if(HostName)
		Marshal::FreeHGlobal((IntPtr)(void*)HostName);
	if(PortNum)
		Marshal::FreeHGlobal((IntPtr)(void*)PortNum);
	if(ServiceName)
		Marshal::FreeHGlobal((IntPtr)(void*)ServiceName);
	if(ItemName)
		Marshal::FreeHGlobal((IntPtr)(void*)ItemName);

	free(dataDictionary);
	free(_pnacVal);
	free(_langVal);
	free(_timeVal);
	free(_nextLinkVal);
	free(_bcastWVal);
	free(_segWVal);
}


void NewsConsumer::disconnect()
{
	active = RSSL_FALSE;
	consThread->Join();
	rsslUninitialize();
}

void NewsConsumer::setStatusCallBackFn(statusCallBack^ func)
{
	_pStatusFunc = func;
}

void NewsConsumer::setDataCallBackFn(dataCallBack^ func)
{
	_pDataFunc = func;
}

void NewsConsumer::setConnectionStatusCallbackFn(connectionStatusCallback^ func)
{
	_pConnFunc = func;
}

void NewsConsumer::Connect(String^ host, String^ port, String^ service, String^ item)
{
	RsslError err;
	
	rsslInitialize(RSSL_LOCK_GLOBAL, &err);

	HostName = (char*)(void*)Marshal::StringToHGlobalAnsi(host);
	PortNum = (char*)(void*)Marshal::StringToHGlobalAnsi(port);
	ServiceName = (char*)(void*)Marshal::StringToHGlobalAnsi(service);
	ItemName = (char*)(void*)Marshal::StringToHGlobalAnsi(item);

	consThread = gcnew Thread(gcnew ThreadStart(this, &NewsConsumer::consumerThread ));

	active = RSSL_TRUE;

	_pConnFunc(true);

	consThread->Start();
	
	return;
}

void NewsConsumer::consumerThread()
{
	RsslConnectOptions cOpts = RSSL_INIT_CONNECT_OPTS;
	RsslError err;
	RsslRet ret;
	int selRet;
	char errorTxt[256];
	RsslBuffer errorText = {255, (char*)errorTxt};
	
	struct timeval time_interval;

	RsslBuffer* msgBuf;

	RsslInProgInfo inProg;

	fd_set readSet;
	fd_set writeSet;
	fd_set exceptSet;
	
	fd_set useRead;
	fd_set useWrt;
	fd_set useExcept;

	cOpts.guaranteedOutputBuffers = 500;
	cOpts.connectionInfo.unified.address = HostName;
	cOpts.connectionInfo.unified.serviceName = PortNum;
	cOpts.connectionType = RSSL_CONN_TYPE_SOCKET;
	cOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
	cOpts.minorVersion = RSSL_RWF_MINOR_VERSION;
	cOpts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	cOpts.blocking = RSSL_FALSE;

	time_t nextSendPingTime = 0;
	time_t currentTime;

	this->chnl = rsslConnect(&cOpts, &err);
	if(chnl == NULL)
	{
		_pStatusFunc("Connection Failed.");
		_pConnFunc(false);
		return;
	}

	active = RSSL_TRUE;

	if(rsslLoadFieldDictionary("RDMFieldDictionary", dataDictionary, &errorText) == RSSL_RET_SUCCESS)
	{
		fieldDictionaryLoaded = RSSL_TRUE;	
	}
	else
	{
		_pStatusFunc("Unable to load field dictionary, will attempt to download from provider.");
	}

	if(rsslLoadEnumTypeDictionary("enumtype.def", dataDictionary, &errorText) == RSSL_RET_SUCCESS)
	{
		enumDictionaryLoaded = RSSL_TRUE;	
	}
	else
	{
		_pStatusFunc("Unable to load enum type dictionary, will attempt to download from provider.");
	}

	if(fieldDictionaryLoaded == RSSL_TRUE && enumDictionaryLoaded == RSSL_TRUE)
	{
		if(verifyDictionary() == RSSL_TRUE)
		{
			_pStatusFunc("Dictionary verified, will send requests after source directory");
		}
		else
		{
			_pStatusFunc("Dictionary fields do not match required types, will attempt to download from provider.");
			
			fieldDictionaryLoaded = RSSL_FALSE;	
			enumDictionaryLoaded = RSSL_FALSE;
		}
	}

	readfds = &readSet;
	wrtfds = &writeSet;
	exceptfds = &exceptSet;


	FD_ZERO(readfds);
	FD_ZERO(wrtfds);
	FD_ZERO(exceptfds);


	FD_SET(chnl->socketId, readfds);
	FD_SET(chnl->socketId, wrtfds);
	FD_SET(chnl->socketId, exceptfds);
	
	while(chnl != NULL && chnl->state != RSSL_CH_STATE_ACTIVE)
	{
		useRead = *readfds;
		useWrt = *wrtfds;
		useExcept = *exceptfds;
		
		time_interval.tv_sec = 60;
		time_interval.tv_usec = 0;
		selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &time_interval);
		
	
		if(selRet > 0 && FD_ISSET(chnl->socketId, &useRead) || FD_ISSET(chnl->socketId, &useWrt) || FD_ISSET(chnl->socketId, &useExcept))
		{
			if (chnl->state == RSSL_CH_STATE_INITIALIZING)
			{
				FD_CLR(chnl->socketId,wrtfds);
				if ((ret = rsslInitChannel(chnl, &inProg, &err)) < RSSL_RET_SUCCESS)
				{
					_pStatusFunc("Channel Inactive");
					active = RSSL_FALSE;
					rsslCloseChannel(chnl, &err);
					_pConnFunc(false);
					return;
				}
				else 
				{
					switch (ret)
					{
					case RSSL_RET_CHAN_INIT_IN_PROGRESS:
						if (inProg.flags & RSSL_IP_FD_CHANGE)
						{
							_pStatusFunc("Channel In Progress - FD Change event ");

							FD_CLR(inProg.oldSocket,readfds);
							FD_CLR(inProg.oldSocket,exceptfds);
							FD_SET(chnl->socketId,readfds);
							FD_SET(chnl->socketId,exceptfds);
							FD_SET(chnl->socketId,wrtfds);
						}
						else
						{
							_pStatusFunc("Channel In Progress...\n");
						}
						break;
					case RSSL_RET_SUCCESS:
						{
							
							_pStatusFunc("Channel Is Active\n");
								
						}
						break;
					default:
						_pStatusFunc("Unhandled return value from rsslInitChannel\n");
						break;
					}
				}
			}
		}
	}
	
	
	time(&currentTime);

	nextSendPingTime = currentTime + chnl->pingTimeout/3;
	
	if(this->sendLoginRequest() != RSSL_RET_SUCCESS)
	{
	}
	
	while(active == RSSL_TRUE)
	{
		useRead = *readfds;
		useWrt = *wrtfds;
		useExcept = *exceptfds;

		time_interval.tv_sec = 1;
		time_interval.tv_usec = 0;
		
		selRet = select(FD_SETSIZE, &useRead, &useWrt, &useExcept, &time_interval);
		
		if(selRet > 0)
		{
			ret = 1;
			while(FD_ISSET(chnl->socketId, &useRead) && ret > RSSL_RET_SUCCESS && active == RSSL_TRUE)
			{
				msgBuf = rsslRead(chnl, &ret, &err);
			
				if(msgBuf == NULL)
				{
					switch (ret)
					{
						case RSSL_RET_FAILURE:
						{
							_pStatusFunc("Channel Inactive.\n");
							active = RSSL_FALSE;
							rsslCloseChannel(chnl, &err);
							_pConnFunc(false);
							return;
						}
						break;
						case RSSL_RET_READ_FD_CHANGE:
						{
							_pStatusFunc("FD Change Event\n");
							FD_CLR(chnl->oldSocketId, readfds);
							FD_CLR(chnl->oldSocketId, exceptfds);
							FD_SET(chnl->socketId, readfds);
							FD_SET(chnl->socketId, exceptfds);
						}
						break;
						default:
							if (ret < 0 && !(ret == RSSL_RET_READ_WOULD_BLOCK || ret == RSSL_RET_READ_PING))
							{
								_pStatusFunc("Read Error.\n");
						
							}
						break;
					}
				}
				else
				{
					if(processMsg(msgBuf) != RSSL_RET_SUCCESS)
					{
						active = RSSL_FALSE;
						rsslCloseChannel(chnl, &err);
						_pConnFunc(false);
						return;
					}
				}
			}
			 
			if(FD_ISSET(chnl->socketId, &useWrt))
			{
				if(ret = rsslFlush(chnl, &err) == RSSL_RET_SUCCESS)
				{
					FD_CLR(chnl->socketId, wrtfds);
				}
				else if(ret < RSSL_RET_SUCCESS)
				{
					_pStatusFunc("Flush failed.\n");
				}
			}

		}

		time(&currentTime);
		if(currentTime > nextSendPingTime)
		{
			rsslPing(chnl, &err);
		}
		
	}

	rsslCloseChannel(chnl, &err);
}

RsslRet NewsConsumer::processMsg(RsslBuffer* msgBuf)
{
	RsslMsg decMsg;
	RsslDecodeIterator decIter;
	RsslRet ret;

	char rdmBuf[1024];
	RsslBuffer rdmMemory = { sizeof(rdmBuf), rdmBuf };
	char sourceBuf[16384];
	RsslBuffer sourceMemory = { 16384, sourceBuf };
	char tempCharCache[1000];
	RsslErrorInfo errInfo;

	
	rsslClearDecodeIterator(&decIter);
	rsslSetDecodeIteratorRWFVersion(&decIter, this->chnl->majorVersion, this->chnl->minorVersion);

	rsslSetDecodeIteratorBuffer(&decIter, msgBuf);
	
	ret = rsslDecodeMsg(&decIter, &decMsg);
	
	if(ret < 0)
		_pStatusFunc("Message Decode Error\n");
		
	switch(decMsg.msgBase.domainType)
	{
		case RSSL_DMT_LOGIN:

			RsslRDMLoginMsg loginMsg;
			
			if(rsslDecodeRDMLoginMsg(&decIter, &decMsg, &loginMsg, &rdmMemory, &errInfo) != RSSL_RET_SUCCESS)
			{
				_pStatusFunc("Msg Decode error\n");
			}
			
			
			switch(loginMsg.rdmMsgBase.rdmMsgType)
			{
				case RDM_LG_MT_REFRESH:
					if(loginMsg.refresh.state.streamState == RSSL_STREAM_OPEN)
					{
						_pStatusFunc("Login accepted, sending Directory Request\n");
						sendDirectoryRequest();
					}
					else
						_pStatusFunc("login denied\n");
					break;
				default:
					_pStatusFunc("Unhandled RDMLoginMsgType");
					break;
			}
			break;
		case RSSL_DMT_SOURCE:

			RsslRDMDirectoryMsg directoryMsg;
			
			if(rsslDecodeRDMDirectoryMsg(&decIter, &decMsg, &directoryMsg, &sourceMemory, &errInfo) != RSSL_RET_SUCCESS)
			{
				return false;
			}
			_pStatusFunc("directory msg received");

			switch(directoryMsg.rdmMsgBase.rdmMsgType)
			{
				case RDM_DR_MT_REFRESH:
					RsslRDMDirectoryRefresh *pRefresh = &directoryMsg.refresh;
					RsslRDMService *pService;

					RsslUInt32 i;

					for(i = 0; i < pRefresh->serviceCount; ++i)
					{
						pService = &pRefresh->serviceList[i];
						if(strncmp(pService->info.serviceName.data, ServiceName, strlen(ServiceName)) == 0)
						{
							serviceId = (RsslUInt16)pService->serviceId;
							if(fieldDictionaryLoaded == RSSL_TRUE && enumDictionaryLoaded == RSSL_TRUE)
							{
								_pStatusFunc("Service found, sending request");
								if(sendRequest((const char*)ItemName, HEADLINE_STREAMID, RSSL_TRUE) == RSSL_RET_SUCCESS)
									_pStatusFunc("Request Sent");
								break;
							}
							else
							{
								_pStatusFunc("Requesting dictionary.");
								sendDictionaryRequest();
								break;
							}
						}
					}
					if(i == pRefresh->serviceCount)
					{
						_pStatusFunc("Service not found, disconnecting");
						return RSSL_RET_FAILURE;
						break;
					}
					break;

			}
			break;

		case RSSL_DMT_DICTIONARY:
		{
			RsslRDMDictionaryMsg dictionaryMsg;

			char errTxt[128];
			RsslBuffer errorText = {128, (char*)errTxt};

			if(rsslDecodeRDMDictionaryMsg(&decIter, &decMsg, &dictionaryMsg, &rdmMemory, &errInfo) != RSSL_RET_SUCCESS)
			{
				return false;
			}

			switch(dictionaryMsg.rdmMsgBase.rdmMsgType)
			{
				case RDM_DC_MT_REFRESH:
					if(dictionaryMsg.rdmMsgBase.streamId == FIELD_DICTIONARY_STREAMID)
					{
						if(rsslDecodeFieldDictionary(&decIter, dataDictionary, RDM_DICTIONARY_NORMAL, &errorText) != RSSL_RET_SUCCESS)
						{
							sprintf(tempCharCache, "rsslDecodeFieldDictionary error: %s\n", errorText.data);
							_pStatusFunc(gcnew String(tempCharCache));
							return RSSL_RET_FAILURE;
						}

						if(dictionaryMsg.refresh.flags & RDM_DC_RFF_IS_COMPLETE)
						{
							_pStatusFunc("Field Dictionary downloaded.");
							fieldDictionaryLoaded = RSSL_TRUE;
						}
					}
					else if(dictionaryMsg.rdmMsgBase.streamId == ENUM_DICTIONARY_STREAMID)
					{
						if(rsslDecodeEnumTypeDictionary(&decIter, dataDictionary, RDM_DICTIONARY_NORMAL, &errorText) != RSSL_RET_SUCCESS)
						{
							sprintf(tempCharCache, "rsslDecodeFieldDictionary error: %s\n", errorText.data);
							_pStatusFunc(gcnew String(tempCharCache));
							return RSSL_RET_FAILURE;
						}

						if(dictionaryMsg.refresh.flags & RDM_DC_RFF_IS_COMPLETE)
						{
							_pStatusFunc("Enum Dictionary downloaded.");
							enumDictionaryLoaded = RSSL_TRUE;
						}
					}

					if(fieldDictionaryLoaded == RSSL_TRUE && enumDictionaryLoaded == RSSL_TRUE)
					{
						if(verifyDictionary() == RSSL_TRUE)
						{
							_pStatusFunc("Dictionary verified, sending request");
							if(sendRequest((const char*)ItemName, HEADLINE_STREAMID, RSSL_TRUE) == RSSL_TRUE)
								_pStatusFunc("Request Sent");
							break;
						}
						else
						{
							_pStatusFunc("Dictionary fields do not match required types.");
							return RSSL_RET_FAILURE;
						}
					}
					break;
				default:
					_pStatusFunc("Unhandled dictionary message type received.");
					break;
			}

			break;
		}
		case RSSL_DMT_MARKET_PRICE:
		{

			_pnacVal[0] = '\0';
			_langVal[0] = '\0';
			_timeVal[0] = '\0';
			_nextLinkVal[0] = '\0';
			_bcastWVal[0] = L'\0';
			_segWVal[0] = L'\0';

			RsslBuffer cacheBuf;
			RsslRmtesCacheBuffer rmtesCache;
			RsslU16Buffer segBuf;
			RsslU16Buffer bcastBuf;

			RsslFieldList fList;
			RsslFieldEntry fEntry;

			RsslDateTime storyTime;

			rsslClearU16Buffer(&segBuf);
			rsslClearU16Buffer(&bcastBuf);

			memset(_pnacVal, 0, 128);
			memset(_langVal, 0, 128);
			memset(_timeVal, 0, 128);
			memset(_nextLinkVal, 0, 128);
			memset(_bcastWVal, 0, 1000*sizeof(wchar_t));
			memset(_segWVal, 0, 1000*sizeof(wchar_t));



			if(decMsg.msgBase.msgClass == RSSL_MC_STATUS)
			{
				sprintf(tempCharCache, "Status message received, with data state of: %s\n", rsslDataStateToString(decMsg.statusMsg.state.dataState));
				_pStatusFunc(gcnew String(tempCharCache));
				return RSSL_RET_SUCCESS;
			}


			if(rsslDecodeFieldList(&decIter, &fList, 0) != RSSL_RET_SUCCESS)
			{	
				return RSSL_RET_FAILURE;
			}
			
			while((ret = rsslDecodeFieldEntry(&decIter, &fEntry)) != RSSL_RET_END_OF_CONTAINER)
			{
				if(ret == RSSL_RET_SUCCESS)
				{
					switch(fEntry.fieldId)
					{
						case PNAC:
						{
							if((ret = rsslDecodeBuffer(&decIter, &cacheBuf)) == RSSL_RET_SUCCESS)
							{
								strncpy((char*)_pnacVal, cacheBuf.data, cacheBuf.length);
								_pnacVal[cacheBuf.length] = '\0';
							} 
							else if(ret != RSSL_RET_BLANK_DATA)
								return RSSL_RET_FAILURE;
								
							break;
						}
						case LANG_IND:
						{
							if((ret = rsslDecodeBuffer(&decIter, &cacheBuf)) == RSSL_RET_SUCCESS)
							{
								strncpy((char*)_langVal, cacheBuf.data, cacheBuf.length);
								_langVal[cacheBuf.length] = '\0';
							}
							else if(ret != RSSL_RET_BLANK_DATA)
								return RSSL_RET_FAILURE;
							break;
						}
						case NEXT_LR:
						{
							if((ret = rsslDecodeBuffer(&decIter, &cacheBuf)) == RSSL_RET_SUCCESS)
							{
								strncpy((char*)_nextLinkVal, cacheBuf.data, cacheBuf.length);
								_nextLinkVal[cacheBuf.length] = '\0';
							}
							else if(ret != RSSL_RET_BLANK_DATA)
								return RSSL_RET_FAILURE;
							break;
						}
						case STORY_TIME:
						{
							if((ret = rsslDecodeTime(&decIter, &storyTime.time)) == RSSL_RET_SUCCESS)
							{
								cacheBuf.data = _timeVal;
								cacheBuf.length = 128;

								if(rsslDateTimeToString(&cacheBuf, RSSL_DT_TIME, &storyTime) >= RSSL_RET_SUCCESS)
								{
									_timeVal[cacheBuf.length] = '\0';
								}
								else
									return RSSL_RET_FAILURE;
							}
							else if(ret != RSSL_RET_BLANK_DATA)
								return RSSL_RET_FAILURE;
							break;
						}
						case BCAST_TEXT:
						{
							if((ret = rsslDecodeBuffer(&decIter, &cacheBuf)) == RSSL_RET_SUCCESS)
							{
								rmtesCache.data = tempCharCache;
								rmtesCache.length = 0;
								rmtesCache.allocatedLength = 1000;

								rsslRMTESApplyToCache(&cacheBuf, &rmtesCache);

								bcastBuf.data = (RsslUInt16*)_bcastWVal;
								bcastBuf.length = 1000;

								if(rsslRMTESToUCS2(&rmtesCache, &bcastBuf) != RSSL_RET_SUCCESS)
								{
									_pStatusFunc("RMTES Parse error\n");
									return RSSL_RET_FAILURE;
								}
								bcastBuf.data[bcastBuf.length] = L'\0';

							}
							else if(ret != RSSL_RET_BLANK_DATA)
								return RSSL_RET_FAILURE;
							break;
						}
						case SEG_TEXT:
						{
							if((ret = rsslDecodeBuffer(&decIter, &cacheBuf)) == RSSL_RET_SUCCESS)
							{
								rmtesCache.data = tempCharCache;
								rmtesCache.length = 0;
								rmtesCache.allocatedLength = 1000;

								rsslRMTESApplyToCache(&cacheBuf, &rmtesCache);

								segBuf.data = (RsslUInt16*)_segWVal;
								segBuf.length = 1000;

								if(rsslRMTESToUCS2(&rmtesCache, &segBuf) != RSSL_RET_SUCCESS)
								{
									_pStatusFunc("RMTES Parse error\n");
									return RSSL_RET_FAILURE;
								}
								
								segBuf.data[segBuf.length] = L'\0';

								switch(decMsg.msgBase.msgClass)
								{
									case RSSL_MC_UPDATE:
										if((decMsg.updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY) && decMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
										{
											strncpy((char*)_pnacVal, decMsg.msgBase.msgKey.name.data, decMsg.msgBase.msgKey.name.length);
											_pnacVal[decMsg.msgBase.msgKey.name.length] = '\0';
										}

										break;
									case RSSL_MC_REFRESH:
										if((decMsg.refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY) && decMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
										{
											strncpy((char*)_pnacVal, decMsg.msgBase.msgKey.name.data, decMsg.msgBase.msgKey.name.length);
											_pnacVal[decMsg.msgBase.msgKey.name.length] = '\0';
										}

										break;
									}

							}
							else if(ret != RSSL_RET_BLANK_DATA)
								return RSSL_RET_FAILURE;

							break;
						}
					}
					
				}
			}
			
			if ( segBuf.length != 0)
			{
				_pDataFunc(gcnew String(_pnacVal), gcnew String("\0"), gcnew String("\0"), 0, _segWVal, gcnew String(_nextLinkVal));
			}
			else if ( bcastBuf.length != 0 ) //only send news of headline text
			{
				_pDataFunc(gcnew String(_pnacVal), gcnew String(_timeVal), gcnew String(_langVal), _bcastWVal, _segWVal, gcnew String(_nextLinkVal));
			}

			break;
		}
		
	}

		return RSSL_RET_SUCCESS;

}


RsslRet NewsConsumer::sendRequest(const char* ricName, RsslInt32 streamId, RsslBool streaming)
{
	RsslRequestMsg msg;
	RsslBuffer* msgBuf;
	RsslEncodeIterator encIter;
	RsslError err;

	rsslClearRequestMsg(&msg);
	rsslClearEncodeIterator(&encIter);

	msg.msgBase.msgClass = RSSL_MC_REQUEST;
	msg.msgBase.streamId = streamId;
	msg.msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	msg.msgBase.containerType = RSSL_DT_NO_DATA;

	if(streaming == RSSL_TRUE)
	{
		msg.flags = RSSL_RQMF_STREAMING | RSSL_RQMF_HAS_PRIORITY;
	}
	else
	{
		msg.flags = RSSL_RQMF_HAS_PRIORITY;
	}

	msg.priorityClass = 1;
	msg.priorityCount = 1;

	msg.msgBase.msgKey.serviceId = serviceId;

	msg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME_TYPE | RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
	msg.msgBase.msgKey.nameType = RDM_INSTRUMENT_NAME_TYPE_RIC;
	msg.msgBase.msgKey.name.data = (char*)ricName;
	msg.msgBase.msgKey.name.length = (RsslUInt32)strlen(ricName);

	msgBuf = rsslGetBuffer(chnl, 500, RSSL_FALSE, &err);

	if(msgBuf != NULL)
	{
		if(rsslSetEncodeIteratorBuffer(&encIter, msgBuf) < RSSL_RET_SUCCESS)
		{
			return RSSL_RET_FAILURE;
		}

		rsslSetEncodeIteratorRWFVersion(&encIter, chnl->majorVersion, chnl->minorVersion);

		if(rsslEncodeMsg(&encIter, (RsslMsg*)&msg) < RSSL_RET_SUCCESS)
		{
			return RSSL_RET_FAILURE;
		}

		msgBuf->length = rsslGetEncodedBufferLength(&encIter);

		return sendMessage(msgBuf);

	}
	else
		return RSSL_RET_FAILURE;
}

RsslRet NewsConsumer::sendLoginRequest()
{
	RsslBuffer* msgBuf;
	RsslRDMLoginMsg loginReqMsg;
	RsslError err;
	RsslErrorInfo errInfo;
	RsslUInt32 bufLen;

	RsslEncodeIterator encIter;
		
	msgBuf = rsslGetBuffer(chnl, 500, RSSL_FALSE, &err);
	
	if(msgBuf != NULL)
	{
		rsslInitDefaultRDMLoginRequest((RsslRDMLoginRequest*)&loginReqMsg, LOGIN_STREAMID);
		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorBuffer(&encIter, msgBuf);
		rsslSetEncodeIteratorRWFVersion(&encIter, chnl->majorVersion, chnl->minorVersion);
		rsslEncodeRDMLoginMsg(&encIter, &loginReqMsg, &bufLen, &errInfo);
		msgBuf->length = bufLen;
		
		return sendMessage(msgBuf);
	}
	else
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

RsslRet NewsConsumer::sendDirectoryRequest()
{
	RsslBuffer* msgBuf;
	RsslRDMDirectoryMsg directoryReqMsg;
	RsslError err;
	RsslErrorInfo errInfo;
	RsslUInt32 bufLen;

	RsslEncodeIterator encIter;
		
	msgBuf = rsslGetBuffer(chnl, 500, RSSL_FALSE, &err);
	
	if(msgBuf != NULL)
	{
		rsslInitDefaultRDMDirectoryRequest((RsslRDMDirectoryRequest*)&directoryReqMsg, DIRECTORY_STREAMID);
		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorBuffer(&encIter, msgBuf);
		rsslSetEncodeIteratorRWFVersion(&encIter, chnl->majorVersion, chnl->minorVersion);
		rsslEncodeRDMDirectoryMsg(&encIter, &directoryReqMsg, &bufLen, &errInfo);
		msgBuf->length = bufLen;
		
		return sendMessage(msgBuf);
	}
	else
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}


RsslRet NewsConsumer::sendDictionaryRequest()
{
	RsslBuffer* msgBuf;
	RsslRDMDictionaryMsg dictionaryReqMsg;
	RsslError err;
	RsslErrorInfo errInfo;
	RsslUInt32 bufLen;

	RsslEncodeIterator encIter;
		
	msgBuf = rsslGetBuffer(chnl, 500, RSSL_FALSE, &err);
	
	if(msgBuf != NULL)
	{
		rsslClearRDMDictionaryRequest((RsslRDMDictionaryRequest*)&dictionaryReqMsg);
		dictionaryReqMsg.request.dictionaryName.data = (char*)"RWFFld";
		dictionaryReqMsg.request.dictionaryName.length = 6;
		dictionaryReqMsg.rdmMsgBase.streamId = FIELD_DICTIONARY_STREAMID;


		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorBuffer(&encIter, msgBuf);
		rsslSetEncodeIteratorRWFVersion(&encIter, chnl->majorVersion, chnl->minorVersion);
		rsslEncodeRDMDictionaryMsg(&encIter, &dictionaryReqMsg, &bufLen, &errInfo);
		msgBuf->length = bufLen;
		
		if(sendMessage(msgBuf) != RSSL_RET_SUCCESS)
		{
			return RSSL_RET_FAILURE;
		}
	}
	else
		return RSSL_RET_FAILURE;

	msgBuf = rsslGetBuffer(chnl, 500, RSSL_FALSE, &err);

	if(msgBuf != NULL)
	{
		rsslClearRDMDictionaryRequest((RsslRDMDictionaryRequest*)&dictionaryReqMsg);
		dictionaryReqMsg.request.dictionaryName.data = (char*)"RWFEnum";
		dictionaryReqMsg.request.dictionaryName.length = 7;
		dictionaryReqMsg.rdmMsgBase.streamId = ENUM_DICTIONARY_STREAMID;


		rsslClearEncodeIterator(&encIter);
		rsslSetEncodeIteratorBuffer(&encIter, msgBuf);
		rsslSetEncodeIteratorRWFVersion(&encIter, chnl->majorVersion, chnl->minorVersion);
		rsslEncodeRDMDictionaryMsg(&encIter, &dictionaryReqMsg, &bufLen, &errInfo);
		msgBuf->length = bufLen;
		
		return sendMessage(msgBuf);
	}
	else
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

RsslBool NewsConsumer::verifyDictionary()
{
	if(dataDictionary->entriesArray[PNAC] != NULL)
	{
		if(dataDictionary->entriesArray[PNAC]->rwfType != RSSL_DT_ASCII_STRING)
			return RSSL_FALSE;
	}

	if(dataDictionary->entriesArray[BCAST_TEXT] != NULL)
	{
		if(dataDictionary->entriesArray[BCAST_TEXT]->rwfType != RSSL_DT_RMTES_STRING)
			return RSSL_FALSE;
	}

	if(dataDictionary->entriesArray[LANG_IND] != NULL)
	{
		if(dataDictionary->entriesArray[LANG_IND]->rwfType != RSSL_DT_RMTES_STRING)
			return RSSL_FALSE;
	}

	if(dataDictionary->entriesArray[STORY_TIME] != NULL)
	{
		if(dataDictionary->entriesArray[STORY_TIME]->rwfType != RSSL_DT_TIME)
			return RSSL_FALSE;
	}

	if(dataDictionary->entriesArray[SEG_TEXT] != NULL)
	{
		if(dataDictionary->entriesArray[SEG_TEXT]->rwfType != RSSL_DT_RMTES_STRING)
			return RSSL_FALSE;
	}

	if(dataDictionary->entriesArray[NEXT_LR] != NULL)
	{
		if(dataDictionary->entriesArray[NEXT_LR]->rwfType != RSSL_DT_ASCII_STRING)
			return RSSL_FALSE;
	}

	return RSSL_TRUE;
}

RsslRet NewsConsumer::sendMessage(RsslBuffer* msgBuf)
{
	RsslError error;
	RsslRet	retval = 0;
	RsslUInt32 outBytes = 0;
	RsslUInt32 uncompOutBytes = 0;
	RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;

	/* send the request */
	if ((retval = rsslWrite(chnl, msgBuf, RSSL_HIGH_PRIORITY, writeFlags, &outBytes, &uncompOutBytes, &error)) > RSSL_RET_FAILURE)
	{
		/* set write fd if there's still data queued */
		/* flush is done by application */
		if (retval > RSSL_RET_SUCCESS)
		{
			FD_SET(chnl->socketId, wrtfds);
		}
	}
	else
	{
		if (retval == RSSL_RET_WRITE_CALL_AGAIN)
		{
			/* call flush and write again */
			while (retval == RSSL_RET_WRITE_CALL_AGAIN)
			{
				if ((retval = rsslFlush(chnl, &error)) < RSSL_RET_SUCCESS)
				{
					_pStatusFunc("rsslFlush() failed.\n");
				}
				retval = rsslWrite(chnl, msgBuf, RSSL_HIGH_PRIORITY, writeFlags, &outBytes, &uncompOutBytes, &error);
			}
			/* set write fd if there's still data queued */
			/* flush is done by application */
			if (retval > RSSL_RET_SUCCESS)
			{
				FD_SET(chnl->socketId, wrtfds);
			}
		}
		else if (retval == RSSL_RET_WRITE_FLUSH_FAILED && chnl->state != RSSL_CH_STATE_CLOSED)
		{
			/* set write fd if flush failed */
			/* flush is done by application */
			FD_SET(chnl->socketId, wrtfds);
		}
		else	/* Connection should be closed, return failure */
		{
			/* rsslWrite failed, release buffer */
			_pStatusFunc("rsslWrite() failed.\n");
			rsslReleaseBuffer(msgBuf, &error);
			return RSSL_RET_FAILURE;
		}
	}

	return RSSL_RET_SUCCESS;
}