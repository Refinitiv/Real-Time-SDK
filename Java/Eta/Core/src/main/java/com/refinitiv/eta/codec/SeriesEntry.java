package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * An entry in a {@link Series} container that can house other container types
 * only. {@link Series} is a uniform type, where {@link Series}.containerType
 * indicates the single type housed in each entry. As entries are received, they
 * are appended to any previously received entries.
 * 
 * @see Series
 */
public interface SeriesEntry
{
    /**
     * Clears {@link SeriesEntry} object. Useful for object reuse during
     * encoding. While decoding, {@link SeriesEntry} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();

    /**
     * Encodes single series item, "moving" to the next row if necessary. Must
     * be called after encodeSetDefsComplete() and/or
     * encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Series.encodeInit()<BR>
     * 2. Call SeriesEntry.encode() for each entry using the same buffer<BR>
     * 3. Call Series.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Prepare a single series item for encoding. Must be called after
     * encodeSetDefsComplete() and/or encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Series.encodeInit()<BR>
     * 2. Call SeriesEntry.encodeInit()..SeriesEntry.encodeComplete() for each
     * entry using the same buffer<BR>
     * 3. Call Series.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator
     * @param maxEncodingSize max encoding size of the data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int maxEncodingSize);

    /**
     * Completes a series element encoding.
     * 
     * Typical use:<BR>
     * 1. Call Series.encodeInit()<BR>
     * 2. Call SeriesEntry.encodeInit()..SeriesEntry.encodeComplete() for each
     * entry using the same buffer<BR>
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
     * Decode a series row.
     * 
     * @param iter Decode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Encoded content of this SeriesEntry.
     * This refers to this encoded SeriesEntry's payload data.
     * 
     * @return encodedData
     */
    public Buffer encodedData();

    /**
     * Encoded content of this SeriesEntry. If set, this indicates that data is
     * pre-encoded and encodedData will be copied while encoding.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);
}