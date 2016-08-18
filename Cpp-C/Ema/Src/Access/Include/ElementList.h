/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ElementList_h
#define __thomsonreuters_ema_access_ElementList_h

/**
	@class thomsonreuters::ema::access::ElementList ElementList.h "Access/Include/ElementList.h"
	@brief ElementList is a heterogeneous container of complex and primitive data type entries.
	
	ElementList entries are identified by name.

	ElementList supports two methods of adding containers; they are:
	- adding of already populated containers, (e.g. complete() was called) and 
	- adding of clear containers (e.g. clear() was called) which would be populated after that.

	The first method of adding of already populated containers allows for easy data
	manipulation but incurs additional memory copy. This method is useful in
	applications extracting data containers from some messages or containers and then
	setting them on other containers.

	The second method allows for fast container population since it avoids additional
	memory copy incurred by the first method. This method is useful in source applications
	setting OMM data from native data formats.

	The following code snippet shows addition of primitive data types to ElementList.

	\code

	ElementList eList;

	EmaString entryName;
	eList.addAscii( entryName.set( "string" ), EmaString( "entry 1" ) )
		.addInt( entryName.set( "int" ), 123 )
		.addRealFromDouble( entryName.set( "real" ), 1.2345 )
		.complete();

	\endcode

	The following code snippet shows addition of pre-populated containers to ElementList.

	\code

	ElementList eList;
	OmmArray array;
	EmaString entryName;

	// first step - populate array
	array.addInt( 12 ).addInt( -23 ).addInt( 234 ).complete();

	// second step - add already populated array
	eList.addArray( entryName.set( "populated array" ), array );
	
	// third step - complete ElementList
	eList.complete();

	\endcode

	The following code snippet shows addition of clear containers to ElementList.
	The added containers are populated right after addition.

	\code

	ElementList eList;
	EmaString entryName;
	OmmArray array;

	// first step - add clear array
	eList.addArray( entryName.set( "clear array" ), array );

	// second step - populate array
	array.addInt( 12 ).addInt( -23 ).addInt( 234 ).complete();

	// third step - complete ElementList
	eList.complete()

	\endcode

	The following code snippet shows extracting data from ElementList.

	\code

	const MapEntry& mEntry = map.getEntry();

	if ( mEntry.getLoadType() == DataType::ElementListEnum )
	{
		const ElementList& eList = map.getElementList();

		while ( eList.forth() )
		{
			const ElementEntry& eEntry = eList.getEntry();
			switch ( eEntry.getLoadType() )
			{
				case DataType::IntEnum :
					Int64 value = eEntry.getInt();
					break;
				case DataType::RealEnum :
					const OmmReal& real = eEntry.getReal();
					break;

				...
			}
		}
	}

	\endcode

	\remark These two methods apply to containers only: OmmArray, ElementList,
			FieldList, FilterList, Map, Series, and Vector.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of ElementList and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		ElementEntry,
		ReqMsg,
		RefreshMsg,
		UpdateMsg,
		StatusMsg,
		GenericMsg,
		PostMsg,
		AckMsg,
		FieldList,
		Map,
		Vector,
		Series,
		FilterList,
		OmmOpaque,
		OmmXml,
		OmmAnsiPage,
		OmmError,
		EmaString,
		EmaBuffer
*/

#include "Access/Include/ComplexType.h"
#include "Access/Include/OmmReal.h"
#include "Access/Include/OmmState.h"
#include "Access/Include/OmmQos.h"
#include "Access/Include/ElementEntry.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class EmaString;
class EmaBuffer;
class ReqMsg;
class RefreshMsg;
class StatusMsg;
class UpdateMsg;
class GenericMsg;
class PostMsg;
class AckMsg;
class FieldList;
class Map;
class Vector;
class Series;
class FilterList;
class OmmArray;
class OmmOpaque;
class OmmXml;
class OmmAnsiPage;

class ElementListDecoder;
class ElementListEncoder;

