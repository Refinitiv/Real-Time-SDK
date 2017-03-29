package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * A UPA Array is a uniform primitive type that can contain 0 to N simple primitive entries,u
 * where zero entries indicates an empty Array.
 * <p>
 * Each Array entry can house only simple primitive types such as {@link Int},
 * {@link Real}, or {@link Date}. An array entry cannot house any container
 * types or other {@link Array} types. This is a uniform type, where
 * {@link Array#primitiveType()} indicates the single, simple primitive type of
 * each entry.
 * 
 * <p>
 * Array uses simple replacement rules for change management. When new entries
 * are added, or any array entry requires a modification, all entries must be
 * sent with the {@link Array}. This new {@link Array} entirely replaces any
 * previously stored or displayed data.
 * 
 * <p>
 * An Array entry can be encoded from pre-encoded data or by encoding individual
 * pieces of data as provided. The {@link Array} does not use a specific entry
 * structure. When encoding, the application passes a pointer to the primitive
 * type value (when data is not encoded) or a Buffer (containing the
 * pre-encoded primitive).
 * 
 * <p>
 * <b>Example: Encoding of Array</b>
 * <p>
 * The following code example demonstrates how to encode an {@link Array}. The
 * array is set to encode unsigned integer entries, where the entries have a
 * fixed length of two bytes each. The example encodes two array entries. The
 * first entry is encoded from a primitive {@link UInt} type; the second entry
 * is encoded from a {@link Buffer} containing a pre-encoded {@link UInt} type.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * int retval;
 * // Create Array using CodecFactory
 * Array array = CodecFactory.createArray();
 * 
 * // Populate array prior to calling encodeInit()
 * array.primitiveType(DataTypes.UINT);
 * array.itemLength(2);
 * 
 * if (array.encodeInit(encIter) &lt; CodecReturnCodes.SUCCESS)
 * {
 *     // error condition
 * }
 * else
 * {
 *     // encode first entry from a UInt from a primitive type
 *     retval = uint.encode(encIter);
 * 
 *     // encode second entry from a UInt from pre-encoded integer contained in a
 *     // buffer. This buffer.data should point to encoded int and the length
 *     // should be number of bytes encoded.
 *     retval = preencodedUint.encode(encIter);
 * }
 * 
 * // Complete array encoding. If success parameter is true, this will finalize
 * // encoding.
 * // If success parameter is false, this will roll back encoding prior to
 * // encodeArrayInit
 * 
 * retval = array.encodeComplete(encIter, success);
 * </pre>
 * 
 * </li>
 * </ul>
 * 
 * 
 * <p>
 * <b>Example: Decoding of Array</b>
 * <p>
 * The following example decodes an {@link Array} and each of its entries to the
 * primitive value. This sample code assumes the contained primitive type is a
 * {@link UInt}. Typically an application invokes the specific primitive decoder
 * for the contained type or uses a switch statement to allow for a more generic
 * array entry decoder. This example uses the same {@link DecodeIterator} when
 * calling the primitive decoder method. An application could optionally use a
 * new {@link DecodeIterator} by setting the encoded entry buffer on a new
 * iterator. To simplify the example, some error handling is omitted.
 * 
 * <ul class="blockList">
 * <li class="blockList">
 * 
 * <pre>
 * {@code
 * int retval;
 * 
 * //decode array
 * if (array.decode(decIter) >= CodecReturnCodes.SUCCESS)
 * {
 *    //decode array entry
 *    while((retval = arrayentry.decode(decIter, entrybuffer)) != CodecReturnCodes.END_OF_CONTAINER)
 *    {
 *          if(retval < CodecReturnCodes.SUCCESS)
 *          {
 *              //decoding failure tends to be unrecoverable
 *          }
 *          else
 *          {
 *              // decode array entry into primitive type 
 *              // we can use the same decode iterator, or set the encoded
 *              // entry buffer onto a new iterator 
 *              retval = uInt.decode(decIter);
 *          }
 *    }
 * }
 * 
 * }
 * </pre>
 * 
 * </li>
 * </ul>
 */
public interface Array extends XMLDecoder
{
    /**
     * Sets all members in {@link Array} to an initial value. Useful for object
     * reuse during encoding. While decoding, {@link Array} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();
    
    /**
     * Is Array blank.
     */
    public boolean isBlank();

    /**
     * Prepares array for encoding.
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
    public int encodeInit(EncodeIterator iter);

    /**
     * Completes array encoding.
     * 
     * Typical use:<BR>
     * 1. Call Array.encodeInit()<BR>
     * 2. Call ArrayEntry.encode() for each entry using the same buffer<BR>
     * 3. Call Array.encodeComplete()<BR>
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
     * Decode an array.
     * 
     * @param iter The decoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * Primitive type for all items in the array.
     * primitiveType must be from the {@link DataTypes} enumeration.
     * 
     * @return the primitiveType
     * 
     * @see DataTypes
     */
    public int primitiveType();

    /**
     * Primitive type for all items in the array. primitiveType must be from the
     * {@link DataTypes} enumeration and must be greater than {@link DataTypes#UNKNOWN}
     * and less than or equals to {@link DataTypes#BASE_PRIMITIVE_MAX}.
     * primitiveType cannot be {@link DataTypes#ARRAY}.
     * 
     * @param primitiveType the primitiveType to set
     * 
     * @see DataTypes
     */
    public void primitiveType(int primitiveType);

    /**
     * If items are fixed length populate length here - otherwise make 0 for
     * length specified item encoding.
     * 
     * @return the itemLength
     */
    public int itemLength();

    /**
     * If items are fixed length populate length here - otherwise make 0 for
     * length specified item encoding. Must be in the range of 0 - 65535.
     * 
     * @param itemLength the itemLength to set
     */
    public void itemLength(int itemLength);

    /**
     * Raw data contents of the array. Set by decode() method during decode.
     * 
     * @return the encodedData
     * 
     * @see Buffer
     */
    public Buffer encodedData();
}