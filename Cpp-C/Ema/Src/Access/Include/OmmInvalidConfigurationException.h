/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmInvalidConfigurationException_h
#define __refinitiv_ema_access_OmmInvalidConfigurationException_h

/**
	@class rtsdk::ema::access::OmmInvalidConfigurationException OmmInvalidConfigurationException.h "Access/Include/OmmInvalidConfigurationException.h"
	@brief OmmInvalidConfigurationException represents and indicates fatal configuration errors.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"
#include "Access/Include/EmaString.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmInvalidConfigurationException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmInvalidConfigurationExceptionEnum
	*/
	OmmException::ExceptionType getExceptionType() const;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	const EmaString& getText() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmInvalidConfigurationException();
	//@}

protected:

	OmmInvalidConfigurationException();

	OmmInvalidConfigurationException( const OmmInvalidConfigurationException& );
	OmmInvalidConfigurationException& operator=( const OmmInvalidConfigurationException& );
};

}

}

}

#endif
