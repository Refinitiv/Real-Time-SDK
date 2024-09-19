/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * A container of field identifier - value paired entries, each known as a
 * {@link FieldEntry}. A field identifier, also referred to as a fieldId, is a
 * value that refers to specific name and type information defined by an external
 * field dictionary, such as the RDMFieldDictionary. A field list can contain zero
 * to N entries, where zero indicates an empty field list.
 * 
 * 
 * <p>
 * <b>FieldList Encoding Example</b>
 * <p>
 * The following example demonstrates how to encode a {@link FieldList} and
 * encodes four {@link FieldEntry} values:
 * <ul>
 * <li>
 * The first encodes an entry from a primitive {@link Date} type.</li>
 * <li>
 * The second from a pre-encoded buffer containing an encoded {@link UInt}.</li>
 * <li>
 * The third as a blank {@link Real} value.</li>
 * <li>
 * The fourth as an {@link Array} complex type. The pattern followed while
 * encoding the fourth entry can be used for encoding of any container type into
 * a {@link FieldEntry}</li>
 * </ul>
 * <p>
 * This example demonstrates error handling for the initial encode method. To
 * simplify the example, additional error handling is omitted (though it should
 * be performed). This example shows encoding of standard fieldId-value data.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * 
 * FieldList fieldList = CodecFactory.createFieldList();
 * 
 * // create a single FieldEntry and reuse for each entry
 * FieldEntry fieldEntry = CodecFactory.createFieldEntry();
 * 
 * EncodeIterator encIter = CodecFactory.createEncodeIterator();
 * 
 * // NOTE: the fieldId, dictionaryId and fieldListNum values used for this example
 * // do not correspond to actual id values
 * // indicate that standard data will be encoded and that dictionaryId and
 * // fieldListNum are included
 * fieldList.applyHasInfo();
 * fieldList.dictionaryId(2);
 * fieldList.fieldListNum(3);
 * fieldList.applyHasStandardData();
 * 
 * // begin encoding of field list - assumes that encIter is already populated with
 * // buffer and version information, store return value to determine success or
 * // failure
 * 
 * if (fieldList.encodeInit(encIter, null, 40) &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition - switch our success value to false so we can roll back
 *     success = false;
 * }
 * else
 * {
 *     // fieldListInit encoding was successful
 *     // FIRST Field Entry: encode entry from the Date primitive type
 *     Date date = CodecFactory.createDate();
 *     date.day(23);
 *     date.month(5);
 *     date.year(2012);
 *     fieldEntry.fieldId(16);
 *     fieldEntry.dataType(DataTypes.DATE);
 *     retVal = fieldEntry.encode(encIter, date);
 * 
 *     // SECOND Field Entry: encode entry from preencoded buffer containing an
 *     // encoded UInt type
 *     // populate and encode field entry with fieldId and dataType information for
 *     // this field
 *     // because we are re-populating all values on FieldEntry, there is no need
 *     // to clear it.
 *     fieldEntry.clear();
 *     fieldEntry.fieldId(16);
 *     fieldEntry.dataType(DataTypes.UINT);
 *     // assuming encUInt is a Buffer with length and data properly populated
 *     fieldEntry.encodedData(encUInt);
 *     retVal = fieldEntry.encode(encIter, date);
 * 
 *     // THIRD Field Entry: encode entry as a blank Real primitive type
 *     fieldEntry.clear();
 *     fieldEntry.fieldId(22);
 *     fieldEntry.dataType(DataTypes.REAL);
 *     retVal = fieldEntry.encodeBlank(encIter);
 * 
 *     // FOURTH Field Entry: encode entry as a complex type, Array primitive
 *     Array array = CodecFactory.createArray();
 *     fieldEntry.clear();
 *     fieldEntry.fieldId(1021);
 *     fieldEntry.dataType(DataTypes.ARRAY);
 *     // begin complex field entry encoding, we are not sure of the approximate
 *     // max encoding length
 *     retVal = fieldEntry.encodeInit(encIter, 0);
 * 
 *     // now encode nested container using its own specific encode methods
 *     // encode Real values into the array
 *     array.primitiveType(DataTypes.REAL);
 * 
 *     // values are variable length
 *     array.itemLength(0);
 * 
 *     // begin encoding of array - using same encIterator as field list
 *     if ((retVal = array.encodeInit(encIter)) &lt; CodecReturnCodes.SUCCESS)
 *         // Continue encoding array entries
 * 
 *         // Complete nested container encoding
 *         retVal = encode.arrayComplete(encIter, success);
 * 
 *     // complete encoding of complex field entry. If any array encoding failed,
 *     // success is false
 *     retVal = fieldEntry.encodeComplete(encIter, success);
 * }
 * 
 * // complete fieldList encoding. If success parameter is true, this will finalize
 * // encoding.
 * // If success parameter is false, this will roll back encoding prior to
 * // FieldList.encodeInit()
 * 
 * retVal = fieldList.encodeComplete(encIter, success);
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * 
 * <p>
 * <b>FieldList Decoding Example</b>
 * <p>
 * The following example demonstrates how to decode a {@link FieldList} and is
 * structured to decode each entry to the contained value. This example uses a
 * switch statement to invoke the specific decoder for the contained type,
 * however to simplify the example, necessary cases and some error handling are
 * omitted. This example uses the same {@link DecodeIterator} when calling the
 * primitive decoder method. An application could optionally use a new
 * {@link DecodeIterator} by setting the encData on a new iterator.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * // decode into the field list structure
 * if ((retVal = fieldList.decode(decIter, localSetDefs)) &gt; CodecReturnCodes.SUCCESS)
 * {
 *     // decode each field entry
 *     while ((retVal = fieldEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
 *     {
 *         if (retVal &lt; CodecReturnCodes.SUCCESS)
 *         {
 *             // decoding failure tends to be unrecoverable
 *         }
 *         else
 *         {
 *             // look up type in field dictionary and call correct primitive
 *             // decode method
 *             switch (fieldDict.entry(fieldEntry.fieldId()).rwfType)
 *             {
 *                 case DataTypes.REAL:
 *                     retVal = real.decode(decIter);
 *                     break;
 *                 case DataTypes.DATE:
 *                     retVal = date.decode(decIter);
 *                     break;
 *             // full switch statement omitted to shorten sample code
 *             }
 *         }
 *     }
 * }
 * else
 * {
 *     // decoding failure tends to be unrecoverable
 * }
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * @see FieldListFlags
 * @see FieldEntry
 */
public interface FieldList extends XMLDecoder
{
    /**
     * Clears {@link FieldList} object. Useful for object reuse during encoding.
     * While decoding, {@link FieldList} object can be reused without using {@link #clear()}.
     */
    public void clear();

    /**
     * Initializes List for encoding.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() or
     * FieldEntry.encodeInit()..FieldEntry.encodeComplete() for each field in
     * the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param setDb The set definition database to be used, if encoding set data.
     * @param setEncodingMaxSize Max encoding size for field list (If Unknown set to zero).
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see LocalFieldSetDefDb
     */
    public int encodeInit(EncodeIterator iter, LocalFieldSetDefDb setDb, int setEncodingMaxSize);

    /**
     * Completes List encoding.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() or
     * FieldEntry.encodeInit()..FieldEntry.encodeComplete() for each field in
     * the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param success If true - successfully complete the aggregate,
     *                if false - remove the aggregate from the buffer.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeComplete(EncodeIterator iter, boolean success);

    /**
     * Initialize decoding iterator for a field list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.decode()<BR>
     * 2. Call FieldEntry.decode() for each field in the list.<BR>
     * 
     * @param iter The iterator used to parse the field list.
     * @param localSetDb The local set database.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     * @see LocalFieldSetDefDb
     */
    public int decode(DecodeIterator iter, LocalFieldSetDefDb localSetDb);

    /**
     * Checks the presence of the Information presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     * @see #flags()
     */
    public boolean checkHasInfo();

    /**
     * Checks the presence of the Standard Data presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     * @see #flags()
     */
    public boolean checkHasStandardData();

    /**
     * Checks the presence of the Set Id presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     * @see #flags()
     */
    public boolean checkHasSetId();

    /**
     * Checks the presence of the Set Data presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     *
     * @return true - if exists; false if does not exist.
     * @see #flags()
     */
    public boolean checkHasSetData();

    /**
     * Applies the Information presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasInfo();

    /**
     * Applies the Standard Data presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasStandardData();

    /**
     * Applies the Set Id presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSetId();

    /**
     * Applies the Set Data presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSetData();

    /**
     * Dictionary id. Refers to an external field dictionary, such as
     * RDMFieldDictionary.
     * 
     * @return the dictionaryId
     */
    public int dictionaryId();

    /**
     * Dictionary id. Refers to an external field dictionary, such as
     * RDMFieldDictionary. Must be in the range of 0 - 32767.
     * 
     * @param dictionaryId the dictionaryId to set
     */
    public void dictionaryId(int dictionaryId);

    /**
     * Refers to an external fieldlist template, also known as a record template.
     * The record template contains information about all possible fields in a
     * stream and is typically used by caching implementations to pre-allocate storage.
     * 
     * @return the fieldListNum
     */
    public int fieldListNum();

    /**
     * Refers to an external fieldlist template, also known as a record template.
     * The record template contains information about all possible fields in a
     * stream and is typically used by caching implementations to pre-allocate
     * storage. Must be in the range of -32768 - 32767.
     * 
     * @param fieldListNum the fieldListNum to set
     */
    public void fieldListNum(int fieldListNum);

    /**
     * Set id. Corresponds to the Set Definition used for encoding or decoding
     * the set defined data in this {@link FieldList}.
     * 
     * @return the setId
     */
    public int setId();

    /**
     * Set id. Corresponds to the Set Definition used for encoding or decoding
     * the set defined data in this {@link FieldList}. Must be in the range of 0 - 32767.
     * 
     * @param setId the setId to set
     */
    public void setId(int setId);

    /**
     * Encoded set data. If populated, contents are described by the set
     * definition associated with the setId member.
     * 
     * @return the encodedSetData
     */
    public Buffer encodedSetData();

    /**
     * Encoded set data. If populated, contents are described by the set
     * definition associated with the setId member.
     * 
     * @param encodedSetData the encodedSetData to set
     */
    public void encodedSetData(Buffer encodedSetData);

    /**
     * Length and pointer to the encoded fieldId-value pair encoded data, if
     * any, contained in the message. This would refer to encoded
     * {@link FieldList} payload and length information.
     * 
     * @return the encodedEntries
     */
    public Buffer encodedEntries();

    /**
     * Sets all the flags applicable to this field list.
     * Must be in the range of 0 - 255.
     * 
     * @param flags An integer containing all the flags applicable to this field list
     * 
     * @see FieldListFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this field list.
     * 
     * @return All the flags applicable to this field list
     * 
     * @see FieldListFlags
     */
    public int flags();
}
