/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// An entry for the ETA <see cref="Vector"/> that can house other Container Types
	/// only. <see cref="Vector"/> is a uniform type, where <see cref="Vector.ContainerType"/>
	/// indicates the single type housed in each entry. Each entry has an associated
	/// action which informs the user of how to apply the data contained in the entry
	/// </summary>
	/// <seealso cref="VectorEntryActions"/>
	/// <seealso cref="VectorEntryFlags"/>
	/// <seealso cref="Vector"/>
	sealed public class VectorEntry
	{
		internal readonly int MAX_INDEX = 0x3fffffff;
		internal System.UInt32 _index;
		internal readonly Buffer _permData = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <see cref="VectorEntry"/>.
        /// </summary>
        /// <seealso cref="VectorEntry"/>
        public VectorEntry()
        {
        }

		/// <summary>
		/// Clears <see cref="VectorEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <see cref="VectorEntry"/> object can be reused
		/// without using <see cref="Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			Flags = 0;
			Action = 0;
			_index = 0;
			_permData.Clear();
			_encodedData.Clear();
		}

        /// <summary>
        /// Encodes single vector item, "moving" to the next row if necessary. Must
        /// be called after EncodeSetDefsComplete() and/or
        /// EncodeSummaryDataComplete().
        /// </summary>
        /// <remarks>
        /// 
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Vector.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <see cref="VectorEntry.Encode(EncodeIterator)"/>
        ///   for each entry using the same buffer</item>
        /// <item> Call <see cref="Vector.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Decode(DecodeIterator)"/>
		/// <seealso cref="EncodeInit(EncodeIterator, int)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeVectorEntry(iter, this);
		}

        /// <summary>
        /// Prepare a single vector item for encoding. Must be called after
        /// <see cref="Vector.EncodeSetDefsComplete(EncodeIterator, bool)"/> and/or
		/// <see cref="Vector.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        /// 
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Vector.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call
        ///   <see cref="VectorEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="VectorEntry.EncodeComplete(EncodeIterator, bool)"/>
        ///   for each entry using the same buffer</item>
        /// <item> Call <see cref="Vector.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator </param>
        /// <param name="maxEncodingSize"> max encoding size of the data
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
		/// <seealso cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, int maxEncodingSize)
		{
			return Encoders.EncodeVectorEntryInit(iter, this, maxEncodingSize);
		}

        /// <summary>
        /// Completes a vector element encoding.
        /// </summary>
        /// <remarks>
        /// 
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Vector.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call
        ///   <see cref="VectorEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="VectorEntry.EncodeComplete(EncodeIterator, bool)"/>
        ///   for each entry using the same buffer</item>
        /// <item> Call <see cref="Vector.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter"> The encoder iterator. </param>
        /// <param name="success"> If <c>true</c> - successfully complete the aggregate,
        ///                if <c>false</c> - remove the aggregate from the buffer.
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <see cref="Encode(EncodeIterator)"/>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeVectorEntryComplete(iter, success);
		}

		/// <summary>
		/// Decode a vector row.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeVectorEntry(iter, this);
		}

		/// <summary>
		/// Checks the presence of the Permission Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if present; <c>false</c> - if not present
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasPermData()
		{
			return (Flags & VectorEntryFlags.HAS_PERM_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Applies the Permission Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasPermData()
		{
			Flags |= VectorEntryFlags.HAS_PERM_DATA;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this vector entry
		/// </summary>
		public VectorEntryFlags Flags { get; set; }

        /// <summary>
		/// Sets or sets Vector action (helps to manage change processing rules and informs the
		/// consumer of how to apply the information contained in the entry).
		/// Must be in the range of 0 - 15.
		/// </summary>
		public VectorEntryActions Action { get; set; }

        /// <summary>
		/// Gets or sets 0-base entry index. Must be in the range of 0 - 1073741823.
		/// </summary>
		public System.UInt32 Index
		{
            get
            {
                return _index;
            }

            set
            {
                Debug.Assert(value >= 0 && value <= 1073741823, "index is out of range (0-1073741823)");

                _index = value;
            }
		}

        /// <summary>
		/// Gets or sets permission expression for this entry.
		/// </summary>
		public Buffer PermData
		{
            get
            {
                return _permData;
            }

            set
            {
                _permData.CopyReferences(value);
            }
		}

        /// <summary>
		/// Gets or sets encoded data to be applied for the entry.
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