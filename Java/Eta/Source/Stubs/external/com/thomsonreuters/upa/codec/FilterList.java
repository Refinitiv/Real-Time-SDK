package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * The {@link FilterList} is a non-uniform container type of filterId - value
 * pair entries. Each entry, known as a {@link FilterEntry}, contains an id
 * corresponding to one of 32 possible bit-value identifiers. These identifiers
 * are typically defined by a domain model specification and can be used to
 * indicate interest or presence of specific entries through the inclusion of
 * the filterId in the message key's filter member.
 * 
 * 
 * 
 * <p>
 * <b>FilterList Encoding Example</b>
 * <p>
 * The following sample illustrates how to encode a {@link FilterList}
 * containing a mixture of housed types. The example encodes three
 * {@link FilterEntry} values:
 * <ul>
 * <li>The first is encoded from an unencoded element list.</li>
 * <li>The second is encoded from a buffer containing a pre-encoded element list.</li>
 * <li>The third is encoded from an unencoded map value.</li>
 * </ul>
 * <p>
 * This example demonstrates error handling only for the initial encode
 * method, and to simplify the example, omits additional error handling
 * (though it should be performed).
 * 
 * <p>
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * 
 * Buffer buf = CodecFactory.createBuffer();
 * buf.data(ByteBuffer.allocate(40));
 * FilterList filterList = CodecFactory.createFilterList();
 * 
 * EncodeIterator encIter = CodecFactory.createEncodeIterator();
 * // populate containerType. Because there are two element lists, this is most
 * // common so specify that type.
 * filterList.containerType(DataTypes.ELEMENT_LIST);
 * 
 * // begin encoding of filterList - assumes that encIter is already populated with
 * // buffer and version information, store return value to determine success or
 * // failure
 * int retVal = filterList.encodeInit(encIter);
 * if (retVal &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition - switch our success value to false so we can roll back
 * }
 * else
 * {
 *     // filterList init encoding was successful
 *     // create a single FilterEntry and reuse for each entry
 *     FilterEntry filterEntry = CodecFactory.createFilterEntry();
 * 
 *     // FIRST Filter Entry: encode entry from unencoded data. Approx. encoded
 *     // length 350 bytes
 *     // populate id and action
 * 
 *     filterEntry.id(1);
 *     filterEntry.action(FilterEntryActions.SET);
 *     retVal = filterEntry.encodeInit(encIter, 350);
 *     // encode contained element list
 *     {
 *         ElementList elementList = CodecFactory.createElementList();
 *         elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
 * 
 *         // now encode nested container using its own specific encode methods
 *         retVal = elementList.encodeInit(encIter, null, 0);
 *         // Continue encoding element entries. See {@link ElementList} for
 *         // element list
 *         // encoding example
 *         // Complete nested container encoding
 *         retVal = elementList.encodeComplete(encIter, success);
 *     }
 * 
 *     retVal = filterEntry.encodeComplete(encIter, success);
 * 
 *     // SECOND Filter Entry: encode entry from pre-encoded buffer containing an
 *     // encoded element list
 *     // assuming encElemList Buffer contains the pre-encoded payload with data
 *     // and length populated.
 * 
 *     filterEntry.id(2);
 *     filterEntry.action(FilterEntryActions.UPDATE);
 *     filterEntry.encodedData(encElemList);
 * 
 *     retVal = filterEntry.encode(encIter);
 * 
 *     // THIRD Filter Entry: encode entry from an unencoded map
 *     filterEntry.id(3);
 *     filterEntry.action(FilterEntryActions.UPDATE);
 *     filterEntry.flags(FilterEntryFlags.HAS_CONTAINER_TYPE);
 *     filterEntry.containerType(DataTypes.MAP);
 * 
 *     // encode contained map
 *     {
 *         Map map = CodecFactory.createMap();
 *         map.keyPrimitiveType(DataTypes.ASCII_STRING);
 *         map.containerType(DataTypes.FIELD_LIST);
 * 
 *         // now encode nested container using its own specific encode methods
 *         retVal = map.encodeInit(encIter, 0, 0);
 * 
 *         // Continue encoding map entries. See {@link Map} for encoding example.
 * 
 *         // Complete nested container encoding
 * 
 *         retVal = map.encodeComplete(encIter, success);
 *     }
 * 
 *     retVal = filterEntry.encodeComplete(encIter, success);
 * }
 * 
 * // complete filterList encoding. If success parameter is true, this will
 * // finalize encoding.
 * // If success parameter is false, this will roll back encoding prior to
 * // FilterList.encodeInit
 * retVal = filterList.encodeComplete(encIter, success);
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * <p>
 * <b>FilterList Decoding Example</b>
 * <p>
 * The following sample illustrates how to decode a {@link FilterList} and is
 * structured to decode each entry to its contained value. The sample code uses
 * a switch statement to decode the contents of each filter entry. Typically an
 * application invokes the specific container type decoder for the housed type
 * or uses a switch statement to use a more generic filter entry decoder. This
 * example uses the same {@link DecodeIterator} when calling the content's
 * decoder method. Optionally, an application could use a new
 * {@link DecodeIterator} by setting the encData on a new iterator. To simplify
 * the example, some error handling is omitted.
 * <p>
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * // decode contents into the filter list structure
 * retVal = filterList.decode(decIter);
 * if (retVal &gt;= CodecReturnCodes.SUCCESS)
 * {
 *     // create single filter entry and reuse while decoding each entry
 *     FilterEntry filterEntry = CodecFactory.createFilterEntry();
 * 
 *     // decode each filter entry until there are no more left
 *     while ((retVal = filterEntry.decode(decIter) != CodecReturnCodes.END_OF_CONTAINER))
 *     {
 *         if (retVal &lt; CodecReturnCodes.SUCCESS)
 *         {
 *             // decoding failure tends to be unrecoverable
 *         }
 *         else
 *         {
 *             // if filterEntry.containerType is present, switch on that,
 *             // Otherwise switch on filterList.containerType
 *             int containterType;
 *             if (filterEntry.checkHasContainerType())
 *             {
 *                 containterType = filterEntry.containterType();
 *             }
 *             else
 *             {
 *                 containterType = filterList.containterType();
 *             }
 * 
 *             switch (containterType)
 *             {
 *                 case DataTypes.MAP:
 *                     retVal = map.decode(decIter);
 *                     // Continue decoding map entries.
 *                     // See {@link Map} for map decoding example.
 *                     break;
 *                 case DataTypes.ELEMENT_LIST:
 *                     retVal = elementList.decode(decIter);
 *                     // Continue decoding element entries.
 *                     // See {@link ElementList} for element list decoding
 *                     // example.
 *                     break;
 *             // full switch statement omitted to shorten sample code.
 *             }
 * 
 *         }
 *     }
 * }
 * else
 * {
 *     // error: decoding failure tends to be unrecoverable.
 * }
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * @see FilterListFlags
 * @see FilterEntry
 */
