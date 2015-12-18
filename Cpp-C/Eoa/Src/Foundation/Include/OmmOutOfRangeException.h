/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommoutofrangeexception_h
#define __thomsonreuters_eoa_foundation_ommoutofrangeexception_h

/**
	@class thomsonreuters::eoa::foundation::OmmOutOfRangeException OmmOutOfRangeException.h "Foundation/Include/OmmOutOfRangeException.h"
	@brief OmmOutOfRangeException is thrown when a passed in method argument is out of range.

	\reoark All methods in this class are \ref SingleThreaded.

	@see OmmException
*/

#include "Foundation/Include/OmmException.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API OmmOutOfRangeException : public OmmException
{
public :
	
	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmOutOfRangeExceptionEnum
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
	virtual ~OmmOutOfRangeException();
	//@}

protected :

	OmmOutOfRangeException();

	OmmOutOfRangeException( const OmmOutOfRangeException& );
	OmmOutOfRangeException& operator=( const OmmOutOfRangeException& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommoutofrangeexception_h
