/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerEvent_h
#define __thomsonreuters_ema_access_OmmConsumerEvent_h

/**
	@class thomsonreuters::ema::access::OmmConsumerEvent OmmConsumerEvent.h "Access/Include/OmmConsumerEvent.h"
	@brief OmmConsumerEvent encapsulates item identifiers.

	OmmConsumerEvent is used to convey item identifiers to application. OmmConsumerEvent is returned
	through OmmConsumerClient callback methods.

	\remark OmmConsumerEvent is a read only class. This class is used for item identification only.
	\remark All methods in this class are \ref SingleThreaded.

	@see OmmConsumer,
		OmmConsumerClient
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Item;

class EMA_ACCESS_API OmmConsumerEvent
{
public :

	///@name Accessors
	//@{
	/** Returns a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.
		Item identifier is returned from OmmConsumer::registerClient().
		@return item identifier or handle
	*/
	UInt64 getHandle() const;

	/** Returns an identifier (a.k.a., closure) associated with an open stream by consumer application
		Application associates the closure with an open item stream on OmmConsumer::registerClient( ... , ... , void* closure, ... )
		@return closure value
	*/
	void* getClosure() const;

	/** Returns current item's parent item identifier (a.k.a. parent item handle).
		Application specifies parent item identifier on OmmConsumer::registerClient( ... , ... , ... , UInt64 parentHandle ) 
		@return parent item identifier or parent handle
	*/
	UInt64 getParentHandle() const;
	//@}

private :

	friend class ItemCallbackClient;
	friend class LoginCallbackClient;
	friend class DirectoryCallbackClient;
	friend class DictionaryCallbackClient;

	Item*		_pItem;

	OmmConsumerEvent();
	virtual ~OmmConsumerEvent();
	OmmConsumerEvent( const OmmConsumerEvent& );
	OmmConsumerEvent& operator=( const OmmConsumerEvent& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerEvent_h
