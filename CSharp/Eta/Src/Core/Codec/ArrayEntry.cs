/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{

    /// <summary>
	/// ETA entry for <see cref="Array"/> that can house only simple primitive types
	/// such as <see cref="Int"/>, <see cref="Real"/>, or <see cref="Date"/>.
	/// </summary>
	/// <seealso cref="Array" />
	sealed public class ArrayEntry
	{
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
		/// Creates <see cref="ArrayEntry"/>.
		/// </summary>
		/// <seealso cref="ArrayEntry"/>
        public ArrayEntry()
        {
        }

		/// <summary>
		/// Clears <see cref="ArrayEntry"/> object. Useful for object reuse during encoding.
		/// While decoding, <see cref="ArrayEntry"/> object can be reused without using <see cref="ArrayEntry.Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			_encodedData.Clear();
		}

        /// <summary>
        /// Decode an array entry.
        /// </summary>
        /// <param name="iter"> Decode iterator
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="DecodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeArrayEntry(iter, this);
        }

        /// <summary>
        /// Encodes an <see cref="ArrayEntry"/> from pre-encoded data. This method
        /// expects the same <see cref="EncodeIterator"/> that was used with
        /// <see cref="Array.EncodeInit(EncodeIterator)"/>.
		/// </summary>
		/// 
        /// <remarks>
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item>Call <see cref="Array.EncodeInit(EncodeIterator)"/></item>
        /// <item>Call <see cref="Encode(EncodeIterator)"/> for each entry using the same buffer</item>
        /// <item>Call <see cref="Array.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
		/// </remarks>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
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
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodePreencodedArrayEntry(iter, _encodedData);
		}

        /// <summary>
        /// Perform array item encoding of blank data.
        /// </summary>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeBlank(EncodeIterator iter)
		{
			_encodedData.Clear();

			return Encoders.EncodePreencodedArrayEntry(iter, _encodedData);
		}

        /// <summary>
        /// Perform array item encoding of an <see cref="Int"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <see cref="Int"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Int"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Int data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="UInt"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <see cref="UInt"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="UInt"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, UInt data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Float"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The Float.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Float"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Float data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Double"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The Double.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Double"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Double data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Real"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <seealso cref="Real"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Real"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Real data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Date"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <see cref="Date"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Date"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Date data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Time"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <see cref="Time"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Time"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Time data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="DateTime"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <see cref="DateTime"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="DateTime"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, DateTime data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Qos"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <seealso cref="Qos"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Qos"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Qos data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="State"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <seealso cref="State"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="State"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, State data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of an <see cref="Enum"/>.
        /// </summary>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <seealso cref="Enum"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Enum"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Enum data)
		{
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Perform array item encoding of a <see cref="Buffer"/>.
        /// </summary>
        /// 
        /// <remarks>
        /// <para>The length of the buffer should be the same as the itemLength of the array;
        /// only the shorter of the two will be copied.</para>
        /// 
        /// <para>
        /// The entry is encoded as blank when the data is null or when the data length is 0.
        /// </para>
        /// </remarks>
        /// 
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="data"> The <seealso cref="Buffer"/>.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        /// <seealso cref="Buffer"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter, Buffer data)
		{
			if ((data == null) || (data.GetLength() == 0))
			{
				_encodedData.Clear();
				return Encoders.EncodeArrayEntry(iter, _encodedData);
			}
			return Encoders.EncodeArrayEntry(iter, data);
		}

        /// <summary>
        /// Gets or sets raw data contents of the array entry. Set by Decode() method during
        /// decode or set with encoded content and used with the Encode(iter) method.
        /// </summary>
        /// <returns> encodedData.
        /// </returns>
        public Buffer EncodedData
        {
            get
            {
                return _encodedData;
            }

            set
            {
                Debug.Assert(value != null, "encodedData must be non-null");

                _encodedData.CopyReferences(value);
            }
        }
	}
}