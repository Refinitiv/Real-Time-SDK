/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_access_Map_h
#define __rtsdk_ema_access_Map_h

/**
	@class rtsdk::ema::access::Map Map.h "Access/Include/Map.h"
	@brief Map is a homogeneous container of complex data type entries.

	Map entries are identified by a map key. All entries must have key of the
	same primitive data type. All entries must have same complex data type
	(except for delete action).

	The following code snippet shows addition of entry and summaryData to Map.

	\code

	FieldList fList;
	fList.addInt( 1, 1 ).
		addUInt( 100, 2 ).
		addArray( 2000, Array().addInt( 1 ).addInt( 2 ).complete() ).
		complete();

	Map map;
	map.totalCountHint( 1 ).
		summaryData( fList ).
		addKeyBuffer( EmaBuffer( "1234567" ), MapEntry::AddEnum, fList ).
		complete();

	\endcode

	The following code snippet shows extracting of Map and its content.

	\code 

	void decodeMap( const Map& map )
	{
		switch ( map.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
			decodeFieldList( map.getSummaryData().getFieldList() );
			break;
		case DataType::NoDataEnum :
			break;
		}

		while ( map.forth() )
		{
			const MapEntry& mEntry = map.getEntry();

			const Key& key = mEntry.getKey();
			switch ( key.getDataType() )
			{
				case DataType::BufferEnum :
					const EmaBuffer& keyBuffer = key.getBuffer();
					break;
			}

			cout << "action = " << mEntry.getMapActionAsString() << "\n";

			switch ( mEntry.getLoad().getDataType() )
			{
			case DataType::FieldListEnum :
				decodeFieldList( mEntry.getLoad().getFieldList() );
				break;
			case DataType::NoDataEnum :
				break;
			}
		}
	}

	\endcode

	\remark These two methods apply to containers only: ElementList, FieldList,
			FilterList, Map, Series, and Vector.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of Map and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		MapEntry,
		SummaryData,
		ReqMsg,
		RefreshMsg,
		UpdateMsg,
		StatusMsg,
		GenericMsg,
		PostMsg,
		AckMsg,
		ElementList,
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

#include "Access/Include/OmmReal.h"
#include "Access/Include/OmmState.h"
#include "Access/Include/MapEntry.h"
#include "Access/Include/SummaryData.h"

namespace rtsdk {

namespace ema {

namespace access {

class MapDecoder;
class MapEncoder;

class EMA_ACCESS_API Map : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs Map
	*/
	Map();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Map();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::MapEnum
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

	/** Iterates through a list of Data of any DataType. Typical usage is to extract the entry during each iteration via getEntry().
		@return false at the end of Map; true otherwise
	*/
	bool forth() const;

	/** Resets iteration to start of container.
	*/
	void reset() const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return MapEntry
	*/
	const MapEntry& getEntry() const;

	/** Indicates presence of KeyFieldId.
		@return true if key field id is set; false otherwise
	*/
	bool hasKeyFieldId() const;

	/** Indicates presence of TotalCountHint.
		@return true if total count hint is set; false otherwise
	*/
	bool hasTotalCountHint() const;

	/** Returns KeyFieldId.
		@throw OmmInvalidUsageException if hasKeyFieldId() returns false
		@return key field id
	*/
	Int16 getKeyFieldId() const;

	/** Returns TotalCountHint.
		@throw OmmInvalidUsageException if hasTotalCountHint() returns false
		@return total count hint
	*/
	UInt32 getTotalCountHint() const;

	/** Returns the contained summaryData Data based on the summaryData DataType.
		\remark SummaryData contains no data if SummaryData::getDataType() returns DataType::NoDataEnum
		@return SummaryData
	*/
	const SummaryData& getSummaryData() const;
	//@}

	///@name Operations
	//@{
	/** Clears the Map.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	Map& clear();

	/** Specifies KeyFieldId.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@pram[in] fieldId specifies key field id
		@return reference to this object
	*/
	Map& keyFieldId( Int16 fieldId );

	/** Specifies TotalCountHint.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] totalCountHint specifies total count hint
		@return reference to this object
	*/
	Map& totalCountHint( UInt32 totalCountHint );

	/** Specifies the SummaryData OMM Data.
		\remark Call to summaryData( ) must happen prior to calling any add***( ) method
		\remark the data type of summary data must match with the load type of Map entry
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] summaryData specifies complex type as summaryData
		@return reference to this object
	*/
	Map& summaryData( const ComplexType& summaryData );

	/** Specifies a primitive type for Map entry key.
		\remark This method is used to override the default BUFFER data type.
		\remark Call this method or any add**() method can override the default key type.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@pram[in] keyPrimitiveType specifies a primitive type for the Key
		@return reference to this object
	*/
	Map& keyType( DataType::DataTypeEnum keyPrimitiveType );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing Int64 key information
		@param[in] action specifies action to be applied to the entry
		@param[in] value specifies complex type associated with this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyInt( Int64 key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing Int64 key information
		@param[in] action specifies action to be applied to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyInt( Int64 key, MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing UInt64 key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyUInt( UInt64 key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing UInt64 key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyUInt( UInt64 key, MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] mantissa specifies OmmReal::mantissa part of key information
		@param[in] magnitudeType specifies OmmReal::magnitudeType part of key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] mantissa specifies OmmReal::mantissa part of key information
		@param[in] magnitudeType specifies OmmReal::magnitudeType part of key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyReal( Int64 mantissa, OmmReal::MagnitudeType magnitudeType, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key specifies double to be converted to OmmReal
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] magnitudeType OmmReal::magnitudeType used for the conversion (default value is OmmReal::Exponent0Enum)
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyRealFromDouble( double key, MapEntry::MapAction action,
		const ComplexType& value,
		OmmReal::MagnitudeType magnitudeType = OmmReal::Exponent0Enum,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key specifies double to be converted to OmmReal
		@param[in] action specifies action to apply to the entry
		@param[in] magnitudeType OmmReal::magnitudeType used for the conversion (default value is OmmReal::Exponent0Enum)
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyRealFromDouble( double key, MapEntry::MapAction action,
		OmmReal::MagnitudeType magnitudeType = OmmReal::Exponent0Enum,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing float key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyFloat( float key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing float key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyFloat( float key, MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing double key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyDouble( double key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key containing double key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyDouble(double key, MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer());

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDate is invalid
		@param[in] year specifies OmmDate::year part of key information (0 - 4095)
		@param[in] month specifies OmmDate::month part of key information (0 - 12)
		@param[in] day specifies OmmDate::day part of key information (0 - 31)
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyDate( UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDate is invalid
		@param[in] year specifies OmmDate::year part of key information (0 - 4095)
		@param[in] month specifies OmmDate::month part of key information (0 - 12)
		@param[in] day specifies OmmDate::day part of key information (0 - 31)
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyDate( UInt16 year, UInt8 month, UInt8 day, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmTime is invalid
		@param[in] hour specifies OmmTime::hour part of key information (0 - 23)
		@param[in] minute specifies OmmTime::minute part of key information (0 - 59)
		@param[in] second specifies OmmTime::second part of key information (0 - 60)
		@param[in] millisecond specifies OmmTime::millisecond part of key information (0 - 999)
		@param[in] microsecond specifies OmmTime::microsecond part of key information (0 - 999)
		@param[in] nanosecond specifies OmmTime::nanosecond part of key information (0 - 999)
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyTime( UInt8 hour, UInt8 minute, UInt8 second,
		UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
		MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmTime is invalid
		@param[in] hour specifies OmmTime::hour part of key information (0 - 23)
		@param[in] minute specifies OmmTime::minute part of key information (0 - 59)
		@param[in] second specifies OmmTime::second part of key information (0 - 60)
		@param[in] millisecond specifies OmmTime::millisecond part of key information (0 - 999)
		@param[in] microsecond specifies OmmTime::microsecond part of key information (0 - 999)
		@param[in] nanosecond specifies OmmTime::nanosecond part of key information (0 - 999)
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyTime( UInt8 hour, UInt8 minute, UInt8 second,
		UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
		MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDateTime is invalid
		@param[in] year specifies OmmDateTime::year part of key information (0 - 4095)
		@param[in] month specifies OmmDateTime::month part of key information (0 - 12)
		@param[in] day specifies OmmDateTime::day part of key information (0 - 31)
		@param[in] hour specifies OmmDateTime::hour part of key information (0 - 23)
		@param[in] minute specifies OmmDateTime::minute part of key information (0 - 59)
		@param[in] second specifies OmmDateTime::second part of key information (0 - 60)
		@param[in] millisecond specifies OmmDateTime::millisecond part of key information  (0 - 999)
		@param[in] microsecond specifies OmmDateTime::microsecond part of key information  (0 - 999)
		@param[in] nanosecond specifies OmmDateTime::nanosecond part of key information  (0 - 999)
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyDateTime( UInt16 year, UInt8 month, UInt8 day,
		UInt8 hour, UInt8 minute, UInt8 second,
		UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
		MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDateTime is invalid
		@param[in] year specifies OmmDateTime::year part of key information (0 - 4095)
		@param[in] month specifies OmmDateTime::month part of key information (0 - 12)
		@param[in] day specifies OmmDateTime::day part of key information (0 - 31)
		@param[in] hour specifies OmmDateTime::hour part of key information (0 - 23)
		@param[in] minute specifies OmmDateTime::minute part of key information (0 - 59)
		@param[in] second specifies OmmDateTime::second part of key information (0 - 60)
		@param[in] millisecond specifies OmmDateTime::millisecond part of key information  (0 - 999)
		@param[in] microsecond specifies OmmDateTime::microsecond part of key information  (0 - 999)
		@param[in] nanosecond specifies OmmDateTime::nanosecond part of key information  (0 - 999)
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyDateTime( UInt16 year, UInt8 month, UInt8 day,
		UInt8 hour, UInt8 minute, UInt8 second,
		UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond,
		MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] timeliness specifies OmmQos::timeliness part of key information
		@param[in] rate specifies OmmQos::rate part of key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] timeliness specifies OmmQos::timeliness part of key information
		@param[in] rate specifies OmmQos::rate part of key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyQos( UInt32 timeliness, UInt32 rate, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] streamState specifies OmmState::streamState part of key information
		@param[in] dataState specifies OmmState::dataState part of key information
		@param[in] statusCode specifies OmmState::statusCode part of key information
		@param[in] statusText specifies OmmState::text part of key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
		MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] streamState specifies OmmState::streamState part of key information
		@param[in] dataState specifies OmmState::dataState part of key information
		@param[in] statusCode specifies OmmState::statusCode part of key information
		@param[in] statusText specifies OmmState::text part of key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyState( OmmState::StreamState streamState, OmmState::DataState dataState, UInt8 statusCode, const EmaString& statusText,
		MapEntry::MapAction action, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key UInt16 containing Enum key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyEnum( UInt16 key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key UInt16 containing Enum key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyEnum( UInt16 key, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaBuffer containing Buffer key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaBuffer containing Buffer key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyBuffer( const EmaBuffer& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaString containing Ascii key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyAscii( const EmaString& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaString containing Ascii key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyAscii( const EmaString& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaBuffer containing Utf8 key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaBuffer containing Utf8 key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyUtf8( const EmaBuffer& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds complex OMM data identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaBuffer containing Rmtes key information
		@param[in] action specifies action to apply to the entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
		const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer()	);

	/** Adds no payload identified by a specific simple type of OMM data.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] key EmaBuffer containing Rmtes key information
		@param[in] action specifies action to apply to the entry
		@param[in] permissionData EmaBuffer containing permission data related to this entry
		@return reference to this object
	*/
	Map& addKeyRmtes( const EmaBuffer& key, MapEntry::MapAction action,
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Completes encoding of Map.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return const reference to this object
	*/
	const Map& complete();
	//@}

private :

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	const EmaString& toString( UInt64 indent ) const;

	mutable EmaString		_toString;
	MapEntry				_entry;
	SummaryData				_summary;
	MapDecoder*				_pDecoder;
	mutable MapEncoder*		_pEncoder;

	Map( const Map& );
	Map& operator=( const Map& );
};

}

}

}

#endif // __rtsdk_ema_access_Map_h
