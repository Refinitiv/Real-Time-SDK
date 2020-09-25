/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_OmmIProviderImpl_h
#define __refinitiv_ema_access_OmmIProviderImpl_h

#include "OmmServerBaseImpl.h"
#include "OmmProviderImpl.h"
#include "OmmIProviderActiveConfig.h"
#include "DirectoryServiceStore.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmIProviderImpl : public OmmProviderImpl, public OmmServerBaseImpl, public DirectoryServiceStoreClient
{
public:
	OmmIProviderImpl(OmmProvider* ommProvider, const OmmIProviderConfig&, OmmProviderClient&, void* closure);
	OmmIProviderImpl(OmmProvider* ommProvider, const OmmIProviderConfig&, OmmProviderClient&, OmmProviderErrorClient&, void* closure);
	OmmIProviderImpl(const OmmIProviderConfig&, OmmProviderClient&); //for internal UnitTest only

	virtual ~OmmIProviderImpl();

	bool isApiDispatching() const;

	void readCustomConfig(EmaConfigServerImpl*);

	const EmaString& getInstanceName() const;

	OmmProviderConfig::ProviderRole getProviderRole() const;

	OmmProvider* getProvider() const;

	UInt64 registerClient(const ReqMsg&, OmmProviderClient&, void* closure = 0, UInt64 parentHandle = 0);

	void reissue(const ReqMsg& reqMsg, UInt64 handle);

	void submit(const GenericMsg&, UInt64);

	void submit(const RefreshMsg&, UInt64);

	void submit(const UpdateMsg&, UInt64);

	void submit(const StatusMsg&, UInt64);

	Int64 dispatch(Int64 timeOut = 0);

	void unregister(UInt64);

	void submit(const AckMsg& ackMsg, UInt64 handle);

	DirectoryServiceStore& getDirectoryServiceStore();

	void onServiceDelete(ClientSession* clientSession, RsslUInt serviceId);

	void onServiceStateChange(ClientSession* clientSession, RsslUInt serviceId, const RsslRDMServiceState&);

	void onServiceGroupChange(ClientSession* clientSession, RsslUInt serviceId, RsslRDMServiceGroupState*&, RsslUInt32 groupStateCount);

	ImplementationType getImplType();

	ItemWatchList& getItemWatchList();

	bool getServiceId(const EmaString&, UInt64&);

	bool getServiceName(UInt64, EmaString&);

	void processChannelEvent(RsslReactorChannelEvent*);

	UInt32 getRequestTimeout();

    void getConnectedClientChannelInfo(EmaVector<ChannelInformation>&);
    void getChannelInformation(ChannelInformation&);

	void getConnectedClientChannelStats(UInt64 clientHandle, ChannelStatistics& cs);

	void modifyIOCtl(Int32 code, Int32 value, UInt64 handle);

	void closeChannel(UInt64 clientHandle);

private:

	bool encodeServiceIdFromName(const EmaString& serviceName, RsslUInt16& serviceId, RsslMsgBase& rsslMsgBase );

	bool validateServiceId(RsslUInt16 serviceId, RsslMsgBase& rsslMsgBase);

	bool submit(RsslReactorSubmitMsgOptions submitMsgOptions, const EmaVector< ItemInfo* >& itemList, EmaString& text, bool applyDirectoryFilter, RsslErrorInfo& rsslErrorInfo);

	void handleItemInfo(int domainType, UInt64 handle, RsslState& state, bool refreshComplete = false);

	void handleItemGroup(ItemInfo* itemInfo, RsslBuffer& groupId, RsslState&);

	OmmIProviderActiveConfig						_ommIProviderActiveConfig;
	OmmIProviderDirectoryStore						_ommIProviderDirectoryStore;
	bool											_storeUserSubmitted;
	RsslRDMDirectoryMsg								_rsslDirectoryMsg;
	RsslBuffer										_rsslDirectoryMsgBuffer;
	ItemWatchList									_itemWatchList;

	OmmIProviderImpl();
	OmmIProviderImpl(const OmmIProviderImpl&);
	const OmmIProviderImpl& operator=(const OmmIProviderImpl&);

};

}

}

}

#endif // __refinitiv_ema_access_OmmIProviderImpl_h
