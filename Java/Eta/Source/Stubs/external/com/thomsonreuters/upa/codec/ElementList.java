package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * The {@link ElementList} is a self-describing container type that can contain
 * zero to N^23 {@link ElementEntry element entries}, where zero indicates an
 * empty element list. Each entry, known as an {@link ElementEntry}, contains an
 * element name, dataType enumeration, and value.An element list is equivalent
 * to {@link FieldList}, where name and type information is present in each
 * element entry instead of optimized via a field dictionary.
 * 
 * <p>
 * <b>ElementList Encoding Example</b>
 * <p>
 * The following example demonstrates how to encode an {@link ElementList} and
 * encodes four {@link ElementEntry} values:
 * <ul>
 * <li>
 * The first encodes an entry from a primitive {@link Time} type.</li>
 * <li>
 * The second encodes from a pre-encoded buffer containing an encoded
 * {@link UInt}.</li>
 * <li>
 * The third encodes as a blank {@link Real} value.</li>
 * <li>
 * The fourth encodes as an {@link ElementList} container type.</li>
 * </li>
 * </ul>
 * <p>
 * The pattern used to encode the fourth entry can be used to encode any
 * container type into an {@link ElementEntry}. This example demonstrates error
 * handling for the initial encode method. However, additional error handling
 * is omitted to simplify the example. This example shows the encoding of
 * standard name, dataType, and value data.
 * <p>
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * 
 * EncodeIterator encIter = CodecFactory.createEncodeIterator();
 * ElementList elementList = CodecFactory.createElementList();
 * // indicate that standard data will be encoded and that elementListNum is
 * // included
 * elementList.applyHasStandardData();
 * elementList.elementListNum(5);
 * if (elementList.encodeInit(encIter, null, 0) &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition
 * }
 * else
 * {
 *     // elementList.encodeInit was successful
 * 
 *     // create a single ElementEntry and reuse for each entry
 *     ElementEntry elementEntry = CodecFactory.createElementEntry();
 * 
 *     Buffer name = CodecFactory.createBuffer();
 * 
 *     // FIRST Element Entry: encode entry from the Time primitive type
 *     name.data(&quot;Element1 - Primitive&quot;);
 *     elementEntry.name(name);
 *     elementEntry.dataType(DataTypes.TIME);
 * 
 *     // create and populate time
 *     Time time = CodecFactory.createTime();
 *     time.hour(11);
 *     time.minute(30);
 *     time.second(15);
 *     time.millisecond(7);
 * 
 *     retVal = elementEntry.encode(encIter, time);
 * 
 *     // SECOND Element Entry: encode entry from preencoded buffer containing an
 *     // encoded UInt type
 *     elementEntry.clear();
 *     name.data(&quot;Element2 - Primitive&quot;);
 *     elementEntry.name(name);
 *     elementEntry.dataType(DataTypes.UINT);
 *     UInt uint = CodecFactory.createUInt();
 *     uint.value(1234);
 *     retVal = elementEntry.encode(encIter, uint);
 * 
 *     // THIRD Element Entry: encode entry as a blank Real primitive
 *     // need to ensure that ElementEntry is appropriately cleared clearing will
 *     // ensure that encData is properly emptied
 *     elementEntry.clear();
 *     name.data(&quot;Element3 - Blank&quot;);
 *     elementEntry.name(name);
 *     retVal = elementEntry.encodeBlank(encIter);
 * 
 *     // FOURTH Element Entry: encode entry as a container type, ElementList
 *     // need to ensure that ElementEntry is appropriately cleared clearing will
 *     // ensure that encData is properly emptied
 *     elementEntry.clear();
 *     name.data(&quot;Element4 - Container&quot;);
 *     elementEntry.name(name);
 *     elementEntry.dataType(DataTypes.ELEMENT_LIST);
 *     retVal = elementEntry.encodeInit(encIter, 0);
 *     // has standard data
 *     retVal = elementList.encodeInit(encIter, null, 50);
 *     name.data(&quot;string type&quot;);
 *     elementEntry.name(name);
 *     elementEntry.dataType(DataTypes.ASCII_STRING);
 *     stringBuffer.data(&quot;ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKL&quot;);
 *     retVal = elementEntry.encode(encIter, stringBuffer);
 *     retVal = elementList.encodeComplete(encIter, true);
 *     // complete encoding of complex element entry. If any element list encoding
 *     // failed, success is false
 *     retVal = elementEntry.encodeComplete(encIter, success);
 * }
 * // complete elementList encoding. If success parameter is true, this will
 * // finalize encoding.
 * // If success parameter is false, this will roll back encoding prior to
 * // elementList.encodeInit
 * retval = elementList.encodeComplete(encIter, success);
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * <p>
 * <b>ElementList decoding example</b>
 * <p>
 * The following sample demonstrates how to decode an ElementList and is
 * structured to decode each entry to its contained value. This example uses a
 * switch statement to invoke the specific decoder for the contained type,
 * however for sample clarity, necessary cases have been omitted. This example
 * uses the same {@link DecodeIterator} when calling the primitive decoder
 * method. An application could optionally use a new {@link DecodeIterator} by
 * setting the encData on a new iterator. For simplification, the example omits
 * some error handling.
 * 
 * <p>
 * <p>
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * //decode into the element list structure
 * if(retVal  = elementList.decode(decIter, localSetDefs)) >= CodecReturnCodes.SUCCESS)
 * {
 *  //decode each element entry
 *  while((retVal = elementEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER))
 *  {
 *      if (retVal < CodecReturnCodes.SUCCESS)
 *      {
 *      
 *      }
 *      else
 *      {
 *          //elemEntry.dataType to call correct primitive decode method
 *          switch (elemEntry.dataType())
 *          {
 *              case DataTypes.REAL:
 *                  retVal = real.decode(decIter);
 *              break;
 *              case DataTypes.DATE:
 *                  retVal = date.decode(decIter);
 *              break;
 *              //switch statement omitted to shorten sample code
 *          }
 *      }
 *  }
 * }
 * else
 * {
 *      //decoding failure tends to be unrecoverable
 * }
 * 
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * @see ElementEntry
 * @see ElementListFlags
 */
