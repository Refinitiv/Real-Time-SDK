/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommNiProviderConfig_h
#define __thomsonreuters_ema_access_ommNiProviderConfig_h

/**
        @class thomsonreuters::ema::access::OmmNiProviderConfig OmmNiProviderConfig.h "Access/Include/OmmNiProviderConfig.h"
        @brief OmmNiProviderConfig is used to modify configuration and behaviour of OmmNiProvider

        OmmNiProviderConfig provides a default basic OmmNiProvider configuration.

        The default configuration may be modified and or appended by using EmaNiProviderConfig.xml file or any interface
	  methods of this class.

        The EmaNiProviderConfig.xml file is read in if it is present in the working directory of the application.

        Calling any interface methods of OmmNiProviderConfig class overrides or appends the existing configuration.

        \remark All methods in this class are \ref SingleThreaded.

        @see OmmNiProvider
*/

#include "Access/Include/EmaString.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Data;
class ReqMsg;
class OmmNiProviderConfigImpl;

class EMA_ACCESS_API OmmNiProviderConfig
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
	/** Constructs OmmNiProviderConfig
	*/
	OmmNiProviderConfig();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	~OmmNiProviderConfig();
	//@}

	///@name Operations
	//@{
	/** Clears the OmmNiProviderConfig and sets all the defaults.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmNiProviderConfig& clear();

	/** Specifies the username. Overrides a value specified in Login domain via the AddReqMsg(..) method.
		@param[in] username specifies name used on login request
		@return reference to this object
	*/
	OmmNiProviderConfig& username( const EmaString& username );

	/** Specifies the password. Overrides a value specified in Login domain via the AddReqMsg(..) method.
		@param[in] password specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& password( const EmaString& password );

	/** Specifies the position. Overrides a value specified in Login domain via the AddReqMsg(..) method.
		@param[in] position specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& position( const EmaString& position );

	/** Specifies the authorization application identifier. Must be unique for each application.
	    Range 257 to 65535 is available for site-specific use. Range 1 to 256 is reserved.
		@param[in] applicationId specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& applicationId( const EmaString& applicationId );

	/** Specifies a hostname and port.  Overrides prior value.
		\remark Implies usage of TCP IP channel or RSSL_SOCKET.
		@param[in] host specifies server and port to which OmmNiProvider will connect
		\remark if host set to "<hostname>:<port>", then hostname:port is assumed
		\remark if host set to "", then localhost:14003 is assumed
		\remark if host set to ":", then localhost:14003 is assumed
		\remark if host set to "<hostname>", then hostname:14003 is assumed
		\remark if host set to "<hostname>:", then hostname:14003 is assumed
		\remark if host set to ":<port>", then localhost:port is assumed
		@return reference to this object
	*/
	OmmNiProviderConfig& host( const EmaString& host = "localhost:14003" );

	/** Specifies the operation model, overriding the default. The operation model specifies whether
	    to dispatch messages in the user or application thread of control.
		@param[in] specifies threading and dispatching model used by application
		@return reference to this object
	*/
	OmmNiProviderConfig& operationModel( OperationModel operationModel = ApiDispatchEnum );

	/** Create an OmmNiProvider with niProviderName. The OmmNiProvider enables functionality that includes
	    subscribing, posting and distributing generic messages. This name identifies configuration
		section to be used by OmmNiProvider instance.
		@param[in] niProviderName specifies name of OmmNiProvider instance
		@return reference to this object
	*/
	OmmNiProviderConfig& niProviderName( const EmaString& niProviderName );

	/** Specifies the local configuration, overriding and adding to the current content. 
		@param[in] config specifies OmmNiProvider configuration
		@return reference to this object
	*/
	OmmNiProviderConfig& config( const Data& config );

	/** Specifies an administrative request message to override the default administrative request.
	    Application may call multiple times prior to initialization. Supports Login domain.
		@param[in] reqMsg specifies administrative domain request message
		@return reference to this object
	*/
	OmmNiProviderConfig& addAdminMsg( const ReqMsg& reqMsg );

	/** Specifies the instance identifier. Can be any ASCII string, e.g. "Instance1".
		Used to differentiate applications running on the same client host.  
		@param[in] instanceId specifies respective login request attribute
		@return reference to this object
	*/
	OmmNiProviderConfig& instanceId( const EmaString& instanceId );
	//@}

private:

	friend class OmmNiProviderImpl;

	OmmNiProviderConfigImpl* _pImpl;

	OmmNiProviderConfigImpl* getConfigImpl() const;
	OmmNiProviderConfig( const OmmNiProviderConfig & );
	OmmNiProviderConfig& operator=( const OmmNiProviderConfig & );
};

}

}

}

#endif // __thomsonreuters_ema_access_ommNiProviderConfig_h
