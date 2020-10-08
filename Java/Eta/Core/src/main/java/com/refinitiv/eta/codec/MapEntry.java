package com.refinitiv.eta.codec;

import com.refinitiv.eta.codec.Buffer;

/**
 * An entry for a ETA {@link Map} that can house only other Container Types.
 * 
 * {@link Map} is a uniform type, where the {@link Map#containerType()}
 * indicates the single type housed in each entry. Each entry has an associated
 * action which informs the user of how to apply the information contained in the entry.
 * 
 * @see Map
 * @see MapEntryFlags
 * @see MapEntryActions
 */
public interface MapEntry
{
    /**
     * Clears {@link FieldEntry} object. Useful for object reuse during
     * encoding. While decoding, {@link FieldEntry} object can be reused without
     * using {@link #clear()}.
     */
    public void clear();

    /**
     * Encode a single map entry with pre-encoded primitive key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Set encodedKey with pre-encoded data before calling this method.
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Encode a single map entry with {@link Int} key. Must be called after
     * Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Int} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Int
     */
    public int encode(EncodeIterator iter, Int keyData);

    /**
     * Encode a single map entry with {@link UInt} key. Must be called after
     * Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link UInt} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see UInt
     */
    public int encode(EncodeIterator iter, UInt keyData);

    /**
     * Encode a single map entry with Float key. Must be called after
     * Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The Float key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Float
     */
    public int encode(EncodeIterator iter, Float keyData);

    /**
     * Encode a single map entry with Double key. Must be called after
     * Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     *
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     *
     * @param iter The encoder iterator.
     * @param keyData The Double key.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see Double
     */
     public int encode(EncodeIterator iter, Double keyData);

    /**
     * Encode a single map entry with Real key. Must be called after
     * Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The Real key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Real
     */
    public int encode(EncodeIterator iter, Real keyData);

    /**
     * Encode a single map entry with {@link Date} key. Must be called after
     * Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Date} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Date
     */
    public int encode(EncodeIterator iter, Date keyData);

    /**
     * Encode a single map entry with {@link Time} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Time} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Time
     */
    public int encode(EncodeIterator iter, Time keyData);

    /**
     * Encode a single map entry with {@link DateTime} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link DateTime} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see DateTime
     */
    public int encode(EncodeIterator iter, DateTime keyData);

    /**
     * Encode a single map entry with {@link Qos} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Qos} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Qos
     */
    public int encode(EncodeIterator iter, Qos keyData);

    /**
     * Encode a single map entry with {@link State} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link State} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see State
     */
    public int encode(EncodeIterator iter, State keyData);

    /**
     * Encode a single map entry with {@link Enum} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Enum} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Enum
     */
    public int encode(EncodeIterator iter, Enum keyData);

    /**
     * Encode a single map entry with {@link Buffer} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Buffer} key.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Buffer
     */
    public int encode(EncodeIterator iter, Buffer keyData);

