/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommdate_h
#define __thomsonreuters_eoa_foundation_ommdate_h

/**
	@class thomsonreuters::eoa::foundation::OmmDate OmmDate.h "Foundation/Include/OmmDate.h"
	@brief OmmDate represents Date info in Omm.

	OmmDate encapsulates year, month and day information.

	\reoark OmmDate is a read only class.
	\reoark All methods in this class are \ref SingleThreaded.

	@see Data,
		EoaString,
		EoaBuffer,
		OmmMemoryExhaustionException
*/

#include "Foundation/Include/DataType.h"
#include "Foundation/Include/EoaString.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EoaBuffer;

class EOA_FOUNDATION_API OmmDate
{
public :

	///@name Accessors
	//@{
	/** Returns Year.
		@return value of year
	*/
	UInt16 getYear() const throw();

	/** Returns Month.
		@return value of month
	*/
	UInt8 getMonth() const throw();

	/** Returns Day.
		@return value of day
	*/
	UInt8 getDay() const throw();

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EoaBuffer with the object hex information
	*/
	const EoaBuffer& getAsHex() const throw();

	/** Returns a string representation of the class instance.
		@throw OmmMemoryExhaustionException if app runs out of memory
		@return string representation of the class instance
	*/
	const EoaString& toString() const;

	/** Operator const char* overload.
		@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

private :

	friend class Tag;
	friend class LeafDecoder;
	friend class Decoder;
	friend class CacheLeaf;

	UInt16					_year;
	UInt8					_month;
	UInt8					_day;
	bool					_isLocal;

	const void*				_ptr;
	mutable EoaString		_toString;
	mutable UInt16			_eoaBuffer[24];

	const EoaString& toString( UInt64 indent ) const;
	OmmDate( UInt16 year, UInt8 month, UInt8 day );

	OmmDate();
	virtual ~OmmDate();
	OmmDate( const OmmDate& );
	OmmDate& operator=( const OmmDate& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommdate_h
