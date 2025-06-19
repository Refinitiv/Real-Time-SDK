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
	/// An entry for ETA <see cref="ElementList"/> that can house any <see cref="DataTypes"/>,
	/// including primitive types, set-defined types, or container types.
	/// </summary>
	/// 
	/// <remarks>
	/// If <see cref="ElementEntry"/> is a part of updating information and contains a
	/// primitive type, any previously stored or displayed data is replaced.
	/// If the <see cref="ElementEntry"/> contains another container type, action values
	/// associated with that type indicate how to modify data.
	/// </remarks>
	/// 
	/// <seealso cref="DataTypes"/>
	/// <seealso cref="ElementList"/>
	sealed public class ElementEntry
	{
		internal readonly Buffer _name = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <see cref="ElementEntry"/>.
        /// </summary>
        /// <seealso cref="ElementEntry"/>
        public ElementEntry()
        {
        }

		/// <summary>
		/// Clears <see cref="ElementEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <see cref="ElementEntry"/> object can be reused
		/// without using <see cref="ElementEntry.Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_name.Clear();
			DataType = 0;
			_encodedData.Clear();
		}

        /// <summary>
        /// Encodes <see cref="ElementEntry"/> from pre-encoded data.
        /// </summary>
		/// <remarks>
        /// <para>This method expects the same <see cref="EncodeIterator"/> that was used with
        /// <see cref="ElementList.EncodeInit(EncodeIterator, LocalElementSetDefDb, int)"/>
        /// <see cref="ElementEntry.Name"/> and <see cref="ElementEntry.DataType"/> must be
        /// properly populated.</para>
        /// 
        /// <para>Set encodedData with pre-encoded data before calling this method.</para>
        /// </remarks>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
		/// <seealso cref="Encode(EncodeIterator, Buffer)"/>
		/// <seealso cref="Encode(EncodeIterator, Date)"/>
		/// <seealso cref="Encode(EncodeIterator, DateTime)"/>
		/// <seealso cref="Encode(EncodeIterator, Double)"/>
		/// <seealso cref="Encode(EncodeIterator, Enum)"/>
		/// <seealso cref="Encode(EncodeIterator, Float)"/>
		/// <seealso cref="Encode(EncodeIterator, Int)"/>
		/// <seealso cref="Encode(EncodeIterator, Qos)"/>
		/// <seealso cref="Encode(EncodeIterator, Real)"/>
		/// <seealso cref="Encode(EncodeIterator, State)"/>
		/// <seealso cref="Encode(EncodeIterator, Time)"/>
		/// <seealso cref="Encode(EncodeIterator, UInt)"/>
		/// <seealso cref="EncodeBlank(EncodeIterator)"/>
		/// <seealso cref="EncodeComplete(EncodeIterator, bool)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeElementEntry(iter, this, null);
		}

		/// <summary>
		/// Encode a single element as blank.
		/// </summary>
		/// <param name="iter"> The encoder iterator.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeBlank(EncodeIterator iter)
		{
			_encodedData.Clear();

			return Encoders.EncodeElementEntry(iter, this, null);
		}

        /// <summary>
        /// Encode a single element as <see cref="Int"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Int"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Int data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="UInt"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="UInt"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, UInt data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="Float"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Float"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Float data)
        {
            return Encoders.EncodeElementEntry(iter, this, data);
        }

        /// <summary>
        /// Encode a single element as as <see cref="Double"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Double"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Double data)
        {
            return Encoders.EncodeElementEntry(iter, this, data);
        }

        /// <summary>
        /// Encode a single element as <see cref="Real"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Real"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Real data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="Date"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Date"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Date data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="Time"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Time"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Time data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="DateTime"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="DateTime"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, DateTime data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="Qos"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Qos"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Qos data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="State"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="State"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, State data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="Enum"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Enum"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Enum data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encode a single element as <see cref="Buffer"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The data to be encoded.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Buffer"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Buffer data)
		{
			return Encoders.EncodeElementEntry(iter, this, data);
		}

        /// <summary>
        /// Encodes an <see cref="ElementEntry"/> from a complex type, such as a container type or an array.
        /// </summary>
        /// 
        /// <remarks>
        /// <para>
        /// Used for aggregate elements, such as element lists.</para>
        /// 
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="EncodeInit(EncodeIterator, int)"/></item>
        /// <item> Call one or more encoding methods for the complex type using the
        ///   same buffer</item>
        /// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="encodingMaxSize"> max expected encoding size of element data
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, int encodingMaxSize)
		{
			return Encoders.EncodeElementEntryInit(iter, this, encodingMaxSize);
		}

        /// <summary>
        /// Completes encoding an <see cref="ElementEntry"/> for a complex type.
        /// </summary>
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item>Call <see cref="EncodeInit(EncodeIterator, int)"/></item>
        /// <item> Call one or more encoding methods for the complex type using the
        /// same buffer</item>
        /// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="success"> If <c>true</c> - successfully complete the aggregate,
        ///            if <c>false</c> - remove the aggregate from the buffer.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeElementEntryComplete(iter, success);
		}

		/// <summary>
		/// Decode a single element.
		/// </summary>
		/// <param name="iter"> The iterator used to parse the element list.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeElementEntry(iter, this);
		}

        /// <summary>
		/// Gets or sets <see cref="Buffer"/> containing the name associated with this
		/// <see cref="ElementEntry"/>. Element names are defined outside of ETA, typically
		/// as part of a domain model specification or dictionary. It Is possible for
		/// a name to be empty; however this provides no identifying information for
		/// the element.
		/// </summary>
        public Buffer Name
        {
            get
            {
                return _name;
            }

            set
            {
                _name.CopyReferences(value);
            }
        }


        /// <summary>
		/// Gets or sets the <see cref="DataTypes"/> of this <see cref="ElementEntry"/>'s contents.
		/// Must be in the range of <see cref="DataTypes.INT"/> - 255.
        /// </summary>
        /// <remarks>
		/// 
		/// <para>While encoding, set this to the enumerated value of the target type.
		/// </para>
        /// 
		/// <para>While decoding, dataType describes the type of contained data so that the
		/// correct decoder can be used. If set-defined data is used, dataType will
		/// indicate any specific <see cref="DataTypes"/> information as defined in the set
		/// definition.</para>
		/// 
		/// </remarks>
		public int DataType { get; set; }

        /// <summary>
		/// Gets or sets encoded content in this <see cref="ElementEntry"/>. If populated on encode
		/// methods, this indicates that data is pre-encoded and encData copies while encoding.
		/// While decoding, this refers to the encoded ElementEntry's payload.
		/// </summary>
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