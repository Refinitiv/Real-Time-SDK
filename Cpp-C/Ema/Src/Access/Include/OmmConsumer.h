/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmConsumer_h
#define __refinitiv_ema_access_OmmConsumer_h

/**
	@class refinitiv::ema::access::OmmConsumer OmmConsumer.h "Access/Include/OmmConsumer.h"
	@brief OmmConsumer class encapsulates functionality of an Omm consuming type application.

	OmmConsumer provides interfaces to open, modify and close items. It establishes and maintains
	connection to server, maintains open item watch list, performs connection and item recovery, etc.

	OmmConsumer provides a default behaviour / functionality. This may be tuned / modified by
	application when using OmmConsumerConfig.

	Application interacts with server through the OmmConsumer interface methods. The results of
	these interactions are communicated back to application through OmmConsumerClient and
	OmmConsumerErrorclient.

	The following code snippet shows basic usage of OmmConsumer class in a simple consumer type app.
	This application opens a regular streaming item named RTR from a service RDF from the 1.1.1.1 server
	on port 14002.

	\code

	// create an implementation for OmmConsumerClient to process received item messages
	class AppClient : public OmmConsumerClient
	{
		void onRefreshMsg( const RefreshMsg& , const OmmConsumerEvent& );
		void onUpdateMsg( const UpdateMsg& , const OmmConsumerEvent& );
		void onStatusMsg( const StatusMsg& , const OmmConsumerEvent& );
	};

	AppClient appClient;

	// instantiate OmmConsumer object and connect it to a server
	OmmConsumer consumer( OmmConsumerConfig().host( "1.1.1.1:14002" ) );

	// open an item of interest
	consumer.registerClient( ReqMsg().name( "RTR" ).serviceName( "RDF" ) );

	\endcode

	The following code snippet shows usage of OmmConsumer class to obtain data from the Auxiliary
	Services Gateway (or ASG). This application opens a tunnel stream to an ASG through the 1.1.1.1 server
	on port 14002. The tunnel stream is open from the ASG provided service named ASG_SERVICE. Then 
	a streaming item named IBM_ASG is open on that stream from a service Id of 1.

	\code

	// create an implementation for OmmConsumerClient to process received item messages.
	// this object will process tunnel item and sub item messages
	class AppClient : public OmmConsumerClient
	{
		void onRefreshMsg( const RefreshMsg& , const OmmConsumerEvent& );
		void onUpdateMsg( const UpdateMsg& , const OmmConsumerEvent& );
		void onStatusMsg( const StatusMsg& , const OmmConsumerEvent& );
	};

	AppClient appClient;

	// instantiate OmmConsumer object and connect it to a server
	OmmConsumer consumer( OmmConsumerConfig().host( "1.1.1.1:14002" ) );

	// create and populate TunnelStreamRequest object
	CosAuthentication cosAuthentication;
	cosAuthentication.type( CosAuthentication::OmmLoginEnum );

	CosDataIntegrity cosDataIntegrity;
	cosDataIntegrity.type( CosDataIntegrity::ReliableEnum );

	CosFlowControl cosFlowControl;
	cosFlowControl.type( CosFlowControl::BidirectionalEnum )
		.recvWindowSize( 1200 )
		.sendWindowSize( 1200 );

	CosGuarantee cosGuarantee;
	cosGuarantee.type( CosGuarantee::NoneEnum );

	ClassOfService cos;
	cos.authentication( cosAuthentication )
		.dataIntegrity( cosDataIntegrity )
		.flowControl( cosFlowControl )
		.guarantee( cosGuarantee );

	TunnelStreamRequest tsr;
	tsr.classOfService( cos )
		.domainType( MMT_SYSTEM )
		.name( "TUNNEL_STREAM" )
		.serviceName( "ASG_SERVICE" )
		.responseTimeout( 45 );

	// open tunnel stream
	UInt64 tunnelStreamHandle = consumer.registerClient( tsr, appClient );

	// open sub item
	UInt64 subItemHandle = consumer.registerClient(
							ReqMsg().domainType( MMT_MARKET_PRICE ).name( "IBM_ASG" ).serviceId( 1 ),
							client, (void*)1, tunnelStreamHandle );

	\endcode

	@see OmmConsumerConfig,
		OmmConsumerClient,
		OmmConsumerErrorClient,
		OmmException,
		TunnelStreamRequest
*/

