/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmIProviderConfig_h
#define __thomsonreuters_ema_access_OmmIProviderConfig_h

/**
	@class thomsonreuters::ema::access::OmmIProviderConfig OmmIProviderConfig.h "Access/Include/OmmIProviderConfig.h"
	@brief OmmIProviderConfig is used to specify configuration and behaviour of Interactive OmmProvider.

	OmmIProviderConfig provides a default basic Interactive OmmProvider configuration.

	The default configuration may be modified and or appended by using EmaConfig.xml file or any interface
	methods of this class.

	The EmaConfig.xml file is read in if it is present in the working directory of the application.

	Calling any interface methods of OmmIProviderConfig class overrides or appends the existing configuration.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmProvider,
		OmmProviderConfig
*/

#include "Access/Include/EmaString.h"
#include "Access/Include/OmmProviderConfig.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class Data;
class RefreshMsg;
class OmmIProviderConfigImpl;

class EMA_ACCESS_API OmmIProviderConfig : public OmmProviderConfig
{
public :

	/** @enum OperationalModel
	*/
	enum OperationModel
	{
		UserDispatchEnum,		/*!< specifies callbacks happen on user thread of control */
		ApiDispatchEnum			/*!< specifies callbacks happen on API thread of control */
	};

	/** @enum AdminControl
	*/
	enum AdminControl
	{
		UserControlEnum,		/*!< specifies user submit directory refresh message */
		ApiControlEnum			/*!< specifies API sends down directory refresh message based on the configuration */
	};

	///@name Constructor
	//@{
	/** Constructs OmmIProviderConfig
	*/
	OmmIProviderConfig();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmIProviderConfig();
	//@}

	///@name Accessors
	//@{
	/** Retrieve Provider's role
		@return role of this OmmIProviderConfig instance
	*/
	ProviderRole getProviderRole() const;
	//@}

	///@name Operations
	//@{
	/** Clears the OmmNiProviderConfig and sets all the defaults.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	OmmIProviderConfig& clear();

	/** Specifies a port.  Overrides prior value.
		\remark Implies usage of TCP IP channel or RSSL_SOCKET.
		@param[in] port specifies server port on which OmmProvider will accept client connections
		@return reference to this object
	*/
	OmmIProviderConfig& port( const EmaString& port = "14002" );

	/** Specifies the operation model, overriding the default. The operation model specifies whether
	    to dispatch messages in the user or application thread of control.
		@param[in] specifies threading and dispatching model used by application
		@return reference to this object
	*/
	OmmIProviderConfig& operationModel( OperationModel operationModel = ApiDispatchEnum );

	/** Specifies whether API or user controls sending of Directory refresh message.
		@param[in] control specifies who sends down the directory refresh message
		@return reference to this object
	*/
	OmmIProviderConfig& adminControlDirectory( AdminControl control = ApiControlEnum );

	/** Specifies whether API or user controls responding to dictionary requests.
		@param[in] control specifies who responds to dictioanry requests
		@return reference to this object
	*/
	OmmIProviderConfig& adminControlDictionary( AdminControl control = ApiControlEnum );

	/** Create an OmmProvider with providerName.
		This name identifies configuration section to be used by OmmProvider instance.
		@param[in] providerName specifies name of OmmProvider instance
		@return reference to this object
	*/
	OmmIProviderConfig& providerName( const EmaString& providerName );

	/** Specifies the local configuration, overriding and adding to the current content.
		@param[in] config specifies OmmProvider configuration
		@return reference to this object
	*/
	OmmIProviderConfig& config( const Data& config );

	/** Specifies an administrative refresh message to override the default administrative refresh.
	    Application may call multiple times prior to initialization. Supports Directory and Dictionary domains only.
		@param[in] refreshMsg specifies administrative domain refresh message
		@return reference to this object
	*/
	OmmIProviderConfig& addAdminMsg( const RefreshMsg& refreshMsg );
	//@}

private:

	friend class OmmIProviderImpl;

	OmmIProviderConfigImpl*		_pImpl;

	OmmIProviderConfigImpl* getConfigImpl() const;

	OmmIProviderConfig( const OmmIProviderConfig& );
	OmmIProviderConfig& operator=( const OmmIProviderConfig& );
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmIProviderConfig_h
