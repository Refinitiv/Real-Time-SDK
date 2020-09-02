/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommProviderEvent_h
#define __thomsonreuters_ema_access_ommProviderEvent_h

/**
	@class rtsdk::ema::access::OmmProviderEvent OmmProviderEvent.h "Access/Include/OmmProviderEvent.h"
	@brief OmmProviderEvent encapsulates item identifiers.

	OmmProviderEvent is used to convey item identifiers to application. OmmProviderEvent is returned
	through OmmProviderClient callback methods.

	\remark OmmProviderEvent is a read only class. This class is used for item identification only.
	\remark All methods in this class are \ref SingleThreaded.

	@see OmmProvider
		OmmProviderClient
*/

#include "Access/Include/Common.h"
#include "Access/Include/ChannelInformation.h"
#include "Access/Include/ChannelStatistics.h"

namespace rtsdk {

namespace ema {

namespace access {

class Item;
class OmmProvider;
class LoginHandler;
class EmaString;

class EMA_ACCESS_API OmmProviderEvent
{
public :

	///@name Accessors
	//@{
	/** Returns a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.
		Item identifier is returned from OmmProvider::registerClient().
		@return item identifier or handle
	*/
	UInt64 getHandle() const;

	/** Returns an identifier (a.k.a., closure) associated with an open stream on a OmmProvider application
		Application associates the closure with an open item stream on OmmProvider::registerClient( ... , ... , void* closure )
		@return closure value
	*/
	void* getClosure() const;

	/** Return OmmProvider instance for this event.
		@return reference to OmmProvider
	*/
	OmmProvider& getProvider() const;

	/** Returns a unique client identifier (a.k.a., client handle) associated by EMA with a connected client.
		@return client identifier or handle
	*/
	UInt64 getClientHandle() const;

	/** Returns the Channel Information for this event
		@return the channel information for this event
	*/
	const ChannelInformation& getChannelInformation() const;

	/** Returns the Channel Statistical Information for this event
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return the channel statistical information for this event
	*/
	const ChannelStatistics& getChannelStatistics() const;
private :

	friend class ProviderItem;
	friend class NiProviderLoginItem;
	friend class NiProviderSingleItem;
	friend class LoginHandler;
	friend class DictionaryHandler;
	friend class DirectoryHandler;
	friend class MarketItemHandler;
	friend class OmmServerBaseImpl;

	UInt64			_handle;
	UInt64			_clientHandle;
	void*			_closure;
	void*           _channel;
	OmmProvider*	_provider;
	ChannelInformation	_channelInfo;
	ChannelStatistics	_channelStats;

	OmmProviderEvent();
	virtual ~OmmProviderEvent();
	OmmProviderEvent( const OmmProviderEvent& );
	OmmProviderEvent& operator=( const OmmProviderEvent& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ommProviderEvent_h
