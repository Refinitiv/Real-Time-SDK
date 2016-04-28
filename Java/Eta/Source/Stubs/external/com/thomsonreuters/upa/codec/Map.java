package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * The {@link Map} is a uniform container type of associated key - value pair
 * entries, each known as a {@link MapEntry}. Each {@link MapEntry} contains an
 * entry key, which is a base primitive type, and a value.
 * 
 * <p>
 * <b>Map Encoding Example</b>
 * </p>
 * <p>
 * The following sample illustrates the encoding of a {@link Map} containing
 * {@link FieldList} values. The example encodes three {@link MapEntry} values
 * as well as summary data:
 * <ul>
 * <li>The first entry is encoded with an update action type and a passed in key
 * value.</li>
 * <li>The second entry is encoded with an add action type, pre-encoded data,
 * and pre-encoded key.</li>
 * <li>The third entry is encoded with a delete action type.</li>
 * </ul>
 * <p>
 * This example also demonstrates error handling for the initial encode
 * method. To simplify the example, additional error handling is omitted,
 * though it should be performed.
 * 
 * <p>
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * 
 * Map map = CodecFactory.createMap();
 * EncodeIterator encIter = CodecFactory.createEncodeIterator();
 * 
 * // populate map structure prior to call to map.encodeInit()
 * // NOTE: the key names used for this example may not correspond to actual name
 * // values
 * 
 * // indicate that summary data and a total count hint will be encoded
 * map.flags(MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_TOTAL_COUNT_HINT);
 * 
 * // populate maps keyPrimitiveType and containerType
 * map.containerType(DataTypes.FIELD_LIST);
 * map.keyPrimitiveType(DataTypes.UINT);
 * 
 * // populate total count hint with approximate expected entry count
 * map.totalCountHint(3);
 * 
 * // begin encoding of map - assumes that encIter is already populated with
 * // buffer and version information, store return value to determine success or
 * // failure
 * // expect summary data of approx. 100 bytes, no set definition data
 * 
 * retVal = map.encodeInit(encIter, 100, 0);
 * if (retVal &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition - switch our success value to false so we can roll back
 *     success = false;
 * }
 * else
 * {
 *     // mapInit encoding was successful
 *     // create a single MapEntry and FieldList and reuse for each entry
 *     MapEntry mapEntry = CodecFactory.createMapEntry();
 *     FieldList fieldList = CodecFactory.createFieldList();
 *     UInt entryKeyUInt = CodecFactory.createUInt();
 *     entryKeyUInt.value(0);
 * 
 *     // encode expected summary data, init for this was done by map.encodeInit()
 *     // this type should match map.containerType.
 *     {
 *         fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);
 *         retVal = fieldList.encodeInit(encIter, null, 0);
 * 
 *         // Continue encoding field entries - see encoding example in FieldList
 * 
 *         // Complete nested container encoding
 *         retVal = fieldList.encodeComplete(encIter, success);
 *     }
 *     // complete encoding of summary data. If any field list encoding failed,
 *     // success is false
 *     retVal = map.encodeSummaryDataComplete(encIter, success);
 * 
 *     // FIRST Map Entry: encode entry from non pre-encoded data and key. Approx.
 *     // encoded length unknown
 *     mapEntry.action(MapEntryActions.UPDATE);
 *     entryKeyUInt.value(1);
 *     mapEntry.encodeInit(encIter, entryKeyUInt, 0);
 * 
 *     // encode contained field list - this type should match Map.containerType
 *     {
 *         fieldList.clear();
 *         fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);
 *         retVal = fieldList.encodeInit(encIter, null, 0);
 * 
 *         // Continue encoding field entries
 * 
 *         // Complete nested container encoding
 *         retVal = fieldList.encodeComplete(encIter, success);
 *     }
 * 
 *     retVal = mapEntry.encodeComplete(encIter, success);
 * 
 *     // SECOND Map Entry: encode entry from pre-encoded buffer containing an
 *     // encoded FieldList
 *     // because we are re-populating all values on MapEntry, there is no need to
 *     // clear it.
 *     mapEntry.action(MapEntryActions.ADD);
 * 
 *     // assuming encUInt Buffer contains the pre-encoded key with length and
 *     // data properly populated
 *     mapEntry.encodedKey(encUInt);
 * 
 *     // assuming encFieldList Buffer contains the pre-encoded payload with data
 *     // and length populated
 *     mapEntry.encodedData(encFieldList);
 * 
 *     retVal = mapEntry.encode(encIter);
 * 
 *     // THIRD Map Entry: encode entry with delete action. Delete actions have no
 *     // payload
 *     // need to ensure that MapEntry is appropriately cleared - clearing will
 *     // ensure that encData and encKey are properly emptied.s
 *     mapEntry.clear();
 *     mapEntry.action(MapEntryActions.DELETE);
 *     entryKeyUInt.value(3);
 *     retVal = mapEntry.encode(encIter, entryKeyUInt);
 * 
 * }
 * 
 * // complete map encoding. If success parameter is true, this will finalize
 * // encoding.
 * // If success parameter is false, this will roll back encoding prior to mapInit.
 * retVal = map.encodeComplete(encIter, success);
 * 
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * <p>
 * <b>Map Decoding Example</b>
 * <p>
 * The following sample demonstrates the decoding of a {@link Map} and is
 * structured to decode each entry to the contained value. This sample assumes
 * that the housed container type is a {@link FieldList} and that the
 * keyPrimitiveType is DataTypes#INT. This sample also uses the
 * mapEntry.decode() method to perform key decoding. Typically an application
 * would invoke the specific container-type decoder for the housed type or use a
 * switch statement to allow for a more generic map entry decoder. This example
 * uses the same {@link DecodeIterator} when calling the content's decoder
 * method. An application could optionally use a new {@link DecodeIterator} by
 * setting the encData on a new iterator. To simplify the sample, some error
 * handling is omitted.
 * <p>
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * // decode contents into the map structure
 * retVal = map.decode(decIter);
 * if (retVal &gt;= CodecReturnCodes.SUCCESS)
 * {
 *     // create primitive value to have key decoded into and a single map entry to
 *     // reuse
 *     Int intVal = CodecFactory.createInt();
 *     intVal.value(0);
 *     mapEntry = CodecFactory.createMapEntry();
 *     if (mapEntry.checkHasSummaryData())
 *     {
 *         // summary data is present. Its type should be that of map.containerType
 *         FieldList fieldList = CodecFactory.createFieldList();
 *         retVal = fieldList.decode(decIter, 0);
 * 
 *         // Continue decoding field entries
 *     }
 * 
 *     // decode each map entry, passing in pointer to keyPrimitiveType decodes
 *     // mapEntry key as well
 *     while ((retVal = mapEntry.decode(decIter, intVal) != CodecReturnCodes.END_OF_CONTAINER))
 *     {
 *         if (retVal &lt; CodecReturnCodes.SUCCESS)
 *         {
 *             // decoding failure tends to be unrecoverable
 *         }
 *         else
 *         {
 *             FieldList fieldList = CodecFactory.createFieldList();
 *             retVal = fieldList.decode(decIter, 0);
 *             // Continue decoding field entries. See {@link FieldList} for
 *             // decoding example.
 *         }
 *     }
 * }
 * else
 * {
 *     // decoding failure tends to be unrecoverable
 * }
 * 
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * @see MapEntry
 * 
 * @see MapFlags
 */
