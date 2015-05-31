/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_vector_h
#define __thomsonreuters_ema_access_vector_h

/**
	@class thomsonreuters::ema::access::Vector Vector.h "Access/Include/Vector.h"
	@brief Vector is a homogeneous container of complex data type entries.

	Vector entries are identified by index.

	Vector supports two methods of adding containers; they are:
	- adding of already populated containers, (e.g. complete() was called on these containers) and 
	- adding of clear containers (e.g. clear() was called on these containers) which would be populated after that.

	The first method of adding of already populated containers allows for easy data
	manipulation but incurs additional memory copy. This method is useful in
	applications extracting data containers from some messages or containers and then
	setting them on other containers.

	The second method allows for fast container population since it avoids additional
	memory copy incurred by the first method. This method is useful in source applications
	setting OMM data from native data formats.

	The following code snippet shows addition of entry and summary to Vector.

	\code

	FieldList fList;
	fList.addInt( 1, 1 ).addUInt( 100, 2 ).addArray( 2000, Array().addInt( 1 ).addInt( 2 ).complete() ).complete();

	Vector vector;
	vector.sortable( true ).
		totalCountHint( 1 ).
		summary( fList ).
		add( 1, VectorEntry::SetEnum, fList ).
		complete();

	\endcode

	The following code snippet shows extracting of Vector and its content.

	\code 

	void decodeVector( const Vector& vector )
	{
		switch ( vector.getSummary().getDataType() )
		{
		case DataType::FieldListEnum :
			decodeFieldList( vector.getSummary().getFieldList() );
			break;
		case DataType::NoDataEnum :
			break;
		}

		while ( !vector.forth() )
		{
			const VectorEntry& vEntry = vector.getEntry();

			switch ( vEntry.getLoad().getDataType() )
			{
			case DataType::FieldListEnum :
				decodeFieldList( vEntry.getLoad().getFieldList() );
				break;
			case DataType::NoDataEnum :
				break;
			}
		}
	}

	\endcode

	\remark These two methods apply to containers only; e.g.,: ElementList,
			FieldList, FilterList, Map, Series, and Vector.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of Vector and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		VectorEntry,
		Summary,
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

#include "Access/Include/VectorEntry.h"
#include "Access/Include/Summary.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class VectorDecoder;
class VectorEncoder;

class EMA_ACCESS_API Vector : public ComplexType
{
public :
	
	///@name Constructor
	//@{
	/** Constructs Vector.
	*/
	Vector();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~Vector();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::VectorEnum
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
		@return true at the end of Vector; false otherwise
	*/
	bool forth() const;

	/** Resets iteration to start of container.
	*/
	void reset() const;

	/** Indicates presence of TotalCountHint.
		@return true if total count hint is set; false otherwise
	*/
	bool hasTotalCountHint() const;

	/** Returns Sortable.
		@return true if sortable flag is set; false otherwise
	*/
	bool getSortable() const;

	/** Returns TotalCountHint.
		@throw OmmInvalidUsageException if hasTotalCountHint() returns false
		@return total count hint
	*/
	UInt32 getTotalCountHint() const;

	/** Returns the contained summary Data based on the summary DataType.
		\remark Summary contains no data if Summary::getDataType() returns DataType::NoDataEnum
		@return Summary
	*/
	const Summary& getSummary() const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return VectorEntry
	*/
	const VectorEntry& getEntry() const;
	//@}

	///@name Operations
	//@{
	/** Clears the Vector.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	Vector& clear();

	/** Adds complex OMM data identified by a position.
		\remark All entries must have same complex data type
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] position specifies position of this entry in Vector
		@param[in] action specifies action to be performed on this entry
		@param[in] value complex type contained in this entry
		@param[in] permissionData specifies permission data for this entry
		@return reference to this object
	*/
	Vector& add( UInt32 position, VectorEntry::VectorAction action,
				const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Completes encoding of the Vector.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return const reference to this object
	*/
	const Vector& complete();

	/** Specifies Sortable.
		@param[in] sortable specifies if this objecxt is sortable
		@return reference to this object
	*/
	Vector& sortable( bool sortable = false );

	/** Specifies TotalCountHint.
		@param[in] totalCountHint specifies total count hint
		@return reference to this object
	*/
	Vector& totalCountHint( UInt32 totalCountHint );

	/** Specifies the Summary OMM Data.
		\remark Call to summary( ) must happen prior to calling the add( ) method
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] summary specifies complex type as summary
		@return reference to this object
	*/
	Vector& summary( const ComplexType& data );
	//@}

private :

	Decoder& getDecoder();

	const Encoder& getEncoder() const;

	const EmaString& toString( UInt64 ) const;

	mutable EmaString			_toString;
	VectorEntry					_entry;
	Summary						_summary;
	VectorDecoder*				_pDecoder;
	mutable VectorEncoder*		_pEncoder;

	Vector( const Vector& );
	Vector& operator=( const Vector& );
};

}

}

}

#endif //  __thomsonreuters_ema_access_vector_h
