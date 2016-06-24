/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ommProviderConfig_h
#define __thomsonreuters_ema_access_ommProviderConfig_h

/**
	@class thomsonreuters::ema::access::OmmProviderConfig OmmProviderConfig.h "Access/Include/OmmProviderConfig.h"
	@brief OmmProviderConfig is a base class for the OmmNiProviderConfig

	@see OmmProvider,
		OmmNiProviderConfig
*/

#include "Access/Include/Common.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmProviderConfig
{
public :

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

#endif // __thomsonreuters_ema_access_ommProviderConfig_h
