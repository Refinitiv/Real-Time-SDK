package com.thomsonreuters.upa.codec;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * An entry for a UPA {@link FieldList} that can house any {@link DataTypes}
 * including primitive types, set-defined types, or container types.
 * If updating information, when the {@link FieldEntry} contains a primitive type, it
 * replaces any previously stored or displayed data associated with the same
 * fieldId. If the {@link FieldEntry} contains another container type, action
 * values associated with that type indicate how to modify the information.
 * 
 * @see FieldList
 */
public interface FieldEntry
{
    /**
     * Clears {@link FieldEntry} object. Useful for object reuse during
     * encoding. While decoding, {@link FieldEntry} object can be reused
     * without using {@link #clear()}.
     */
    public void clear();

    /**
     * Encodes a {@link FieldEntry} from pre-encoded data.
     * This method expects the same {@link EncodeIterator} that was used with
     * {@link FieldList#encodeInit(EncodeIterator, LocalFieldSetDefDb, int)}.
     * You must properly populate {@link FieldEntry#fieldId()} and {@link FieldEntry#dataType()}
     * 
     * Set encodedData with pre-encoded data before calling this method.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same
     * buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encode(EncodeIterator iter);

    /**
     * Encodes a blank field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encodeBlank() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeBlank(EncodeIterator iter);

    /**
     * Encodes a field within a list. The field data must cast to a primitive type.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param data The data to be encoded (must cast to a primitive type or an error is returned).
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     * 
     * @deprecated as of ETAJ3.0.0.L1.  Similar, but more type safe, functionality is provided by other FieldEntry interfaces	
     */
    public int encode(EncodeIterator iter, Object data);

    /**
     * Encodes an {@link Int} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link UInt} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Real} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Date} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Time} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link DateTime} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Qos} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link State} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes an {@link Enum} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Buffer} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Float} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link Double} field within a list.
     * 
     * Typical use:<BR>
     * 1. Call FieldList.encodeInit()<BR>
     * 2. Call FieldEntry.encode() for each field in the list using the same buffer<BR>
     * 3. Call FieldList.encodeComplete()<BR>
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
     * Encodes a {@link FieldEntry} from a complex type, such as a container
     * type or an array.
     * 
     * Used to put aggregate fields, such as element lists into a field list.
     * Typical use:<BR>
     * 1. Call encodeInit()<BR>
     * 2. Call one or more encoding methods for the complex type using the
     * same buffer<BR>
     * 3. Call encodeComplete()<BR>
     * 
     * @param iter The encoder iterator.
     * @param encodingMaxSize Max encoding size for field entry (If Unknown set to zero).
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see EncodeIterator
     */
    public int encodeInit(EncodeIterator iter, int encodingMaxSize);

    /**
     * Completes encoding a {@link FieldEntry} for a complex type.
     * 
     * Typical use:<BR>
     * 1. Call encodeInit()<BR>
     * 2. Call one or more encoding methods for the complex type using the same buffer<BR>
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
     * Decode a single field.
     * 
     * @param iter The iterator used to parse the field list.
     * 
     * @return {@link CodecReturnCodes}
     * 
     * @see DecodeIterator
     */
    public int decode(DecodeIterator iter);

    /**
     * The field identifier. Refers to specific name and type information defined
     * by an external field dictionary, such as the RDMFieldDictionary.
     * Negative fieldId values typically refer to user defined values while positive fieldId
     * values typically refer to Refinitiv defined values.
     * Must be in the range of -32768 - 32767.
     * 
     * @param fieldId the fieldId to set
     */
    public void fieldId(int fieldId);

    /**
     * The field identifier. Refers to specific name and type information defined
     * by an external field dictionary, such as the RDMFieldDictionary. Negative
     * fieldId values typically refer to user defined values while positive fieldId
     * values typically refer to Refinitiv defined values.
     * 
     * @return the fieldId
     */
    public int fieldId();

    /**
     * The field's data type. dataType must be from the {@link DataTypes}
     * enumeration. Must be in the range of {@link DataTypes#INT} - 255.
     * 
     * @param dataType the dataType to set
     */
    public void dataType(int dataType);

    /**
     * The field's data type. While decoding, if dataType is
     * {@link DataTypes#UNKNOWN}, the user must determine the type of contained
     * information from the associated field dictionary. If set-defined data is
     * used, dataType will indicate specific {@link DataTypes} information as
     * indicated by the set definition.
     * 
     * @return the dataType
     */
    public int dataType();

    /**
     * The encoded field data. Use to encode pre-encoded data.
     * 
     * @param encodedData the encodedData to set
     */
    public void encodedData(Buffer encodedData);

    /**
     * The encoded field data. Use to encode pre-encoded data.
     * 
     * @return the encodedData
     */
    public Buffer encodedData();
}