#include "Access/Include/Common.h"
#include "Access/Include/ChannelInformation.h"
#include "Access/Include/ChannelStatistics.h"
#include "Access/Include/IOCtlReactorCode.h"
#include "Access/Include/OAuth2CredentialRenewal.h"
#include "Access/Include/LoginMsgCredentialRenewal.h"

namespace refinitiv {

namespace ema {

namespace access {

class EmaString;
class OmmConsumerConfig;
class OmmConsumerClient;
class OmmOAuth2ConsumerClient;
class OmmConsumerErrorClient;
class ReqMsg;
class PostMsg;
class GenericMsg;
class TunnelStreamRequest;
class ChannelInformation;
class OmmConsumerImpl;

class EMA_ACCESS_API OmmConsumer
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
	/** Create an OmmConsumer with OmmConsumerConfig. The OmmConsumer enables functionality
		that includes subscribing, posting and distributing generic messages.
		\remark Enables exception throwing as means of error reporting.
		\remark This affects exceptions thrown from OmmConsumer methods
	 */
	OmmConsumer( const OmmConsumerConfig& config );

	/** Create an OmmConsumer with OmmConsumerConfig. The OmmConsumer enables functionality
	that includes subscribing, posting and distributing generic messages.
	This constructor will also register for all login events via the provided OmmConsumerClient and optional closure.
	\remark Enables exception throwing as means of error reporting.
	\remark This affects exceptions thrown from OmmConsumer methods
	*/
	OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& client, void* closure = 0 );

	/** Create an OmmConsumer with OmmConsumerConfig with an OmmConsumerErrorClient that provides
	select global errors via callbacks opposed to exception.The OmmConsumer enables functionality
	that includes subscribing, posting and distributing generic messages.
	This constructor will also register for any oAuth credentials requests, allowing the application to use a secure credentials storage and an optional closure.
	\remark Enables OmmConsumerErrorClient's callbacks as means of error reporting.
	\remark This affects exceptions thrown from OmmConsumer methods
	*/
	OmmConsumer(const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, void* closure = 0);

	/** Create an OmmConsumer with OmmConsumerConfig with an OmmConsumerErrorClient that provides
	select global errors via callbacks opposed to exception.The OmmConsumer enables functionality
	that includes subscribing, posting and distributing generic messages.
	This constructor will also register for any oAuth credentials requests, allowing the application to use a secure credentials storage and an optional closure.
	\remark Enables OmmConsumerErrorClient's callbacks as means of error reporting.
	\remark This affects OmmConsumer methods that would throw exceptions otherwise.
	*/
	OmmConsumer(const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* closure = 0);

	/** Create an OmmConsumer with OmmConsumerConfig with an OmmConsumerErrorClient that provides
		select global errors via callbacks opposed to exception.The OmmConsumer enables functionality
		that includes subscribing, posting and distributing generic messages.
		\remark Enables OmmConsumerErrorClient's callbacks as means of error reporting.
		\remark This affects OmmConsumer methods that would throw exceptions otherwise.
	 */
	OmmConsumer( const OmmConsumerConfig& config, OmmConsumerErrorClient& client );

	/** Create an OmmConsumer with OmmConsumerConfig with an OmmConsumerErrorClient that provides
	select global errors via callbacks opposed to exception.The OmmConsumer enables functionality
	that includes subscribing, posting and distributing generic messages.
	This constructor will also register for all login events via the provided OmmConsumerClient and optional closure.
	\remark Enables exception throwing as means of error reporting.
	\remark This affects exceptions thrown from OmmConsumer methods
	*/
	OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, void* closure = 0);

