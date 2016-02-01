/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommmemoryexhaustionexception_h
#define __thomsonreuters_eoa_foundation_ommmemoryexhaustionexception_h

/**
	@class thomsonreuters::eoa::foundation::OmmMemoryExhaustionException OmmMemoryExhaustionException.h "Foundation/Include/OmmMemoryExhaustionException.h"
	@brief OmmMemoryExhaustionException represents out of memory exceptions.

	OmmMemoryExhaustionException are thrown when malloc() returns a null pointer or
	operator new throws std::bad_alloc exception.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException
*/

#include "Foundation/Include/OmmException.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API OmmMemoryExhaustionException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmMemoryExhaustionExceptionEnum
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
	virtual ~OmmMemoryExhaustionException();
	//@}

protected :

	OmmMemoryExhaustionException();

	OmmMemoryExhaustionException( const OmmMemoryExhaustionException& );
	OmmMemoryExhaustionException& operator=( const OmmMemoryExhaustionException& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommmemoryexhaustionexception_h
