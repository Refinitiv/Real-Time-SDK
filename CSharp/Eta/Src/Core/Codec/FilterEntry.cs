/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// An entry for a ETA <see cref="FilterList"/> that can house only other container
	/// types. FilterList is a non-uniform type, where the
	/// <see cref="FilterList.ContainerType"/> should indicate the most common type
	/// housed in each entry. Entries that differ from this type must specify their
	/// own type via <see cref="FilterList.ContainerType"/>.
	/// </summary>
	/// <seealso cref="FilterEntryActions"/>
	/// <seealso cref="FilterEntryFlags"/>
	/// <seealso cref="FilterList"/>
	sealed public class FilterEntry
	{
		internal readonly Buffer _permData = new Buffer();
		internal readonly Buffer _encodedData = new Buffer();

        /// <summary>
        /// Creates <see cref="FilterEntry"/>.
        /// </summary>
        /// <seealso cref="FilterEntry"/>
        public FilterEntry()
        {
            ContainerType = DataTypes.CONTAINER_TYPE_MIN;
        }

		/// <summary>
		/// Clears <see cref="FilterEntry"/> object. Useful for object reuse during
		/// encoding. While decoding, <see cref="FilterEntry"/> object can be reused
		/// without using <see cref="Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			Flags = 0;
			Action = 0;
			Id = 0;
		    ContainerType = DataTypes.CONTAINER_TYPE_MIN;
			_permData.Clear();
			_encodedData.Clear();
		}

		/// <summary>
		/// Encodes single <see cref="FilterEntry"/> using the data provided.
		/// </summary>
		///
		/// <remarks>
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FilterList.EncodeInit(EncodeIterator)"/></item>
		/// <item> Call <see cref="Encode(EncodeIterator)"/> for each FilterEntry in the list using the
		/// previously encoded FilterEntry data</item>
		/// <item> Call <see cref="FilterList.EncodeComplete(EncodeIterator, bool)"/></item>
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
			return Encoders.EncodeFilterEntry(iter, this);
		}

		/// <summary>
		/// Prepares FilterEntry for encoding.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FilterList.EncodeInit(EncodeIterator)"/><br/>
		/// For each row in the FilterEntry using the same buffer perform steps 2, 3,
		/// and 4 below:</item>
		/// <item> Call <see cref="Encode(EncodeIterator)"/> </item>
		/// <item> Encode FilterEntry contents </item>
		/// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/> </item>
		/// <item> Call <see cref="FilterList.EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="maxEncodingSize"> Expected max encoding size of the FilterEntry data
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, int maxEncodingSize)
		{
			return Encoders.EncodeFilterEntryInit(iter, this, maxEncodingSize);
		}

		/// <summary>
		/// Completes FilterEntry encoding.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="FilterList.EncodeInit(EncodeIterator)"/><br/>
		/// For each row in the FilterEntry using the same buffer perform steps 2, 3,
		/// and 4 below</item>
		/// <item> Call <see cref="EncodeInit(EncodeIterator, int)"/> </item>
		/// <item> Encode FilterEntry contents </item>
		/// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/> </item>
		/// <item> Call <see cref="FilterList.EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If <c>true</c> - successfully complete the aggregate, 
		///      if <c>false</c> - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeFilterEntryComplete(iter, success);
		}

		/// <summary>
		/// Decode a single FilterEntry.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeFilterEntry(iter, this);
		}

        /// <summary>
        /// Checks the presence of the Permission Data presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> <c>true</c> if exists, <c>false</c> if does not exist
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasPermData()
		{
			return (Flags & FilterEntryFlags.HAS_PERM_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Container Type presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> if exists, <c>false</c> if does not exist
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasContainerType()
		{
			return (Flags & FilterEntryFlags.HAS_CONTAINER_TYPE) > 0 ? true : false;
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
			Flags = Flags | FilterEntryFlags.HAS_PERM_DATA;
		}

		/// <summary>
		/// Applies the Container Type presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasContainerType()
		{
			Flags = Flags | FilterEntryFlags.HAS_CONTAINER_TYPE;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this filter entry. Must be in the range of 0 - 15.
		/// </summary>
		public FilterEntryFlags Flags { get; set; }

        /// <summary>
		/// Gets or sets FilterEntry action. Helps to manage change processing rules and informs
		/// the consumer of how to apply the information contained in the entry.
		/// Must be in the range of 0 - 15.
		/// </summary>
		public FilterEntryActions Action { get; set; }

        /// <summary>
		/// Gets or sets FilterEntry id. FilterEntry id. An ID associated with the entry. Each
		/// possible id corresponds to a bit-value that can be used with the message
		/// key's filter member. This bit-value can be specified on the filter to
		/// indicate interest in the id when present in a <see cref="IRequestMsg"/> or to
		/// indicate presence of the id when present in other messages.
		/// Must be in the range of 0 - 255.
		/// </summary>
		public int Id { get; set; }

        /// <summary>
		/// A <see cref="DataTypes"/> enumeration value describing the type of this
		/// FilterEntr. containerType must be from the <see cref="DataTypes"/> enumeration
		/// in the range from <see cref="DataTypes.CONTAINER_TYPE_MIN"/> to 255.
		/// </summary>
		public int ContainerType { get; set; }

        /// <summary>
		/// Gets or sets permission expression data. Permission expression for this FilterEntry.
		/// Specifies authorization information for this entry.
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
		/// Gets or sets raw data contents that is payload of the FilterEntry.
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