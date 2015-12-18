/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_consumeritemclient_h
#define __thomsonreuters_eoa_foundation_consumeritemclient_h

/**
	@class thomsonreuters::eoa::foundation::ConsumerItemClient ConsumerItemClient.h 
	"Foundation/Include/ConsumerItemClient.h"
	@brief ConsumerItemClient class provides callback interfaces notifying application about changes 
	to registered items.

	Application needs to implement an application client class inheriting from ConsumerItemClient.
	In its own class, application needs to override callback methods it desires to use for item processing.
	Default empty callback methods are implemented by ConsumerItemClient class.

	\remark Thread safety of all the methods in this class depends on the user's implementation.
*/

#include "Foundation/Include/Common.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class ConsumerItem;
class RefreshInfo;
class StatusInfo;
class UpdateInfo;

class EOA_FOUNDATION_API ConsumerItemClient
{
public:

	///@name Callbacks
	//@{
	/** Invoked upon receiving item sync. 
		@param[out] current - accrued item information
		@param[out] change - received item refresh
		@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
		@return void
	*/
	virtual void onConsumerItemSync( const ConsumerItem& current, const RefreshInfo& change, void* closure ) {}

	/** Invoked upon receiving item sync. 
		@param[out] current - accrued item information
		@param[out] change - received item update
		@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
		@return void
	*/
	virtual void onConsumerItemUpdate( const ConsumerItem& current, const UpdateInfo& change, void* closure ) {}

	/** Invoked upon receiving item status. 
		@param[out] current - accrued item information
		@param[out] change - received item status
		@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
		@return void
	*/
	virtual void onConsumerItemStatus( const ConsumerItem& current, const StatusInfo& change, void* closure ) {}

	/** Invoked upon receiving partial sync. 
		@param[out] current - accrued item information
		@param[out] change - received item refresh
		@param[out] closure - item closure specified on subscribe( ... , ConsumerItemClient& , void* closure ) or snap( ... , ConsumerItemClient& , void* closure )
		@return void
	*/
	virtual void onConsumerItemPartial( const ConsumerItem& current, const RefreshInfo& change, void* closure ) {}
	//@}

protected:

	ConsumerItemClient();

	virtual ~ConsumerItemClient();
	
private:

	ConsumerItemClient( const ConsumerItemClient& );
	ConsumerItemClient& operator=( const ConsumerItemClient& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_consumeritemclient_h
