/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmInaccessibleLogFileException_h
#define __thomsonreuters_ema_access_OmmInaccessibleLogFileException_h

/**
	@class thomsonreuters::ema::access::OmmInaccessibleLogFileException OmmInaccessibleLogFileException.h "Access/Include/OmmInaccessibleLogFileException.h"
	@brief OmmInaccessibleLogFileException is thrown when EMA log file can not be created.

	OmmInaccessibleLogFileException is thrown when user running EAM application has no
	proper file writing permissions in a configured folder / directory.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException,
		OmmConsumerErrorClient,
		OmmProviderErrorClient
*/

#include "Access/Include/OmmException.h"
#include "Access/Include/EmaString.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EMA_ACCESS_API OmmInaccessibleLogFileException : public OmmException
{
public:

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmInaccessibleLogFileExceptionEnum
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

	/** Returns the inaccessible file name.
		@return directory and file name that caused this exception
	*/
	const EmaString& getFilename() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmInaccessibleLogFileException();
	//@}

protected:

	OmmInaccessibleLogFileException( const EmaString& );

	OmmInaccessibleLogFileException( const OmmInaccessibleLogFileException& );
	OmmInaccessibleLogFileException& operator=( const OmmInaccessibleLogFileException& );

	EmaString				_fileName;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmInvalidHandleException_h
