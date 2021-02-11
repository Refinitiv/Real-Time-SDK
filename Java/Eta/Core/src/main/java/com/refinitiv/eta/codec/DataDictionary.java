package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.transport.Error;

/**
 * A class that houses all known fields loaded from an RDM field dictionary and
 * their corresponding enum types loaded from an enum type dictionary.
 * The dictionary also saves general information about the dictionary itself
 * This is found in the "!tag" comments of the file or in the summary data of
 * dictionaries encoded via the official domain model.
 * The data dictionary must be loaded prior to using the methods to access dictionary entries.
 *
 * @see DictionaryEntry
 * @see EnumType
 * @see MfFieldTypes
 */
public interface DataDictionary
{
    /**
     * Clears {@link DataDictionary}. This should be done prior to the first
     * call of a dictionary loading method, if the initializer is not used.
     */
    public void clear();

    /**
     * Returns the entry in the dictionary corresponding to the given fieldId, if the entry exists.
     *
     * @param fieldId the fieldId to get the dictionary entry for
     *
     * @return the dictionary entry if it exists, NULL otherwise
     */
    public DictionaryEntry entry(int fieldId);

    /**
     * Returns the corresponding enumerated type in the dictionary entry's
     * table, if the type exists.
     *
     * @param entry the dictionary entry to get the enumerated type from
     * @param value the value of the enumerated type to get
     *
     * @return the enumerated type if it exists, NULL otherwise
     */
    public EnumType entryEnumType(DictionaryEntry entry, Enum value);

    /**
     * Adds information from a field dictionary file to the data dictionary
     * object. Subsequent calls to this method may be made to the same
     * {@link DataDictionary} to load additional dictionaries (provided the
     * fields do not conflict).
     *
     * @param filename the filename
     * @param error the error
     * @return the int
     */
    public int loadFieldDictionary(String filename, Error error);

    /**
     * Adds information from an enumerated types dictionary file to the data
     * dictionary object. Subsequent calls to this method may be made to the
     * same {@link DataDictionary} to load additional dictionaries (provided
     * that there are no duplicate table references for any field).
     *
     * @param filename the filename
     * @param error the error
     * @return the int
     */
    public int loadEnumTypeDictionary(String filename, Error error);

    /**
     * Extract dictionary type from the encoded payload of a ETA message where
     * the domain type is DICTIONARY.
     *
     * Typical use:<BR>
     * 1. Call Msg.decode().<BR>
     * 2. If domainType is DICTIONARY, call this method.<BR>
     * 3. Call appropriate dictionary decode method based on returned
     * dictionary type (e.g., if returned type is FIELD_DEFINITIONS, call
     * decodeFieldDictionary()).<BR>
     *
     * @param iter An iterator to use. Must be set to the encoded payload of the
     *            dictionary message.
     * @param dictionaryType The dictionary type, from DictionaryTypes.
     * @param error ETA Error, to be populated in event of an error.
     *
     * @return {@link CodecReturnCodes}. If success, dictionary type is
     *         populated. If failure, dictionary type not available.
     *
     * @see DecodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.Types
     */
    public int extractDictionaryType(DecodeIterator iter, Int dictionaryType, Error error);

    /**
     * Encode the field definitions dictionary information into a data payload
     * according the domain model, using the field information from the entries
     * present in this dictionary. This method supports building the encoded
     * data in multiple parts -- if there is not enough available buffer space
     * to encode the entire dictionary, subsequent calls can be made to this
     * method, each producing the next segment of fields.
     *
     * @param iter Iterator to be used for encoding. Prior to each call, the
     *            iterator must be cleared and initialized to the buffer to be used for encoding.
     *
     * @param currentFid Tracks which fields have been encoded in case of
     *            multi-part encoding. Must be initialized to {@literal dictionary->minFid}
     *            on the first call and is updated with each successfully encoded part.
     *
     * @param verbosity The desired verbosity to encode.
     * @param error ETA error, to be populated in event of an error.
     *
     * @return {@link CodecReturnCodes},
     *         {@link CodecReturnCodes#DICT_PART_ENCODED} when encoding parts is
     *         success {@link CodecReturnCodes#SUCCESS} for final part or single
     *         complete payload.
     *
     * @see EncodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.VerbosityValues
     */
    public int encodeFieldDictionary(EncodeIterator iter, Int currentFid, int verbosity, Error error);

