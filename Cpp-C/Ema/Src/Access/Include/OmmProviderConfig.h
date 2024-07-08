/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ommProviderConfig_h
#define __refinitiv_ema_access_ommProviderConfig_h

/**
	@class refinitiv::ema::access::OmmProviderConfig OmmProviderConfig.h "Access/Include/OmmProviderConfig.h"
	@brief OmmProviderConfig is a base class for the OmmNiProviderConfig

	@see OmmProvider,
		OmmNiProviderConfig
*/

#include "Access/Include/Common.h"

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmProviderConfig
{
public :

	/** @enum ProviderRole
	*/
	enum ProviderRole
	{
		NonInteractiveEnum,			/*!< indicates a non interactive provider config */
		InteractiveEnum				/*!< indicates interactive provider config */
	};

	///@name Accessors
	//@{
	/** Retrieve Provider's role
		@return role of this OmmProviderConfig instance
	*/
	virtual ProviderRole getProviderRole() const = 0;
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~OmmProviderConfig();
	//@}

protected:

	///@name Constructor
	//@{	
	/** Constructs OmmProviderConfig
	*/
	OmmProviderConfig();
	//@}

private:

	OmmProviderConfig( const OmmProviderConfig & );
	OmmProviderConfig& operator=( const OmmProviderConfig & );
};

}

}

}

#endif // __refinitiv_ema_access_ommProviderConfig_h
