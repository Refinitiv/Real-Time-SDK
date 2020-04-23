///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.rdm;

import java.util.List;

import com.thomsonreuters.ema.access.OmmInvalidUsageException;
import com.thomsonreuters.ema.access.Series;

/**
 * This class houses all known fields loaded from an RDM field dictionary and
 * their corresponding enum types loaded from an enum type dictionary.
 * The dictionary also saves general information about the dictionary itself
 * This is found in the "!tag" comments of the file or in the summary data of
 * dictionaries encoded via the official domain model.
 * The data dictionary must be loaded prior to using the methods to access dictionary entries.
 * 
 * @see DictionaryEntry
 * @see EnumType
 */
public interface DataDictionary
{
    /**
    * Clears DataDictionary. 
    *
    * This method is used to clear the existing dictionary information.
    */
    void clear();
    
    /**
     * The lowest fieldId present in the dictionary.
     * 
     * @return the minFid
     */
    public int minFid();

    /**
     * The highest fieldId present in the dictionary.
     * 
     * @return the maxFid
     */
    public int maxFid();
    
    /**
     * The list of DictionaryEntry of this DataDictionary.
     * 
     * @return the list of DictionaryEntry
     */
    public List<DictionaryEntry> entries();
    
    /**
	* The list of EnumTypeTable of this DataDictionary.
	*
	* @return the list of EnumTypeTable
	*/
    public List<EnumTypeTable> enumTables();

    /**
     * DictionaryId Tag. All dictionaries loaded using this object will have this tag matched if found.
     * 
     * @return the dictionaryId
     */
    public int dictionaryId();

    /**
     * Field Version Tag.
     * 
     * @return the fieldVersion
     */
    public String fieldVersion();

    /**
     * Enum RT_Version Tag.
     * 
     * @return the enumRecordTemplateVersion
     */
    public String enumRecordTemplateVersion();

    /**
     * Enum DT_Version Tag.
     * 
     * @return the enumDisplayTemplateVersion
     */
    public String enumDisplayTemplateVersion();

    /**
     * Field Filename Tag.
     * 
     * @return the fieldFilename
     */
    public String fieldFilename();

    /**
     * Field Description Tag.
     * 
     * @return the fieldDescription
     */
    public String fieldDescription();

    /**
     * Field Build Tag.
     * 
     * @return the fieldBuild
     */
    public String fieldBuild();

    /**
     * Field Date Tag.
     * 
     * @return the fieldDate
     */
    public String fieldDate();

    /**
     * Enum Filename Tag.
     * 
     * @return the enumFilename
     */
    public String enumFilename();

    /**
     * Enum Description Tag.
     * 
     * @return the enumDescription
     */
    public String enumDescription();

    /**
     * Enum Date Tag.
     * 
     * @return the enumDate
     */
    public String enumDate();
    
    /**
     * Adds information from a field dictionary file to the data dictionary
     * object. Subsequent calls to this method may be made to the same
     * {@link DataDictionary} to load additional dictionaries (provided the
     * fields do not conflict).
     * 
     * @param filename specifies a field dictionary file
     * 
     * @throws OmmInvalidUsageException if fails to load from the specified
     * file name from <code>filename</code>.
     * 
     */
    public void loadFieldDictionary(String filename);

    /**
     * Adds information from an enumerated types dictionary file to the data
     * dictionary object. Subsequent calls to this method may be made to the
     * same {@link DataDictionary} to load additional dictionaries (provided
     * that there are no duplicate table references for any field).
     * 
     * @param filename specifies an enumerated types dictionary file
     * 
     * @throws OmmInvalidUsageException if fails to load from the specified
     * file name from <code>filename</code>.
     * 
     */
    public void loadEnumTypeDictionary(String filename);
    
