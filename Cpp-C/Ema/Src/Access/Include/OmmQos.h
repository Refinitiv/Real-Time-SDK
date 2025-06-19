/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmQos_h
#define __refinitiv_ema_access_OmmQos_h

/**
	@class refinitiv::ema::access::Qos Qos.h "Access/Include/Qos.h"
	@brief OmmQos represents Quality Of Service information in Omm.

	\remark OmmQos is a read only class.
	\remark This class is used for extraction of OmmQos only.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/Data.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmQosDecoder;

class EMA_ACCESS_API OmmQos : public Data
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
	const EmaString& getRateAsString() const;

	/** Returns the QosTimeliness value as a string format.
		@return string representation of this object timeliness
	*/
	const EmaString& getTimelinessAsString() const;
		
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::QosEnum
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

	/** Returns Timeliness.
		@return value of OmmQos::Timeliness
	*/
	UInt32 getTimeliness() const;

	/** Returns Rate.
		@return value of OmmQos::Rate
	*/
	UInt32 getRate() const;
	//@}

private :

	friend class Decoder;
	friend class StaticDecoder;
	friend class RefreshMsgDecoder;
	friend class ReqMsgDecoder;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 indent ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	OmmQos();
	virtual ~OmmQos();
	OmmQos( const OmmQos& );
	OmmQos& operator=( const OmmQos& );

	OmmQosDecoder*			_pDecoder;
	UInt64					_space[16];
};

}

}

}

#endif // __refinitiv_ema_access_OmmQos_h
