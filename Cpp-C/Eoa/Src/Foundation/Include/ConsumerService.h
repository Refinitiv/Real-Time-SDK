/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_consumerservice_h
#define __thomsonreuters_eoa_foundation_consumerservice_h

/**
	@class thomsonreuters::eoa::foundation::ConsumerService ConsumerService.h "Foundation/Include/ConsumerService.h"
	@brief ConsumerService represents a service provided by the platform.
	
	Application creates ConsumerService to register interest in items from a service.

	\remark All methods in this class are \ref ObjectLevelSafe

	@see ConsumerItem,
		ConsumerItemClient,
		ReqSpec,
		EoaString
*/

#include "Foundation/Include/ConsumerItem.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class ReqSpec;
class ConsumerItemClient;

class EOA_FOUNDATION_API ConsumerService
{
public:

	///@name Constructor
	//@{
	/** Constructs ConsumerService.
		@param[in] name specifies a known service name.
	*/
	ConsumerService( const EoaString& serviceName );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ConsumerService();
	//@}

	///@name Operations
	//@{
	/** Request streaming subscription.
		this is an asynchronous subscription request; e.g. this method does not wait for any response
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] itemSpec specifies item.
		@param[in] consumerItemClient specifies a client callback class
		@param[in] closure specifies application assigned value for the item. The default value is NULL.
		@return ConsumerItem object
	*/
	ConsumerItem subscribe( const ReqSpec& itemSpec, ConsumerItemClient& consumerItemClient, void* closure = 0 );

	/** Request streaming subscription.
		this is an asynchronous subscription request; e.g. this method does not wait for any response
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] itemSpec specifies item.
		@return ConsumerItem object
	*/
	ConsumerItem subscribe( const ReqSpec& itemSpec );

	/** Request non streaming subscription.
		this is a synchronous subscription request; e.g. this method blocks till refresh complete is received
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] itemSpec specifies item.
		@param[in] consumerItemClient specifies a client callback class
		@param[in] closure specifies application assigned value for the item. The default value is NULL.
		@return ConsumerItem object
	*/
	ConsumerItem snap( const ReqSpec& itemSpec, ConsumerItemClient& client, void* closure = 0 );

	/** Request non streaming subscription
		this is a synchronous subscription request; e.g. this method blocks till refresh complete is received
		@throw OmmMemoryExhaustionException if app runs out of memory
		@param[in] itemSpec specifies item.
		@return ConsumerItem object
	*/
	ConsumerItem snap( const ReqSpec& itemSpec );

	/** Request to "pull" item's data from EOA's internal cache to ConsumerItem owned by application
		@param[in] item ConsumerItem to be refreshed
		return void
	*/
	void refresh( ConsumerItem& item );
	//@}

private:

	ConsumerService( const ConsumerService& );
	ConsumerService& operator=( const ConsumerService& );

	ConsumerServiceImpl*	_pConsumerServiceImpl;
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_consumerservice_h