public interface Map extends XMLDecoder
{
    /**
     * 
     * Sets all members in {@link Map} to an initial value. Useful for object
     * reuse during encoding. While decoding, {@link Map} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();

    /**
     * Initialize Map encoding.
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() or
     * MapEntry.encodeInit()..MapEntry.encodeComplete() for each map entry in
     * the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param summaryMaxSize max encoding size of the summary data, if encoding
     * @param setMaxSize max encoding size of the set information, if encoding
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize);

    /**
     * Complete set data encoding for a map. If both encodeSetDefsComplete() and
     * encodeSummaryDataComplete() are called, encodeSetDefsComplete() must be
     * called first.
     * 
     * @param iter Encoding iterator
     * @param success True if encoding of set data was successful, false for
     *            rollback
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeSetDefsComplete(EncodeIterator iter, boolean success);

    /**
     * Complete summary data encoding for a map. If both encodeSetDefsComplete()
     * and encodeSummaryDataComplete() are called, encodeSetDefsComplete() must
     * be called first.
     * 
     * @param iter Encoding iterator
     * @param success True if encoding of summary data was successful, false for
     *            rollback
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeSummaryDataComplete(EncodeIterator iter, boolean success);

    /**
     * Completes Map encoding.
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() or
     * MapEntry.encodeInit()..MapEntry.encodeComplete() for each map entry in
     * the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param success If true - successfully complete the aggregate, if false -
     *            remove the aggregate from the buffer.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeComplete(EncodeIterator iter, boolean success);

    /**
     * Decode map.
     * 
     * @param iter {@link DecodeIterator} for decoding with
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Checks the presence of the local Set Definition presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSetDefs();

    /**
     * Checks the presence of the Summary Data presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSummaryData();

    /**
     * Checks the presence of the Per Entry Permission presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasPerEntryPermData();

    /**
     * Checks the presence of the Total Count Hint presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasTotalCountHint();

    /**
     * Checks the presence of the Key Field Id presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if key is fid; false if not.
     */
    public boolean checkHasKeyFieldId();

    /**
     * Applies the local Set Definition presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSetDefs();

    /**
     * Applies the Summary Data presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSummaryData();

    /**
     * Applies the Per Entry Permission Data presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasPerEntryPermData();

    /**
     * Applies the Total Count Hint presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasTotalCountHint();

    /**
     * Applies the Key Field Id presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasKeyFieldId();

    /**
     * Raw encoded map data.
     * 
     * @return encodedEntries
     */
    public Buffer encodedEntries();

