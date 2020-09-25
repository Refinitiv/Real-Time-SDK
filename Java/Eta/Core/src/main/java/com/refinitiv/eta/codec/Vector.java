package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * The {@link Vector} is a uniform container type of index-value pair entries.
 * Each entry, known as a {@link VectorEntry}, contains an index, correlating
 * to the entry's position in the information stream and value. A
 * {@link Vector} can contain zero to N entries (zero entries indicates an empty
 * {@link Vector}).
 * 
 * <p>
 * <b>Vector Encoding Example</b>
 * <p>
 * The following sample demonstrates how to encode a {@link Vector} containing
 * {@link Series} values. The example encodes three {@link VectorEntry} values
 * as well as summary data:
 * <ul>
 * <li>The first entry is encoded from an unencoded series</li>
 * <li>
 * The second entry is encoded from a buffer containing a pre-encoded series and
 * has perm data</li>
 * <li>The third is a clear action type with no payload.</li>
 * </ul>
 * This example demonstrates error handling for the initial encode method. To
 * simplify the example, additional error handling is omitted (though it should
 * be performed).
 * 
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * // populate vector structure prior to call to vector.encodeInit()
 * 
 * // indicate that summary data and a total count hint will be encoded
 * vector.flags(VectorFlags.HAS_SUMMRAY_DATA | VectorFlags.HAS_TOTAL_COUNT | VectorFlags.HAS_PER_ENTRY_PERM_DATA);
 * 
 * // populate containerType and total count hint
 * vector.containerType(DataTypes.SERIES);
 * vector.totalCountHint(3);
 * 
 * // begin encoding of vector - assumes that encIter is already populated with
 * // buffer and version information, store return value to determine success or
 * // failure
 * 
 * if ((retVal = vector.encodeInit(encIter, 50, 0)) &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition - switch our success value to false so we can roll back
 *     success = false;
 * }
 * else
 * {
 *     // vector init encoding was successful
 *     // create a single VectorEntry and Series and reuse for each entry
 *     VectorEntry vectorEntry = CodecFactory.createVectorEntry();
 *     Series series = CodecFactory.createSeries();
 * 
 *     // encode expected summary data, init for this was done by
 *     // vector.encodeInit - this type should match vector.containerType
 *     {
 *         // now encode nested container using its own specific encode methods
 *         // begin encoding of series - using same encIterator as vector
 * 
 *         retVal = series.encodeInit(encIter, 0, 0);
 *         {
 *             // ----- Continue encoding series entries, see Series encoding
 *             // example -----
 * 
 *             // Complete nested container encoding
 *             retVal = series.encodeComplete(encIter, success);
 *         }
 * 
 *         // complete encoding of summary data. If any series entry encoding
 *         // failed, success is false
 *         retVal = vector.encodeSummaryComplete(encIter, success);
 * 
 *         // FIRST Vector Entry: encode entry from unencoded data. Approx. encoded
 *         // length 90 bytes
 *         
 *         //populate index and action, no perm data on this entry
 *         vectorEntry.index(1);
 *         vectorEntry.flags(VectorEntryFlags.NONE);
 *         vectorEntry.action(VectorEntryActions.UPDATE);
 *         retVal = vectorEntry.encodeInit(encIter, 90);
 *         
 *         //encode contained series - this type should match vector.containerType
 *         {
 *              //now encode nested container using its own specific encode methods
 *              //clear, then begin encoding of series - using same encIterator as vector
 *              series.clear();
 *              retVal = series.encodeInit(encIter, 0, 0);
 *              
 *              //-----Continue encoding series entries. See example in Series ---
 *              retVal = series.encodeComplete(encIter, success);    
 *         }
 *         
 *         //SECOND Vector Entry: encode entry from pre-encoded buffer containing an encoded Series
 *         //assuming pEncSeries Buffer contains the pre-encoded payload with data and length populated
 *         //and pPermData contains permission data information
 *         
 *         vectorEntry.index(2);
 *         //by passing permData on an entry, the map encoding functionality will implicitly set the
 *         //Vector.HAS_PER_ENTRY_PERM flag
 *         vectorEntry.flags(VectorEntryFlags.HAS_PERM_DATA);
 *         vectorEntry.action(VectorEntryActions.SET);
 *         vectorEntry.permData(permData);
 *         vectorEntry.encodedData(encSeries);
 *         
 *         retVal = vectorEntry.encode(encIter);
 *         
 *         //THIRD Vector Entry: encode entry with clear action, no payload on clear
 *         //Should clear entry for safety, this will set flags to NONE 
 *         vectorEntry.clear();
 *         vectorEntry.index(3);
 *         vectorEntry.action(VectorEntryFlags.CLEAR);
 *         
 *         retVal = vectorEntry.encode(encIter);
 * }
 * 
 * //complete vector encoding.  If success parameter is true, this will finalize encoding.
 * //If success parameter is false, this will roll back encoding prior to vector.encodeInit();
 *  retVal = vector.encodeComplete(encIter, success);
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * <p>
 * <b>Vector decoding example</b>
 * <p>
 * The following sample illustrates how to decode a {@link Vector} and is
 * structured to decode each entry to the contained value. This sample code
 * assumes the housed container type is a {@link Series}. Typically an
 * application would invoke the specific container type decoder for the housed
 * type or use a switch statement to allow a more generic series entry decoder.
 * This example uses the same {@link DecodeIterator} when calling the content's
 * decoder method. Optionally, an application could use a new
 * {@link DecodeIterator} by setting the encData on a new iterator. To simplify
 * the sample, some error handling is omitted.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * {@code
 * //decode contents into the vector
 * if((retVal = vector.decode(decIter) >= CodecReturnCodes.SUCCESS)
 * {
 *      //create single vector entry and reuse while decoding each entry
 *      VectorEntry vectorEntry = CodecFactory.createVectory();
 *      
 *      //if summary data is present, invoking decoder for that type (instead of DecodeEntry)
 *      //indicates to UPA that user wants to decode summary data 
 *      if(vector.flags() & VectorFlags.HAS_SUMMARY_DATA)
 *      {
 *          //summary data is present.  Its type should be that of vector.containerType
 *          Series series = CodecFactory.createSeries();
 *          retVal = series.decode(decIter, series);
 *          
 *           //Continue decoding series entries, see Series decoding example
 *      }
 *      
 *      //decode each vector entry until there are no more left  
 *      while((retVal = vectorEntry.decode(decIter)) != CodecReturnCodes.END_OF_CONTAINER)
 *      {
 *          if(retVal <  CodecReturnCodes.SUCCESS)
 *          {
 *              //decoding failure tends to be unrecoverable 
 *          }
 *          else
 *          {
 *              Series series = CodecFactory.createSeries();
 *              retVal = series.decode(decIter);
 *              
 *              //Continue decoding series entries, see Series decoding example
 *          }
 *      }
 * }
 * else
 * {
 *      //decoding failure tends to be unrecoverable
 * }
 * }
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * @see VectorEntry
 * @see VectorFlags
 */
public interface Vector extends XMLDecoder
{
    /**
     * Sets all members in {@link Vector} to an initial value. Useful for object
     * reuse during encoding. While decoding, {@link Vector} object can be
     * reused without using {@link #clear()}.
     */
    public void clear();

    /**
     * Prepares vector for encoding.
     * 
     * Typical use:<BR>
     * 1. Call Vector.encodeInit()<BR>
     * 2. Call VectorEntry.encode() or
     * VectorEntry.encodeInit()..VectorEntry.encodeComplete() for each entry
     * using the same buffer<BR>
     * 3. Call Vector.encodeComplete()<BR>
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
     * Complete set data encoding for a vector. If both encodeSetDefsComplete()
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
     * Complete summary data encoding for a vector.
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
     * Completes vector encoding.
     * 
     * Typical use:<BR>
     * 1. Call Vector.encodeInit()<BR>
     * 2. Call VectorEntry.encode() or
     * VectorEntry.encodeInit()..VectorEntry.encodeComplete() for each entry
     * using the same buffer<BR>
     * 3. Call Vector.encodeComplete()<BR>
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
     * Decode Vector.
     * 
     * @param iter decode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Checks the presence of the local Set Definition presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if present; false - if not present
     */
    public boolean checkHasSetDefs();

    /**
     * Checks the presence of the Summary Data presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if present; false - if not present
     */
    public boolean checkHasSummaryData();

    /**
     * Checks the presence of the Per Entry Permission Data presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if present; false - if not present
     */
    public boolean checkHasPerEntryPermData();

    /**
     * Checks the presence of the Total Count Hint presence flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if present; false - if not present
     */
    public boolean checkHasTotalCountHint();

    /**
     * Checks the presence of the Supports Sorting indication flag.
     *
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if present; false - if not present
     */
    public boolean checkSupportsSorting();

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
     * Applies the Per Entry Permission Data presence flag.
     *
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasPerEntryPermData();

    /**
     * Applies the Total Count Hint presence flag.
     *
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasTotalCountHint();

    /**
     * Applies the Supports Sorting indication flag.
     *
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applySupportsSorting();

    /**
     * A {@link DataTypes} enumeration value that describes the container type
     * of each {@link VectorEntry}'s payload.
     * 
     * @return containerType
     */
    public int containerType();

    /**
     * A {@link DataTypes} enumeration value that describes the container type
     * of each {@link VectorEntry}'s payload. containerType must be from the
     * {@link DataTypes} enumeration in the range {@link DataTypes#CONTAINER_TYPE_MIN} to 255.
     * 
     * @param containerType the containerType to set
     */
    public void containerType(int containerType);

    /**
     * Encoded set definitions. If populated, these definitions correspond to
     * data contained within this Vector's entries and are used to encode or
     * decode their contents.
     * 
     * @return encodedSetDefs
     */
    public Buffer encodedSetDefs();

    /**
     * Encoded set definitions. If populated, these definitions correspond to
     * data contained within this Vector's entries and are used to encode or
     * decode their contents.
     * 
     * @param encodedSetDefs the encodedSetDefs to set
     */
    public void encodedSetDefs(Buffer encodedSetDefs);

    /**
     * Encoded summary data. If populated, summary data contains information
     * that applies to every entry encoded in the {@link Vector} (e.g. currency
     * type). The container type of summary data must match the containerType
     * specified on the {@link Vector}. If encodedSummaryData is populated while
     * encoding, contents are used as pre-encoded summary data.
     * 
     * @return encodedSummaryData
     */
    public Buffer encodedSummaryData();

    /**
     * Raw encoded summary data. If populated, summary data contains information
     * that applies to every entry encoded in the {@link Vector} (e.g. currency
     * type). The container type of summary data must match the containerType
     * specified on the {@link Vector}. If encodedSummaryData is populated while
     * encoding, contents are used as pre-encoded summary data.
     * 
     * @param encodedSummaryData the encodedSummaryData to set
     */
    public void encodedSummaryData(Buffer encodedSummaryData);

    /**
     * Indicates the approximate total number of entries sent across all vectors
     * on all parts of the refresh message. totalCountHint is typically used when
     * multiple {@link Vector} containers are spread across multiple parts of a
     * refresh message.Such information helps in determining the amount of resources
     * to allocate for caching or displaying all expected entries.
     * 
     * @return totalCountHint
     */
    public int totalCountHint();

    /**
     * Indicates the approximate total number of entries sent across all vectors
     * on all parts of the refresh message. totalCountHint is typically used when
     * multiple {@link Vector} containers are spread across multiple parts of a
     * refresh message.Such information helps in determining the amount of resources
     * to allocate for caching or displaying all expected entries.
     * Must be in the range of 0 - 1073741823.
     * 
     * @param totalCountHint the totalCountHint to set
     */
    public void totalCountHint(int totalCountHint);

    /**
     * Encoded index-value pair encoded data contained in the message.
     * This would refer to encoded {@link Vector} payload.
     * 
     * @return encodedEntries
     */
    public Buffer encodedEntries();

    /**
     * Sets all the flags applicable to this vector.
     * Must be in the range of 0 - 255.
     * 
     * @param flags An integer containing all the flags applicable to this vector
     * 
     * @see VectorFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this vector.
     * 
     * @return All the flags applicable to this vector
     * 
     * @see VectorFlags
     */
    public int flags();
}