    /**
     * Encode the field dictionary information into a data payload
     * according the domain model, using the field information from the entries
     * present in this dictionary. This method supports building the encoded
     * data in one full part.
     * 
     * @param series Series to be used for encoding dictionary information into.
     * @param verbosity The desired verbosity to encode.
     * 
     * @throws OmmInvalidUsageException if fails to encode field dictionary
     * information.
     * 
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public void encodeFieldDictionary(Series series, long verbosity);
    
    /**
     * Encode the field dictionary information into a data payload
     * according the domain model, using the field information from the entries
     * present in this dictionary. This method supports building the encoded
     * data in multiple parts according to the fragmentation size.
     * 
     * @param series Series to be used for encoding dictionary information into.
     * @param currentFid tracks which fields have been encoded in case of multi-part encoding.
     * @param verbosity The desired verbosity to encode.
     * @param fragmentationSize The fragmentation size in number of bytes.
     * 
     * @return Field ID that have been encoded or equal to  {@link #maxFid()} to indicate final part
	 * or single complete payload.
     * 
     * @throws OmmInvalidUsageException if fails to encode field dictionary
     * information.
     *         
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public int encodeFieldDictionary(Series series, int currentFid, long verbosity, int fragmentationSize);

    /**
     * Decode the field dictionary information contained in a data payload
     * according to the domain model. This method may be called multiple times
     * on the same dictionary, to load information from dictionaries that have
     * been encoded in multiple parts.
     * 
     * @param series Series to be used for decoding dictionary information from.
     * @param verbosity The desired verbosity to decode. 
     *        
     * @throws OmmInvalidUsageException if fails to decode field dictionary
     * information.
     * 
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public void decodeFieldDictionary(Series series, long verbosity);

    /**
     * Encode the enumerated types dictionary according the domain model, using
     * the information from the tables and referencing fields present in this
     * dictionary. Note: This method will use the type Ascii for the DISPLAY array.
     * 
     * @param series Series to be used for encoding enumerated types dictionary into.
     * @param verbosity The desired verbosity to encode.
     * 
     * @throws OmmInvalidUsageException if fails to encode enumerated types dictionary.
     * 
     * @see Series
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public void encodeEnumTypeDictionary(Series series, long verbosity);
    
    /**
     * Encode the enumerated types dictionary according the domain model, using
     * the information from the tables and referencing fields present in this
     * dictionary. This method supports building the encoded data in multiple parts according
     * to the fragmentation size.
     * Note: This method will use the type Ascii for the DISPLAY array.
     * 
     * @param series Series to be used for encoding enumerated types dictionary into.
     * @param currentEnumTableEntry Tracks the index of enumerated type table entry have been encoded.
     *  		Must be initialized to 0 on the first call and is updated with each successfully encoded part.
     * @param verbosity The desired verbosity to encode.
     * @param fragmentationSize The fragmentation size in number of bytes.
     * 
     * @return the index of enumerated type table entry that have been encoded or equal to the size
     * of {@link #enumTables()} to indicate final part or single complete payload.
     * 
     * @throws OmmInvalidUsageException if fails to encode enumerated types dictionary.
     * 
     * @see Series
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public int encodeEnumTypeDictionary(Series series, int  currentEnumTableEntry, long verbosity, int fragmentationSize);
    
    /**
     * Decode the enumerated types information contained in an encoded enum
     * types dictionary according to the domain model. This method may be called
     * multiple times on the same dictionary, to load information from
     * dictionaries that have been encoded in multiple parts.
     * 
     * @param series Series to be used for decoding enumerated types information from.
     * @param verbosity The desired verbosity to decode.
     * 
     * @throws OmmInvalidUsageException if fails to decode enumerated types dictionary.
     * 
     * @see Series
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public void decodeEnumTypeDictionary(Series series, long verbosity);
    
    /**
     * Extract dictionary type from the encoded payload of a EMA message where
     * the domain type is DICTIONARY.
     * 
     * @param series Series to be used for extracting dictionary type.
     * 
     * @return The dictionary type defined in EmaRdm.
     * 
     * @throws OmmInvalidUsageException If dictionary type is not available.
     * 
     * @see Series
     * @see com.thomsonreuters.ema.rdm.EmaRdm
     */
    public int extractDictionaryType(Series series);
    
    /**
     * Check whether the DictionaryEntry exists 
     * 
     * @param fieldName the field name to check the dictionary entry
     * 
     * @return true if the DictionaryEntry exists otherwise false
     */
    public boolean hasEntry(String fieldName);
    
    /**
     * Returns the entry in the dictionary corresponding to the given field name, if the entry exists.
     * 
     * @param fieldName the field name to get the dictionary entry for
     * 
     * @throws OmmInvalidUsageException if the entry does not exist.
     * 
     * @return the dictionary entry if it exists
     */
    public DictionaryEntry entry(String fieldName);

    /**
     * Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
     *
     * @param fieldName the field name to get the dictionary entry for
     * @param entryDst out parameter, dictionary entry.
     *
     * @throws OmmInvalidUsageException if the entry does not exist or entryDst is null or is owned by API.
     *
     * Same like {@link DataDictionary#entry(String)} note, entryDst should be created first and managed by the user
     * Otherwise Dictionary is owned by API and method will throw {@link OmmInvalidUsageException }
     * with {@link OmmInvalidUsageException.ErrorCode.INVALID_USAGE} error code
     */
    void entry(String fieldName, DictionaryEntry entryDst);

    /**
     * Check whether the DictionaryEntry exists 
     * 
     * @param fieldId the fieldId to check the dictionary entry
     * 
     * @return true if the DictionaryEntry exists otherwise false
     */
    public boolean hasEntry(int fieldId);
    
    /**
     * Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
     * 
     * @param fieldId the fieldId to get the dictionary entry for
     * 
     * @throws OmmInvalidUsageException if the entry does not exist.
     * 
     * @return the dictionary entry if it exists
     */
    public DictionaryEntry entry(int fieldId);

    /**
     * Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
     *
     * @param fieldId the fieldId to get the dictionary entry for
     * @param entryDst out parameter, dictionary entry.
     *
     * @throws OmmInvalidUsageException if the entry does not exist or entryDst is null or is owned by API.
     *
     * Same like {@link DataDictionary#entry(int)} note, entryDst should be created first and managed by the user.
     * Otherwise Dictionary is owned by API and method will throw {@link OmmInvalidUsageException }
     * with {@link OmmInvalidUsageException.ErrorCode.INVALID_USAGE} error code
     */
    void entry(int fieldId, DictionaryEntry entryDst);

    /**
     * Check whether the EnumEntry exists
     * 
     * @param fieldId the fieldId to check the enumerated type
     * @param value the value of the enumerated type to check
     * 
     * @return the enumerated type if it exists
     */
    public boolean hasEnumType(int fieldId, int value);
    
    /**
     * Returns the corresponding enumerated type in the dictionary entry's
     * table, if the type exists.
     * 
     * @param fieldId the fieldId to get the enumerated type from
     * @param value the value of the enumerated type to get
     * 
     * @throws OmmInvalidUsageException if the entry does not exist.
     * 
     * @return the enumerated type if it exists
     */
    public EnumType enumType(int fieldId, int value);
    
    /**
     * Convert information contained in the data dictionary to a string.
     * 
     * @return the string representation of this {@link DataDictionary}
     */
    public String toString();
}
