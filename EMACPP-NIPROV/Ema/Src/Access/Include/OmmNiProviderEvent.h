/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommNiProviderEvent_h
#define __thomsonreuters_ema_access_ommNiProviderEvent_h

/**
	@class thomsonreuters::ema::access::OmmNiProviderEvent OmmNiProviderEvent.h "Access/Include/OmmNiProviderEvent.h"
	@brief OmmNiProviderEvent encapsulates item identifiers.

	OmmNiProviderEvent is used to convey item identifiers to application. OmmNiProviderEvent is returned
	through OmmNiProviderClient callback methods.

	\remark OmmNiProviderEvent is a read only class. This class is used for item identification only.
	\remark All methods in this class are \ref SingleThreaded.

	@see OmmNiProvider
		OmmNiProviderClient
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Item;

class EMA_ACCESS_API OmmNiProviderEvent
{
public :

	///@name Accessors
	//@{
	/** Returns a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.
		Item identifier is returned from OmmNiProvider::registerClient().
		@return item identifier or handle
	*/
	UInt64 getHandle() const;

	/** Returns an identifier (a.k.a., closure) associated with an open stream by a OmmNiProvider application
		Application associates the closure with an open item stream on OmmNiProvider::registerClient( ... , ... , void* closure )
		@return closure value
	*/
	void* getClosure() const;

	//@}

private :

	Item*		_pItem;

	OmmNiProviderEvent();
	virtual ~OmmNiProviderEvent();
	OmmNiProviderEvent( const OmmNiProviderEvent& );
	OmmNiProviderEvent& operator=( const OmmNiProviderEvent& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ommNiProviderEvent_h
