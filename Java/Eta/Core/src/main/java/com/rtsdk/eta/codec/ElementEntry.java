package com.rtsdk.eta.codec;

/**
 * An entry for UPA {@link ElementList} that can house any {@link DataTypes},
 * including primitive types, set-defined types, or container types.
 * If {@link ElementEntry} is a part of updating information and contains a
 * primitive type, any previously stored or displayed data is replaced.
 * If the {@link ElementEntry} contains another container type, action values
 * associated with that type indicate how to modify data.
 * 
 * @see ElementList
 */
public interface ElementEntry
{
    /**
     * Clears {@link ElementEntry} object. Useful for object reuse during
     * encoding. While decoding, {@link ElementEntry} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();

    /**
     * Encodes {@link ElementEntry} from pre-encoded data.
     * 
     * This method expects the same {@link EncodeIterator} that was used with
     * {@link ElementList#encodeInit(EncodeIterator, LocalElementSetDefDb, int)}
     * {@link ElementEntry#name()} and {@link ElementEntry#dataType()} must be
     * properly populated.
     * 
     * Set encodedData with pre-encoded data before calling this method.
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Encode a single element as blank.
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeBlank(EncodeIterator iter);

    /**
     * Encode a single element as {@link Int}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Int
     */
    public int encode(EncodeIterator iter, Int data);

    /**
     * Encode a single element as {@link UInt}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see UInt
     */
    public int encode(EncodeIterator iter, UInt data);

    /**
     * Encode a single element as {@link Float}.
     *
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see Float
     */
     public int encode(EncodeIterator iter, Float data);

     /**
     * Encode a single element as Double.
     *
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see Double
    */
    public int encode(EncodeIterator iter, Double data);

    /**
     * Encode a single element as {@link Real}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Real
     */
    public int encode(EncodeIterator iter, Real data);

    /**
     * Encode a single element as {@link Date}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Date
     */
    public int encode(EncodeIterator iter, Date data);

    /**
     * Encode a single element as {@link Time}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Time
     */
    public int encode(EncodeIterator iter, Time data);

    /**
     * Encode a single element as {@link DateTime}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see DateTime
     */
    public int encode(EncodeIterator iter, DateTime data);

    /**
     * Encode a single element as {@link Qos}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Qos
     */
    public int encode(EncodeIterator iter, Qos data);

    /**
     * Encode a single element as {@link State}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see State
     */
    public int encode(EncodeIterator iter, State data);

    /**
     * Encode a single element as {@link Enum}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Enum
     */
    public int encode(EncodeIterator iter, Enum data);

    /**
     * Encode a single element as {@link Buffer}.
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Buffer
     */
    public int encode(EncodeIterator iter, Buffer data);

    /**
     * Encodes an {@link ElementEntry} from a complex type, such as a container type or an array.
     * 
     * Used for aggregate elements, such as element lists.
     * 
     * Typical use:<BR>
     * 1. Call encodeInit()<BR>
     * 2. Call one or more encoding methods for the complex type using the
     * same buffer<BR>
     * 3. Call encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param encodingMaxSize max expected encoding size of element data
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int encodingMaxSize);

    /**
     * Completes encoding an {@link ElementEntry} for a complex type.
     * 
     * Typical use:<BR>
     * 1. Call encodeInit()<BR>
     * 2. Call one or more encoding methods for the complex type using the
     * same buffer<BR>
     * 3. Call encodeComplete()<BR>
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
     * Decode a single element.
     * 
     * @param iter The iterator used to parse the element list.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * A {@link Buffer} containing the name associated with this
     * {@link ElementEntry}. Element names are defined outside of UPA, typically
     * as part of a domain model specification or dictionary. It Is possible for
     * a name to be empty; however this provides no identifying information for
     * the element.
     * 
     * @param name the name to set
     */
    public void name(Buffer name);

    /**
     * A {@link Buffer} containing the name associated with this
     * {@link ElementEntry}. Element names are defined outside of UPA, typically
     * as part of a domain model specification or dictionary. It Is possible for
     * a name to be empty; however this provides no identifying information for
     * the element.
     * 
     * @return the name
     */
    public Buffer name();

    /**
     * Defines the {@link DataTypes} of this {@link ElementEntry}'s contents.
     * Must be in the range of {@link DataTypes#INT} - 255.
     * <ul>
     * <li>
     * While encoding, set this to the enumerated value of the target type.</li>
     * <li>
     * While decoding, dataType describes the type of contained data so that the
     * correct decoder can be used. If set-defined data is used, dataType will
     * indicate any specific {@link DataTypes} information as defined in the set
     * definition.</li>
     * </ul>
     * 
     * @param dataType the dataType to set
     */
    public void dataType(int dataType);

    /**
     * Defines the {@link DataTypes} of this {@link ElementEntry}'s contents.
     * <ul>
     * <li>
     * While encoding, set this to the enumerated value of the target type.</li>
     * <li>
     * While decoding, dataType describes the type of contained data so that the
     * correct decoder can be used. If set-defined data is used, dataType will
     * indicate any specific {@link DataTypes} information as defined in the set
     * definition.</li>
     * </ul>
     * 
     * @return the dataType
     */
    public int dataType();

    /**
     * Encoded content in this {@link ElementEntry}. If populated on encode
     * methods, this indicates that data is pre-encoded and encData copies
     * while encoding.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);

    /**
     * Encoded content in this {@link ElementEntry}.
     * While decoding, this refers to the encoded ElementEntry's payload.
     * 
     * @return the encodedData
     */
    public Buffer encodedData();
}