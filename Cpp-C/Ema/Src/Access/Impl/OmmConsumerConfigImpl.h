/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerConfigImpl_h
#define __thomsonreuters_ema_access_OmmConsumerConfigImpl_h

#ifdef WIN32
#include "direct.h"
#endif

#include "OmmConsumerConfig.h"
#include "EmaConfigImpl.h"
#include "ExceptionTranslator.h"
#include "ProgrammaticConfigure.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmConsumerConfigImpl : public EmaConfigImpl
{
public:

	OmmConsumerConfigImpl(const EmaString &);

	virtual ~OmmConsumerConfigImpl();

	void consumerName( const EmaString& );

	EmaString getConfiguredName();

	void operationModel( OmmConsumerConfig::OperationModel );

	OmmConsumerConfig::OperationModel operationModel() const;

	bool getDictionaryName( const EmaString& , EmaString& ) const;

	bool getDirectoryName( const EmaString& , EmaString& ) const;

private:

	OmmConsumerConfig::OperationModel		_operationModel;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerConfigImpl_h
