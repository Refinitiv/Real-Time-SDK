/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmProvider_h
#define __thomsonreuters_ema_access_OmmProvider_h

/**
	@class thomsonreuters::ema::access::OmmProvider OmmProvider.h "Access/Include/OmmProvider.h"
	@brief OmmProvider class encapsulates functionality of a NonInteractive OmmProvider application.

	OmmProvider provides interfaces to publish items. It establishes and maintains
	connection to ADH.

	OmmProvider provides a default behaviour / functionality. This may be tuned / modified by
	application when using OmmNiProviderConfig.

	Application interacts with ADH through the OmmProvider interface methods. The results of
	these interactions are communicated back to application through OmmProviderClient and
	OmmProviderErrorClient.

	The following code snippet shows basic usage of OmmProvider class in a simple non-interactive
	provider application.

	\code
	// create an implementation for OmmProviderClient to process received Login and Dictionary messages
	class AppClient : public OmmProviderClient
	{
		void onRefreshMsg( const RefreshMsg& , const OmmProviderEvent& );
		void onStatusMsg( const StatusMsg& , const OmmProviderEvent& );
	};

	AppClient appClient;

	// instantiate OmmProvider object and connect it to a host
	OmmProvider provider( OmmNiProviderConfig().host( "..." ).user( "..." ) );

	UInt64 itemHandle = 5;

	provider.submit( RefreshMsg().serviceName( "NI_PUB" ).name( "IBM.N" )
		.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed" )
		.payload( FieldList()
			.addReal( 22, 3990, OmmReal::ExponentNeg2Enum )
			.addReal( 25, 3994, OmmReal::ExponentNeg2Enum )
			.addReal( 30, 9, OmmReal::Exponent0Enum )
			.addReal( 31, 19, OmmReal::Exponent0Enum )
			.complete() )
		.complete(), itemHandle );	
	
	\endcode

	@see OmmProviderConfig,
		OmmNiProviderConfig,
		OmmProviderClient,
		OmmProviderErrorClient,
		OmmException
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmProviderClient;
class OmmProviderConfig;
class OmmProviderErrorClient;
class OmmProviderImpl;
class EmaString;
class GenericMsg;
class RefreshMsg;
class ReqMsg;
class StatusMsg;
class UpdateMsg;

class EMA_ACCESS_API OmmProvider
{
public :

	/** @enum DipatchTimeout
	*/ 
	enum DispatchTimeout
	{
		InfiniteWaitEnum = -1,		/*!< dispatch blocks till a message arrives */
		NoWaitEnum = 0				/*!< dispatch exits immediately even if there is no message */
	};

	/** @enum DispatchReturn
	*/
	enum DispatchReturn
	{
		TimeoutEnum = -1,		/*!< no message was dispatch on this dispatch call */
		DispatchedEnum = 0		/*!< a message was dispatched on this dispatch call */
	};

	///@name Constructor
	//@{
	/** Create an OmmProvider with OmmNiProviderConfig. The OmmProvider enables functionality
		that includes subscribing, posting and distributing generic messages.
		\remark Enables exception throwing as means of error reporting.
		\remark This affects exceptions thrown from OmmProvider methods
	 */
	OmmProvider( const OmmProviderConfig& config );

	/** Create an OmmProvider with OmmNiProviderConfig with an OmmProviderErrorClient that provides
		select global errors via callbacks instead of via exceptions.The OmmProvider enables functionality
		that includes subscribing, posting and distributing generic messages.
		\remark Enables OmmProviderErrorClient's callbacks as means of error reporting.
		\remark This affects OmmProvider methods that would throw exceptions otherwise.
	 */
	OmmProvider( const OmmProviderConfig& config, OmmProviderErrorClient& client );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmProvider();
	//@}

	///@name Accessors
	//@{
	/** Retrieve internally generated OmmProvider instance name.
		@return name of this OmmProvider instance
	*/
	const EmaString& getProviderName() const;
	//@}

	///@name Operations
	//@{
	/** Opens an item stream (i.e. login stream and dictionary stream)
		@param[in] reqMsg specifies item and its unique attributes
		@param[in] client specifies OmmProviderClient instance receiving notifications about this item
		@param[in] closure specifies application defined item identification
		@return item identifier (a.k.a. handle)
		@throw OmmMemoryExhaustionException if system runs out of memory
		@throw OmmInvalidUsageException if application passes invalid ReqMsg
		@throw OmmInvalidHandlException if application passes invalid parent item handle
		\remark This method is \ref ObjectLevelSafe
		\remark if OmmProviderErrorClient is used and an error condition is encountered, then null handle is returned
	*/
	UInt64 registerClient( const ReqMsg& reqMsg, OmmProviderClient& client, void* closure = 0 );

	/** Sends a GenericMsg.
		@param[in] genericMsg specifies GenericMsg to be sent on the open item stream
		@param[in] identifies handle associated with an item stream on which to send the GenericMsg
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		\remark This method is \ref ObjectLevelSafe
	*/
	void submit( const GenericMsg& genericMsg, UInt64 handle );

	/** Sends a RefreshMsg.
		@param[in] refreshMsg specifies RefreshMsg to be sent
		@param[in] identifies handle associated with an item stream on which to send the RefreshMsg
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		\remark This method is \ref ObjectLevelSafe
	*/
	void submit( const RefreshMsg& refreshMsg, UInt64 handle );

	/** Sends a UpdateMsg.
		@param[in] updateMsg specifies UpdateMsg to be sent
		@param[in] identifies handle associated with an item stream on which to send the UpdateMsg
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		\remark This method is \ref ObjectLevelSafe
	*/
	void submit( const UpdateMsg& updateMsg, UInt64 handle );
	
	/** Sends a StatusMsg.
		@param[in] statusMsg specifies StatusMsg to be sent
		@param[in] identifies handle associated with an item stream on which to send the StatusMsg
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		\remark This method is \ref ObjectLevelSafe
	*/	
	void submit( const StatusMsg& statusMsg, UInt64 handle );

	/** Relinquish application thread of control to receive callbacks via OmmProviderClient descendant.
		\remark Requires OperationalModel to be set to UserDispatchEnum.
		@param[in] time-out specifies time in microseconds to wait in dispatch() for a message to dispatch
		@return TimeoutEnum if nothing was dispatched; DispatchedEnum otherwise
		@throw OmmInvalidUsageException if OperationalModel is not set to UserDispatchEnum
		\remark This method is \ref ObjectLevelSafe
	*/
	Int64 dispatch( Int64 timeOut = NoWaitEnum );

	/** Relinquishes interest in an open item stream.
		@param[in] handle identifies item to close
		@return void
		\remark This method is \ref ObjectLevelSafe
	*/
	void unregister( UInt64 handle );
	//@}

private :

	OmmProviderImpl*	_pImpl;

	OmmProvider( const OmmProvider& );
	OmmProvider& operator=( const OmmProvider& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmProvider_h
