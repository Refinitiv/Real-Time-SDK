/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2025 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "EmaTestClients.h"
#include "TestUtilities.h"
#include "ActiveConfig.h"
#include "Ema.h"
#include "rtr/rsslThread.h"
#include "EmaTestClients.h"
#include <iostream>

ConsumerTestOptions::ConsumerTestOptions()
{
	getChannelInformation = false;
	getSessionChannelInfo = false;
}

ProviderTestOptions::ProviderTestOptions() :
	directoryPayload(NULL),
	groupId()
{
	sendLoginRefresh = true;
	acceptLoginRequest = true;
	sendDirectoryRefresh = true;
	sendDictionaryRefresh = true;
	sendRefreshAttrib = true;
	sendUpdateMsg = true;
	sendGenericMsg = false;
	closeItemRequest = false;
	sendItemRefresh = true;
	supportWSB = true;
}

ProviderTestOptions::~ProviderTestOptions()
{
}

EmaTestClientBase::EmaTestClientBase() :
	_messageQueue(),
	_channelInfoQueue(),
	_sessionChannelInfoQueue(),
	_handleVector()
{
	RSSL_MUTEX_INIT(&_poolLock);
}

EmaTestClientBase::~EmaTestClientBase()
{
	clear();
	RSSL_MUTEX_DESTROY(&_poolLock);
}

void EmaTestClientBase::clear()
{
	RSSL_MUTEX_LOCK(&_poolLock);
	for (UInt32 i = 0; i < _messageList.size(); ++i)
	{
		if (_messageList[i] == NULL)
			continue;
		delete _messageList[i];
		_messageList[i] = NULL;
	}
	_messageList.clear();
	_messageQueue.clear();

	for (UInt32 i = 0; i < _channelInfoList.size(); ++i)
	{
		if (_channelInfoList[i] == NULL)
			continue;
		delete _channelInfoList[i];
		_channelInfoList[i] = NULL;
	}
	_channelInfoList.clear();
	_channelInfoQueue.clear();

	for (UInt32 i = 0; i < _sessionChannelInfoList.size(); ++i)
	{
		if (_sessionChannelInfoList[i] == NULL)
			continue;
		delete _sessionChannelInfoList[i];
		_sessionChannelInfoList[i] = NULL;
	}
	_sessionChannelInfoList.clear();
	_sessionChannelInfoQueue.clear();

	RSSL_MUTEX_UNLOCK(&_poolLock);
}

