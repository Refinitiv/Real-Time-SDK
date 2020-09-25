package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * An entry for the UPA {@link Vector} that can house other Container Types
 * only. {@link Vector} is a uniform type, where {@link Vector#containerType()}
 * indicates the single type housed in each entry. Each entry has an associated
 * action which informs the user of how to apply the data contained in the entry
 * 
 * @see VectorEntryActions
 * @see VectorEntryFlags
 * @see Vector
 */
public interface VectorEntry
{
    /**
     * Clears {@link VectorEntry} object. Useful for object reuse during
     * encoding. While decoding, {@link VectorEntry} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();

    /**
     * Encodes single vector item, "moving" to the next row if necessary. Must
     * be called after encodeSetDefsComplete() and/or
     * encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Vector.encodeInit()<BR>
     * 2. Call VectorEntry.encode() for each entry using the same buffer<BR>
     * 3. Call Vector.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Prepare a single vector item for encoding. Must be called after
     * encodeSetDefsComplete() and/or encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Vector.encodeInit()<BR>
     * 2. Call VectorEntry.encodeInit()..VectorEntry.encodeComplete() for each
     * entry using the same buffer<BR>
     * 3. Call Vector.encodeComplete()<BR>
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
     * Completes a vector element encoding.
     * 
     * Typical use:<BR>
     * 1. Call Vector.encodeInit()<BR>
     * 2. Call VectorEntry.encodeInit()..VectorEntry.encodeComplete() for each
     * entry using the same buffer<BR>
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
     * Decode a vector row.
     * 
     * @param iter Decode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Checks the presence of the Permission Data presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if present; false - if not present
     */
    public boolean checkHasPermData();

    /**
     * Applies the Permission Data presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasPermData();

    /**
     * Vector action (helps to manage change processing rules and informs the
     * consumer of how to apply the information contained in the entry).
     * 
     * @return the action
     */
    public int action();

    /**
     * Vector action (helps to manage change processing rules and informs the
     * consumer of how to apply the information contained in the entry).
     * Must be in the range of 0 - 15.
     * 
     * @param action the action to set
     */
    public void action(int action);

    /**
     * 0-base entry index.
     * 
     * @return the index
     */
    public long index();

    /**
     * 0-base entry index. Must be in the range of 0 - 1073741823.
     * 
     * @param index the index to set
     */
    public void index(long index);

    /**
     * Permission expression for this entry.
     * 
     * @return the permData
     */
    public Buffer permData();

    /**
     * Permission expression for this entry.
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Data to be applied for the entry.
     * 
     * @return encodedData
     */
    public Buffer encodedData();

    /**
     * Data to be applied for the entry.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);

    /**
     * Sets all the flags applicable to this vector entry
     * 
     * @param flags An integer containing all the flags applicable to this vector entry
     * 
     * @see VectorEntryFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this vector entry
     * 
     * @return All the flags applicable to this vector entry
     * 
     * @see VectorEntryFlags
     */
    public int flags();
}
