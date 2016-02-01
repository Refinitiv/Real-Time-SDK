/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommdatetime_h
#define __thomsonreuters_eoa_foundation_ommdatetime_h

/**
	@class thomsonreuters::eoa::foundation::OmmDateTime OmmDateTime.h "Foundation/Include/OmmDateTime.h"
	@brief OmmDateTime represents DateTime info in Omm.
	
	OmmDateTime encapsulates year, month, day, hour, minute, second, millisecond,
	microsecond and nanosecond information.

	\reoark OmmDateTime is a read only class.
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

class EOA_FOUNDATION_API OmmDateTime
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

	/** Returns Hour.
		@return value of hour
	*/
	UInt8 getHour() const throw();

	/** Returns Minute.
		@return value of minute
	*/
	UInt8 getMinute() const throw();

	/** Returns Second.
		@return value of second
	*/
	UInt8 getSecond() const throw();

	/** Returns Millisecond.
		@return value of millisecond
	*/
	UInt16 getMillisecond() const throw();

	/** Returns Microsecond.
		@return value of microsecond
	*/
	UInt16 getMicrosecond() const throw();

	/** Returns Nanosecond.
		@return value of nanosecond
	*/
	UInt16 getNanosecond() const throw();

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
	friend class Decoder;
	friend class LeafImplDecoder;
	friend class NoLeafDecoder;
	friend class CacheLeaf;

	UInt16					_year;
	UInt8					_month;
	UInt8					_day;
	UInt8					_hour;
	UInt8					_minute;
	UInt8					_second;
	UInt16					_millisecond;
	UInt16					_microsecond;
	UInt16					_nanosecond;
	bool					_isLocal;

	const void*				_ptr;
	mutable EoaString		_toString;
	mutable UInt16			_eoaBuffer[24];

	const EoaString& toString( UInt64 indent ) const;

	OmmDateTime( UInt16	year, UInt8 month, UInt8 day, UInt8 hour, UInt8	minute, UInt8 second, 
				UInt16	millisecond, UInt16 microsecond, UInt16 nanosecond );

	OmmDateTime();
	virtual ~OmmDateTime();
	OmmDateTime( const OmmDateTime& );
	OmmDateTime& operator=( const OmmDateTime& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommdatetime_h