public interface ElementList extends XMLDecoder
{
    /**
     * Clears {@link ElementList} object. Useful for object reuse during encoding.
     * While decoding, {@link ElementList} object can be reused without using {@link #clear()}.
     */
    public void clear();

    /**
     * Prepares element list for Element List for encoding.
     * 
     * Typical use:<BR>
     * 1. Call encodeInit()<BR>
     * 2. Call one or more element encoding methods using the same buffer<BR>
     * 3. Call encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param setDb A set definition database to refer use, if encoding set data in the list.
     * @param setEncodingMaxSize Maximum amount of space for the set definition database.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see LocalElementSetDefDb
     */
    public int encodeInit(EncodeIterator iter, LocalElementSetDefDb setDb, int setEncodingMaxSize);

    /**
     * Completes elementList encoding.
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
     * Initialize decoding iterator for a element list.
     * 
     * Typical use:<BR>
     * 1. Call decode() on the list<BR>
     * 2. Call decode() for each element in the list<BR>
     * 
     * @param iter The iterator used to parse the element list.
     * @param localSetDb The local set database.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     * @see LocalElementSetDefDb
     */
    public int decode(DecodeIterator iter, LocalElementSetDefDb localSetDb);

    /**
     * Checks the presence of the Information presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasInfo();

    /**
     * Checks the presence of the Standard Data presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasStandardData();

    /**
     * Checks the presence of the Set Id presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSetId();

    /**
     * Checks the presence of the Set Data presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
     */
    public boolean checkHasSetData();

    /**
     * Applies the Information presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasInfo();

    /**
     * Applies the Standard Data presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasStandardData();

    /**
     * Applies the Set Id presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSetId();

    /**
     * Applies the Set Data presence flag.<br />
     * <br />
     * Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSetData();

    /**
     * Refers to an external element-list template, also known as a record template.
     * A record template contains information about all possible entries contained
     * in the stream and is typically used by caching mechanisms to pre-allocate
     * storage. Must be in the range of -32768 - 32767.
     * 
     * @param elementListNum the elementListNum to set
     */
    public void elementListNum(int elementListNum);

    /**
     * Refers to an external element-list template, also known as a record template.
     * A record template contains information about all possible entries contained
     * in the stream and is typically used by caching mechanisms to pre-allocate storage.
     * 
     * @return the elementListNum
     */
    public int elementListNum();

    /**
     * Corresponds to the set definition used for encoding or decoding the
     * set-defined data in this {@link ElementList}. When encoding, this is
     * the set definition used to encode any set-defined content. When decoding,
     * this is the set definition used for decoding any set-defined content.
     * If a setId value is not present on a message containing set-defined data,
     * a setId of 0 is implied. Must be in the range of 0 - 32767.
     * 
     * @param setId the setId to set
     */
    public void setId(int setId);

    /**
     * Corresponds to the set definition used for encoding or decoding the
     * set-defined data in this {@link ElementList}. When encoding, this is
     * the set definition used to encode any set-defined content. When decoding,
     * this is the set definition used for decoding any set-defined content.
     * If a setId value is not present on a message containing set-defined data,
     * a setId of 0 is implied.
     * 
     * @return the setId
     */
    public int setId();

    /**
     * Length and pointer to the encoded set-defined data, if any, contained in
     * the message. If populated, contents are described by the set definition
     * associated with the setId member.
     * 
     * @param encodedSetData the encodedSetData to set
     */
    public void encodedSetData(Buffer encodedSetData);

    /**
     * Length and pointer to the encoded set-defined data, if any, contained in
     * the message. Contents are described by the set definition associated with the setId member.
     * 
     * @return the encodedSetData
     */
    public Buffer encodedSetData();


    /**
     * All encoded element name, dataType, value encoded data, if any, contained
     * in the message. This would refer to encoded {@link ElementList} payload and length information.
     * 
     * @return the encodedEntries
     */
    public Buffer encodedEntries();

    /**
     * Combination of bit values that indicate whether optional, element-list
     * content is present. For more information about flag values see
     * {@link ElementListFlags}. Must be in the range of 0 - 255.
     * 
     * @param flags An integer containing all the flags applicable to this element list
     * 
     * @see ElementListFlags
     */
    public void flags(int flags);

    /**
     * Combination of bit values that indicate whether optional, element-list
     * content is present. For more information about flag values see {@link ElementListFlags}.
     * 
     * @return All the flags applicable to this element list
     * 
     * @see ElementListFlags
     */
    public int flags();
}