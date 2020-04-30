/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|----------------------------------------------------------------------------- 
 */

#ifndef __thomsonreuters_ema_rdm_DataDictionary_h
#define __thomsonreuters_ema_rdm_DataDictionary_h

/**
	class thomsonreuters::ema::rdm::DataDictionary DataDictionary.h "Rdm/Include/DataDictionary.h"
	@brief This class houses all known fields loaded from an RDM field dictionary and
	their corresponding enum types loaded from an enum type dictionary.
    
	The dictionary also saves general information about the dictionary itself
    This is found in the "!tag" comments of the file or in the summary data of
    dictionaries encoded via the official domain model.
    
	The data dictionary must be loaded prior to using the methods to access dictionary entries.

	\remark All methods in this class are \ref SingleThreaded.

	@see Series,
	     EmaString,
		 DictionaryEntry,
		 EnumType

*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaVector.h"
#include "Access/Include/Series.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class FieldListDecoder;

}

namespace rdm {

class DataDictionaryImpl;
class DictionaryEntry;
class EnumType;
class EnumTypeTable;

class EMA_ACCESS_API DataDictionary
{

public:

	///@name Constructor
	//@{
	/** Constructs DataDictionary.
	*/
	DataDictionary();

	/** Copy Constructor.
	*/
	DataDictionary(const DataDictionary&);
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~DataDictionary();
	//@}

	///@name Accessors
	//@{
	/**
	* The lowest fieldId present in the dictionary.
	*
	* @return the minFid
	*/
	thomsonreuters::ema::access::Int32 getMinFid() const;

	/**
	* The highest fieldId present in the dictionary.
	*
	* @return the maxFid
	*/
	thomsonreuters::ema::access::Int32 getMaxFid() const;

	/**
	* The list of DictionaryEntry of this DataDictionary.
	*
	* @return the list of DictionaryEntry
	*/
	const thomsonreuters::ema::access::EmaVector<DictionaryEntry>& getEntries() const;

	/**
	* The list of EnumTypeTable of this DataDictionary.
	*
	* @return the list of EnumTypeTable
	*/
	const thomsonreuters::ema::access::EmaVector<EnumTypeTable>& getEnumTables() const;

	/**
	* DictionaryId Tag. All dictionaries loaded using this object will have this tag matched if found.
	*
	* @return the DictionaryId
	*/
	thomsonreuters::ema::access::Int32 getDictionaryId() const;

	/**
	* Field Version Tag.
	*
	* @return the FieldVersion
	*/
	const thomsonreuters::ema::access::EmaString& getFieldVersion() const;

	/**
	* Enum RT_Version Tag.
	*
	* @return the EnumRecordTemplateVersion
	*/
	const thomsonreuters::ema::access::EmaString& getEnumRecordTemplateVersion() const;

	/**
	* Enum DT_Version Tag.
	*
	* @return the EnumDisplayTemplateVersion
	*/
	const thomsonreuters::ema::access::EmaString& getEnumDisplayTemplateVersion() const;

	/**
	* Field Filename Tag.
	*
	* @return the FieldFilename
	*/
	const thomsonreuters::ema::access::EmaString& getFieldFilename() const;

	/**
	* Field Description Tag.
	*
	* @return the FieldDescription
	*/
	const thomsonreuters::ema::access::EmaString& getFieldDescription() const;

	/**
	* Field Build Tag.
	*
	* @return the FieldBuild
	*/
	const thomsonreuters::ema::access::EmaString& getFieldBuild() const;

	/**
	* Field Date Tag.
	*
	* @return the FieldDate
	*/
	const thomsonreuters::ema::access::EmaString& getFieldDate() const;

	/**
	* Enum Filename Tag.
	*
	* @return the EnumFilename
	*/
	const thomsonreuters::ema::access::EmaString& getEnumFilename() const;

	/**
	* Enum Description Tag.
	*
	* @return the EnumDescription
	*/
	const thomsonreuters::ema::access::EmaString& getEnumDescription() const;

	/**
	* Enum Date Tag.
	*
	* @return the EnumDate
	*/
	const thomsonreuters::ema::access::EmaString& getEnumDate() const;

	/**
	* Check whether the DictionaryEntry exists
	*
	* @param[in] fieldId the fieldId to check the dictionary entry
	*
	* @return true if the DictionaryEntry exists otherwise false
	*/
	bool hasEntry(thomsonreuters::ema::access::Int16 fieldId) const;

	/**
	* Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
	*
	* @param[in] fieldId specifies the fieldId to get the dictionary entry for
	* @entry[out] the entry in the dictionary corresponding to the given fieldId, if the entry exists
	*
	* @throw OmmInvalidUsageException if the entry does not exist
	* Same like {@link DataDictionary#getEntry(thomsonreuters::ema::access::Int16)} note, entry should be created first and managed by the user
	*/
	void getEntry(thomsonreuters::ema::access::Int16 fieldId, DictionaryEntry& entry) const;

	/**
	* Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
	*
	* @param[in] fieldId specifies the fieldId to get the dictionary entry for
	*
	* @throw OmmInvalidUsageException if the entry does not exist
	*
	* @return the dictionary entry if it exists
	*/
	const DictionaryEntry& getEntry(thomsonreuters::ema::access::Int16 fieldId) const;

	/**
	* Check whether the DictionaryEntry exists
	*
	* @param[in] fieldName the field name to check the dictionary entry
	*
	* @return true if the DictionaryEntry exists otherwise false
	*/
	bool hasEntry(const thomsonreuters::ema::access::EmaString& fieldName) const;

	/**
	* Returns the entry in the dictionary corresponding to the given fieldName, if the entry exists.
	*
	* @param[in] fieldName specifies the fieldId to get the dictionary entry for
	* @entry[out] the entry in the dictionary corresponding to the given fieldName, if the entry exists
	*
	* @throw OmmInvalidUsageException if the entry does not exist
	* Same like {@link DataDictionary#getEntry(const thomsonreuters::ema::access::EmaString&)} note, entry should be created first and managed by the user
	*/
	void getEntry(const thomsonreuters::ema::access::EmaString& fieldName, DictionaryEntry& entry) const;

	/**
	* Returns the entry in the dictionary corresponding to the given fieldName, if the entry exists.
	*
	* @param[in] fieldName specifies the fieldId to get the dictionary entry for
	*
	* @throw OmmInvalidUsageException if the entry does not exist
	*
	* @return the dictionary entry if it exists
	*/
	const DictionaryEntry& getEntry(const thomsonreuters::ema::access::EmaString& fieldName) const;

	/**
	* Check whether the EnumType exists
	*
	* @param[in] fieldId the fieldId to check the enumerated type
	* @param[in] value the value of the enumerated type to check
	*
	* @return the enumerated type if it exists
	*/
	bool hasEnumType(thomsonreuters::ema::access::Int16 fieldId, thomsonreuters::ema::access::UInt16 value) const;

	/**
	* Returns the corresponding enumerated type in the dictionary entry's
	* table, if the type exists.
	*
	* @param[in] fieldId specifies the fieldId to get the enumerated type from
	* @param[in] value specifies the value of the enumerated type to get
	*
	* @throw OmmInvalidUsageException if the entry does not exist
	*
	* @return the enumerated type if it exists
	*/
	const EnumType& getEnumType(thomsonreuters::ema::access::Int16 fieldId, thomsonreuters::ema::access::UInt16 value) const;
	//@}

	///@name Operations 
	//@{
	/**
	* Clears DataDictionary. 
	*
	* This method is used to clear the existing dictionary information.
	*/
	void clear();

	/**
	* Adds information from a field dictionary file to the data dictionary
	* object. Subsequent calls to this method may be made to the same
	* DataDictionary to load additional dictionaries (provided the
	* fields do not conflict).
	*
	* @param[in] filename specifies a field dictionary file
	*
	* @throw OmmInvalidUsageException if fails to load from the specified
	* file name from \p filename.
	*
	*/
	void loadFieldDictionary(const thomsonreuters::ema::access::EmaString& filename);

	/**
	* Adds information from an enumerated types dictionary file to the data
	* dictionary object. Subsequent calls to this method may be made to the
	* same DataDictionary to load additional dictionaries (provided
	* that there are no duplicate table references for any field).
	*
	* @param[in] filename specifies an enumerated types dictionary file
	*
	* @throw OmmInvalidUsageException if fails to load from the specified
	* file name from \p filename.
	*
	*/
	void loadEnumTypeDictionary(const thomsonreuters::ema::access::EmaString& filename);

	/**
	* Encode the field dictionary information into a data payload
	* according the domain model, using the field information from the entries
	* present in this dictionary. This method supports building the encoded
	* data in one full part.
	*
	* @param[in] series specifies Series to be used for encoding dictionary information into.
	* @param[in] verbosity specifies the desired verbosity to encode.
	*
	* @throw OmmInvalidUsageException if fails to encode field dictionary
	* information.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	void encodeFieldDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	/**
	* Encode the field dictionary information into a data payload
	* according the domain model, using the field information from the entries
	* present in this dictionary. This method supports building the encoded
	* data in multiple parts according to the fragmentation size.
	*
	* @param[in] series specifies Series to be used for encoding dictionary information into.
	* @param[in] currentFid tracks which fields have been encoded in case of multi-part encoding.
	* Must be initialized to getMinFid() on the first call and is updated with each successfully encoded part
	* @param[in] verbosity specifies the desired verbosity to encode.
	* @param[in] fragmentationSize specifies the fragmentation size in number of bytes.
	*
	* @return true to indicate final part or single complete payload otherwise false
	*
	* @throw OmmInvalidUsageException if fails to encode field dictionary
	* information.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	bool encodeFieldDictionary(thomsonreuters::ema::access::Series& series,
		thomsonreuters::ema::access::Int32& currentFid, thomsonreuters::ema::access::UInt32 verbosity, 
		thomsonreuters::ema::access::UInt32 fragmentationSize);

	/**
	* Decode the field dictionary information contained in a data payload
	* according to the domain model. This method may be called multiple times
	* on the same dictionary, to load information from dictionaries that have
	* been encoded in multiple parts.
	*
	* @param[in] series specifies Series to be used for decoding dictionary information from.
	* @param[in] verbosity specifies the desired verbosity to decode.
	*
	* @throw OmmInvalidUsageException if fails to decode field dictionary
	* information.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	void decodeFieldDictionary(const thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	/**
	* Encode the enumerated types dictionary according the domain model, using
	* the information from the tables and referencing fields present in this
	* dictionary. Note: This method will use the type Ascii for the DISPLAY array.
	*
	* @param[in] series specifies Series to be used for encoding enumerated types dictionary into.
	* @param[in] verbosity specifies the desired verbosity to encode.
	*
	* @throw OmmInvalidUsageException if fails to encode enumerated types dictionary.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	void encodeEnumTypeDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	/**
	* Encode the enumerated types dictionary according the domain model, using
	* the information from the tables and referencing fields present in this
	* dictionary. Note: This method will use the type Ascii for the DISPLAY array.
	* This method supports building the encoded data in multiple parts according to
	* the fragmentation size.
	*
	* @param[in] series specifies Series to be used for encoding enumerated types dictionary into.
	* @param[in] enumTableCount tracks how many Enum table have been encoded in case of multi-part encoding.
	* Must be initialized to 0 on the first call and is updated with each successfully encoded part
	* @param[in] verbosity specifies the desired verbosity to encode.
	* @param[in] fragmentationSize specifies the fragmentation size in number of bytes.
	*
	* @return true to indicate final part or single complete payload otherwise false
	*
	* @throw OmmInvalidUsageException if fails to encode enumerated types dictionary.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	bool encodeEnumTypeDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::Int32& enumTableCount,
		thomsonreuters::ema::access::UInt32 verbosity, thomsonreuters::ema::access::UInt32 fragmentationSize);

	/**
	* Decode the enumerated types information contained in an encoded enum
	* types dictionary according to the domain model. This method may be called
	* multiple times on the same dictionary, to load information from
	* dictionaries that have been encoded in multiple parts.
	*
	* @param[in] series specifies Series to be used for decoding enumerated types information from.
	* @param[in] verbosity specifies the desired verbosity to decode.
	*
	* @throw OmmInvalidUsageException if fails to decode enumerated types dictionary.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	void decodeEnumTypeDictionary(const thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	/**
	* Extract dictionary type from the encoded payload of a EMA message where
	* the domain type is DICTIONARY.
	*
	* @param[in] series Series to be used for extracting dictionary type.
	*
	* @return The dictionary type defined in EmaRdm.
	*
	* @throw OmmInvalidUsageException If dictionary type is not available.
	*
	* \remark see verbosity definition at "Rdm/Include/EmaRdm.h"
	*/
	thomsonreuters::ema::access::UInt32 extractDictionaryType(const thomsonreuters::ema::access::Series& series);

	/** Returns a string representation of the class instance.
	@throw OmmMemoryExhaustionException if app runs out of memory
	@return string representation of the class instance
	*/
	const thomsonreuters::ema::access::EmaString& toString() const;

	/** Operator const char* overload.
	@throw OmmMemoryExhaustionException if app runs out of memory
	*/
	operator const char* () const;
	//@}

private:

	friend class thomsonreuters::ema::access::FieldListDecoder;

	DataDictionary(bool);

	DataDictionary& operator=(const DataDictionary&);

	DataDictionaryImpl*        _pImpl;
};

}

}

}

#endif // __thomsonreuters_ema_rdm_DataDictionary_h



