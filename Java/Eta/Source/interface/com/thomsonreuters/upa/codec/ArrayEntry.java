package com.thomsonreuters.upa.codec;

/**
 * UPA entry for {@link Array} that can house only simple primitive types
 * such as {@link Int}, {@link Real}, or {@link Date}.
 * 
 * @see Array
 */
public interface ArrayEntry
{
    /**
     * Clears {@link ArrayEntry} object. Useful for object reuse during encoding.
     * While decoding, {@link ArrayEntry} object can be reused without using {@link #clear()}.
     */
    public void clear();

    /**
     * Encodes an {@link ArrayEntry} from pre-encoded data. This method
     * expects the same {@link EncodeIterator} that was used with
     * {@link Array#encodeInit(EncodeIterator)}.
     * 
     * Typical use:<BR>
     * 1. Call Array.encodeInit()<BR>
     * 2. Call ArrayEntry.encode() for each entry using the same buffer<BR>
     * 3. Call Array.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Perform array item encoding of blank data.<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeBlank(EncodeIterator iter);

    /**
     * Perform array item encoding of an {@link Int}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Int}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Int
     */
    public int encode(EncodeIterator iter, Int data);

    /**
     * Perform array item encoding of a {@link UInt}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link UInt}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see UInt
     */
    public int encode(EncodeIterator iter, UInt data);

     /**
     * Perform array item encoding of a {@link Float}.<BR>
     *
     * @param iter The encoder iterator.
     * @param data The Float.
     *
     * @return {@link CodecReturnCodes}
     *
     * @see EncodeIterator
     * @see Float
     */
     public int encode(EncodeIterator iter, Float data);

      /**
      * Perform array item encoding of a {@link Double}.<BR>
      *
      * @param iter The encoder iterator.
      * @param data The Double.
      *
      * @return {@link CodecReturnCodes}
      *
      * @see EncodeIterator
      * @see Double
      */
      public int encode(EncodeIterator iter, Double data);

    /**
     * Perform array item encoding of a {@link Real}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Real}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Real
     */
    public int encode(EncodeIterator iter, Real data);

    /**
     * Perform array item encoding of a {@link Date}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Date}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Date
     */
    public int encode(EncodeIterator iter, Date data);

    /**
     * Perform array item encoding of a {@link Time}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Time}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Time
     */
    public int encode(EncodeIterator iter, Time data);

    /**
     * Perform array item encoding of a {@link DateTime}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link DateTime}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see DateTime
     */
    public int encode(EncodeIterator iter, DateTime data);

    /**
     * Perform array item encoding of a {@link Qos}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Qos}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Qos
     */
    public int encode(EncodeIterator iter, Qos data);

    /**
     * Perform array item encoding of a {@link State}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link State}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see State
     */
    public int encode(EncodeIterator iter, State data);

    /**
     * Perform array item encoding of an {@link Enum}.<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Enum}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Enum
     */
    public int encode(EncodeIterator iter, Enum data);

    /**
     * Perform array item encoding of a {@link Buffer}.<BR>
     * 
     * The length of the buffer should be the same as the itemLength of the array;
     * only the shorter of the two will be copied.
     * The entry is encoded as blank when the data is null or when the data length is 0.
     * 
     * @param iter The encoder iterator.
     * @param data The {@link Buffer}.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * @see Buffer
     */
    public int encode(EncodeIterator iter, Buffer data);

    /**
     * Decode an array entry.
     * 
     * @param iter Decode iterator
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Raw data contents of the array entry. Set by decode() method during
     * decode or set with encoded content and used with the encode(iter) method.
     * 
     * @return encodedData
     */
    public Buffer encodedData();

    /**
     * Raw data contents of the array entry.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);
}