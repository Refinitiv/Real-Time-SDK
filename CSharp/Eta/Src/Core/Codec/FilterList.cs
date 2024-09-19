/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// <para>
	/// The <see cref="FilterList"/> is a non-uniform container type of filterId - value
	/// pair entries.</para>
	///
	/// <para>
	/// Each entry, known as a <see cref="FilterEntry"/>, contains an id
	/// corresponding to one of 32 possible bit-value identifiers.</para>
	///
	/// <para>
	/// These identifiers are typically defined by a domain model specification and
	/// can be used to indicate interest or presence of specific entries through the
	/// inclusion of the filterId in the message key's filter member.</para>
	/// </summary>
	/// <remarks>
	///
	/// <para>
	/// <b>FilterList Encoding Example</b>
	/// </para>
	/// <para>
	/// The following sample illustrates how to encode a <see cref="FilterList"/>
	/// containing a mixture of housed types. The example encodes three
	/// <see cref="FilterEntry"/> values:
	/// <ul>
	/// <li>The first is encoded from an unencoded element list.</li>
	/// <li>The second is encoded from a buffer containing a pre-encoded element list.</li>
	/// <li>The third is encoded from an unencoded map value.</li>
	/// </ul>
	/// </para>
	/// <para>
	/// This example demonstrates error handling only for the initial encode
	/// method, and to simplify the example, omits additional error handling
	/// (though it should be performed).
	///
	/// </para>
	///
	/// <code>
	/// Buffer buf = new Buffer();
	/// buf.data(new ByteBuffer(40));
	/// FilterList filterList = new FilterList();
	///
	/// EncodeIterator encIter = new EncodeIterator();
	/// // populate containerType. Because there are two element lists, this is most
	/// // common so specify that type.
	/// filterList.ContainerType = DataTypes.ELEMENT_LIST;
	///
	/// // begin encoding of filterList - assumes that encIter is already populated with
	/// // buffer and version information, store return value to determine success or
	/// // failure
	/// CodecReturnCode retVal = filterList.EncodeInit(encIter);
	/// if (retVal &lt; CodecReturnCode.SUCCESS)
	/// {
	///     // error condition - switch our success value to false so we can roll back
	/// }
	/// else
	/// {
	///     // filterList init encoding was successful
	///     // create a single FilterEntry and reuse for each entry
	///     FilterEntry filterEntry = new FilterEntry();
	///
	///     // FIRST Filter Entry: encode entry from unencoded data. Approx. encoded
	///     // length 350 bytes
	///     // populate id and action
	///
	///     filterEntry.Id = 1;
	///     filterEntry.Action = FilterEntryActions.SET;
	///     retVal = filterEntry.EncodeInit(encIter, 350);
	///     // encode contained element list
	///     {
	///         ElementList elementList = new ElementList();
	///         elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;
	///
	///         // now encode nested container using its own specific encode methods
	///         retVal = elementList.EncodeInit(encIter, null, 0);
	///         // Continue encoding element entries. See ElementList for
	///         // element list encoding example
	///         // Complete nested container encoding
	///         retVal = elementList.EncodeComplete(encIter, success);
	///     }
	///
	///     retVal = filterEntry.EncodeComplete(encIter, success);
	///
	///     // SECOND Filter Entry: encode entry from pre-encoded buffer containing an
	///     // encoded element list
	///     // assuming encElemList Buffer contains the pre-encoded payload with data
	///     // and length populated.
	///
	///     filterEntry.Id = 2;
	///     filterEntry.Action = FilterEntryActions.UPDATE;
	///     filterEntry.EncodedData = encElemList;
	///
	///     retVal = filterEntry.Encode(encIter);
	///
	///     // THIRD Filter Entry: encode entry from an unencoded map
	///     filterEntry.Id = 3;
	///     filterEntry.Action = FilterEntryActions.UPDATE;
	///     filterEntry.Flags = FilterEntryFlags.HAS_CONTAINER_TYPE;
	///     filterEntry.ContainerType = DataTypes.MAP;
	///
	///     // encode contained map
	///     {
	///         Map map = new Map();
	///         map.KeyPrimitiveType = DataTypes.ASCII_STRING;
	///         map.ContainerType = DataTypes.FIELD_LIST;
	///
	///         // now encode nested container using its own specific encode methods
	///         retVal = map.EncodeInit(encIter, 0, 0);
	///
	///         // Continue encoding map entries. See Map for encoding example.
	///
	///         // Complete nested container encoding
	///
	///         retVal = map.EncodeComplete(encIter, success);
	///     }
	///
	///     retVal = filterEntry.EncodeComplete(encIter, success);
	/// }
	///
	/// // complete filterList encoding. If success parameter is true, this will
	/// // finalize encoding.
	/// // If success parameter is false, this will roll back encoding prior to
	/// // FilterList.encodeInit
	/// retVal = filterList.EncodeComplete(encIter, success);
	/// </code>
	///
	/// <para>
	/// <b>FilterList Decoding Example</b>
	/// </para>
	/// <para>
	/// The following sample illustrates how to decode a <see cref="FilterList"/> and is
	/// structured to decode each entry to its contained value. The sample code uses
	/// a switch statement to decode the contents of each filter entry. Typically an
	/// application invokes the specific container type decoder for the housed type
	/// or uses a switch statement to use a more generic filter entry decoder. This
	/// example uses the same <see cref="DecodeIterator"/> when calling the content's
	/// decoder method. Optionally, an application could use a new
	/// <see cref="DecodeIterator"/> by setting the encData on a new iterator. To simplify
	/// the example, some error handling is omitted.
	/// </para>
	///
	/// <code>
	/// // decode contents into the filter list structure
	/// retVal = filterList.Decode(decIter);
	/// if (retVal &gt;= CodecReturnCode.SUCCESS)
	/// {
	///     // create single filter entry and reuse while decoding each entry
	///     FilterEntry filterEntry = new FilterEntry();
	///
	///     // decode each filter entry until there are no more left
	///     while ((retVal = filterEntry.Decode(decIter) != CodecReturnCode.END_OF_CONTAINER))
	///     {
	///         if (retVal &lt; CodecReturnCode.SUCCESS)
	///         {
	///             // decoding failure tends to be unrecoverable
	///         }
	///         else
	///         {
	///             // if filterEntry.containerType is present, switch on that,
	///             // Otherwise switch on filterList.containerType
	///             int containterType;
	///             if (filterEntry.CheckHasContainerType())
	///             {
	///                 containterType = filterEntry.ContainterType();
	///             }
	///             else
	///             {
	///                 containterType = filterList.ContainterType();
	///             }
	///
	///             switch (containterType)
	///             {
	///                 case DataTypes.MAP:
	///                     retVal = map.Decode(decIter);
	///                     // Continue decoding map entries.
	///                     // See Map for map decoding example.
	///                     break;
	///                 case DataTypes.ELEMENT_LIST:
	///                     retVal = elementList.Decode(decIter);
	///                     // Continue decoding element entries.
	///                     // See ElementList for element list decoding
	///                     // example.
	///                     break;
	///             // full switch statement omitted to shorten sample code.
	///             }
	///
	///         }
	///     }
	/// }
	/// else
	/// {
	///     // error: decoding failure tends to be unrecoverable.
	/// }
	/// </code>
	///
	/// </remarks>
	/// <seealso cref="ElementList"/>
	/// <seealso cref="Map"/>
	/// <seealso cref="FilterListFlags"/>
	/// <seealso cref="FilterEntry"/>
	sealed public class FilterList : IXMLDecoder
    {
		internal readonly Buffer _encodedEntries = new Buffer();

        /// <summary>
		/// Creates <see cref="FilterList"/>.
		/// </summary>
		/// <seealso cref="FilterList"/>
        public FilterList()
        {
            ContainerType = DataTypes.CONTAINER_TYPE_MIN;
        }

		/// <summary>
		/// Clears <see cref="FilterList"/> object. Useful for object reuse during encoding.
		/// While decoding, <see cref="FilterList"/> object can be reused without using <see cref="Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			Flags = 0;
			ContainerType = DataTypes.CONTAINER_TYPE_MIN;
			TotalCountHint = 0;
			_encodedEntries.Clear();
		}

		/// <summary>
		/// Prepares Filter List for encoding.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="EncodeInit(EncodeIterator)"/></item>
		/// <item> Call one or more <see cref="FilterEntry"/> encoding methods using the same buffer</item>
		/// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoding iterator
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter)
		{
			return Encoders.EncodeFilterListInit(iter, this);
		}

		/// <summary>
		/// Completes Filter List encoding.
		/// </summary>
		/// <remarks>
		///
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="EncodeInit(EncodeIterator)"/></item>
		/// <item> Call one or more <see cref="FilterEntry"/> encoding methods using the same buffer</item>
		/// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator </param>
		/// <param name="success"> If <c>true</c> - successfully complete the aggregate,
		///                if <c>false</c> - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeFilterListComplete(iter, success, this);
		}

		/// <summary>
		/// Decode Filter List.
		/// </summary>
		/// <param name="iter"> The Decode iterator </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeFilterList(iter, this);
		}

        /// <summary>
		/// Decodes to XML the data next in line to be decoded with the iterator.
		/// Data that require dictionary lookups are decoded to hexadecimal strings.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> The XML representation of the container
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
        public string DecodeToXml(DecodeIterator iter)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.FILTER_LIST, null, null, null, iter);
        }

        /// <summary>
		/// Decodes to XML the data next in line to be decoded with the iterator.
		/// </summary>
		/// <param name="iter"> Decode iterator </param>
		/// <param name="dictionary"> Data dictionary </param>
		/// <returns> The XML representation of the container
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="DataDictionary"/>
        public string DecodeToXml(DecodeIterator iter, DataDictionary dictionary)
        {
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.FILTER_LIST, null, dictionary, null, iter);
        }

		/// <summary>
		/// Checks the presence of the Per Entry Permission presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> if exists, <c>false</c> if does not exist
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasPerEntryPermData()
		{
			return (Flags & FilterListFlags.HAS_PER_ENTRY_PERM_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Total Count Hint presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> if exists, <c>false</c> if does not exist
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasTotalCountHint()
		{
			return (Flags & FilterListFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false;
		}

		/// <summary>
		/// Applies the Per Entry Permission presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasPerEntryPermData()
		{
			Flags = Flags | FilterListFlags.HAS_PER_ENTRY_PERM_DATA;
		}

		/// <summary>
		/// Applies the Total Count Hint presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasTotalCountHint()
		{
			Flags = Flags | FilterListFlags.HAS_TOTAL_COUNT_HINT;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this filter list. Must be in the range of 0 - 255.
		/// </summary>
		public FilterListFlags Flags { get; set; }

        /// <summary>
		/// Gets or sets a <see cref="DataTypes"/> enumeration value that, for most efficient bandwidth
		/// use, should describe the most common container type across all housed
		/// filter entries. All housed entries may match this type, though one or
		/// more entries may differ. If an entry differs, the entry specifies its own
		/// type via the <see cref="FilterEntry.ContainerType"/> member. containerType must
		/// be in the range <see cref="DataTypes.CONTAINER_TYPE_MIN"/> to 255.
		/// </summary>
		public int ContainerType { get; set; }

        /// <summary>
		/// Gets or sets an approximate total number of entries associated with this
		/// stream. totalCountHint is used typically when multiple <see cref="FilterList"/>
		/// containers are spread across multiple parts of a refresh message. Must be
		/// in the range of 0 - 255.
		/// </summary>
		public int TotalCountHint { get; set; }

        /// <summary>
		/// Gets FilterId-value pair encoded data, if any, contained in the message. This
		/// would refer to the encoded <see cref="FilterList"/> payload.
		/// </summary>
		/// <value> encodedEntries </value>
		public Buffer EncodedEntries
		{
            get
            {
                return _encodedEntries;
            }
		}
	}
}