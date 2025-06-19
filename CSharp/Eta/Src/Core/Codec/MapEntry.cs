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
    /// An entry for a ETA <see cref="Map"/> that can house only other Container Types.
    /// </summary>
    /// <remarks>
    ///
    /// <see cref="Map"/> is a uniform type, where the <see cref="Map.ContainerType"/>
    /// indicates the single type housed in each entry. Each entry has an associated
    /// action which informs the user of how to apply the information contained in the entry.
    /// </remarks>
    ///
    /// <seealso cref="Map"/>
    /// <seealso cref="MapEntryFlags"/>
    /// <seealso cref="MapEntryActions"/>
	sealed public class MapEntry
	{
		internal readonly Buffer _permData = new Buffer();
		internal readonly Buffer _encodedKey = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
		/// Creates <see cref="MapEntry"/>.
		/// </summary>
		/// <seealso cref="Map"/>
        public MapEntry()
        {
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
		{
			Flags = 0;
			Action = 0;
			_permData.Clear();
			_encodedKey.Clear();
			_encodedData.Clear();
		}

        /// <summary>
        /// Encode a single map entry with pre-encoded primitive key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>
        /// Set <see cref="EncodedKey"/> with pre-encoded data before calling this method.
        /// </para>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
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
        /// <seealso cref="EncodeComplete(EncodeIterator, bool)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter)
		{
			return Encoders.EncodeMapEntry(iter, this, null);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="Int"/> key. Must be called after
        /// <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The <see cref="Int"/> key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Int keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="UInt"/> key. Must be called after
        /// <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The <see cref="UInt"/> key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, UInt keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with Real key. Must be called after
        /// <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Real key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Real keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="Date"/> key. Must be called after
        /// <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Date key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Date keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="Time"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Time key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Time keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="DateTime"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The DateTime key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, DateTime keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="Qos"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Qos key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Qos keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="State"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The State key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, State keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="Enum"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Enum key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Enum keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with <see cref="Buffer"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Buffer key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="Encode(EncodeIterator)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Buffer keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Initialize Map Entry encoding with pre-encoded primitive key.  Must be called
        /// after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// Set <see cref="EncodedKey"/> with pre-encoded data before calling this method.
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, Buffer, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Date, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, DateTime, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Double, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Enum, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Float, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Int, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Qos, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Real, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, State, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, Time, int)"/>
        /// <seealso cref="EncodeInit(EncodeIterator, UInt, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, null, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="Int"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The <see cref="Int"/> key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Int keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="UInt"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The UInt key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, UInt keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with Float key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Real key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Real keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="Date"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Date key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Date keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="Time"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Time key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, Time keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="DateTime"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The DateTime key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, DateTime keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="Qos"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Qos key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Qos keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="State"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The State key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, State keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="Enum"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Enum key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Enum keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with <see cref="Buffer"/> key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Buffer key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Buffer keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Completes Map Entry encoding.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call MapEntry.encodeInit()..MapEntry.encodeComplete() for each map
        /// entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="success">If <c>true</c> - successfully complete the aggregate,
        ///                if <c>false</c> - remove the aggregate from the buffer.</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeMapEntryComplete(iter, this, success);
		}

        /// <summary>
        /// Decode a single map entry.
        /// </summary>
        ///
        /// <param name="iter">Decode iterator</param>
        /// <param name="keyData">The decoded key data. If the user provides this pointer,
        ///            this method will automatically decode the key to it
        ///            (the pointer MUST point to memory large enough to contain the
        ///            primitive that will be written).</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Decode(DecodeIterator iter, object keyData)
		{
			return Decoders.DecodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Checks the presence of the Permission Data presence flag.
        /// </summary>
        /// <remarks>
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </remarks>
        /// <returns> <c>true</c> - if exists; <c>false</c> if does not exist
        /// </returns>
        /// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public bool CheckHasPermData()
		{
			return (Flags & MapEntryFlags.HAS_PERM_DATA) > 0 ? true : false;
		}

        /// <summary>
        /// Applies the Permission Data presence flag.
        /// </summary>
        /// <remarks>
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </remarks>
        /// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ApplyHasPermData()
		{
			Flags = Flags | MapEntryFlags.HAS_PERM_DATA;
		}

        /// <summary>
        /// The entry action helps to manage change processing rules and tells the
        /// consumer how to apply the information contained in the entry.
        /// </summary>
        /// <remarks>
        ///
        /// <para>For specific information about possible action's associated with a MapEntry,
        /// see <see cref="MapEntryActions"/>.</para>
        ///
        /// <para>Must be in the range of 0 - 15.</para>
        ///
        /// </remarks>
		public MapEntryActions Action { get; set; }

        /// <summary>
        /// All the flags applicable to this map entry.
        /// </summary>
        /// <remarks>
        ///
        /// Must be in the range of 0 - 15.
        /// </remarks>
        /// <seealso cref="MapEntryFlags"/>
		public MapEntryFlags Flags { get; set; }

        /// <summary>
        /// Specifies authorization information for this specific entry.
        /// If present <see cref="MapEntryFlags.HAS_PERM_DATA"/> should be set.
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
        /// Encoded map entry key information.
        /// </summary>
        /// <remarks>
        ///
        /// <para>The encoded type of the key corresponds to the Map's keyPrimitiveType. The key
        /// value must be a base primitive type and cannot be blank, <see cref="DataTypes.ARRAY"/>,
        /// or <see cref="DataTypes.UNKNOWN"/> primitive types.</para>
        ///
        /// <para>Encoded map entry key information. The encoded type of the key corresponds to
        /// the Map's keyPrimitiveType. The key value must be a base primitive type and
        /// cannot be blank, <see cref="DataTypes.ARRAY"/>, or <see cref="DataTypes.UNKNOWN"/>
        /// primitive types.If populated on encode methods, this indicates that the key is
        /// pre-encoded and encKey will be copied while encoding.</para>
        /// </remarks>
		public Buffer EncodedKey
		{
            get
            {
                return _encodedKey;
            }

            set
            {
                (_encodedKey).CopyReferences(value);
            }
		}

        /// <summary>
        /// Gets or sets encoded data
        /// </summary>
        /// <remarks>
        ///
        /// The encoded content of this <c>MapEntry</c>. If populated on encode methods,
        /// this indicates that data is pre-encoded, and assigned value will be copied while encoding.
        /// </remarks>
        ///
        /// <value>The <see cref="Buffer"/> containing encoded data</value>
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

        /// <summary>
        /// Encode a single map entry with Float key. Must be called after
        /// <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Float key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Float keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Encode a single map entry with Double key. Must be called after
        /// <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Double key</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode Encode(EncodeIterator iter, Double keyData)
		{
			return Encoders.EncodeMapEntry(iter, this, keyData);
		}

        /// <summary>
        /// Initialize Map Entry encoding with Float key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Float key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Float keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}

        /// <summary>
        /// Initialize Map Entry encoding with Double key.
        /// Must be called after <see cref="Map.EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and/or <see cref="Map.EncodeSummaryDataComplete(EncodeIterator, bool)"/>.
        /// </summary>
        /// <remarks>
        ///
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <c>MapEntry.Encode()</c> for each map entry in the list using the same buffer</item>
        /// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
        /// <param name="iter">The encoder iterator</param>
        /// <param name="keyData">The Double key</param>
        /// <param name="maxEncodingSize">max encoding size of map entry data</param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeInit(EncodeIterator, int)"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public CodecReturnCode EncodeInit(EncodeIterator iter, Double keyData, int maxEncodingSize)
		{
			return Encoders.EncodeMapEntryInit(iter, this, keyData, maxEncodingSize);
		}
	}

}