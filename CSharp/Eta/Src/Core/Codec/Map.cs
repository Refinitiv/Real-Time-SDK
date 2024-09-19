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
	/// The <see cref="Map"/> is a uniform container type of associated key - value pair
	/// entries, each known as a <see cref="MapEntry"/>. Each <see cref="MapEntry"/> contains an
	/// entry key, which is a base primitive type, and a value.
	/// </summary>
	/// <remarks>
	///
	/// <para>
	/// <b>Map Encoding Example</b>
	/// </para>
	/// <para>
	/// The following sample illustrates the encoding of a <see cref="Map"/> containing
	/// <see cref="FieldList"/> values. The example encodes three <see cref="MapEntry"/> values
	/// as well as summary data:
	/// <ul>
	/// <li>The first entry is encoded with an update action type and a passed in key
	/// value.</li>
	/// <li>The second entry is encoded with an add action type, pre-encoded data,
	/// and pre-encoded key.</li>
	/// <li>The third entry is encoded with a delete action type.</li>
	/// </ul>
	/// </para>
	/// <para>
	/// This example also demonstrates error handling for the initial encode
	/// method. To simplify the example, additional error handling is omitted,
	/// though it should be performed.
	///
	/// </para>
	///
	/// <code>
	/// Map map = new Map();
	/// EncodeIterator encIter = new EncodeIterator();
	/// 
	/// // populate map structure prior to call to map.EncodeInit()
	/// // NOTE: the key names used for this example may not correspond to actual name
	/// // values
	/// 
	/// // indicate that summary data and a total count hint will be encoded
	/// map.Flags = MapFlags.HAS_SUMMARY_DATA | MapFlags.HAS_TOTAL_COUNT_HINT;
	/// 
	/// // populate maps keyPrimitiveType and containerType
	/// map.ContainerType = DataTypes.FIELD_LIST;
	/// map.KeyPrimitiveType = DataTypes.UINT;
	/// 
	/// // populate total count hint with approximate expected entry count
	/// map.TotalCountHint = 3;
	/// 
	/// // begin encoding of map - assumes that encIter is already populated with
	/// // buffer and version information, store return value to determine success or
	/// // failure
	/// // expect summary data of approx. 100 bytes, no set definition data
	/// 
	/// retVal = map.EncodeInit(encIter, 100, 0);
	/// if (retVal &lt; CodecReturnCode.SUCCESS)
	/// {
	///     // error condition - switch our success value to false so we can roll back
	///     success = false;
	/// }
	/// else
	/// {
	///     // mapInit encoding was successful
	///     // create a single MapEntry and FieldList and reuse for each entry
	///     MapEntry mapEntry = new MapEntry();
	///     FieldList fieldList = new FieldList();
	///     UInt entryKeyUInt =new UInt();
	///     entryKeyUInt.Value(0);
	/// 
	///     // encode expected summary data, init for this was done by map.EncodeInit()
	///     // this type should match map.ContainerType.
	///     {
	///         fieldList.Flags = FieldListFlags.HAS_STANDARD_DATA;
	///         retVal = fieldList.EncodeInit(encIter, null, 0);
	/// 
	///         // Continue encoding field entries - see encoding example in FieldList
	/// 
	///         // Complete nested container encoding
	///         retVal = fieldList.EncodeComplete(encIter, success);
	///     }
	///     // complete encoding of summary data. If any field list encoding failed,
	///     // success is false
	///     retVal = map.EncodeSummaryDataComplete(encIter, success);
	/// 
	///     // FIRST Map Entry: encode entry from non pre-encoded data and key. Approx.
	///     // encoded length unknown
	///     mapEntry.Action = MapEntryActions.UPDATE;
	///     entryKeyUInt.Value(1);
	///     mapEntry.EncodeInit(encIter, entryKeyUInt, 0);
	/// 
	///     // encode contained field list - this type should match Map.containerType
	///     {
	///         fieldList.Clear();
	///         fieldList.Flags = ieldListFlags.HAS_STANDARD_DATA;
	///         retVal = fieldList.EncodeInit(encIter, null, 0);
	/// 
	///         // Continue encoding field entries
	/// 
	///         // Complete nested container encoding
	///         retVal = fieldList.EncodeComplete(encIter, success);
	///     }
	/// 
	///     retVal = mapEntry.EncodeComplete(encIter, success);
	/// 
	///     // SECOND Map Entry: encode entry from pre-encoded buffer containing an
	///     // encoded FieldList
	///     // because we are re-populating all values on MapEntry, there is no need to
	///     // clear it.
	///     mapEntry.Action = MapEntryActions.ADD;
	/// 
	///     // assuming encUInt Buffer contains the pre-encoded key with length and
	///     // data properly populated
	///     mapEntry.EncodedKey(encUInt);
	/// 
	///     // assuming encFieldList Buffer contains the pre-encoded payload with data
	///     // and length populated
	///     mapEntry.EncodedData(encFieldList);
	/// 
	///     retVal = mapEntry.Encode(encIter);
	/// 
	///     // THIRD Map Entry: encode entry with delete action. Delete actions have no
	///     // payload
	///     // need to ensure that MapEntry is appropriately cleared - clearing will
	///     // ensure that encData and encKey are properly emptied.s
	///     mapEntry.Clear();
	///     mapEntry.Action = MapEntryActions.DELETE;
	///     entryKeyUInt.Value = 3;
	///     retVal = mapEntry.Encode(encIter, entryKeyUInt);
	/// 
	/// }
	/// 
	/// // complete map encoding. If success parameter is true, this will finalize
	/// // encoding.
	/// // If success parameter is false, this will roll back encoding prior to mapInit.
	/// retVal = map.EncodeComplete(encIter, success);
	/// </code>
	///
	/// <para>
	/// <b>Map Decoding Example</b>
	/// </para>
	/// <para>
	/// The following sample demonstrates the decoding of a <see cref="Map"/> and is
	/// structured to decode each entry to the contained value. This sample assumes
	/// that the housed container type is a <see cref="FieldList"/> and that the
	/// keyPrimitiveType is DataTypes#INT. This sample also uses the
	/// mapEntry.decode() method to perform key decoding. Typically an application
	/// would invoke the specific container-type decoder for the housed type or use a
	/// switch statement to allow for a more generic map entry decoder. This example
	/// uses the same <see cref="DecodeIterator"/> when calling the content's decoder
	/// method. An application could optionally use a new <see cref="DecodeIterator"/> by
	/// setting the encData on a new iterator. To simplify the sample, some error
	/// handling is omitted.
	/// </para>
	/// 
	/// <code>
	/// // decode contents into the map structure
	/// retVal = map.Decode(decIter);
	/// if (retVal &gt;= CodecReturnCode.SUCCESS)
	/// {
	///     // create primitive value to have key decoded into and a single map entry to
	///     // reuse
	///     Int intVal = new Int();
	///     intVal.Value(0);
	///     mapEntry = new MapEntry();
	///     if (mapEntry.CheckHasSummaryData())
	///     {
	///         // summary data is present. Its type should be that of map.containerType
	///         FieldList fieldList = new FieldList();
	///         retVal = fieldList.Decode(decIter, 0);
	/// 
	///         // Continue decoding field entries
	///     }
	/// 
	///     // decode each map entry, passing in pointer to keyPrimitiveType decodes
	///     // mapEntry key as well
	///     while ((retVal = mapEntry.Decode(decIter, intVal) != CodecReturnCode.END_OF_CONTAINER))
	///     {
	///         if (retVal &lt; CodecReturnCode.SUCCESS)
	///         {
	///             // decoding failure tends to be unrecoverable
	///         }
	///         else
	///         {
	///             FieldList fieldList = new FieldList();
	///             retVal = fieldList.Decode(decIter, 0);
	///             // Continue decoding field entries. See FieldList for
	///             // decoding example.
	///         }
	///     }
	/// }
	/// else
	/// {
	///     // decoding failure tends to be unrecoverable
	/// }
	/// </code>
	///
	/// </remarks>
	/// <seealso cref="FieldList"/>
	/// <seealso cref="MapEntry"/>
	/// <seealso cref="MapFlags"/>
	sealed public class Map : IXMLDecoder
    {
		internal readonly Buffer _encodedSetDefs = new Buffer();
		internal readonly Buffer _encodedSummaryData = new Buffer();
		internal readonly Buffer _encodedEntries = new Buffer();

        /// <summary>
        /// Creates <see cref="Map"/>.
        /// </summary>
        /// <seealso cref="Map"/>
        public Map()
        {
            ContainerType = DataTypes.CONTAINER_TYPE_MIN;
        }

		/// <summary>
		/// Sets all members in <see cref="Map"/> to an initial value. Useful for object
		/// reuse during encoding. While decoding, <see cref="Map"/> object can be reused
		/// without using <see cref="Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
			Flags = 0;
			KeyPrimitiveType = 0;
			KeyFieldId = 0;
			ContainerType = DataTypes.CONTAINER_TYPE_MIN;
			_encodedSetDefs.Clear();
			_encodedSummaryData.Clear();
			TotalCountHint = 0;
			_encodedEntries.Clear();
		}

		/// <summary>
		/// Initialize Map encoding.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="EncodeInit(EncodeIterator, int, int)"/></item>
		/// <item> Call <see cref="MapEntry.Encode(EncodeIterator)"/> or
		///   <see cref="MapEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="MapEntry.EncodeComplete(EncodeIterator, bool)"/>
		///   for each map entry in list using the same buffer</item>
		/// <item> Call <see cref="EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="summaryMaxSize"> max encoding size of the summary data, if encoding </param>
		/// <param name="setMaxSize"> max encoding size of the set information, if encoding
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize)
		{
			return Encoders.EncodeMapInit(iter, this, summaryMaxSize, setMaxSize);
		}

        /// <summary>
        /// Complete set data encoding for a map. If both <c>EncodeSetDefsComplete()</c> and
        /// <see cref="EncodeSummaryDataComplete(EncodeIterator, bool)"/> are called, <c>EncodeSetDefsComplete()</c>
		/// must be called first.
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> <c>true</c> if encoding of set data was successful, <c>false</c> for rollback
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeSetDefsComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeMapSetDefsComplete(iter, success);
		}

        /// <summary>
        /// Complete summary data encoding for a map. If both <see cref="EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// and <c>EncodeSummaryDataComplete()</c> are called, <see cref="EncodeSetDefsComplete(EncodeIterator, bool)"/>
        /// must be called first.
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> True if encoding of summary data was successful, false for rollback
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeSummaryDataComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeMapSummaryDataComplete(iter, this, success);
		}

		/// <summary>
		/// Completes Map encoding.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="Map.EncodeInit(EncodeIterator, int, int)"/></item>
		/// <item> Call <see cref="MapEntry.Encode(EncodeIterator)"/> or
		///   <see cref="MapEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="MapEntry.EncodeComplete(EncodeIterator, bool)"/>
		///   for each map entry in the list using the same buffer</item>
		/// <item> Call <see cref="Map.EncodeComplete(EncodeIterator, bool)"/></item>
        /// </list>
        /// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If <c>true</c> - successfully complete the aggregate,
		///                if <c>false</c> - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeMapComplete(iter, this, success);
		}

		/// <summary>
		/// Decode map.
		/// </summary>
		/// <param name="iter"> <see cref="DecodeIterator"/> for decoding with
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeMap(iter, this);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.MAP, null, null, null, iter);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.MAP, null, dictionary, null, iter);
        }

		/// <summary>
		/// Checks the presence of the local Set Definition presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasSetDefs()
		{
			return (Flags & MapFlags.HAS_SET_DEFS) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Summary Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasSummaryData()
		{
			return (Flags & MapFlags.HAS_SUMMARY_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Per Entry Permission presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasPerEntryPermData()
		{
			return (Flags & MapFlags.HAS_PER_ENTRY_PERM_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Total Count Hint presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if exists; <c>false</c> if does not exist.
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasTotalCountHint()
		{
			return (Flags & MapFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Key Field Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> <c>true</c> - if key is fid; <c>false</c> if not.
		/// </returns>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasKeyFieldId()
		{
			return (Flags & MapFlags.HAS_KEY_FIELD_ID) > 0 ? true : false;
		}

		/// <summary>
		/// Applies the local Set Definition presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasSetDefs()
		{
			Flags = Flags | MapFlags.HAS_SET_DEFS;
		}

		/// <summary>
		/// Applies the Summary Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasSummaryData()
		{
			Flags = Flags | MapFlags.HAS_SUMMARY_DATA;
		}

		/// <summary>
		/// Applies the Per Entry Permission Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasPerEntryPermData()
		{
			Flags = Flags | MapFlags.HAS_PER_ENTRY_PERM_DATA;
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
			Flags = Flags | MapFlags.HAS_TOTAL_COUNT_HINT;
		}

		/// <summary>
		/// Applies the Key Field Id presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplyHasKeyFieldId()
		{
			Flags = Flags | MapFlags.HAS_KEY_FIELD_ID;
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this map. Must be in the range of 0 - 255.
		/// </summary>
		public MapFlags Flags { get; set; }

        /// <summary>
        /// Gets raw encoded map data.
        /// </summary>
        public Buffer EncodedEntries
		{
            get
            {
                return _encodedEntries;
            }
		}

        /// <summary>
        /// Gets or sets the <see cref="DataTypes"/> enumeration value that describes the base primitive
        /// type of each <see cref="MapEntry"/>'s key. keyPrimitiveType must be from the
        /// <see cref="DataTypes"/> enumeration and greater than <see cref="DataTypes.UNKNOWN"/>
        /// and less than or equal to <see cref="DataTypes.BASE_PRIMITIVE_MAX"/>, cannot be
        /// specified as blank, and cannot be the <see cref="DataTypes.ARRAY"/> or
        /// <see cref="DataTypes.UNKNOWN"/> primitive types.
        /// </summary>
        public int KeyPrimitiveType { get; set; }

        /// <summary>
		/// Specifies a fieldId associated with the entry key information. This is
		/// mainly used as an optimization to avoid inclusion of redundant data. In
		/// situations where key information is also a member of the entry payload
		/// (e.g. Order Id for Market By Order domain type), this allows removal of
		/// data from each entry's payload prior to encoding as it is already present
		/// via the key and keyFieldId. Must be in the range of -32768 - 32767.
		/// </summary>
		public int KeyFieldId { get; set; }

        /// <summary>
        /// Gets or sets the <see cref="DataTypes"/> enumeration value that describes the container type
        /// of each <see cref="MapEntry"/>'s payload. containerType must be from the
        /// <see cref="DataTypes"/> enumeration in the range
        /// <see cref="DataTypes.CONTAINER_TYPE_MIN"/> to 255.
        /// </summary>

        public int ContainerType { get; set; }

        /// <summary>
		/// Gets or sets the encoded local set definitions, if any, contained in the message. If
		/// populated, these definitions correspond to data contained within the
		/// <see cref="Map"/>'s entries and are used for encoding or decoding their contents.
		/// </summary>
		public Buffer EncodedSetDefs
		{
            get
            {
                return _encodedSetDefs;
            }

            set
            {
                (_encodedSetDefs).CopyReferences(value);
            }
		}

        /// <summary>
		/// The encoded summary data, if any, contained in the message. If populated,
		/// summary data contains information that applies to every entry encoded in
		/// the <see cref="Map"/> (e.g. currency type). The container type of summary data
		/// should match the containerType specified on the <see cref="Map"/>. If
		/// encSummaryData is populated while encoding, contents are used as
		/// pre-encoded summary data.
		/// </summary>
		public Buffer EncodedSummaryData
		{
            get
            {
                return _encodedSummaryData;
            }

            set
            {
                _encodedSummaryData.CopyReferences(value);
            }
		}

        /// <summary>
        /// Gets or sets an approximate total number of entries associated with this stream.
        /// This is typically used when multiple <see cref="Map"/> containers are spread across
        /// multiple parts of a refresh message.). totalCountHint provides an approximation
        /// of the total number of entries sent across all maps on all parts of the refresh
        /// message. This information is useful when determining the amount of resources to
        /// allocate for caching or displaying all expected entries.
        /// Must be in the range of 0 - 1073741823.
        /// </summary>
        public int TotalCountHint { get; set; }
	}

}