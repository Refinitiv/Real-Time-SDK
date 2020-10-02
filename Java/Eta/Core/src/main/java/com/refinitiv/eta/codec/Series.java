package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * The {@link Series} is a uniform container type that contains 0 to N^33
 * entries where where zero indicates an empty series. Each entry, known as,
 * {@link SeriesEntry} contains only encoded data. This container is often used
 * to represent table based information, where no explicit indexing is present
 * or required.
 * 
 * <p>
 * <b>Series Encoding Example</b>
 * <p>
 * The following sample illustrates how to encode a {@link Series} containing
 * {@link ElementList} values. The example encodes two {@link SeriesEntry}
 * values as well as summary data.
 * <ul>
 * <li>The first entry is encoded from an unencoded element list.</li>
 * <li>The second entry is encoded from a buffer containing a pre-encoded
 * element list.</li>
 * </ul>
 * The example demonstrates error handling for the initial encode method. To
 * simplify the example, additional error handling is omitted, though it should
 * be performed.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * // populate series structure prior to call to series.encodeInit()
 * 
 * // indicate that summary data and a total count hint will be encoded
 * series.flags(SeriesFlags.HAS_SUMMARY_DATA | SeriesFlags.HAS_TOTAL_COUNT_HINT);
 * // populate containerType and total count hint
 * series.containerType(DataTypes.ELEMENT_LIST);
 * series.totalCountHint(2);
 * 
 * // begin encoding of series - assumes that encIter is already populated with
 * // buffer and version information, store return value to determine success or
 * // failure
 * // summary data approximate encoded length is unknown, pass in 0
 * if ((retVal = series.encodeInit(encIter, 0, 0)) &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition - switch our success value to false so we can roll back
 *     success = false;
 * }
 * else
 * {
 *     // series init encoding was successful
 *     // create a single SeriesEntry and ElementList and reuse for each entry
 *     SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
 *     ElementList elementList = CodecFactory.createElementList();
 *     // encode expected summary data, init for this was done by
 *     // seriesEntry.encodeInit
 *     // - this type should match series.containerType
 *     {
 *          // now encode nested container using its own specific encode methods
 *          elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
 * 
 *          retVal = elementList.encodeInit(encIter, null, 0));
 * 
 *          // ------ Continue encoding element entries.  See example in ElementList ----
 * 
 *          // Complete nested container encoding 
 *          retVal = elementList.encodeComplete(encIter, success);
 *      }
 *      // complete encoding of summary data.  If any element list encoding failed, success is false
 *      retVal = series.encodeSummaryDataComplete(encIter, success);    
 * 
 *      //FIRST Series Entry: encode entry from unencoded data.  Approx. encoded length unknown
 *      retVal = seriesEntry.encodeInit(encIter, 0);
 *      
 *      //encode contained element list - this type should match series.containerType
 *      {
 *          //now encode nested container using its own specific encode methods 
 *          //clear, then begin encoding of element list - using same encIterator as series
 *          elementList.clear();
 *          elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
 *          elementList.encodeInit(encIter, null, 0);
 *          
 *          //Continue encoding element entries
 *          
 *          //Complete nested container encoding 
 *          retVal = encodeList.encodeComplete(encIter, success);
 *       }  
 *       retVal = seriesEntry.encodeComplete(encIter, success);
 *       
 *       //SECOND Series Entry: encode entry from pre-encoded buffer containing an encoded ElementList 
 *       /assuming encElementList Buffer contains the pre-encoded payload with data and length populated
 *       seriesEntry.encodedData().data(encElementList);
 *       retVal = seriesEntry.encode(encIter);
 * }
 * 
 *  //complete series encoding. If success parameter is true, this will finalize encoding.  
 *  //If success parameter is false, this will roll back encoding prior to series.encodeInit
 *  retVal = series.encodeComplete(encIter, success);
 * 
 * 
 * </pre>
 * 
 * </li>
 * </ul>
 * <p>
 * <b>Series Decoding Example</b>
 * <p>
 * The following sample illustrates how to decode a {@link Series} and is
 * structured to decode each entry to the contained value. The sample code
 * assumes the housed container type is an {@link ElementList}. Typically an
 * application invokes the specific container type decoder for the housed type
 * or uses a switch statement to allow for a more generic series entry decoder.
 * This example uses the same {@link DecodeIterator} when calling the content's
 * decoder method. An application could optionally use a new
 * {@link DecodeIterator} by setting encData on a new iterator. To simplify the
 * sample, some error handling is omitted.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * // decode contents into the series
 * retVal = series.decode(encIter);
 * if (retVal &gt;= CodecReturnCodes.SUCCESS)
 * {
 *     // create single series entry and reuse while decoding each entry
 *     SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
 * 
 *     // if summary data is present, invoking decoder for that type (instead of
 *     // DecodeEntry) indicates to ETA that user wants to decode summary data
 *     if ((series.flags() &amp; SeriesFlags.HAS_STANDARD_DATA) != 0)
 *     {
 *         // summary data is present. Its type should be that of
 *         // series.containerType
 *         ElementList elementList = CodecFactory.createElementList();
 *         retVal = elementList.decodeElementList(decIter, 0);
 * 
 *         // Continue decoding element entries. See example for ElementList
 *     }
 * 
 *     // decode each series entry until there are no more left
 *     while ((retVal = seriesEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
 *     {
 *         if (retVal &lt; CodecReturnCodes.SUCCESS)
 *         {
 *             // error condition - decoding failure tends to be unrecoverable
 *         }
 *         else
 *         {
 *             ElementList elementList = CodecFactory.createElementList();
 *             retVal = elementList.decodeElementList(decIter, 0);
 * 
 *             // Continue decoding element entries. See example for ElementList
 *         }
 *     }
 * }
 * else
 * {
 *     // error condition - decoding failure tends to be unrecoverable
 * }
 * 
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * @see SeriesEntry
 * @see SeriesFlags
 */
public interface Series extends XMLDecoder
{
    /**
     * Sets all members in {@link Series} to an initial value. Useful for object
     * reuse during encoding. While decoding, {@link Series} object can be
     * reused without using {@link #clear()}.
     */
    public void clear();

    /**
     * Prepares series for encoding.
     * 
     * Typical use:<BR>
     * 1. Call Series.encodeInit()<BR>
     * 2. Call SeriesEntry.encode() or SeriesEntry.encodeInit()..SeriesEntry.encodeComplete()
     *    for each entry using the same buffer<BR>
     * 3. Call Series.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param summaryMaxSize max encoding size of summary data, if present
     * @param setMaxSize max encoding size of the set data, if present
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize);

    /**
     * Complete set data encoding for a series. If both encodeSetDefsComplete()
     * and encodeSummaryDataComplete() are called, encodeSetDefsComplete() must be called first.
     * 
     * @param iter Encoding iterator
     * @param success True if encoding of set data was successful, false for rollback
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeSetDefsComplete(EncodeIterator iter, boolean success);

    /**
     * Complete summary data encoding for a series.
     * If both encodeSetDefsComplete() and encodeSummaryDataComplete() are called,
     * encodeSetDefsComplete() must be called first.
     * 
     * @param iter Encoding iterator
     * @param success True if encoding of summary data was successful, false for rollback
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeSummaryDataComplete(EncodeIterator iter, boolean success);

    /**
     * Completes series encoding.
     * 
     * Typical use:<BR>
     * 1. Call Series.encodeInit()<BR>
     * 2. Call SeriesEntry.encode() or SeriesEntry.encodeInit()..SeriesEntry.encodeComplete()
     *    for each entry using the same buffer<BR>
     * 3. Call Series.encodeComplete()<BR>
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
     * Decode Series.
     * 
     * @param iter decode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Checks the presence of the Set Definition presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist
     */
    public boolean checkHasSetDefs();

    /**
     * Checks the presence of the Summary Data presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist
     */
    public boolean checkHasSummaryData();

    /**
     * Checks the presence of the Total Count Hint presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist
     */
    public boolean checkHasTotalCountHint();

    /**
     * Applies the local Set Definition presence flag.
     *
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSetDefs();

    /**
     * Applies the Summary Data presence flag.
     *
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasSummaryData();

    /**
     * Applies the Total Count Hint presence flag.
     *
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasTotalCountHint();

    /**
     * The {@link DataTypes} enumeration value that describes the container type
     * of each {@link SeriesEntry}'s payload.
     * 
     * @return containerType
     */
    public int containerType();

    /**
     * The {@link DataTypes} enumeration value that describes the container type
     * of each {@link SeriesEntry}'s payload. containerType must be from the
     * {@link DataTypes} enumeration in the range {@link DataTypes#CONTAINER_TYPE_MIN}
     * to 255.
     * 
     * @param containerType the containerType to set.
     */
    public void containerType(int containerType);

    /**
     * Encoded set definitions, if any, contained in the message. If populated,
     * these definitions correspond to data contained within this {@link Series}
     * 's entries and are used for encoding or decoding their contents.
     * 
     * @return encodedSetDefs
     */
    public Buffer encodedSetDefs();

    /**
     * Sets encoded set definitions. If populated, these definitions correspond
     * to data contained within this {@link Series} 's entries and are used for
     * encoding or decoding their contents.
     * 
     * @param encodedSetDefs the encodedSetDefs to set
     */
    public void encodedSetDefs(Buffer encodedSetDefs);

    /**
     * Encoded summary data, if any, contained in the message. If populated,
     * summary data contains information that applies to every entry encoded in
     * the {@link Series} (e.g., currency type). The container type of summary
     * data should match the containerType specified on the {@link Series}.
     * 
     * @return encodedSummaryData
     */
    public Buffer encodedSummaryData();

    /**
     * Encoded summary data. If populated, summary data contains information
     * that applies to every entry encoded in the {@link Series} (e.g., currency
     * type). The container type of summary data should match the containerType
     * specified on the {@link Series}.
     * 
     * @param encodedSummaryData the encodedSummaryData to set
     */
    public void encodedSummaryData(Buffer encodedSummaryData);

    /**
     * Indicates an approximate total number of entries associated with this stream.
     * This is typically used when multiple {@link Series} containers are spread
     * across multiple parts of a refresh message. The totalCountHint provides an
     * approximation of the total number of entries sent across all series on all
     * parts of the refresh message. This information is useful when determining the
     * amount of resources to allocate for caching or displaying all expected entries.
     * 
     * @return totalCountHint
     */
    public int totalCountHint();

    /**
     * Indicates an approximate total number of entries associated with this stream.
     * This is typically used when multiple {@link Series} containers are spread
     * across multiple parts of a refresh message. The totalCountHint provides an
     * approximation of the total number of entries sent across all series on all
     * parts of the refresh message. This information is useful when determining the
     * amount of resources to allocate for caching or displaying all expected entries.
     * Must be in the range of 0 - 1073741823.
     * 
     * @param totalCountHint the totalCountHint to set
     */
    public void totalCountHint(int totalCountHint);

    /**
     * All encoded key-value pair encoded data, if any, contained in the message.
     * This refers to encoded {@link Series} payload data.
     * 
     * @return encodedEntries
     */
    public Buffer encodedEntries();

    /**
     * Sets all the flags applicable to this series.
     * Must be in the range of 0 - 255.
     * 
     * @param flags An integer containing all the flags applicable to this series
     * 
     * @see SeriesFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this series.
     * 
     * @return All the flags applicable to this series
     * 
     * @see SeriesFlags
     */
    public int flags();
}
