/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_FilterList_h
#define __thomsonreuters_ema_access_FilterList_h

/**
	@class thomsonreuters::ema::access::FilterList FilterList.h "Access/Include/FilterList.h"
	@brief FilterList is a heterogeneous container of complex data type entries.

	FilterList entries are identified by Filter Id. For the source directory information,
	the Filter Ids are defined in the RDM (see also EmaRdm.h file).

	FilterList supports two methods of adding containers; they are:
	- adding of already populated containers, (e.g. complete() was called) and 
	- adding of clear containers (e.g. clear() was called) which would be populated after that.

	The first method of adding of already populated containers allows for easy data
	manipulation but incurs additional memory copy. This method is useful in
	applications extracting data containers from some messages or containers and then
	setting them on other containers.

	The second method allows for fast container population since it avoids additional
	memory copy incurred by the first method. This method is useful in source applications
	setting OMM data from native data formats.

	The following code snippet shows addition of pre populated containers to FilterList.

	\code

	FieldList fieldList;
	Map map;

	// first step - populate Map with the fieldList
	map.addKeyBuffer( EmaBuffer( "1234321" ), MapEntry::AddEnum, fieldList ).complete();

	// second step - add already populated Map to FilterList
	FilterList filterList;
	filterList.add( 2, FilterEntry::SetEnum; map );
	
	// third step - complete FilterList
	filterList.complete();

	\endcode

	The following code snippet shows addition of clear containers to FilterList.
	The added containers are populated right after addition.

	\code

	FilterList fList;
	Map map;

	// first step - add clear map to filterList
	fList.add( 2, FilterEntry::SetEnum; map )

	// second step - populate Map
	map.addKeyBuffer( EmaBuffer( "1234321" ), MapEntry::AddEnum, fieldList ).complete();

	// third step - complete FilterList
	fList.complete()

	\endcode

	The following code snippet shows extracting data from FilterList.

	\code

	while ( filterList.forth() )
	{
		const FilterEntry& entry = filterList.getEntry();
		switch ( entry.getLoadType() )
		{
		case DataType::MapEnum :
			cout << "filter id = " << (UInt32)entry.getFilterId() << "\n";
			cout << "action    = " << entry.getFilterActionAsString() << "\n";
			decode( entry.getMap() );
			break;

			...
		}
	}

	\endcode

	\remark These two methods apply to containers only: ElementList,
			FieldList, FilterList, Map, Series, and Vector.
	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark This class is designed to efficiently perform setting and extracting of FilterList and its content.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data,
		FilterEntry,
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
#include "Access/Include/FilterEntry.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class FilterListDecoder;
class FilterListEncoder;

class EMA_ACCESS_API FilterList : public ComplexType
{
public :

	///@name Constructor
	//@{
	/** Constructs FilterList.
	*/
	FilterList();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~FilterList();
	//@}

	///@name Accessors
	//@{
	/** Returns the DataType, which is the type of Omm data. Results in this class type.
		@return DataType::FilterListEnum
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

	/** Indicates presence of TotalCountHint.
		@return true if total count hint is set; false otherwise
	*/
	bool hasTotalCountHint() const;

	/** Returns TotalCountHint.
		@throw OmmInvalidUsageException if hasTotalCountHint() returns false
		@return total count hint
	*/
	UInt32 getTotalCountHint() const;

	/** Iterates through a list of Data of any DataType.
		Typical usage is to extract the entry during each iteration via getEntry().
		@return false at the end of FilterList; true otherwise
	*/
	bool forth() const;

	/** Iterates through a list of Data having matched actual filterId with the one passed in.
		Typical usage is to extract the entry during each iteration via getEntry().
		@param[in] filterId looked up filter id
		@return false at the end of FilterList; true otherwise
	*/
	bool forth( UInt8 filterId ) const;

	/** Returns Entry.
		@throw OmmInvalidUsageException if forth() was not called first
		@return FilterEntry
	*/
	const FilterEntry& getEntry() const;

	/** Resets iteration to start of container.
	*/
	void reset() const;
	//@}

	///@name Operations
	//@{
	/** Clears the FilterList.
		\remark Invoking clear() method clears all the values and resets all the defaults
		@return reference to this object
	*/
	FilterList& clear();

	/** Specifies TotalCountHint.
		@throw OmmInvalidUsageException if this method is called after adding an entry to FilterList.
		@param[in] totalCountHint specifies estimated total number of entries
		@return reference to this object
	*/
	FilterList& totalCountHint( UInt32 totalCountHint );

	/** Adds a complex type of OMM data to the FilterList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] filterId specifies filter id for the added FilterEntry
		@param[in] action specifies action for the added FilterEntry
		@param[in] value specifies load for the added FilterEntry
		@param[in] permissionData specifies permissions for the added FilterEntry
		@return reference to this object
	*/
	FilterList& add( UInt8 filterId, FilterEntry::FilterAction action,
					const ComplexType& value, const EmaBuffer& permissionData = EmaBuffer() );

	/** Adds no payload to the FilterList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@param[in] filterId specifies filter id for the added FilterEntry
		@param[in] action specifies action for the added FilterEntry
		@param[in] permissionData specifies permissions for the added FilterEntry
		@return reference to this object
	*/
	FilterList& add( UInt8 filterId, FilterEntry::FilterAction action, 
		const EmaBuffer& permissionData = EmaBuffer() );

	/** Completes encoding of FilterList.
		@throw OmmInvalidUsageException if an error is detected (exception will specify the cause of the error)
		@return const reference to this object
	*/
	const FilterList& complete();
	//@}

private :

	Decoder& getDecoder();
	bool hasDecoder() const;

	const EmaString& toString( UInt64 indent ) const;

	const Encoder& getEncoder() const;
	bool hasEncoder() const;

	mutable EmaString			_toString;
	FilterEntry					_entry;
	FilterListDecoder*			_pDecoder;
	mutable FilterListEncoder*	_pEncoder;

	FilterList( const FilterList& );
	FilterList& operator=( const FilterList& );
};

}

}

}

#endif // __thomsonreuters_ema_access_FilterList_h
