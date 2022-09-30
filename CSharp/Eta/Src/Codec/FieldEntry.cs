/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Codec
{
    /// <summary>
	/// An entry for a ETA <seealso cref="FieldList"/> that can house any <seealso cref="DataTypes"/>
	/// including primitive types, set-defined types, or container types.
	/// If updating information, when the <seealso cref="FieldEntry"/> contains a primitive type, it
	/// replaces any previously stored or displayed data associated with the same
	/// fieldId. If the <seealso cref="FieldEntry"/> contains another container type, action
	/// values associated with that type indicate how to modify the information.
	/// </summary>
	/// <seealso cref="FieldList"/>
	sealed public class FieldEntry
	{
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <seealso cref="FieldEntry"/>.
        /// </summary>
        /// <returns> FieldEntry object
        /// </returns>
        /// <seealso cref="FieldEntry"/>
        public FieldEntry()
        {
        }

        /// <summary>
		/// Clears <seealso cref="FieldEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <seealso cref="FieldEntry"/> object can be reused
		/// without using <seealso cref="FieldEntry.Clear()"/>.
		/// </summary>
		public void Clear()
		{
			FieldId = 0;
			DataType = 0;
			_encodedData.Clear();
		}

        /// <summary>
		/// Encodes a <seealso cref="FieldEntry"/> from pre-encoded data.
		/// This method expects the same <seealso cref="EncodeIterator"/> that was used with
		/// <seealso cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/>.
		/// You must properly populate <seealso cref="FieldEntry.FieldId"/> and <seealso cref="FieldEntry.DataType"/>
		/// 
		/// Set encodedData with pre-encoded data before calling this method.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same
		/// buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeFieldEntry(iter, this, null);
		}

        /// <summary>
		/// Encodes a blank field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.EncodeBlank() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <seealso cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeBlank(EncodeIterator iter)
		{
			_encodedData.Clear();

			return Encoders.EncodeFieldEntry(iter, this, null);
		}

        /// <summary>
        /// Encodes an <seealso cref="Int"/> field within a list.
        /// 
        /// Typical use:<para />
        /// 1. Call FieldList.EncodeInit()<para />
        /// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
        /// 3. Call FieldList.EncodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Int"/>
        public CodecReturnCode Encode(EncodeIterator iter, Int data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
        /// Encodes a <seealso cref="UInt"/> field within a list.
        /// 
        /// Typical use:<para />
        /// 1. Call FieldList.EncodeInit()<para />
        /// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
        /// 3. Call FieldList.EncodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <seealso cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="UInt"/>
        public CodecReturnCode Encode(EncodeIterator iter, UInt data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="Real"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Real"/>
		public CodecReturnCode Encode(EncodeIterator iter, Real data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="Date"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Date"/>
		public CodecReturnCode Encode(EncodeIterator iter, Date data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="Time"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Time"/>
		public CodecReturnCode Encode(EncodeIterator iter, Time data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
        /// Encodes a <see cref="DateTime"/> field within a list.
        /// 
        /// Typical use:<para />
        /// 1. Call FieldList.EncodeInit()<para />
        /// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
        /// 3. Call FieldList.EncodeComplete()<para />
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="DateTime"/>
        public CodecReturnCode Encode(EncodeIterator iter, DateTime data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <seealso cref="Qos"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Qos"/>
		public CodecReturnCode Encode(EncodeIterator iter, Qos data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="State"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="State"/>
		public CodecReturnCode Encode(EncodeIterator iter, State data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes an <seealso cref="Enum"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Enum"/>
		public CodecReturnCode Encode(EncodeIterator iter, Enum data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <seealso cref="Buffer"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Buffer"/>
		public CodecReturnCode Encode(EncodeIterator iter, Buffer data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="Float"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Float"/>
		public CodecReturnCode Encode(EncodeIterator iter, Float data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="Double"/> field within a list.
		/// 
		/// Typical use:<para />
		/// 1. Call FieldList.EncodeInit()<para />
		/// 2. Call FieldEntry.Encode() for each field in the list using the same buffer<para />
		/// 3. Call FieldList.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Double"/>
		public CodecReturnCode Encode(EncodeIterator iter, Double data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

        /// <summary>
		/// Encodes a <see cref="FieldEntry"/> from a complex type, such as a container
		/// type or an array.
		/// 
		/// Used to put aggregate fields, such as element lists into a field list.
		/// Typical use:<para />
		/// 1. Call EncodeInit()<para />
		/// 2. Call one or more encoding methods for the complex type using the
		/// same buffer<para />
		/// 3. Call EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="encodingMaxSize"> Max encoding size for field entry (If Unknown set to zero).
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeInit(EncodeIterator iter, int encodingMaxSize)
		{
			return Encoders.EncodeFieldEntryInit(iter, this, encodingMaxSize);
		}

        /// <summary>
		/// Completes encoding a <see cref="FieldEntry"/> for a complex type.
		/// 
		/// Typical use:<para />
		/// 1. Call EncodeInit()<para />
		/// 2. Call one or more encoding methods for the complex type using the same buffer<para />
		/// 3. Call EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If true - successfully complete the aggregate, if false -
		///            remove the aggregate from the buffer.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeFieldEntryComplete(iter, success);
		}

        /// <summary>
		/// Decode a single field.
		/// </summary>
		/// <param name="iter"> The iterator used to parse the field list.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeFieldEntry(iter, this);
		}

        /// <summary>
		/// Gets or sets the field identifier. Refers to specific name and type information defined
		/// by an external field dictionary, such as the RDMFieldDictionary.
		/// Negative fieldId values typically refer to user defined values while positive fieldId
		/// values typically refer to Thomson Reuters defined values.
		/// Must be in the range of -32768 - 32767.
		/// </summary>
		public int FieldId { get; set; }

        /// <summary>
        /// Gets or sets the field's data type. dataType must be from the <see cref="DataTypes"/>
        /// enumeration. Must be in the range of <see cref="DataTypes.INT"/> - 255.
        /// </summary>
        public int DataType { get; set; }

        /// <summary>
		/// Gets or sets the encoded field data. Use to encode pre-encoded data.
		/// </summary>
		/// <returns> the encodedData </returns>
		public Buffer EncodedData
		{
            get
            {
                return _encodedData;
            }

            set
            {
                (_encodedData).CopyReferences(value);
            }
		}
	}

}