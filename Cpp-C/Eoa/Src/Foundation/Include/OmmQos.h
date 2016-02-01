/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_ommqos_h
#define __thomsonreuters_eoa_foundation_ommqos_h

/**
	@class thomsonreuters::eoa::foundation::Qos Qos.h "Foundation/Include/Qos.h"
	@brief OmmQos represents Quality Of Service information in Omm.

	\reoark OmmQos is a read only class.
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

class EOA_FOUNDATION_API OmmQos
{
public :

	/** @enum QosRate
		An enumeration representing qos rate.
	*/
	enum Rate
	{
		TickByTickEnum = 0,							/*!< Indicates tick by tick rate */

		JustInTimeConflatedEnum = 0xFFFFFF00		/*!< Indicates just in time conflated rate */
	};

	/** @enum QosTimeliness
		An enumeration representing Qos timeliness.
	*/
	enum Timeliness
	{
		RealTimeEnum = 0,					/*!< Indicates real time timeliness */

		InexactDelayedEnum = 0xFFFFFFFF		/*!< Indicates timeliness with an unknown delay value */
	};

	///@name Accessors
	//@{	
	/** Returns the QosRate value as a string format.
		@return string representation of this object Rate
	*/
	const EoaString& getRateAsString() const throw();
		
	/** Returns the QosTimeliness value as a string format.
		@return string representation of this object timeliness
	*/
	const EoaString& getTimelinessAsString() const throw();

	/** Returns Timeliness.
		@return value of OmmQos::Timeliness
	*/
	UInt32 getTimeliness() const throw();

	/** Returns Rate.
		@return value of OmmQos::Rate
	*/
	UInt32 getRate() const throw();

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
	friend class RefreshInfoImpl;
	friend class StatusInfoImpl;
	friend class CacheLeaf;

	UInt32					_timeliness;
	UInt32					_rate;
	bool					_isLocal;

	const void*				_ptr;
	mutable UInt16			_rateString[24];
	mutable UInt16			_timelinessString[24];
	mutable EoaString		_toString;
	mutable UInt16			_eoaBuffer[24];

	const EoaString& toString( UInt64 indent ) const;

	OmmQos( UInt32 timeliness, UInt32 rate );

	OmmQos();
	virtual ~OmmQos();
	OmmQos( const OmmQos& );
	OmmQos& operator=( const OmmQos& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_ommqos_h
