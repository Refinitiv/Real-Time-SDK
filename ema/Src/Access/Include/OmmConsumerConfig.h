/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerConfig_h
#define __thomsonreuters_ema_access_OmmConsumerConfig_h

/**
	@class thomsonreuters::ema::access::OmmConsumerConfig OmmConsumerConfig.h "Access/Include/OmmConsumerConfig.h"
	@brief OmmConsumerConfig is used to modify configuration and behaviour of OmmConsumer.

	OmmConsumerConfig provides a default basic OmmConsumer configuration.

	The default configuration may be modified and or appended by using EmaConfig.xml file or any interface methods
	of this class.

	The Emaconfig.xml file is read in if it is present in the working directory fo the application.

	Calling any interface methods of OmmConsumerconfig class overrides or appends the existing configuration.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmConsumer
*/

#include "Access/Include/EmaString.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Data;
class ReqMsg;
class OmmConsumerConfigImpl;

class EMA_ACCESS_API OmmConsumerConfig
{
public :

	/** @enum OperationalModel
	*/
	enum OperationModel
	{
		UserDispatchEnum,		/*!< specifies callbacks happen on user thread of control */
		ApiDispatchEnum			/*!< specifies callbacks happen on API thread of control */
	};

	///@name Constructor
	//@{
	/** Create an OmmConsumerConfig that enables customization of default implicit administrative domains and local configuration. 
	*/
	OmmConsumerConfig();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmConsumerConfig();
	//@}

	///@name Operations
	//@{
	/** Clears the OmmConsumerConfig and sets all the defaults.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmConsumerConfig& clear();

	/** Specifies the username. Overrides a value specified in Login domain via the AddReqMsg(..) method.
		@param[in] username specifies name used on login request
		@return reference to this object
	*/
	OmmConsumerConfig& username( const EmaString& username );

	/** Specifies the password. Overrides a value specified in Login domain via the AddReqMsg(..) method.
		@param[in] password specifies respective login request attribute
		@return reference to this object
	*/
	OmmConsumerConfig& password( const EmaString& password );

	/** Specifies the position. Overrides a value specified in Login domain via the AddReqMsg(..) method.
		@param[in] position specifies respective login request attribute
		@return reference to this object
	*/
	OmmConsumerConfig& position( const EmaString& position );

	/** Specifies the authorization application identifier. Must be unique for each application.
	    Range 257 to 65535 is available for site-specific use. Range 1 to 256 is reserved.
		@param[in] applicationId specifies respective login request attribute
		@return reference to this object
	*/
	OmmConsumerConfig& applicationId( const EmaString& applicationId );

	/** Specifies a hostname and port.  Overrides prior value.
		\remark Implies usage of TCP IP channel or RSSL_CONN_TYPE_SOCKET.
		@param[in] host specifies server and port to which OmmConsumer will connect
		\remark if host set to "<hostname>:<port>", then hostname:port is assumed
		\remark if host set to "", then localhost:14002 is assumed
		\remark if host set to ":", then localhost:14002 is assumed
		\remark if host set to "<hostname>", then hostname:14002 is assumed
		\remark if host set to "<hostname>:", then hostname:14002 is assumed
		\remark if host set to ":<port>", then localhost:port is assumed
		@return reference to this object
	*/
	OmmConsumerConfig& host( const EmaString& host = "localhost:14002" );

	/** Specifies the operation model, overriding the default. The operation model specifies whether
	    to dispatch messages in the user or application thread of control.
		@param[in] specifies threading and dispatching model used by application
		@return reference to this object
	*/
	OmmConsumerConfig& operationModel( OperationModel operationModel = ApiDispatchEnum );

	/** Create an OmmConsumer with consumer name. The OmmConsumer enables functionality that includes
	    subscribing, posting and distributing generic messages. This name identifies configuration
		section to be used by Ommconsumer instance.
		@param[in] consumerName specifies name of OmmConsumer instance
		@return reference to this object
	*/
	OmmConsumerConfig& consumerName( const EmaString& consumerName );

	/** Specifies the local configuration, overriding and adding to the current content. 
		@param[in] config specifies Ommconsumer configuration
		@return reference to this object
	*/
	OmmConsumerConfig& config( const Data& config );

	/** Specifies an administrative request message to override the default administrative request.
	    Application may call multiple times prior to initialization. Supported domains include Login,
	    Directory, and Dictionary. 
		@param[in] reqMsg specifies administrative domain request message
		@return reference to this object
	*/
	OmmConsumerConfig& addAdminMsg( const ReqMsg& reqMsg );
	//@}

private :

	friend class OmmConsumerImpl;

	OmmConsumerConfigImpl* getConfigImpl() const;

	OmmConsumerConfigImpl* _pImpl;

	OmmConsumerConfig( const OmmConsumerConfig& );
	OmmConsumerConfig& operator=( const OmmConsumerConfig& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerConfig_h