UInt32 EmaTestClientBase::getMessageQueueSize()
{
	UInt32 tmp;
	RSSL_MUTEX_LOCK(&_poolLock);
	tmp = _messageQueue.size();
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

Msg* EmaTestClientBase::popMsg()
{
	Msg* tmp = NULL;
	RSSL_MUTEX_LOCK(&_poolLock);
	if (_messageQueue.size() > 0)
	{
		tmp = _messageQueue[0];
		_messageQueue.removePosition(0);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

void EmaTestClientBase::addMsg(ReqMsg& reqMsg)
{
	ReqMsg* tmpMsg = new ReqMsg(reqMsg);
	RSSL_MUTEX_LOCK(&_poolLock);
	_messageQueue.push_back(tmpMsg);
	_messageList.push_back(tmpMsg);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void EmaTestClientBase::addMsg(RefreshMsg& refreshMsg)
{
	RefreshMsg* tmpMsg = new RefreshMsg(refreshMsg);
	RSSL_MUTEX_LOCK(&_poolLock);
	_messageQueue.push_back(tmpMsg);
	_messageList.push_back(tmpMsg);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void EmaTestClientBase::addMsg(UpdateMsg& updateMsg)
{
	UpdateMsg* tmpMsg = new UpdateMsg(updateMsg);
	RSSL_MUTEX_LOCK(&_poolLock);
	_messageQueue.push_back(tmpMsg);
	_messageList.push_back(tmpMsg);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void EmaTestClientBase::addMsg(GenericMsg& genericMsg)
{
	GenericMsg* tmpMsg = new GenericMsg(genericMsg);
	RSSL_MUTEX_LOCK(&_poolLock);
	_messageQueue.push_back(tmpMsg);
	_messageList.push_back(tmpMsg);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void EmaTestClientBase::addMsg(PostMsg& postMsg)
{
	PostMsg* tmpMsg = new PostMsg(postMsg);
	RSSL_MUTEX_LOCK(&_poolLock);
	_messageQueue.push_back(tmpMsg);
	_messageList.push_back(tmpMsg);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void EmaTestClientBase::addMsg(AckMsg& ackMsg)
{
	AckMsg* tmpMsg = new AckMsg(ackMsg);
	RSSL_MUTEX_LOCK(&_poolLock);
	_messageQueue.push_back(tmpMsg);
	_messageList.push_back(tmpMsg);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

UInt32 EmaTestClientBase::getChannelInfoQueueSize()
{
	UInt32 tmp;
	RSSL_MUTEX_LOCK(&_poolLock);
	tmp = _channelInfoQueue.size();
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

ChannelInformation* EmaTestClientBase::popChannelInfo()
{
	ChannelInformation* tmp = NULL;
	RSSL_MUTEX_LOCK(&_poolLock);
	if (_channelInfoQueue.size() > 0)
	{
		tmp = _channelInfoQueue[0];
		_channelInfoQueue.removePosition(0);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

UInt32 EmaTestClientBase::getSessionChannelInfoSize()
{
	UInt32 tmp;
	RSSL_MUTEX_LOCK(&_poolLock);
	tmp = _sessionChannelInfoQueue.size();
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

EmaVector<ChannelInformation>* EmaTestClientBase::popSessionChannelInfo()
{
	EmaVector<ChannelInformation>* tmp = NULL;
	RSSL_MUTEX_LOCK(&_poolLock);
	if (_sessionChannelInfoQueue.size() > 0)
	{
		tmp = _sessionChannelInfoQueue[0];
		_sessionChannelInfoQueue.removePosition(0);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

UInt32 EmaTestClientBase::getHandleSize()
{
	return _handleVector.size();
}

void EmaTestClientBase::addHandle(UInt64 handle)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	_handleVector.push_back(handle);
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

UInt64 EmaTestClientBase::popHandle()
{
	UInt64 tmp = 0;

	RSSL_MUTEX_LOCK(&_poolLock);
	if (_handleVector.size() > 0)
	{
		tmp = _handleVector[0];
		_handleVector.removePosition(0);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);

	return tmp;
}

ConsumerTestClientBase::ConsumerTestClientBase(ConsumerTestOptions& testOptions) :
	EmaTestClientBase()
{
	options = testOptions;

	ommConsumer = NULL;
}

ConsumerTestClientBase::~ConsumerTestClientBase()
{

}

void ConsumerTestClientBase::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& consumerEvent)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	
	RefreshMsg* newRefreshMsg = new RefreshMsg(refreshMsg);
	_messageQueue.push_back(newRefreshMsg);
	_messageList.push_back(newRefreshMsg);

	if (options.getChannelInformation)
	{
		ChannelInformation* chnlInfo = new ChannelInformation(consumerEvent.getChannelInformation());

		_channelInfoQueue.push_back(chnlInfo);
		_channelInfoList.push_back(chnlInfo);
	}

	if (options.getSessionChannelInfo)
	{
		EmaVector<ChannelInformation>* chnlInformation = new EmaVector<ChannelInformation>;

		consumerEvent.getSessionInformation(*chnlInformation);

		_sessionChannelInfoQueue.push_back(chnlInformation);
		_sessionChannelInfoQueue.push_back(chnlInformation);
	}


	RSSL_MUTEX_UNLOCK(&_poolLock);

}

void ConsumerTestClientBase::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& consumerEvent)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	UpdateMsg* newUpdateMsg = new UpdateMsg(updateMsg);
	_messageQueue.push_back(newUpdateMsg);
	_messageList.push_back(newUpdateMsg);

	if (options.getChannelInformation)
	{
		ChannelInformation* chnlInfo = new ChannelInformation(consumerEvent.getChannelInformation());

		_channelInfoQueue.push_back(chnlInfo);
		_channelInfoList.push_back(chnlInfo);
	}

	if (options.getSessionChannelInfo)
	{
		EmaVector<ChannelInformation>* chnlInformation = new EmaVector<ChannelInformation>;

		consumerEvent.getSessionInformation(*chnlInformation);

		_sessionChannelInfoQueue.push_back(chnlInformation);
		_sessionChannelInfoQueue.push_back(chnlInformation);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void ConsumerTestClientBase::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& consumerEvent)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	StatusMsg* newStatusMsg = new StatusMsg(statusMsg);
	_messageQueue.push_back(newStatusMsg);
	_messageList.push_back(newStatusMsg);

	if (options.getChannelInformation)
	{
		ChannelInformation* chnlInfo = new ChannelInformation(consumerEvent.getChannelInformation());

		_channelInfoQueue.push_back(chnlInfo);
		_channelInfoList.push_back(chnlInfo);
	}

	if (options.getSessionChannelInfo)
	{
		EmaVector<ChannelInformation>* chnlInformation = new EmaVector<ChannelInformation>;

		consumerEvent.getSessionInformation(*chnlInformation);

		_sessionChannelInfoQueue.push_back(chnlInformation);
		_sessionChannelInfoQueue.push_back(chnlInformation);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void ConsumerTestClientBase::onGenericMsg(const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	GenericMsg* newGenericMsg = new GenericMsg(genericMsg);
	_messageQueue.push_back(newGenericMsg);
	_messageList.push_back(newGenericMsg);

	if (options.getChannelInformation)
	{
		ChannelInformation* chnlInfo = new ChannelInformation(consumerEvent.getChannelInformation());

		_channelInfoQueue.push_back(chnlInfo);
		_channelInfoList.push_back(chnlInfo);
	}

	if (options.getSessionChannelInfo)
	{
		EmaVector<ChannelInformation>* chnlInformation = new EmaVector<ChannelInformation>;

		consumerEvent.getSessionInformation(*chnlInformation);

		_sessionChannelInfoQueue.push_back(chnlInformation);
		_sessionChannelInfoQueue.push_back(chnlInformation);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void ConsumerTestClientBase::onAckMsg(const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	AckMsg* newAckMsg = new AckMsg(ackMsg);
	_messageQueue.push_back(newAckMsg);
	_messageList.push_back(newAckMsg);

	if (options.getChannelInformation)
	{
		ChannelInformation* chnlInfo = new ChannelInformation(consumerEvent.getChannelInformation());

		_channelInfoQueue.push_back(chnlInfo);
		_channelInfoList.push_back(chnlInfo);
	}

	if (options.getSessionChannelInfo)
	{
		EmaVector<ChannelInformation>* chnlInformation = new EmaVector<ChannelInformation>;

		consumerEvent.getSessionInformation(*chnlInformation);

		_sessionChannelInfoQueue.push_back(chnlInformation);
		_sessionChannelInfoQueue.push_back(chnlInformation);
	}
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void ConsumerTestClientBase::onAllMsg(const Msg& msg, const OmmConsumerEvent& consumerEvent)
{
	return;
}

void ConsumerTestClientBase::setConsumer(OmmConsumer* consumer)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	ommConsumer = consumer;
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void ConsumerTestClientBase::unregisterHandles()
{
	RSSL_MUTEX_LOCK(&_poolLock);
	for (UInt32 i = 0; i < _handleVector.size(); i++)
	{
		ommConsumer->unregister(_handleVector[i]);
	}

	_handleVector.clear();
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

IProviderTestClientBase::IProviderTestClientBase(ProviderTestOptions& provOptions) :
	EmaTestClientBase(),
	loginRefreshAttributes(),
	activeRequests(),
	dictionary()
{
	options = provOptions;
	wsbActiveState = 0;
	loginHandle = 0;
	clientHandle = 0;

	dictionary.loadFieldDictionary("RDMFieldDictionaryTest");
	dictionary.loadEnumTypeDictionary("enumtypeTest.def");

}

IProviderTestClientBase::~IProviderTestClientBase()
{
	EmaTestClientBase::clear();
	activeRequests.clear();
}

void IProviderTestClientBase::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& event)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	
	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void IProviderTestClientBase::onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event)
{
	RSSL_MUTEX_LOCK(&_poolLock);

	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void IProviderTestClientBase::onGenericMsg(const GenericMsg& genericMsg, const OmmProviderEvent& event)
{
	RefreshMsg refreshMsg;
	RSSL_MUTEX_LOCK(&_poolLock);
	GenericMsg* newGenericMsg = new GenericMsg(genericMsg);
	_messageQueue.push_back(newGenericMsg);
	_messageList.push_back(newGenericMsg);

	if (options.supportWSB == true && genericMsg.getName() == ENAME_CONS_CONN_STATUS && genericMsg.getDomainType() == MMT_LOGIN)
	{
		const Map& map = genericMsg.getPayload().getMap();
		map.forth();
		const MapEntry& mapEntry = map.getEntry();

		UInt64 oldWsbState = wsbActiveState;

		if (mapEntry.getKey().getAscii() == ENAME_WARMSTANDBY_INFO)
		{
			const ElementList& elementList = mapEntry.getElementList();
			elementList.forth();
			const ElementEntry& elementEntry = elementList.getEntry();

			if (elementEntry.getName() == ENAME_WARMSTANDBY_MODE)
			{
				wsbActiveState = elementEntry.getUInt();
			}

			if (oldWsbState == 1 && wsbActiveState == 0)
			{
				for (UInt32 i = 0; i < activeRequests.size(); i++)
				{
					FieldList fieldList;
					fieldList.clear().
						addReal(22, 3990, OmmReal::ExponentNeg2Enum).
						addReal(25, 3994, OmmReal::ExponentNeg2Enum).
						addReal(30, 9, OmmReal::Exponent0Enum).
						addReal(31, 19, OmmReal::Exponent0Enum).
						complete();

					refreshMsg.clear();
					refreshMsg.name(activeRequests[i].name).serviceId(activeRequests[i].serviceId).solicited(false).domainType(MMT_MARKET_PRICE).
						state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").payload(fieldList).complete();

					if (options.groupId.length() != 0)
					{
						refreshMsg.itemGroup(options.groupId);
					}

					ommProvider->submit(refreshMsg, activeRequests[i].handle);
				}
			}
		}
	}
	else if (options.supportWSB == true && genericMsg.getName() == ENAME_CONS_CONN_STATUS && genericMsg.getDomainType() == MMT_DIRECTORY)
	{
		const Map& map = genericMsg.getPayload().getMap();
		map.forth();
		const MapEntry& mapEntry = map.getEntry();

		UInt64 oldWsbState = wsbActiveState;

		const ElementList& elementList = mapEntry.getElementList();
		elementList.forth();
		const ElementEntry& elementEntry = elementList.getEntry();

		if (elementEntry.getName() == ENAME_WARMSTANDBY_MODE)
		{
			wsbActiveState = elementEntry.getUInt();
		}

		if (oldWsbState == 1 && wsbActiveState == 0)
		{
			for (UInt32 i = 0; i < activeRequests.size(); i++)
			{
				FieldList fieldList;
				fieldList.clear().
					addReal(22, 3990, OmmReal::ExponentNeg2Enum).
					addReal(25, 3994, OmmReal::ExponentNeg2Enum).
					addReal(30, 9, OmmReal::Exponent0Enum).
					addReal(31, 19, OmmReal::Exponent0Enum).
					complete();

				refreshMsg.clear();
				refreshMsg.name(activeRequests[i].name).serviceId(activeRequests[i].serviceId).solicited(true).
					state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed");

				if (options.groupId.length() != 0)
				{
					refreshMsg.itemGroup(options.groupId);
				}

				ommProvider->submit(refreshMsg, activeRequests[i].handle);
			}
		}
	}


	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void IProviderTestClientBase::onAllMsg(const Msg& msg, const OmmProviderEvent& event)
{
	// Nothing here, all other messages are covered.
}

void IProviderTestClientBase::onPostMsg(const PostMsg& postMsg, const OmmProviderEvent& event)
{
	RSSL_MUTEX_LOCK(&_poolLock);
	PostMsg* newPostMsg = new PostMsg(postMsg);
	_messageQueue.push_back(newPostMsg);
	_messageList.push_back(newPostMsg);

	if (postMsg.getSolicitAck())
	{
		AckMsg ackMsg;
		
		if (postMsg.hasSeqNum())
			ackMsg.seqNum(postMsg.getSeqNum());
		if (postMsg.hasName())
			ackMsg.name(postMsg.getName());
		if (postMsg.hasServiceId())
			ackMsg.serviceId(postMsg.getServiceId());

		ackMsg.ackId(postMsg.getPostId()).domainType(postMsg.getDomainType());

		ommProvider->submit(ackMsg, event.getHandle());
	}

	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void IProviderTestClientBase::onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	FieldList fieldList;
	RefreshMsg refreshMsg;

	RSSL_MUTEX_LOCK(&_poolLock);
	switch (reqMsg.getDomainType())
	{
	case MMT_LOGIN:
	{
		if (options.sendLoginRefresh == false)
			break;

		ommProvider = &event.getProvider();
		clientHandle = event.getClientHandle();
		loginUserName = reqMsg.getName();
		loginHandle = event.getHandle();
		loginRefreshAttributes.clear();

		loginRefreshAttributes.addUInt(ENAME_SUPPORT_POST, 1);

		if (options.supportWSB)
		{
			loginRefreshAttributes.addUInt(ENAME_SUPPORT_STANDBY, 1);
		}

		if (reqMsg.getAttrib().getDataType() == DataType::ElementListEnum)
		{
			while (reqMsg.getAttrib().getElementList().forth())
			{
				const ElementEntry& eEntry = reqMsg.getAttrib().getElementList().getEntry();

				if (eEntry.getName() == ENAME_ALLOW_SUSPECT_DATA || eEntry.getName() == ENAME_SINGLE_OPEN)
				{
					loginRefreshAttributes.addUInt(eEntry.getName(), eEntry.getUInt());
				}
				else if (eEntry.getName() == ENAME_APP_ID || eEntry.getName() == ENAME_POSITION)
				{
					loginRefreshAttributes.addAscii(eEntry.getName(), eEntry.getAscii());
				}
			}

			loginRefreshAttributes.complete();

			if (options.acceptLoginRequest)
			{
				if (options.sendRefreshAttrib)
				{
					ommProvider->submit(RefreshMsg().domainType(MMT_LOGIN).nameType(USER_NAME).name(loginUserName).solicited(true).attrib(loginRefreshAttributes).
						state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login Accepted").complete(), loginHandle);
				}
				else
				{
					ommProvider->submit(RefreshMsg().domainType(MMT_LOGIN).nameType(USER_NAME).name(loginUserName).solicited(true).
						state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login Accepted").complete(), loginHandle);
				}
			}
			else
			{
				ommProvider->submit(RefreshMsg().domainType(MMT_LOGIN).nameType(USER_NAME).name(loginUserName).solicited(true).
					state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotAuthorizedEnum, "Login Denied").complete(), loginHandle);
			}
		}

		break;
	}
	case MMT_MARKET_PRICE:
	{
		if (options.sendItemRefresh == false)
			break;

		if (options.closeItemRequest == true)
		{
			ommProvider->submit(StatusMsg().domainType(reqMsg.getDomainType()).name(reqMsg.getName()).
				state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotAuthorizedEnum, "Item Not Authorized"), event.getHandle());
			break;
		}

		if (reqMsg.hasServiceId())
			serviceId = reqMsg.getServiceId();

		fieldList.clear();
		fieldList.
			addReal(22, 3990, OmmReal::ExponentNeg2Enum).
			addReal(25, 3994, OmmReal::ExponentNeg2Enum).
			addReal(30, 9, OmmReal::Exponent0Enum).
			addReal(31, 19, OmmReal::Exponent0Enum).
			complete();

		refreshMsg.clear();

		if (options.groupId.length() != 0)
		{
			refreshMsg.itemGroup(options.groupId);
		}

		if (reqMsg.getPrivateStream())
		{
			refreshMsg.privateStream(true);
		}

		if (reqMsg.hasQos())
		{
			refreshMsg.qos(reqMsg.getQosRate(), reqMsg.getQosTimeliness());
		}

		if (wsbActiveState == 0)
		{
			refreshMsg.payload(fieldList);
		}

		refreshMsg.name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).domainType(MMT_MARKET_PRICE)
			.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").complete(true);

		RequestAttributes newRequest;
		newRequest.handle = event.getHandle();
		newRequest.name = reqMsg.getName();
		newRequest.serviceId = serviceId;

		activeRequests.push_back(newRequest);

		ommProvider->submit(refreshMsg, event.getHandle());


		break;
	}
	case MMT_DIRECTORY:
	{
		if (options.sendDirectoryRefresh == true)
		{
			if (options.directoryPayload != NULL)
			{
				ommProvider->submit(RefreshMsg().domainType(MMT_DIRECTORY).clearCache(true).filter(SERVICE_INFO_FILTER | SERVICE_STATE_FILTER).payload(*options.directoryPayload)
					.solicited(true).complete(true), event.getHandle());
			}
		}

		break;
	}
	case MMT_DICTIONARY:

		if (options.sendDictionaryRefresh == true)
		{
			if (reqMsg.getName() == "RWFFld")
			{
				Int32 currentValue = dictionary.getMinFid();

				Series dictSeries;
				dictionary.encodeFieldDictionary(dictSeries, currentValue, reqMsg.getFilter(), 5000);

				event.getProvider().submit(refreshMsg.name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).domainType(MMT_DICTIONARY).
					filter(reqMsg.getFilter()).payload(dictSeries).complete(true).solicited(true), event.getHandle());

				refreshMsg.clear();

			}
			else if (reqMsg.getName() == "RWFEnum")
			{
				Int32 currentValue = 0;

				Series dictSeries;
				dictionary.encodeEnumTypeDictionary(dictSeries, currentValue, reqMsg.getFilter(), 5000);

				event.getProvider().submit(refreshMsg.name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).domainType(MMT_DICTIONARY).
					filter(reqMsg.getFilter()).payload(dictSeries).complete(true).solicited(true), event.getHandle());

				refreshMsg.clear();
			}
		}
		break;
	case MMT_SYMBOL_LIST:
		FieldList summaryData;

		summaryData.addUInt(1, 74)
			.addRmtes(3, EmaBuffer().setFrom("TOP 25 BY VOLUME", 16)).complete();

		Map map;
		map.totalCountHint(3);
		map.summaryData(summaryData);

		FieldList mapEntryValue;
		mapEntryValue.addUInt(6435, 1).complete();
		map.addKeyBuffer(EmaBuffer().setFrom("ITEM1", 5), MapEntry::AddEnum, mapEntryValue);

		mapEntryValue.clear().addUInt(6435, 2).complete();
		map.addKeyBuffer(EmaBuffer().setFrom("ITEM2", 5), MapEntry::AddEnum, mapEntryValue);

		mapEntryValue.clear().addUInt(6435, 3).complete();
		map.addKeyBuffer(EmaBuffer().setFrom("ITEM3", 5), MapEntry::AddEnum, mapEntryValue).complete();


		RefreshMsg refreshMsg;

		refreshMsg.name(reqMsg.getName()).domainType(MMT_SYMBOL_LIST).serviceId(reqMsg.getServiceId()).solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed")
			.payload(map).complete(true);

		event.getProvider().submit(refreshMsg, event.getHandle());

		break;
	}
	ReqMsg* newReqMsg = new ReqMsg(reqMsg);
	_messageQueue.push_back(newReqMsg);
	_messageList.push_back(newReqMsg);

	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void IProviderTestClientBase::onReissue(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	RSSL_MUTEX_LOCK(&_poolLock);

	RSSL_MUTEX_UNLOCK(&_poolLock);
}

void IProviderTestClientBase::onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	RSSL_MUTEX_LOCK(&_poolLock);

	ReqMsg* newReqMsg = new ReqMsg(reqMsg);
	_messageQueue.push_back(newReqMsg);
	_messageList.push_back(newReqMsg);

	RSSL_MUTEX_UNLOCK(&_poolLock);
}

RequestAttributes::RequestAttributes() :
	handle(0),
	name(),
	serviceId()
{
}

RequestAttributes::~RequestAttributes()
{
}
