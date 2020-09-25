/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_rdm_DictionaryEntry_h
#define __refinitiv_ema_rdm_DictionaryEntry_h

/**
* A data dictionary entry, containing field information and an enumeration table reference if present.
*
* @see MfFieldType
* @see RwfType
*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/EmaVector.h"

namespace rtsdk {

namespace ema {

namespace rdm {

class EnumType;
class EnumTypeTable;
class DictionaryEntryImpl;
class DataDictionaryImpl;

class EMA_ACCESS_API DictionaryEntry
{
public:

	///@name Accessors
	//@{
	/**
	* Acronym.
	*
	* @return the acronym
	*/
	const rtsdk::ema::access::EmaString& getAcronym() const;

	/**
	* DDE Acronym.
	*
	* @return the ddeAcronym
	*/
	const rtsdk::ema::access::EmaString& getDDEAcronym() const;

	/**
	* The fieldId the entry corresponds to.
	*
	* @return the fid
	*/
	rtsdk::ema::access::Int16 getFid() const;

	/**
	* The field to ripple data to.
	*
	* @return the rippleToField
	*/
	rtsdk::ema::access::Int16 getRippleToField() const;

	/**
	* Marketfeed Field Type.
	*
	* @return the fieldType
	*/
	rtsdk::ema::access::Int8 getFieldType() const;

	/**
	* Marketfeed length.
	*
	* @return the length
	*/
	rtsdk::ema::access::UInt16 getLength() const;

	/**
	* RWF type.
	*
	* @return data types defined in DataType::DataTypeEnum
	*/
	rtsdk::ema::access::UInt8 getRwfType() const;

	/**
	* Marketfeed enum length.
	*
	* @return the enumLength
	*/
	rtsdk::ema::access::UInt8 getEnumLength() const;

	/**
	* RWF Length.
	*
	* @return the rwfLength
	*/
	rtsdk::ema::access::UInt32 getRwfLength() const;
	//@}

	///@name Operations
	/**
	* Check whether the EnumType exists
	*
	* @param[in] value the value of the enumerated type to check
	*
	* @return the enumerated type if it exists
	*/
	bool hasEnumType(rtsdk::ema::access::UInt16 value) const;

	/**
	* Returns the corresponding enumerated type in the dictionary entry's
	* table, if the type exists.
	*
	* @param value the value of the enumerated type to get
	* @throw OmmInvalidUsageException if the entry does not exist.
	* @return the enumerated type if it exists
	*/
	const EnumType& getEnumType(rtsdk::ema::access::UInt16 value) const;

	/**
	* Check whether the EnumTypeTable exists
	*
	* @return true if EnumTypeTable exists, otherwise false
	*/
	bool hasEnumTypeTable() const;

	/**
	* Returns the list of EnumTypeTable that is used by this DictionaryEntry,
	* if the type exists.
	* @throw OmmInvalidUsageException if the entry does not exist.
	* @return the array of EnumTypeTable if it exists
	*/
	const EnumTypeTable& getEnumTypeTable() const;

	/** Returns a string representation of the class instance.
	@throw OmmMemoryExhaustionException if app runs out of memory
	@return string representation of the class instance
	*/
	const rtsdk::ema::access::EmaString& toString() const;

	/** Operator const char* overload.
	@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

	///@name Constructor
	//@{
	/** default constructor.
	*/
	DictionaryEntry();
	//@}

	///@name Destructor
	//@{
	/** destructor.
	*
	*/
	virtual ~DictionaryEntry();
	//@}

private:

	friend class DictionaryEntryImpl;
	friend class DataDictionaryImpl;

	template<class T>
	friend class rtsdk::ema::access::EmaVector;

	// isManagedByUser true when the instance is created by user; false when the instance is created by API
	DictionaryEntry(bool isManagedByUser);

	DictionaryEntry(const DictionaryEntry&);

	DictionaryEntry& operator=(const DictionaryEntry&);

	DictionaryEntryImpl*     _pImpl;
};

}

}

}

#endif //  __refinitiv_ema_rdm_DictionaryEntry_h

