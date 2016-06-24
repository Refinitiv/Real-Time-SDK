/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommProviderEvent_h
#define __thomsonreuters_ema_access_ommProviderEvent_h

/**
	@class thomsonreuters::ema::access::OmmProviderEvent OmmProviderEvent.h "Access/Include/OmmProviderEvent.h"
	@brief OmmProviderEvent encapsulates item identifiers.

	OmmProviderEvent is used to convey item identifiers to application. OmmProviderEvent is returned
	through OmmProviderClient callback methods.

	\remark OmmProviderEvent is a read only class. This class is used for item identification only.
	\remark All methods in this class are \ref SingleThreaded.

	@see OmmProvider
		OmmProviderClient
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Item;

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
	//@}

private :

	friend class NiProviderItem;

	UInt64			_handle;
	void*			_closure;

	OmmProviderEvent();
	virtual ~OmmProviderEvent();
	OmmProviderEvent( const OmmProviderEvent& );
	OmmProviderEvent& operator=( const OmmProviderEvent& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ommProviderEvent_h