    /**
     * Initialize Map Entry encoding with pre-encoded primitive key.
     * Must be called after Map.encodeSetDefsComplete() and/or
     * Map.encodeSummaryDataComplete().
     * 
     * Set encodedKey with pre-encoded data before calling this method.
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Int} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Int} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Int
     */
    public int encodeInit(EncodeIterator iter, Int keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link UInt} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link UInt} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see UInt
     */
    public int encodeInit(EncodeIterator iter, UInt keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with Float key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     *
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     *
     * @param iter The encoder iterator.
     * @param keyData The Float key.
     * @param maxEncodingSize max encoding size of map entry data
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see Float
     */
     public int encodeInit(EncodeIterator iter, Float keyData, int maxEncodingSize);

     /**
     * Initialize Map Entry encoding with Double key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     *
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     *
     * @param iter The encoder iterator.
     * @param keyData The Double key.
     * @param maxEncodingSize max encoding size of map entry data
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see Double
     */
     public int encodeInit(EncodeIterator iter, Double keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Real} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Real} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Real
     */
    public int encodeInit(EncodeIterator iter, Real keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Date} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Date} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Date
     */
    public int encodeInit(EncodeIterator iter, Date keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Time} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Time} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Time
     */
    public int encodeInit(EncodeIterator iter, Time keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link DateTime} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link DateTime} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see DateTime
     */
    public int encodeInit(EncodeIterator iter, DateTime keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Qos} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Qos} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Qos
     */
    public int encodeInit(EncodeIterator iter, Qos keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link State} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link State} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see State
     */
    public int encodeInit(EncodeIterator iter, State keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Enum} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Enum} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Enum
     */
    public int encodeInit(EncodeIterator iter, Enum keyData, int maxEncodingSize);

    /**
     * Initialize Map Entry encoding with {@link Buffer} key.
     * Must be called after Map.encodeSetDefsComplete() and/or Map.encodeSummaryDataComplete().
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encode() for each map entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param keyData The {@link Buffer} key.
     * @param maxEncodingSize max encoding size of map entry data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Buffer
     */
    public int encodeInit(EncodeIterator iter, Buffer keyData, int maxEncodingSize);

    /**
     * Completes Map Entry encoding.
     * 
     * Typical use:<BR>
     * 1. Call Map.encodeInit()<BR>
     * 2. Call MapEntry.encodeInit()..MapEntry.encodeComplete() for each map
     * entry in the list using the same buffer<BR>
     * 3. Call Map.encodeComplete()<BR>
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
     * Decode a single map entry.
     * 
     * @param iter Decode iterator
     * @param keyData The decoded key data. If the user provides this pointer,
     *            this method will automatically decode the key to it
     *            (the pointer MUST point to memory large enough to contain the
     *            primitive that will be written).
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter, Object keyData);

    /**
     * Checks the presence of the Permission Data presence flag.
     * 
     * <p>Flags may also be bulk-get via {@link #flags()}.
     * 
     * @see #flags()
     * 
     * @return true - if exists; false if does not exist.
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
     * The entry action helps to manage change processing rules and tells the
     * consumer how to apply the information contained in the entry.
     * For specific information about possible action's associated with a MapEntry,
     * see {@link MapEntryActions}.
     * Must be in the range of 0 - 15.
     * 
     * @param action the action to set
     */
    public void action(int action);

    /**
     * The entry action helps to manage change processing rules and tells the
     * consumer how to apply the information contained in the entry.
     * For specific information about possible action's associated with a MapEntry,
     * see {@link MapEntryActions}.
     * 
     * @return the action
     */
    public int action();

    /**
     * Specifies authorization information for this specific entry.
     * If present {@link MapEntryFlags#HAS_PERM_DATA} should be set.
     * 
     * @return permData
     */
    public Buffer permData();

    /**
     * Specifies authorization information for this specific entry.
     * If present {@link MapEntryFlags#HAS_PERM_DATA} should be set.
     * 
     * @param permData the permData to set
     */
    public void permData(Buffer permData);

    /**
     * Encoded map entry key information. The encoded type of the key
     * corresponds to the Map's keyPrimitiveType. The key value must be a base
     * primitive type and cannot be blank, {@link DataTypes#ARRAY}, or
     * {@link DataTypes#UNKNOWN} primitive types.
     * 
     * @return encodedKey
     */
    public Buffer encodedKey();

    /**
     * Encoded map entry key information. The encoded type of the key
     * corresponds to the Map's keyPrimitiveType. The key value must be a base
     * primitive type and cannot be blank, {@link DataTypes#ARRAY}, or
     * {@link DataTypes#UNKNOWN} primitive types.If populated on encode
     * methods, this indicates that the key is pre-encoded and encKey will be
     * copied while encoding.
     * 
     * @param encodedKey the encodedKey to set
     */
    public void encodedKey(Buffer encodedKey);

    /**
     * The encoded content of this MapEntry.
     * 
     * @return encodedData
     */
    public Buffer encodedData();

    /**
     * The encoded content of this MapEntry. If populated on encode methods,
     * this indicates that data is pre-encoded, and encData will be copied while encoding.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);

    /**
     * Sets all the flags applicable to this map entry.
     * Must be in the range of 0 - 15.
     * 
     * @param flags An integer containing all the flags applicable to this map entry
     * 
     * @see MapEntryFlags
     */
    public void flags(int flags);

    /**
     * Returns all the flags applicable to this map entry.
     * 
     * @return All the flags applicable to this map entry
     * 
     * @see MapEntryFlags
     */
    public int flags();
}