public interface FilterList extends XMLDecoder
{
    /**
     * Clears {@link FilterList} object. Useful for object reuse during encoding.
     * While decoding, {@link FilterList} object can be reused without using {@link #clear()}.
     */
    public void clear();

    /**
     * Prepares Filter List for encoding.
     * 
     * Typical use:<BR>
     * 1. Call FilterList.encodeInit()<BR>
     * 2. Call one or more FilterEntry encoding methods using the same buffer<BR>
     * 3. Call FilterList.encodeComplete()<BR>
     * 
     * @param iter The encoding iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter);

    /**
     * Completes Filter List encoding.
     * 
     * Typical use:<BR>
     * 1. Call FilterList.encodeInit()<BR>
     * 2. Call one or more FilterEntry encoding methods using the same buffer<BR>
     * 3. Call FilterList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator
     * @param success If true - successfully complete the aggregate,
     *                if false - remove the aggregate from the buffer.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeComplete(EncodeIterator iter, boolean success);

    /**
     * Decode Filter List.
     * 
     * @param iter Decode Iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Checks the presence of the Per Entry Permission presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true if exists, false if does not exist
     */
    public boolean checkHasPerEntryPermData();

    /**
     * Checks the presence of the Total Count Hint presence flag.<br />
     * <br />
     * Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true if exists, false if does not exist
     */
    public boolean checkHasTotalCountHint();

    /**
     * Applies the Per Entry Permission presence flag.<br />
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
     * A {@link DataTypes} enumeration value that, for most efficient bandwidth
     * use, should describe the most common container type across all housed
     * filter entries. All housed entries may match this type, though one or
     * more entries may differ. If an entry differs, the entry specifies its own
     * type via the {@link FilterEntry#containerType()} member.
     * 
     * @return containerType
     */
    public int containerType();

    /**
     * A {@link DataTypes} enumeration value that, for most efficient bandwidth
     * use, should describe the most common container type across all housed
     * filter entries. All housed entries may match this type, though one or
     * more entries may differ. If an entry differs, the entry specifies its own
     * type via the {@link FilterEntry#containerType()} member. containerType must
     * be in the range {@link DataTypes#CONTAINER_TYPE_MIN} to 255.
     * 
     * @param containerType the containerType to set
     */
    public void containerType(int containerType);

    /**
     * Indicates an approximate total number of entries associated with this
     * stream. totalCountHint is used typically when multiple {@link FilterList}
     * containers are spread across multiple parts of a refresh message.
     * 
     * @return totalCountHint
     */
    public int totalCountHint();

    /**
     * Indicates an approximate total number of entries associated with this
     * stream. totalCountHint is used typically when multiple {@link FilterList}
     * containers are spread across multiple parts of a refresh message. Must be
     * in the range of 0 - 255.
     * 
     * @param totalCountHint the totalCountHint to set
     */
    public void totalCountHint(int totalCountHint);

    /**
     * FilterId-value pair encoded data, if any, contained in the message. This
     * would refer to the encoded {@link FilterList} payload.
     * 
     * @return encodedEntries
     */
    public Buffer encodedEntries();

    /**
     * Returns all the flags applicable to this filter list.
     * 
     * @return All the flags applicable to this filter list
     * 
     * @see FilterListFlags
     */
    public int flags();

    /**
     * Sets all the flags applicable to this filter list. Must be in the range of 0 - 255.
     * 
     * @param flags An integer containing all the flags applicable to this filter list
     * 
     * @see FilterListFlags
     */
    public void flags(int flags);
}