    /**
     * Decode the field dictionary information contained in a data payload
     * according to the domain model. This method may be called multiple times
     * on the same dictionary, to load information from dictionaries that have
     * been encoded in multiple parts.
     *
     * @param iter An iterator to use. Must be set to the encoded buffer.
     * @param verbosity The desired verbosity to decode. See
     *            {@link com.refinitiv.eta.rdm.Dictionary.VerbosityValues}.
     * @param error ETA error, to be populated in event of an error.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see DecodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.VerbosityValues
     */
    public int decodeFieldDictionary(DecodeIterator iter, int verbosity, Error error);

    /**
     * Encode the enumerated types dictionary according the domain model, using
     * the information from the tables and referencing fields present in this
     * dictionary. Note: This method will use the type ASCII_STRING for the DISPLAY array.
     *
     * @param iter Iterator to be used for encoding.
     * @param verbosity The desired verbosity to encode.
     * @param error ETA Error, to be populated in event of an error.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.VerbosityValues
     */
    public int encodeEnumTypeDictionary(EncodeIterator iter, int verbosity, Error error);

    /**
     * Encode the enumerated types dictionary according the domain model, using
     * the information from the tables and referencing fields present in this
     * dictionary. This method supports building the encoded
     * data in multiple parts -- if there is not enough available buffer space
     * to encode the entire dictionary, subsequent calls can be made to this
     * method, each producing the next segment of fields.
     * Note: This method will use the type ASCII_STRING for the DISPLAY array.
     *
     * @param iter Iterator to be used for encoding.
     * @param currentEnumTableEntry Tracks which fields have been encoded. Must be initialized to 0
     *            on the first call and is updated with each successfully encoded part. 
     * @param verbosity The desired verbosity to encode.
     * @param error ETA Error, to be populated in event of an error.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.VerbosityValues
     */
    public int encodeEnumTypeDictionaryAsMultiPart(EncodeIterator iter, Int currentEnumTableEntry, int verbosity, Error error);

    /**
     * Decode the enumerated types information contained in an encoded enum
     * types dictionary according to the domain model.
     *
     * @param iter An iterator to use. Must be set to the encoded buffer.
     * @param verbosity The desired verbosity to decode.
     * @param error ETA Error, to be populated in event of an error.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see DecodeIterator
     * @see com.refinitiv.eta.rdm.Dictionary.VerbosityValues
     */
    public int decodeEnumTypeDictionary(DecodeIterator iter, int verbosity, Error error);

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
     * The total number of entries in the dictionary.
     *
     * @return the numberOfEntries
     */
    public int numberOfEntries();

    /**
     * The tables present in this dictionary. The entries in entriesArray hold
     * pointers to their respective tables in this list.
     *
     * @return the enumTables
     */
    public EnumTypeTable[] enumTables();

    /**
     * Total number of enumTables present.
     *
     * @return the enumTableCount
     */
    public int enumTableCount();

    /**
     * DictionaryId Tag. All dictionaries loaded using this object will have this tag matched if found.
     *
     * @return the infoDictionaryId
     */
    public int infoDictionaryId();

    /**
     * Field Version Tag.
     *
     * @return the infoFieldVersion
     */
    public Buffer infoFieldVersion();

    /**
     * Enum RT_Version Tag.
     *
     * @return the infoEnumRTVersion
     */
    public Buffer infoEnumRTVersion();

    /**
     * Enum DT_Version Tag.
     *
     * @return the infoEnumDTVersion
     */
    public Buffer infoEnumDTVersion();

    /**
     * Field Filename Tag.
     *
     * @return the infoFieldFilename
     */
    public Buffer infoFieldFilename();

    /**
     * Field Description Tag.
     *
     * @return the infoFieldDesc
     */
    public Buffer infoFieldDesc();

    /**
     * Field Build Tag.
     *
     * @return the infoFieldBuild
     */
    public Buffer infoFieldBuild();

    /**
     * Field Date Tag.
     *
     * @return the infoFieldDate
     */
    public Buffer infoFieldDate();

    /**
     * Enum Filename Tag.
     *
     * @return the infoEnumFilename
     */
    public Buffer infoEnumFilename();

    /**
     * Enum Description Tag.
     *
     * @return the infoEnumDesc
     */
    public Buffer infoEnumDesc();

    /**
     * Enum Date Tag.
     *
     * @return the infoEnumDate
     */
    public Buffer infoEnumDate();

    /**
     * Field set def dictionary.
     *
     * @return the field set definition dictionary
     */
    public FieldSetDefDb fieldSetDef();

    /**
     * Convert information contained in the data dictionary to a string.
     *
     * @return the string representation of this {@link DataDictionary}
     */
    public String toString();

    public boolean hasEntry(String fieldName);

    public DictionaryEntry entry(String fieldName);
}