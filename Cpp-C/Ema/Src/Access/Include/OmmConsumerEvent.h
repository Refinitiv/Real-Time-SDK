/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2018-2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmConsumerEvent_h
#define __refinitiv_ema_access_OmmConsumerEvent_h

/**
	@class refinitiv::ema::access::OmmConsumerEvent OmmConsumerEvent.h "Access/Include/OmmConsumerEvent.h"
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
#include "Access/Include/EmaVector.h"

namespace refinitiv {

namespace ema {

namespace access {

class Item;
class OmmBaseImpl;

class OmmConsumerEvent
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
	EMA_ACCESS_API UInt64 getHandle() const;

	/** Returns an identifier (a.k.a., closure) associated with an open stream by consumer application
		Application associates the closure with an open item stream on OmmConsumer::registerClient( ... , ... , void* closure, ... )
		@return closure value
	*/
	EMA_ACCESS_API void* getClosure() const;

	/** Returns current item's parent item identifier (a.k.a. parent item handle).
		Application specifies parent item identifier on OmmConsumer::registerClient( ... , ... , ... , UInt64 parentHandle ).
		For tunnel stream sub-stream call backs this is the handle of parent tunnel
	    stream. For batch request items this is the item identifier of the top level
	    batch request.
		@return parent item identifier or parent handle
	*/
	EMA_ACCESS_API UInt64 getParentHandle() const;

	/** Returns the Channel Information for this event
		@return the channel information for this event
	*/
	const EMA_ACCESS_API ChannelInformation& getChannelInformation() const;

	/** Returns an EmaVector containing the Session Information for this event.  If Request Routing is turned off, this will return an empty vector.
	@return the channel information for this event
	*/
	const EMA_ACCESS_API void getSessionInformation(EmaVector<ChannelInformation>&) const;

	/** Returns the Channel Statistics for this event
		@throw OmmInvalidUsageException if it cannot get the channel statistics
		@return the channel Statistics for this event
	*/
	const EMA_ACCESS_API ChannelStatistics& getChannelStatistics() const;
	//@}

private :

	friend class ConsumerItem;
	friend class LoginItem;
	friend class DictionaryItem;
	friend class OmmBaseImpl;

	UInt64			_handle;
	UInt64			_parentHandle;
	void*			_closure;
	void*           _channel;
	ChannelInformation _channelInfo;
	ChannelStatistics _channelStats;
	OmmBaseImpl& _ommBaseImpl;

	OmmConsumerEvent(OmmBaseImpl&);
	virtual ~OmmConsumerEvent();
	OmmConsumerEvent( const OmmConsumerEvent& );
	OmmConsumerEvent& operator=( const OmmConsumerEvent& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerEvent_h