	/** Create an OmmConsumer with OmmConsumerConfig with an OmmConsumerErrorClient that provides
	select global errors via callbacks opposed to exception.The OmmConsumer enables functionality
	that includes subscribing, posting and distributing generic messages.
	This constructor will also register for all login events via the provided OmmConsumerClient and optional closure.
	\remark Enables OmmConsumerErrorClient's callbacks as means of error reporting.
	\remark This affects OmmConsumer methods that would throw exceptions otherwise.
	*/
	OmmConsumer( const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmConsumerErrorClient& errorClient, void* closure = 0 );

	/** Create an OmmConsumer with OmmConsumerConfig with an OmmConsumerErrorClient that provides
	select global errors via callbacks opposed to exception.The OmmConsumer enables functionality
	that includes subscribing, posting and distributing generic messages.
	This constructor will also register for all login events via the provided OmmConsumerClient and optional closure.
	This constructor will also register for any oAuth credentials requests, allowing the application to use a secure credentials storage.
	\remark Enables OmmConsumerErrorClient's callbacks as means of error reporting.
	\remark This affects OmmConsumer methods that would throw exceptions otherwise.
	*/
	OmmConsumer(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* closure = 0);
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmConsumer();
	//@}

	///@name Accessors
	//@{
	/** Retrieve internally generated consumer instance name.
		@return name of this OmmConsumer instance
	*/
	const EmaString& getConsumerName() const;
	//@}

	///@name Operations
	//@{
	/** Opens an item stream
		@param[in] reqMsg specifies item and its unique attributes
		@param[in] client specifies OmmConsumerClient instance receiving notifications about this item
		@param[in] closure specifies application defined item identification
		@param[in] parentHandle specifies handle of tunnel stream over which this substream is open (required for substreams)
		@return item identifier (a.k.a. handle)
		@throw OmmMemoryExhaustionException if system runs out of memory
		@throw OmmInvalidUsageException if application passes invalid ReqMsg
		@throw OmmInvalidHandlException if application passes invalid parent item handle
		\remark This method is \ref ObjectLevelSafe
		\remark if OmmConsumerErrorClient is used and an error condition is encountered, then null handle is returned
	*/
	UInt64 registerClient( const ReqMsg& reqMsg, OmmConsumerClient& client, void* closure = 0, UInt64 parentHandle = 0 );

	/** Opens a tunnel stream
		@param[in] tunnelStreamRequest specifies tunnel stream attributes
		@param[in] client specifies OmmConsumerClient instance receiving notifications about this item
		@param[in] closure specifies application defined item identification
		@return tunnel stream handle (a.k.a. parentHandle)
		@throw OmmMemoryExhaustionException if system runs out of memory
		@throw OmmInvalidUsageException if application passes invalid TunnelStreamRequest
		\remark This method is \ref ObjectLevelSafe
		\remark if OmmConsumerErrorClient is used and an error condition is encountered, then null handle is returned
	*/
	UInt64 registerClient( const TunnelStreamRequest& tunnelStreamRequest, OmmConsumerClient& client, void* closure = 0 );

	/** Changes the interest in an open item stream. The first formal parameter houses a ReqMsg. 
		ReqMsg attributes that may change are Priority(), InitialImage(), InterestAfterRefresh(),
		Pause() and Payload ViewData(). The second formal parameter is a handle that identifies
		the open stream to be modified.
		@param[in] reqMsg specifies modifications to the open item stream
		@param[in] handle identifies item to be modified
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		@throw OmmInvalidUsageException if passed in ReqMsg violates reissue rules
		\remark This method is \ref ObjectLevelSafe
	*/
	void reissue( const ReqMsg& reqMsg, UInt64 handle );

	/** Sends a GenericMsg.
		@param[in] genericMsg specifies GenericMsg to be sent on the open item stream
		@param[in] identifies item stream on which to send the GenericMsg
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		\remark This method is \ref ObjectLevelSafe
	*/
	void submit( const GenericMsg& genericMsg, UInt64 handle );

