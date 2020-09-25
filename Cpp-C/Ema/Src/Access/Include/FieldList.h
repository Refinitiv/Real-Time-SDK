/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_FieldList_h
#define __refinitiv_ema_access_FieldList_h

/**
	@class refinitiv::ema::access::FieldList FieldList.h "Access/Include/FieldList.h"
	@brief FieldList is a heterogeneous container of complex and primitive data type entries.

	FieldList entries are identified by Field Id. The meaning of the Field Id is conveyed by
	the RDMFieldDictionary.

	FieldList supports two methods of adding containers; they are:
	- adding of already populated containers, (e.g. complete() was called) and 
	- adding of clear containers (e.g. clear() was called) which would be populated after that.

	The first method of adding of already populated containers allows for easy data
	manipulation but incurs additional memory copy. This method is useful in
	applications extracting data containers from some messages or containers and then
	setting them on other containers.

	The second method allows for fast container population since it avoids additional
	memory copy incurred by the first method. This method is useful in source applications
	setting OMM data from native data formats.

	The following code snippet shows addition of primitive data types to FieldList.

	\code

	FieldList fList;

	fList.addAscii( 235, EmaString( "entry 1" ) )
		.addUInt( 239, 123 )
		.addRealFromDouble( 234, 1.2345, OmmReal::ExponentNeg4Enum )
		.complete();

	\endcode

	The following code snippet shows addition of pre-populated containers to FieldList.

	\code

	FieldList fList;
	OmmArray array;

	// first step - populate array
	array.addInt( 12 ).addInt( -23 ).addInt( 234 ).complete();

	// second step - add already populated array
	fList.addArray( 1234, array );
	
	// third step - complete FieldList
	fList.complete();

	\endcode

	The following code snippet shows addition of clear containers to FieldList.
	The added containers are populated right after addition.

	\code

	FieldList fList;
	OmmArray array;

	// first step - add clear array
	fList.addArray( 1234, array );

	// second step - populate array
	array.addInt( 12 ).addInt( -23 ).addInt( 234 ).complete();

	// third step - complete FieldList
	fList.complete()

	\endcode

	The following code snippet shows getting data from FieldList.

	\code

	const MapEntry& mEntry = map.getEntry();

	if ( mEntry.getLoadType() == DataType::FieldListEnum )
	{
		const FieldList& fList = map.getFieldList();

		while ( fList.forth() )
		{
			const FieldEntry& fEntry = fList.getEntry();
			switch ( fEntry.getLoadType() )
			{
				case DataType::UIntEnum :
					UInt64 value = eEntry.getUInt();
					break;
				case DataType::RealEnum :
					const OmmReal& real = fEntry.getReal();
					break;

				...
			}
		}
	}

	\endcode

	\remark These two methods apply to containers only: OmmArray, ElementList,
			FieldList, FilterList, Map, Series, and Vector.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of FieldList and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		FieldEntry,
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

#include "Access/Include/ComplexType.h"
#include "Access/Include/OmmReal.h"
#include "Access/Include/OmmState.h"
#include "Access/Include/OmmQos.h"
#include "Access/Include/FieldEntry.h"

namespace refinitiv {

namespace ema {

namespace rdm {

class DictionaryUtility;

}

namespace access {

class ReqMsg;
class RefreshMsg;
class StatusMsg;
class UpdateMsg;
class GenericMsg;
class PostMsg;
class AckMsg;
class ElementList;
class Map;
class Vector;
class Series;
class FilterList;
class OmmArray;
class OmmOpaque;
class OmmXml;
class OmmAnsiPage;

class FieldListDecoder;
class FieldListEncoder;

class EMA_ACCESS_API FieldList : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs FieldList.
	*/
	FieldList();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~FieldList();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::FieldListEnum
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
		@return true if FieldList Info is set; false otherwise
	*/
	bool hasInfo() const;

	/** Returns InfoFieldListNum.
		@throw OmmInvalidUsageException if hasInfo() returns false
		@return FieldList Number
	*/
	Int16 getInfoFieldListNum() const;

	/** Returns InfoDictionaryId.
		@throw OmmInvalidUsageException if hasInfo() returns false
		@return DictionaryId associated with this FieldList
	*/
	Int16 getInfoDictionaryId() const;

	/** Iterates through a list of Data of any DataType.
		Typical usage is to extract the entry during each iteration via getEntry().
		@return false at the end of FieldList; true otherwise
	*/
	bool forth() const;

	/** Iterates through a list of Data having the FieldId matching in the field dictionary.
		Typical usage is to extract the entry during each iteration via getEntry().
		@param[in] fieldId looked up FieldEntry's FieldId
		@return false at the end of FieldList; true otherwise
	*/
	bool forth( Int16 fieldId ) const;

	/** Iterates through a list of Data having the name matching the acronym in the field dictionary.
		Typical usage is to extract the entry during each iteration via getEntry().
		@param[in] name looked up FieldEntry's Field name (from RdmFieldDictionary)
		@return false at the end of FieldList; true otherwise
	*/
	bool forth( const EmaString& name ) const;

	/** Iterates through a list of Data having the name matching the specified Data.
	    Typical usage is for the Data to be a view, and thus extract each matched entry during each iteration via getEntry().
		@param[in] data containing ElementList with a view definition specifying looked up field Ids
		@return false at the end of FieldList; true otherwise
	*/
	bool forth( const Data& data ) const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return FieldEntry
	*/
	const FieldEntry& getEntry() const;

	/** Resets iteration to start of container.
	*/
	void reset() const;
	//@}

	///@name Operations
	//@{
	/** Clears the FieldList.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	FieldList& clear();

	/** Specifies Info.
		\remark The FieldList Info is optional. If used, it must be set prior to adding anything to FieldList.
		@throw OmmInvalidUsageException if this method is called after adding an entry to FieldList.
		@param[in] dictionaryId dictionary id of the RdmFieldDictioanry associated with this FieldList
		@param[in] fieldListNum FieldList template number
		@return reference to this object
	*/
	FieldList& info( Int16 dictionaryId, Int16 fieldListNum );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added ReqMsg
		@return reference to this object
	*/
	FieldList& addReqMsg( Int16 fieldId, const ReqMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added RefreshMsg
		@return reference to this object
	*/
	FieldList& addRefreshMsg( Int16 fieldId, const RefreshMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added StatusMsg
		@return reference to this object
	*/
	FieldList& addStatusMsg( Int16 fieldId, const StatusMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added UpdateMsg
		@return reference to this object
	*/
	FieldList& addUpdateMsg( Int16 fieldId, const UpdateMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added PostMsg
		@return reference to this object
	*/
	FieldList& addPostMsg( Int16 fieldId, const PostMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added AckMsg
		@return reference to this object
	*/
	FieldList& addAckMsg( Int16 fieldId, const AckMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added GenericMsg
		@return reference to this object
	*/
	FieldList& addGenericMsg( Int16 fieldId, const GenericMsg& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added FieldList
		@return reference to this object
	*/
	FieldList& addFieldList( Int16 fieldId, const FieldList& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added ElementList
		@return reference to this object
	*/
	FieldList& addElementList( Int16 fieldId, const ElementList& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added Map
		@return reference to this object
	*/
	FieldList& addMap( Int16 fieldId, const Map& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added Vector
		@return reference to this object
	*/
	FieldList& addVector( Int16 fieldId, const Vector& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added Series
		@return reference to this object
	*/
	FieldList& addSeries( Int16 fieldId, const Series& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added FilterList
		@return reference to this object
	*/
	FieldList& addFilterList( Int16 fieldId, const FilterList& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added OmmOpaque
		@return reference to this object
	*/
	FieldList& addOpaque( Int16 fieldId, const OmmOpaque& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added OmmXml
		@return reference to this object
	*/
	FieldList& addXml( Int16 fieldId, const OmmXml& value );

	/** Adds a complex type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added OmmAnsiPage
		@return reference to this object
	*/
	FieldList& addAnsiPage( Int16 fieldId, const OmmAnsiPage& value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added Int64
		@return reference to this object
	*/
	FieldList& addInt( Int16 fieldId, Int64 value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added UInt64
		@return reference to this object
	*/
	FieldList& addUInt( Int16 fieldId, UInt64 value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] mantissa added OmmReal::Mantissa
		@param[in] magnitudeType added OmmReal::MagnitudeType
		@return reference to this object
	*/
	FieldList& addReal( Int16 fieldId, Int64 mantissa, OmmReal::MagnitudeType magnitudeType );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added double to be converted to OmmReal
		@param[in] magnitudeType OmmReal::magnitudeType used for the conversion (default value is OmmReal::Exponent0Enum)
		@return reference to this object
	*/
	FieldList& addRealFromDouble( Int16 fieldId, double value,
		OmmReal::MagnitudeType magnitudeType = OmmReal::Exponent0Enum );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added float
		@return reference to this object
	*/
	FieldList& addFloat( Int16 fieldId, float value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added double
		@return reference to this object
	*/
	FieldList& addDouble( Int16 fieldId, double value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmDate is invalid
		@param[in] fieldId field id value
		@param[in] year added OmmDate::year (0 - 4095 where 0 indicates blank)
		@param[in] month added OmmDate::month (0 - 12 where 0 indicates blank)
		@param[in] day added OmmDate::day (0 - 31 where 0 indicates blank)
		@return reference to this object
	*/
	FieldList& addDate( Int16 fieldId, UInt16 year, UInt8 month, UInt8 day );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@throw OmmOutOfRangeException if passed in OmmTime is invalid
		@param[in] fieldId field id value
		@param[in] hour added OmmTime::hour (0 - 23 where 255 indicates blank)
		@param[in] minute added OmmTime::minute (0 - 59 where 255 indicates blank)
		@param[in] second added OmmTime::second (0 - 60 where 255 indicates blank)
		@param[in] millisecond added OmmTime::millisecond (0 - 999 where 65535 indicates blank)
		@param[in] microsecond added OmmTime::microsecond (0 - 999 where 2047 indicates blank)
		@param[in] nanosecond added OmmTime::nanosecond (0 - 999 where 2047 indicates blank)
		@return reference to this object
	*/
	FieldList& addTime( Int16 fieldId, UInt8 hour = 0, UInt8 minute = 0, UInt8 second = 0,
						UInt16 millisecond = 0, UInt16 microsecond = 0, UInt16 nanosecond = 0 );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmOutOfRangeException if passed in OmmDateTime is invalid
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
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
	FieldList& addDateTime( Int16 fieldId, UInt16 year, UInt8 month, UInt8 day,
							UInt8 hour = 0, UInt8 minute = 0, UInt8 second = 0,
							UInt16 millisecond = 0, UInt16 microsecond = 0, UInt16 nanosecond = 0 );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] timeliness added OmmQos::timeliness (default value is OmmQos::RealTimeEnum)
		@param[in] rate added OmmQos::rate (default value is OmmQos::TickByTickEnum)
		@return reference to this object
	*/
	FieldList& addQos( Int16 fieldId, UInt32 timeliness = OmmQos::RealTimeEnum, UInt32 rate = OmmQos::TickByTickEnum );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] streamState added OmmState::streamState (default value is OmmState::OpenEnum)
		@param[in] dataState added OmmState::dataState (default value is OmmState::OkEnum)
		@param[in] statusCode added OmmState::statusCode (default value is OmmState::NoneEnum)
		@param[in] statusText added OmmState::text (default value is 'empty string')
		@return reference to this object
	*/
	FieldList& addState( Int16 fieldId, OmmState::StreamState streamState = OmmState::OpenEnum,
										OmmState::DataState dataState = OmmState::OkEnum,
										UInt8 statusCode = OmmState::NoneEnum,
										const EmaString& statusText = EmaString() );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added Enum
		@return reference to this object
	*/
	FieldList& addEnum( Int16 fieldId, UInt16 value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added EmaBuffer as OmmBuffer
		@return reference to this object
	*/
	FieldList& addBuffer( Int16 fieldId, const EmaBuffer& value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added EmaString as OmmAscii
		@return reference to this object
	*/
	FieldList& addAscii( Int16 fieldId, const EmaString& value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added EmaBuffer as OmmUtf8
		@return reference to this object
	*/
	FieldList& addUtf8( Int16 fieldId, const EmaBuffer& value );

	/** Adds a specific simple type of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added EmaBuffer as OmmRmtes
		@return reference to this object
	*/
	FieldList& addRmtes( Int16 fieldId, const EmaBuffer& value );

	/** Adds an OmmArray of OMM data to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] fieldId field id value
		@param[in] value added OmmArray
		@return reference to this object
	*/
	FieldList& addArray( Int16 fieldId, const OmmArray& value );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeInt( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeUInt( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeReal( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeFloat( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeDouble( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeDate( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeTime( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeDateTime( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeQos( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeState( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeEnum( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@return reference to this object
	*/
	FieldList& addCodeBuffer( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeAscii( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	FieldList& addCodeUtf8( Int16 fieldId );

	/** Adds a blank data code to the FieldList.
		@return reference to this object
	*/
	FieldList& addCodeRmtes( Int16 fieldId );

	/** Completes encoding of FieldList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return const reference to this object
	*/
	const FieldList& complete();
	//@}

private :

	friend class refinitiv::ema::rdm::DictionaryUtility;

	void getInfoXmlStr( EmaString& ) const;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	const EmaString& toString( UInt64 indent ) const;

	mutable EmaString			_toString;
	FieldEntry					_entry;
	FieldListDecoder*			_pDecoder;
	mutable FieldListEncoder*	_pEncoder;

	FieldList( const FieldList& );
	FieldList& operator=( const FieldList&);
};

}

}

}

#endif // __refinitiv_ema_access_FieldList_h
