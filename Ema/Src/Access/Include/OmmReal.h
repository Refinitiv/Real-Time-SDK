/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmReal_h
#define __thomsonreuters_ema_access_OmmReal_h

/**
	@class thomsonreuters::ema::access::OmmReal OmmReal.h "Access/Include/OmmReal.h"
	@brief OmmReal represents Real number in Omm.

	OmmReal encapsulates magnitude type and mantissa information.

	The following code snippet shows setting of Real in FieldList;

	\code

	FieldList fList;
	flist.addReal( 321, 245, OmmReal::ExponentNeg8Enum ).
		addRealFromDouble( 345, 245.234 ).
		complete();

	\endcode

	The following code snippet shows extraction of OmmReal from FieldList.

	\code

	void decodeFieldList( const FieldList& fList )
	{
		while ( !fList.forth() )
		{
			const FieldEntry& fEntry = fList.getEntry();

			if ( fEntry.getCode() != Data::BlankEnum )
				switch ( fEntry.getLoadType() )
				{
					case DataType::OmmReal :
						const OmmReal& ommReal = fEntry.getReal();
						Int64 mantissa = ommReal.getMantissa();
						break;
				}
		}
	}

	\endcode

	\remark OmmReal is a read only class.
	\remark This class is used for extraction of Real info only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmRealDecoder;

class EMA_ACCESS_API OmmReal : public Data
{
public :

	/** @enum MagnitudeType
		An enumeration representing item stream data state.
	*/
	enum MagnitudeType
	{
		ExponentNeg14Enum = 0,		/*!< Power of -14. */

		ExponentNeg13Enum = 1,		/*!< Power of -13. */

		ExponentNeg12Enum = 2,		/*!< Power of -12. */

		ExponentNeg11Enum = 3,		/*!< Power of -11. */

		ExponentNeg10Enum = 4,		/*!< Power of -10. */

		ExponentNeg9Enum = 5,		/*!< Power of -9. */

		ExponentNeg8Enum = 6,		/*!< Power of -8. */

		ExponentNeg7Enum = 7,		/*!< Power of -7. */

		ExponentNeg6Enum = 8,		/*!< Power of -6. */

		ExponentNeg5Enum = 9,		/*!< Power of -5. */

		ExponentNeg4Enum = 10,		/*!< Power of -4. */

		ExponentNeg3Enum = 11,		/*!< Power of -3. */

		ExponentNeg2Enum = 12,		/*!< Power of -2. */

		ExponentNeg1Enum = 13,		/*!< Power of -1. */

		Exponent0Enum = 14,			/*!< Power of 0. */

		ExponentPos1Enum = 15,		/*!< Power of 1. */

		ExponentPos2Enum = 16,		/*!< Power of 2. */

		ExponentPos3Enum = 17,		/*!< Power of 3. */

		ExponentPos4Enum = 18,		/*!< Power of 4. */

		ExponentPos5Enum = 19,		/*!< Power of 5. */

		ExponentPos6Enum = 20,		/*!< Power of 6. */

		ExponentPos7Enum = 21,		/*!< Power of 7. */

		Divisor1Enum = 22,			/*!< Divisor of 1. */

		Divisor2Enum = 23,			/*!< Divisor of 2 */

		Divisor4Enum = 24,			/*!< Divisor of 4. */

		Divisor8Enum = 25,			/*!< Divisor of 8. */

		Divisor16Enum = 26,			/*!< Divisor of 16. */

		Divisor32Enum = 27,			/*!< Divisor of 32. */

		Divisor64Enum = 28,			/*!< Divisor of 64. */

		Divisor128Enum = 29,		/*!< Divisor of 128. */

		Divisor256Enum = 30,		/*!< Divisor of 256. */

		InfinityEnum = 33,			/*!< Represents infinity. */

		NegInfinityEnum = 34,		/*!< Represents negative infinity. */

		NotANumberEnum = 35			/*!< Represents not a number (NaN). */
	};

	///@name Accessors
	//@{	
	/** Returns the MagnitudeType value as a string format.
		@return string representation of this object MagnitudeType
	*/
	const EmaString& getMagnitudeTypeAsString() const;
		
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::RealEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::BlankEnum if received data is blank; Data::NoCodeEnum otherwise
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the object hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Returns Mantissa.
		@return value of OmmReal::Mantissa
	*/
	Int64 getMantissa() const;

	/** Returns MagnitudeType.
		@return value of OmmReal::MagnitudeType
	*/
	MagnitudeType getMagnitudeType() const;

	/** Returns AsDouble.
		@return value of Real as double
	*/
	double getAsDouble() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;

	Decoder& getDecoder();

	const EmaString& toString( UInt64 ) const;

	const Encoder& getEncoder() const;

	OmmReal();
	virtual ~OmmReal();
	OmmReal( const OmmReal& );
	OmmReal& operator=( const OmmReal& );

	OmmRealDecoder*		_pDecoder;
	UInt64				_space[14];
};

}

}

}

#endif //__thomsonreuters_ema_access_OmmReal_h
