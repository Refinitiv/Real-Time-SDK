/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_omminvalidusageexception_h
#define __thomsonreuters_eoa_foundation_omminvalidusageexception_h

/**
	@class thomsonreuters::eoa::foundation::OmmInvalidUsageException OmmInvalidUsageException.h "Foundation/Include/OmmInvalidUsageException.h"
	@brief OmmInvalidUsageException is thrown when application violates usage of EOA interfaces.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException
*/

#include "Foundation/Include/OmmException.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API OmmInvalidUsageException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmInvalidUsageExceptionEnum
	*/
	OmmException::ExceptionType getExceptionType() const override;

	/** Returns Text.
		@return EoaString with exception text information
	*/
	const EoaString& getText() const override;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EoaString& toString() const override;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmInvalidUsageException();
	//@}

protected :

	OmmInvalidUsageException();

	OmmInvalidUsageException( const OmmInvalidUsageException& );
	OmmInvalidUsageException& operator=( const OmmInvalidUsageException& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_omminvalidusageexception_h