    /**
     * The {@link DataTypes} enumeration value that describes the base primitive
     * type of each {@link MapEntry}'s key. keyPrimitiveType must be from the
     * {@link DataTypes} enumeration.
     * 
     * @return keyPrimitiveType
     */
    public int keyPrimitiveType();

    /**
     * The {@link DataTypes} enumeration value that describes the base primitive
     * type of each {@link MapEntry}'s key. keyPrimitiveType must be from the
     * {@link DataTypes} enumeration and greater than {@link DataTypes#UNKNOWN}
     * and less than or equal to {@link DataTypes#BASE_PRIMITIVE_MAX}, cannot be
     * specified as blank, and cannot be the {@link DataTypes#ARRAY} or
     * {@link DataTypes#UNKNOWN} primitive types.
     * 
     * @param keyPrimitiveType the keyPrimitiveType to set
     */
    public void keyPrimitiveType(int keyPrimitiveType);

    /**
     * Specifies a fieldId associated with the entry key information. This is
     * mainly used as an optimization to avoid inclusion of redundant data. In
     * situations where key information is also a member of the entry payload
     * (e.g. Order Id for Market By Order domain type), this allows removal of
     * data from each entry's payload prior to encoding as it is already present
     * via the key and keyFieldId.
     * 
     * @return keyFieldId
     */
    public int keyFieldId();

    /**
     * Specifies a fieldId associated with the entry key information. This is
     * mainly used as an optimization to avoid inclusion of redundant data. In
     * situations where key information is also a member of the entry payload
     * (e.g. Order Id for Market By Order domain type), this allows removal of
     * data from each entry's payload prior to encoding as it is already present
     * via the key and keyFieldId. Must be in the range of -32768 - 32767.
     * 
     * @param keyFieldId the keyFieldId to set
     */
    public void keyFieldId(int keyFieldId);

    /**
     * The {@link DataTypes} enumeration value that describes the container type
     * of each {@link MapEntry}'s payload.
     * 
     * @return containerType
     */
    public int containerType();

    /**
     * The {@link DataTypes} enumeration value that describes the container type
     * of each {@link MapEntry}'s payload. containerType must be from the
     * {@link DataTypes} enumeration in the range
     * {@link DataTypes#CONTAINER_TYPE_MIN} to 255.
     * 
     * @param containerType the containerType to set
     */
    public void containerType(int containerType);

    /**
     * The encoded local set definitions, if any, contained in the message. If
     * populated, these definitions correspond to data contained within the
     * {@link Map}'s entries and are used for encoding or decoding their
     * contents.
     * 
     * @return encodedSetDefs
     */
    public Buffer encodedSetDefs();

    /**
     * The encoded local set definitions, if any, contained in the message.
     * 
     * @param encodedSetDefs the encodedSetDefs to set
     */
    public void encodedSetDefs(Buffer encodedSetDefs);

    /**
     * The encoded summary data, if any, contained in the message. If populated,
     * summary data contains information that applies to every entry encoded in
     * the {@link Map} (e.g. currency type). The container type of summary data
     * should match the containerType specified on the {@link Map}. If
     * encSummaryData is populated while encoding, contents are used as
     * pre-encoded summary data.
     * 
     * @return encodedSummaryData
     */
    public Buffer encodedSummaryData();

    /**
     * The encoded summary data, if any, contained in the message. If populated,
     * summary data contains information that applies to every entry encoded in
     * the {@link Map} (e.g. currency type). The container type of summary data
     * should match the containerType specified on the {@link Map}. If
     * encSummaryData is populated while encoding, contents are used as
     * pre-encoded summary data.
     * 
     * @param encodedSummaryData the encodedSummaryData to set
     */
    public void encodedSummaryData(Buffer encodedSummaryData);

    /**
     * Indicates an approximate total number of entries associated with this stream.
     * This is typically used when multiple {@link Map} containers are spread across
     * multiple parts of a refresh message.). totalCountHint provides an approximation
     * of the total number of entries sent across all maps on all parts of the refresh
     * message. This information is useful when determining the amount of resources to
     * allocate for caching or displaying all expected entries.
     * 
     * 
     * @return totalCountHint
     */
    public int totalCountHint();

    /**
     * Indicates an approximate total number of entries associated with this stream.
     * This is typically used when multiple {@link Map} containers are spread across
     * multiple parts of a refresh message.). totalCountHint provides an approximation
     * of the total number of entries sent across all maps on all parts of the refresh
     * message. This information is useful when determining the amount of resources to
     * allocate for caching or displaying all expected entries. Must be in the range of
     * 0 - 1073741823.
     * 
     * @param totalCountHint the totalCountHint to set
     */
    public void totalCountHint(int totalCountHint);

    /**
     * Sets all the flags applicable to this map. Must be in the range of 0 - 255.
     * 
     * @param flags An integer containing all the flags applicable to this map
     * 
     * @see MapFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this map.
     * 
     * @return All the flags applicable to this map
     * 
     * @see MapFlags
     */
    public int flags();
}