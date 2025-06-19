/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// An entry for a ETA <see cref="FieldList"/> that can house any <see cref="DataTypes"/>
	/// including primitive types, set-defined types, or container types.
	/// </summary>
	/// <remarks>
    ///
	/// <para>If updating information, when the <see cref="FieldEntry"/> contains a primitive type, it
	/// replaces any previously stored or displayed data associated with the same
	/// fieldId.
	/// </para>
	///
	/// <para>If the <see cref="FieldEntry"/> contains another container type, action
	/// values associated with that type indicate how to modify the information.</para>
	/// </remarks>
	/// <seealso cref="FieldList"/>
	sealed public class FieldEntry
	{
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <see cref="FieldEntry"/>.
        /// </summary>
        /// <seealso cref="FieldEntry"/>
        public FieldEntry()
        {
        }

		/// <summary>
		/// Clears <see cref="FieldEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <see cref="FieldEntry"/> object can be reused
		/// without using <see cref="FieldEntry.Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			FieldId = 0;
			DataType = 0;
			_encodedData.Clear();
        }

        /// <summary>
        /// Encodes a <see cref="FieldEntry"/> from pre-encoded data.
        /// </summary>
        /// <remarks>
        ///
        /// <para>This method expects the same <see cref="EncodeIterator"/> that was used with
        /// <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/>.
        /// You must properly populate <see cref="FieldEntry.FieldId"/> and <see cref="FieldEntry.DataType"/>
		/// </para>
		/// 
        /// <para>Set <see cref="EncodedData"/> with pre-encoded data before calling this method.
		/// </para>
        /// 
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
        /// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
        /// <item> Call <see cref="FieldList.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeFieldEntry(iter, this, null);
		}

		/// <summary>
		/// Encodes a blank field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call <see cref="EncodeBlank(EncodeIterator)"/> for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeBlank(EncodeIterator iter)
		{
			_encodedData.Clear();

			return Encoders.EncodeFieldEntry(iter, this, null);
		}

		/// <summary>
		/// Encodes an <see cref="Int"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Int"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Int data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="UInt"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="UInt"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, UInt data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Real"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Real"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Real data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Date"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Date"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Date data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Time"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Time"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Time data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="DateTime"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="DateTime"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, DateTime data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Qos"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Qos"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Qos data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="State"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="State"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, State data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes an <see cref="Enum"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Enum"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Enum data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Buffer"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Buffer"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Buffer data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Float"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Float"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Float data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="Double"/> field within a list.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call FieldEntry.Encode() for each field in the list using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// </list>
        /// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="data"> The data to be encoded.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="EncodeIterator"/>
		/// <seealso cref="Double"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Double data)
		{
			return Encoders.EncodeFieldEntry(iter, this, data);
		}

		/// <summary>
		/// Encodes a <see cref="FieldEntry"/> from a complex type, such as a container
		/// type or an array.
		/// </summary>
		/// <remarks>
		///
		/// <para>Used to put aggregate fields, such as element lists into a field list.</para>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call one or more encoding methods for the complex type using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="encodingMaxSize"> Max encoding size for field entry (If Unknown set to zero).
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, int encodingMaxSize)
		{
			return Encoders.EncodeFieldEntryInit(iter, this, encodingMaxSize);
		}

		/// <summary>
		/// Completes encoding a <see cref="FieldEntry"/> for a complex type.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FieldList.EncodeInit(EncodeIterator, LocalFieldSetDefDb, int)"/></item>
		/// <item> Call one or more encoding methods for the complex type using the same buffer</item>
		/// <item> Call <see cref="FieldList.EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If <c>true</c> - successfully complete the aggregate,
		///            if <c>false</c> - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeFieldEntryComplete(iter, success);
		}

		/// <summary>
		/// Decode a single field.
		/// </summary>
		/// <param name="iter"> The iterator used to parse the field list.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeFieldEntry(iter, this);
		}

        /// <summary>
		/// Field identifier refers to specific name and type information defined
		/// by an external field dictionary, such as the RDMFieldDictionary.
		/// </summary>
		/// <remarks>
		/// <para>
		/// Negative fieldId values typically refer to user defined values while positive <c>FieldId</c>
		/// values typically refer to LSEG defined values.</para>
		/// <para>
		/// Must be in the range of -32768 - 32767.</para>
		/// </remarks>
		public int FieldId { get; set; }

        /// <summary>
        /// Gets or sets the field's data type. dataType must be from the <see cref="DataTypes"/>
        /// enumeration. Must be in the range of <see cref="DataTypes.INT"/> - 255.
        /// </summary>
        public int DataType { get; set; }

        /// <summary>
		/// Gets or sets the encoded field data. Use to encode pre-encoded data.
		/// </summary>
		/// <returns> the encodedData
		/// </returns>
		public Buffer EncodedData
		{
            get
            {
                return _encodedData;
            }

            set
            {
                _encodedData.CopyReferences(value);
            }
		}
	}
}