	/** Sends a PostMsg.  Accepts a PostMsg and optionally a handle associated to an open item stream. 	
		Specifying an item handle is known as "on stream posting".
		Specifying a login handle is known as "off stream posting".
		@param[in] postMsg specifies PostMsg to be sent on the open item stream
		@param[in] identifies item stream on which to send the PostMsg
		@return void
		@throw OmmInvalidHandleException if passed in handle does not refer to an open stream
		\remark This method is \ref ObjectLevelSafe
	*/
	void submit( const PostMsg& postMsg, UInt64 handle );

	/** Relinquish application thread of control to receive callbacks via OmmConsumerClient descendant.
		\remark Requires OperationalModel to be set to UserDispatchEnum.
		@param[in] dispatchTimeout specifies time in microseconds to wait in dispatch() for a message to dispatch (DispatchTimeout enumerations provide predefined values)
		@return DispatchReturn::TimeoutEnum if nothing was dispatched; DispatchReturn::DispatchedEnum otherwise
		@throw OmmInvalidUsageException if OperationalModel is not set to UserDispatchEnum
		\remark This method is \ref ObjectLevelSafe
	*/
	Int64 dispatch( Int64 dispatchTimeout = NoWaitEnum );

	/** Relinquishes interest in an open item stream.
		@param[in] handle identifies item to close
		@return void
		\remark This method is \ref ObjectLevelSafe
	*/
	void unregister( UInt64 handle );

	/** Provides channel information about the active channel.
		@param[in] caller provider ChannelInformation&; parameter will be cleared and then
				   current channel information will be provided
		@return void
		\remark This method is \ref ObjectLevelSafe
	*/
	void getChannelInformation(ChannelInformation&);

		/** Provides channel statistics about the active channel.
		@param[in] caller provider ChannelStatistics&; parameter will be cleared and then
				   current channel statistics will be provided
		@throw OmmInvalidUsageException if it cannot get the channel statistics
		@return void
		\remark This method is \ref ObjectLevelSafe
	*/
	void getChannelStatistics(ChannelStatistics&);

	/** Allows modifying some I/O values programmatically for a channel to override the default values.
		@param[in] code provides Code of I/O option defined in IOCtlCode::IOCtlCodeEnum to modify.
		@param[in] value provides Value to modify I/O option to
		@return void
		@throw OmmInvalidUsageException if failed to modify I/O option to
		\remark This method is \ref ObjectLevelSafe
	*/
	void modifyIOCtl(Int32 code, Int32 value);

	/** Allows modifying some I/O values programmatically for Reactor to override the default values.
		@param[in] code provides Code of I/O option defined in IOCtlReactorCode::IOCtlReactorCodeEnum to modify.
		@param[in] value provides Value to modify I/O option to
		@return void
		@throw OmmInvalidUsageException if failed to modify I/O option to
		\remark This method is \ref ObjectLevelSafe
	*/
	void modifyReactorIOCtl(Int32 code, Int32 value);


	/** Provide updated OAuth2 credentials when the callback OmmOAuth2ConsumerClient::onCredentialRenewal is called.
		This function allows the application to use a secure credential storage when using RDP functionality such as the RDP token service
		or RDP service discovery.
		@note This function can only be called within the onCredentialRenewal callback.  It will throw an OmmInvalidUsageException if not called 
		in the callback
		@param[in] OAuth2CredentialRenewal object that contains the credentials.
		@return void
		@throw OmmInvalidUsageException if the credential update fails.
	*/
	void renewOAuth2Credentials(OAuth2CredentialRenewal&);

	/** Provide updated Login credentials when the callback OmmLoginCredentialConsumerClient::onCredentialRenewal is called.
	This function allows the application to use update login userName and authenticationExtended information during a reconnection.
	@note This function can only be called within the onLoginCredentialRenewal callback.  It will throw an OmmInvalidUsageException if not called
	in the callback
	@param[in] LoginMsgCredentialRenewal object that contains the credentials.
	@return void
	@throw OmmInvalidUsageException if the credential update fails.
*/
	void renewLoginCredentials(LoginMsgCredentialRenewal&);
	//@}

private :

	OmmConsumerImpl*	_pImpl;

	OmmConsumer( const OmmConsumer& );
	OmmConsumer& operator=( const OmmConsumer& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumer_h
