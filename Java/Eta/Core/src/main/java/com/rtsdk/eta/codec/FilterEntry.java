package com.rtsdk.eta.codec;

import com.rtsdk.eta.codec.Buffer;

/**
 * An entry for a UPA {@link FilterList} that can house only other container
 * types. FilterList is a non-uniform type, where the
 * {@link FilterList#containerType()} should indicate the most common type
 * housed in each entry. Entries that differ from this type must specify their
 * own type via {@link FilterList#containerType()}.
 * 
 * @see FilterEntryActions
 * @see FilterEntryFlags
 * @see FilterList
 */
public interface FilterEntry
{
    /**
     * Clears {@link FilterEntry} object. Useful for object reuse during
     * encoding. While decoding, {@link FilterEntry} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();

    /**
     * Encodes single {@link FilterEntry} using the data provided.
     * 
     * Typical use:<BR>
     * 1. Call FilterList.encodeInit()<BR>
     * 2. Call FilterEntry.encode() for each FilterEntry in the list using the
     * previously encoded FilterEntry data<BR>
     * 3. Call FilterList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Prepares FilterEntry for encoding.
     * 
     * Typical use:<BR>
     * 1. Call FilterList.encodeInit()<BR>
     * For each row in the FilterEntry using the same buffer perform steps 2, 3,
     * and 4 below:
     * 2. Call FilterEntry.encodeInit() <BR>
     * 3. Encode FilterEntry contents <BR>
     * 4. Call FilterEntry.encodeComplete() <BR>
     * 5. Call FilterList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param maxEncodingSize Expected max encoding size of the FilterEntry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int maxEncodingSize);

    /**
     * Completes FilterEntry encoding.
     * 
     * Typical use:<BR>
     * 1. Call FilterList.encodeInit()<BR>
     * For each row in the FilterEntry using the same buffer perform steps 2, 3,
     * and 4 below
     * 2. Call FilterEntry.encodeInit() <BR>
     * 3. Encode FilterEntry contents <BR>
     * 4. Call FilterEntry.encodeComplete() <BR>
     * 5. Call FilterList.encodeComplete()<BR>
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
     * Decode a single FilterEntry.
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
     * @return true if exists, false if does not exist
     */
    public boolean checkHasPermData();

    /**
     * Checks the presence of the Container Type presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true if exists, false if does not exist
     */
    public boolean checkHasContainerType();

    /**
     * Applies the Permission Data presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasPermData();

    /**
     * Applies the Container Type presence flag.
     * 
     * <p>Flags may also be bulk-set via {@link #flags(int)}.
     * 
     * @see #flags(int)
     */
    public void applyHasContainerType();

    /**
     * FilterEntry action. Helps to manage change processing rules and informs
     * the consumer of how to apply the information contained in the entry.
     * 
     * @return the action
     */
    public int action();

    /**
     * FilterEntry action. Helps to manage change processing rules and informs
     * the consumer of how to apply the information contained in the entry.
     * Must be in the range of 0 - 15.
     * 
     * @param action the action to set
     */
    public void action(int action);

    /**
     * FilterEntry id. An ID associated with the entry. Each possible id
     * corresponds to a bit-value that can be used with the message key's filter
     * member. This bit-value can be specified on the filter to indicate
     * interest in the id when present in a {@link RequestMsg} or to indicate
     * presence of the id when present in other messages.
     * 
     * @return the id
     */
    public int id();

    /**
     * FilterEntry id. FilterEntry id. An ID associated with the entry. Each
     * possible id corresponds to a bit-value that can be used with the message
     * key's filter member. This bit-value can be specified on the filter to
     * indicate interest in the id when present in a {@link RequestMsg} or to
     * indicate presence of the id when present in other messages.
     * Must be in the range of 0 - 255.
     * 
     * @param id the id to set
     */
    public void id(int id);

    /**
     * A {@link DataTypes} enumeration value describing the type of this
     * FilterEntry. If present, the {@link FilterEntryFlags#HAS_CONTAINER_TYPE}
     * flag should be set by the user.
     * 
     * @return the containerType
     */
    public int containerType();

    /**
     * A {@link DataTypes} enumeration value describing the type of this
     * FilterEntr. containerType must be from the {@link DataTypes} enumeration
     * in the range {@link DataTypes#CONTAINER_TYPE_MIN} to 255.
     * 
     * @param containerType the containerType to set
     */
    public void containerType(int containerType);

    /**
     * Permission expression data. Permission expression for this FilterEntry.
     * Specifies authorization information for this entry.
     * 
     * @return the permData
     */
    public Buffer permData();

    /**
     * Permission expression data. Specifies authorization information for this entry.
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Raw data contents that is payload of the FilterEntry.
     * 
     * @return the encodedData
     */
    public Buffer encodedData();

    /**
     * Raw data contents that is payload of the FilterEntry.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);

    /**
     * Sets all the flags applicable to this filter entry. Must be in the range of 0 - 15.
     * 
     * @param flags An integer containing all the flags applicable to this filter entry
     * 
     * @see FilterEntryFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this filter entry.
     * 
     * @return All the flags applicable to this filter entry
     * 
     * @see FilterEntryFlags
     */
    public int flags();
}