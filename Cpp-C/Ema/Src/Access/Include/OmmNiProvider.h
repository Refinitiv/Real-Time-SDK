/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmNiProvider_h
#define __thomsonreuters_ema_access_OmmNiProvider_h

/**
	@class thomsonreuters::ema::access::OmmNiProvider OmmNiProvider.h "Access/Include/OmmNiProvider.h"
	@brief OmmNiProvider class encapsulates functionality of an Omm non-interactive provider application.

	OmmNiProvider provides interfaces to publish items. It establishes and maintains
	connection to server, etc.

	OmmNiProvider provides a default behaviour / functionality. This may be tuned / modified by
	application when using OmmNiProviderConfig.

	Application interacts with server through the OmmNiProvider interface methods. The results of
	these interactions are communicated back to application through OmmNiProviderClient and
	OmmNiProviderErrorClient.

	The following code snippet shows basic usage of OmmNiProvider class in a simple non-interactive
	provider application.

	\code
	// create an implementation for OmmNiProviderClient to process received Login and Dictionary messages
	class AppClient : public OmmNiProviderClient
	{
		void onRefreshMsg( const RefreshMsg& , const OmmNiProviderEvent& );
		void onUpdateMsg( const UpdateMsg& , const OmmNiProviderEvent& );
		void onStatusMsg( const StatusMsg& , const OmmNiProviderEvent&);
	};

	AppClient appClient;

	// instantiate OmmNiProvider object and connect it to a host
	OmmNiProvider provider( OmmNiProviderConfig().host( "..." ) );

	// indicate that application is providing ...
	
	\endcode

	@see OmmNiProviderConfig,
		OmmNiProviderClient,
		OmmNiProviderErrorClient,
		OmmException
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmNiProviderClient;
class OmmNiProviderConfig;
class OmmNiProviderErrorClient;
class OmmNiProviderImpl;

class EmaString;

class GenericMsg;
class RefreshMsg;
class ReqMsg;
class StatusMsg;
class UpdateMsg;

class EMA_ACCESS_API OmmNiProvider
{
public :

	/** @enum DipatchTimeout
	*/ 
	enum DispatchTimeout
	{
		InfiniteWaitEnum = -1,		/*!< dispatch  blocks till a message arrives */
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
	/** Create an OmmNiProvider with OmmNiProviderConfig. The OmmNiProvider enables functionality
		that includes subscribing, posting and distributing generic messages.
		\remark Enables exception throwing as means of error reporting.
		\remark This affects exceptions thrown from OmmNiProvider methods
	 */
	OmmNiProvider( const OmmNiProviderConfig& config );

	/** Create an OmmNiProvider with OmmNiProviderConfig with an OmmNiProviderErrorClient that provides
		select global errors via callbacks instead of via exceptions.The OmmNiProvider enables functionality
		that includes subscribing, posting and distributing generic messages.
		\remark Enables OmmNiProviderErrorClient's callbacks as means of error reporting.
		\remark This affects OmmNiProvider methods that would throw exceptions otherwise.
	 */
	OmmNiProvider( const OmmNiProviderConfig& config, OmmNiProviderErrorClient& client );
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmNiProvider();
	//@}

	///@name Accessors
	//@{
	/** Retrieve internally generated OmmNiProvider instance name.
		@return name of this OmmNiProvider instance
	*/
	const EmaString& getNiProviderName() const;
	//@}

	///@name Operations
	//@{
	/** Opens an item stream (i.e. login stream and dictionary stream)
		@param[in] reqMsg specifies item and its unique attributes
		@param[in] client specifies OmmNiProviderClient instance receiving notifications about this item
		@param[in] closure specifies application defined item identification
		@return item identifier (a.k.a. handle)
		@throw OmmMemoryExhaustionException if system runs out of memory
		@throw OmmInvalidUsageException if application passes invalid ReqMsg
		@throw OmmInvalidHandlException if application passes invalid parent item handle
		\remark This method is \ref ObjectLevelSafe
		\remark if OmmNiProviderErrorClient is used and an error condition is encountered, then null handle is returned
	*/
	UInt64 registerClient( const ReqMsg& reqMsg, OmmNiProviderClient& client, void* closure );

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

	/** Relinquish application thread of control to receive callbacks via OmmNiProviderClient descendant.
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

	OmmNiProviderImpl*	_pImpl;

	OmmNiProvider( const OmmNiProvider& );
	OmmNiProvider& operator=( const OmmNiProvider& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProvider_h