class EMA_ACCESS_API ElementList : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs ElementList.
	*/
	ElementList();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ElementList();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@retrun DataType::ElementListEnum
	*/
	DataType::DataTypeEnum getDataType() const;

	/** Returns the Code, which indicates a special state of a DataType.
		@return Data::NoCodeEnum
	*/
	Data::DataCode getCode() const;

	/** Returns a buffer that in turn provides an alphanumeric null-terminated hexadecimal string representation.
		@return EmaBuffer with the message hex information
	*/
	const EmaBuffer& getAsHex() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of Info.
		@return true if ElementList Info is set; false otherwise
	*/
	bool hasInfo() const;

	/** Returns InfoElementListNum.
		@throw OmmInvalidUsageException if hasInfo() returns false
		@return ElementList Number
	*/
	Int16 getInfoElementListNum() const;

	/** Iterates through a list of Data of any DataType.
		Typical usage is to extract the entry during each iteration via getEntry().
		@return false at the end of ElementList; true otherwise
	*/
	bool forth() const;

	/** Iterates through a list of Data matching the name.
		Typical usage is to extract the entry during each iteration via getEntry().
		@param[in] name looked up ElementEntry's name
		@return false at the end of ElementList; true otherwise
	*/
	bool forth( const EmaString& name ) const;

	/** Iterates through a list of Data having the name matching the specified Data.
		Typical usage is for the Data to be a view, and thus extract each matched entry during each iteration via getEntry().
		@param[in] data containing ElementList with a view definition specifying looked up names
		@return false at the end of ElementList; true otherwise
	*/
	bool forth( const Data& data ) const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return ElementEntry
	*/
	const ElementEntry& getEntry() const;

	/** Resets iteration to start of container.
	*/
	void reset() const;
	//@}

	///@name Operations
	//@{
	/** Clears the ElementList.
		\remark Note: allows re-use of ElementList instance during encoding.
		@return reference to this object
	*/
	ElementList& clear();

	/** Specifies Info.
		\remark The ElementList Info is optional. If used, it must be specified before adding anything to ElementList.
		@param[in] elementListNum FieldList template number
		@return reference to this object
	*/
	ElementList& info( Int16 elementListNum );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added ReqMsg
		@return reference to this object
	*/
	ElementList& addReqMsg( const EmaString& name, const ReqMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added RefreshMsg
		@return reference to this object
	*/
	ElementList& addRefreshMsg( const EmaString& name, const RefreshMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added StatusMsg
		@return reference to this object
	*/
	ElementList& addStatusMsg( const EmaString& name, const StatusMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added UpdateMsg
		@return reference to this object
	*/
	ElementList& addUpdateMsg( const EmaString& name, const UpdateMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added PostMsg
		@return reference to this object
	*/
	ElementList& addPostMsg( const EmaString& name, const PostMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added AckMsg
		@return reference to this object
	*/
	ElementList& addAckMsg( const EmaString& name, const AckMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added GenericMsg
		@return reference to this object
	*/
	ElementList& addGenericMsg( const EmaString& name, const GenericMsg& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added FieldList
		@return reference to this object
	*/
	ElementList& addFieldList( const EmaString& name, const FieldList& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added ElementList
		@return reference to this object
	*/
	ElementList& addElementList( const EmaString& name, const ElementList& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added Map
		@return reference to this object
	*/
	ElementList& addMap( const EmaString& name, const Map& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added Vector
		@return reference to this object
	*/
	ElementList& addVector( const EmaString& name, const Vector& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added Series
		@return reference to this object
	*/
	ElementList& addSeries( const EmaString& name, const Series& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added FilterList
		@return reference to this object
	*/
	ElementList& addFilterList( const EmaString& name, const FilterList& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added OmmOpaque
		@return reference to this object
	*/
	ElementList& addOpaque( const EmaString& name, const OmmOpaque& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added OmmXml
		@return reference to this object
	*/
	ElementList& addXml( const EmaString& name, const OmmXml& value );

	/** Adds a complex type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added OmmAnsiPage
		@return reference to this object
	*/
	ElementList& addAnsiPage( const EmaString& name, const OmmAnsiPage& value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added Int64
		@return reference to this object
	*/
	ElementList& addInt( const EmaString& name, Int64 value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added UInt64
		@return reference to this object
	*/
	ElementList& addUInt( const EmaString& name, UInt64 value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] mantissa added OmmReal::mantissa
		@param[in] magnitudeType added OmmReal::magnitudeType
		@return reference to this object
	*/
	ElementList& addReal( const EmaString& name, Int64 mantissa, OmmReal::MagnitudeType magnitudeType );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added double to be converted to OmmReal
		@param[in] magnitudeType OmmReal::magnitudeType used for the conversion (default value is OmmReal::Exponent0Enum)
		@return reference to this object
	*/
	ElementList& addRealFromDouble( const EmaString& name, double value,
								OmmReal::MagnitudeType magnitudeType = OmmReal::Exponent0Enum );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added float
		@return reference to this object
	*/
	ElementList& addFloat( const EmaString& name, float value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added double
		@return reference to this object
	*/
	ElementList& addDouble( const EmaString& name, double value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDate is invalid
		@param[in] name EmaString object containing ElementEntry name
		@param[in] year added OmmDate::year (0 - 4095 where 0 indicates blank)
		@param[in] month added OmmDate::month (0 - 12 where 0 indicates blank)
		@param[in] day added OmmDate::day (0 - 31 where 0 indicates blank)
		@return reference to this object
	*/
	ElementList& addDate( const EmaString& name, UInt16 year, UInt8 month, UInt8 day );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmTime is invalid
		@param[in] name EmaString object containing ElementEntry name
		@param[in] hour added OmmTime::hour (0 - 23 where 255 indicates blank)
		@param[in] minute added OmmTime::minute (0 - 59 where 255 indicates blank)
		@param[in] second added OmmTime::second (0 - 60 where 255 indicates blank)
		@param[in] millisecond added OmmTime::millisecond (0 - 999 where 65535 indicates blank)
		@param[in] microsecond added OmmTime::microsecond (0 - 999 where 2047 indicates blank)
		@param[in] nanosecond added OmmTime::nanosecond (0 - 999 where 2047 indicates blank)
		@return reference to this object
	*/
	ElementList& addTime( const EmaString& name, UInt8 hour = 0, UInt8 minute = 0, UInt8 second = 0,
						UInt16 millisecond = 0, UInt16 microsecond = 0, UInt16 nanosecond = 0 );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDateTime is invalid
		@param[in] name EmaString object containing ElementEntry name
		@param[in] year added OmmDateTime::year (0 - 4095 where 0 indicates blank)
		@param[in] month added OmmDateTime::month (0 - 12 where 0 indicates blank)
		@param[in] day added OmmDateTime::day (0 - 31 where 0 indicates blank)
		@param[in] hour added OmmDateTime::hour (0 - 23 where 255 indicates blank)
		@param[in] minute added OmmDateTime::minute (0 - 59 where 255 indicates blank)
		@param[in] second added OmmDateTime::second (0 - 60 where 255 indicates blank)
		@param[in] millisecond added OmmDateTime::millisecond (0 - 999 where 65535 indicates blank)
		@param[in] microsecond added OmmDateTime::microsecond (0 - 999 where 2047 indicates blank)
		@param[in] picoseconds added OmmDateTime::nanosecond (0 - 999 where 2047 indicates blank)
		@return reference to this object
	*/
	ElementList& addDateTime( const EmaString& name,
							UInt16 year, UInt8 month, UInt8 day,
							UInt8 hour = 0, UInt8 minute = 0, UInt8 second = 0,
							UInt16 millisecond = 0, UInt16 microsecond = 0, UInt16 nanosecond = 0 );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] timeliness added OmmQos::timeliness (default value is OmmQos::RealTimeEnum)
		@param[in] rate added OmmQos::rate (default value is OmmQos::TickByTickEnum)
		@return reference to this object
	*/
	ElementList& addQos( const EmaString& name,
						UInt32 timeliness = OmmQos::RealTimeEnum,
						UInt32 rate = OmmQos::TickByTickEnum );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] streamState added OmmState::streamState (default value is OmmState::OpenEnum)
		@param[in] dataState added OmmState::dataState (default value is OmmState::OkEnum)
		@param[in] statusCode added OmmState::statusCode (default value is OmmState::NoneEnum)
		@param[in] statusText added OmmState::text (default value is 'empty string')
		@return reference to this object
	*/
	ElementList& addState( const EmaString& name,
						OmmState::StreamState streamState = OmmState::OpenEnum,
						OmmState::DataState dataState = OmmState::OkEnum,
						UInt8 statusCode = OmmState::NoneEnum,
						const EmaString& statusText = EmaString() );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added Enum
		@return reference to this object
	*/
	ElementList& addEnum( const EmaString& name, UInt16 value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added EmaBuffer as OmmBuffer
		@return reference to this object
	*/
	ElementList& addBuffer( const EmaString& name, const EmaBuffer& value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added EmaString as OmmAscii
		@return reference to this object
	*/
	ElementList& addAscii( const EmaString& name, const EmaString& value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added EmaBuffer as OmmUtf8
		@return reference to this object
	*/
	ElementList& addUtf8( const EmaString& name, const EmaBuffer& value );

	/** Adds a specific simple type of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added EmaBuffer as OmmRmtes
		@return reference to this object
	*/
	ElementList& addRmtes( const EmaString& name, const EmaBuffer& value );

	/** Adds an OmmArray of OMM data to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] name EmaString object containing ElementEntry name
		@param[in] value added OmmArray
		@return reference to this object
	*/
	ElementList& addArray( const EmaString& name, const OmmArray& value );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeInt( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeUInt( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeReal( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeFloat( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeDouble( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeDate( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeTime( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeDateTime( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeQos( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeState( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeEnum( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeBuffer( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeAscii( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeUtf8( const EmaString& name );

	/** Adds a blank data code to the ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	ElementList& addCodeRmtes( const EmaString& name );

	/** Completes encoding of ElementList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return const reference to this object
	*/
	const ElementList& complete();
	//@}

private :

	void getInfoXmlStr( EmaString& ) const;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	const EmaString& toString( UInt64 indent ) const;

	mutable EmaString			_toString;
	ElementEntry				_entry;
	ElementListDecoder*			_pDecoder;
	mutable ElementListEncoder*	_pEncoder;

	ElementList( const ElementList& );
	ElementList& operator=( const ElementList& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ElementList_h
