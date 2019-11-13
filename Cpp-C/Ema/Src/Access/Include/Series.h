/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_Series_h
#define __thomsonreuters_ema_access_Series_h

/**
	@class thomsonreuters::ema::access::Series Series.h "Access/Include/Series.h"
	@brief Series is a homogeneous container of complex data type entries.

	Series entries have no explicit identification. They are implicitly indexed inside Series.

	Series supports two methods of adding containers; they are:
	- adding of already populated containers, (e.g. complete() was called) and 
	- adding of clear containers (e.g. clear() was called) which would be populated after that.

	The first method of adding of already populated containers allows for easy data
	manipulation but incurs additional memory copy. This method is useful in
	applications extracting data containers from some messages or containers and then
	setting them on other containers.

	The second method allows for fast container population since it avoids additional
	memory copy incurred by the first method. This method is useful in source applications
	setting OMM data from native data formats.

	The following code snippet shows addition of entry and summaryData to Series.

	\code

	FieldList fList;
	fList.addInt( 1, 1 ).
		addUInt( 100, 2 ).
		addArray( 2000, Array().addInt( 1 ).addInt( 2 ).complete() ).
		complete();

	Series series;
	series.summaryData( fList ).
		.add( fList ).
		complete();

	\endcode

	The following code snippet shows extracting of Series and its content.

	\code 

	void decodeSeries( const Series& series )
	{
		switch ( series.getSummaryData().getDataType() )
		{
		case DataType::FieldListEnum :
			decodeFieldList( series.getSummaryData().getFieldList() );
			break;
		case DataType::NoDataEnum :
			break;
		}

		while ( series.forth() )
		{
			const SeriesEntry& sEntry = series.getEntry();

			switch ( sEntry.getLoad().getDataType() )
			{
			case DataType::FieldListEnum :
				decodeFieldList( sEntry.getLoad().getFieldList() );
				break;
			case DataType::NoDataEnum :
				break;
			}
		}
	}

	\endcode

	\remark These two methods apply to containers only, e.g.: ElementList,
			FieldList, FilterList, Map, Series, and Vector.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of Series and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		SeriesEntry,
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

#include "Access/Include/SeriesEntry.h"
#include "Access/Include/SummaryData.h"

namespace thomsonreuters {

namespace ema {

namespace rdm {
	
class DataDictionaryImpl;

}

namespace access {

class SeriesDecoder;
class SeriesEncoder;

class EMA_ACCESS_API Series : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs Series.
	*/
	Series();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Series();
	//@}

	///@name Operations
	//@{
	/** Clears the Series.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	Series& clear();

	/** Specifies TotalCountHint.
		@param[in] totalCountHint specifies total count hint
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	Series& totalCountHint( UInt32 totalCountHint );

	/** Specifies the SummaryData OMM Data.
		\remark Call to summaryData( ) must happen prior to calling the add( ) method
		@param[in] summaryData specifies complex type as summaryData
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	Series& summaryData( const ComplexType& data );

	/** Adds complex OMM data identified by a specific complex type of OMM data.
		\remark All entries must have same complex data type
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] value complex type contained in this entry
		@return reference to this object
	*/
	Series& add( const ComplexType& value );

	/** Adds an entry with no payload to the Series.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return reference to this object
	*/
	Series& add();

	/** Completes encoding of the Series.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return const reference to this object
	*/
	const Series& complete();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::SeriesEnum
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
		@return false at the end of Series; true otherwise
	*/
	bool forth() const;

	/** Resets iteration to start of container.
	*/
	void reset() const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return SeriesEntry
	*/
	const SeriesEntry& getEntry() const;

	/** Indicates presence of TotalCountHint.
		@return true if total count hint is set; false otherwise
	*/
	bool hasTotalCountHint() const;

	/** Returns TotalCountHint.
		\remark will throw OmmInvalidUsageException if hasTotalCountHint() returns false
		@return total count hint
	*/
	UInt32 getTotalCountHint() const;

	/** Returns the contained summaryData Data based on the summaryData DataType.
		\remark SummaryData contains no data if SummaryData::getDataType() returns Data::NoDataEnum
		@return SummaryData
	*/
	const SummaryData& getSummaryData() const;
	//@}

private :

	friend class thomsonreuters::ema::rdm::DataDictionaryImpl;

	Decoder& getDecoder();
	bool hasDecoder() const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	const EmaString& toString( UInt64 ) const;

	mutable EmaString			_toString;
	SeriesEntry					_entry;
	SummaryData					_summary;
	SeriesDecoder*				_pDecoder;
	mutable SeriesEncoder*		_pEncoder;

	Series( const Series& );
	Series& operator=( const Series& );
};

}

}

}

#endif // __thomsonreuters_ema_access_Series_h
