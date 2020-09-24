/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_OmmConsumerEvent_h
#define __rtsdk_ema_access_OmmConsumerEvent_h

/**
	@class rtsdk::ema::access::OmmConsumerEvent OmmConsumerEvent.h "Access/Include/OmmConsumerEvent.h"
	@brief OmmConsumerEvent encapsulates item identifiers.

	OmmConsumerEvent is used to convey item identifiers to application. OmmConsumerEvent is returned
	through OmmConsumerClient callback methods.

	\remark OmmConsumerEvent is a read only class. This class is used for item identification only.
	\remark All methods in this class are \ref SingleThreaded.

	@see OmmConsumer,
		OmmConsumerClient
*/

#include "Access/Include/Common.h"
#include "Access/Include/ChannelInformation.h"
#include "Access/Include/ChannelStatistics.h"

namespace rtsdk {

namespace ema {

namespace access {

class Item;

class EMA_ACCESS_API OmmConsumerEvent
{
public :

	///@name Accessors
	//@{
	/** Returns a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.
		Item identifier is returned from OmmConsumer::registerClient(). For tunnel stream onStatusMsg()
	    call backs this is the parent handle returned by the OmmConsume::registerClient() method. For tunnel
		stream sub-stream call backs this is the handle of the sub-stream itself.
		@return item identifier or handle
	*/
	UInt64 getHandle() const;

	/** Returns an identifier (a.k.a., closure) associated with an open stream by consumer application
		Application associates the closure with an open item stream on OmmConsumer::registerClient( ... , ... , void* closure, ... )
		@return closure value
	*/
	void* getClosure() const;

	/** Returns current item's parent item identifier (a.k.a. parent item handle).
		Application specifies parent item identifier on OmmConsumer::registerClient( ... , ... , ... , UInt64 parentHandle ).
		For tunnel stream sub-stream call backs this is the handle of parent tunnel
	    stream. For batch request items this is the item identifier of the top level
	    batch request.
		@return parent item identifier or parent handle
	*/
	UInt64 getParentHandle() const;

	/** Returns the Channel Information for this event
		@return the channel information for this event
	*/
	const ChannelInformation& getChannelInformation() const;

	/** Returns the Channel Statistics for this event
		@throw OmmInvalidUsageException if it cannot get the channel statistics
		@return the channel Statistics for this event
	*/
	const ChannelStatistics& getChannelStatistics() const;
	//@}

private :

	friend class ConsumerItem;
	friend class LoginItem;
	friend class DictionaryItem;

	UInt64			_handle;
	UInt64			_parentHandle;
	void*			_closure;
	void*           _channel;
	ChannelInformation _channelInfo;
	ChannelStatistics _channelStats;

	OmmConsumerEvent();
	virtual ~OmmConsumerEvent();
	OmmConsumerEvent( const OmmConsumerEvent& );
	OmmConsumerEvent& operator=( const OmmConsumerEvent& );
};

}

}

}

#endif // __rtsdk_ema_access_OmmConsumerEvent_h
