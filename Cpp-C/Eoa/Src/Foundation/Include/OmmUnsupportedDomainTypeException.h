/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommunsupporteddomaintypeexception_h
#define __thomsonreuters_eoa_foundation_ommunsupporteddomaintypeexception_h

/**
	@class thomsonreuters::eoa::foundation::OmmUnsupportedDomainTypeException OmmUnsupportedDomainTypeException.h "Foundation/Include/OmmUnsupportedDomainTypeException.h"
	@brief OmmUnsupportedDomainTypeExceptionis thrown when a domain type value is greater than 255.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException
*/

#include "Foundation/Include/OmmException.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EoaString;

class EOA_FOUNDATION_API OmmUnsupportedDomainTypeException : public OmmException
{
public :

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmException::OmmUnsupportedDomainTypeExceptionEnum
	*/
	OmmException::ExceptionType getExceptionType() const override;

	/** Returns Text.
		@return EmaString with exception text information
	*/
	const EoaString& getText() const override;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EoaString& toString() const override;

	/** Returns the invalid DomainType. DomainTypes must be less than 256.
		@return domain type value that caused the exception
	*/
	UInt16 getDomainType() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmUnsupportedDomainTypeException();
	//@}

protected :

	OmmUnsupportedDomainTypeException();

	UInt16			_domainType;

	OmmUnsupportedDomainTypeException( const OmmUnsupportedDomainTypeException& );
	OmmUnsupportedDomainTypeException& operator=( const OmmUnsupportedDomainTypeException& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommunsupporteddomaintypeexception_h
