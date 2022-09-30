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
	/// The <seealso cref="Series"/> is a uniform container type that contains 0 to N^33
	/// entries where where zero indicates an empty series. Each entry, known as,
	/// <seealso cref="SeriesEntry"/> contains only encoded data. This container is often used
	/// to represent table based information, where no explicit indexing is present
	/// or required.
	/// 
	/// <para>
	/// <b>Series Encoding Example</b>
	/// </para>
	/// <para>
	/// The following sample illustrates how to encode a <seealso cref="Series"/> containing
	/// <seealso cref="ElementList"/> values. The example encodes two <seealso cref="SeriesEntry"/>
	/// values as well as summary data.
	/// <ul>
	/// <li>The first entry is encoded from an unencoded element list.</li>
	/// <li>The second entry is encoded from a buffer containing a pre-encoded
	/// element list.</li>
	/// </ul>
	/// The example demonstrates error handling for the initial encode method. To
	/// simplify the example, additional error handling is omitted, though it should
	/// be performed.
	/// 
	/// </para>
	/// <para>
	/// <ul class="blockList">
	/// <li class="blockList">
	/// 
	/// <pre>
	/// // populate series structure prior to call to series.encodeInit()
	/// 
	/// // indicate that summary data and a total count hint will be encoded
	/// series.Flags =SeriesFlags.HAS_SUMMARY_DATA | SeriesFlags.HAS_TOTAL_COUNT_HINT;
	/// // populate containerType and total count hint
	/// series.ContainerType = DataTypes.ELEMENT_LIST;
	/// series.TotalCountHint = 2;
	/// 
	/// // begin encoding of series - assumes that encIter is already populated with
	/// // buffer and version information, store return value to determine success or
	/// // failure
	/// // summary data approximate encoded length is unknown, pass in 0
	/// if ((retVal = series.EncodeInit(encIter, 0, 0)) &lt; CodecReturnCode.SUCCESS)
	/// {
	///     // error condition - switch our success value to false so we can roll back
	///     success = false;
	/// }
	/// else
	/// {
	///     // series init encoding was successful
	///     // create a single SeriesEntry and ElementList and reuse for each entry
	///     SeriesEntry seriesEntry = new SeriesEntry();
	///     ElementList elementList = new ElementList();
	///     // encode expected summary data, init for this was done by
	///     // seriesEntry.EncodeInit
	///     // - this type should match series.ContainerType
	///     {
	///          // now encode nested container using its own specific encode methods
	///          elementList.Flags ElementListFlags.HAS_STANDARD_DATA;
	/// 
	///          retVal = elementList.EncodeInit(encIter, null, 0));
	/// 
	///          // ------ Continue encoding element entries.  See example in ElementList ----
	/// 
	///          // Complete nested container encoding 
	///          retVal = elementList.EncodeComplete(encIter, success);
	///      }
	///      // complete encoding of summary data.  If any element list encoding failed, success is false
	///      retVal = series.EncodeSummaryDataComplete(encIter, success);    
	/// 
	///      //FIRST Series Entry: encode entry from unencoded data.  Approx. encoded length unknown
	///      retVal = seriesEntry.EncodeInit(encIter, 0);
	/// 
	///      //encode contained element list - this type should match series.containerType
	///      {
	///          //now encode nested container using its own specific encode methods 
	///          //clear, then begin encoding of element list - using same encIterator as series
	///          elementList.Clear();
	///          elementList.Flags = ElementListFlags.HAS_STANDARD_DATA;
	///          elementList.EncodeInit = encIter, null, 0;
	/// 
	///          //Continue encoding element entries
	/// 
	///          //Complete nested container encoding 
	///          retVal = encodeList.EncodeComplete(encIter, success);
	///       }  
	///       retVal = seriesEntry.EncodeComplete(encIter, success);
	/// 
	///       //SECOND Series Entry: encode entry from pre-encoded buffer containing an encoded ElementList 
	///       /assuming encElementList Buffer contains the pre-encoded payload with data and length populated
	///       seriesEntry.EncodedData().Data(encElementList);
	///       retVal = seriesEntry.Encode(encIter);
	/// }
	/// 
	///  //complete series encoding. If success parameter is true, this will finalize encoding.  
	///  //If success parameter is false, this will roll back encoding prior to series.encodeInit
	///  retVal = series.EncodeComplete(encIter, success);
	/// 
	/// 
	/// </pre>
	/// 
	/// </li>
	/// </ul>
	/// </para>
	/// <para>
	/// <b>Series Decoding Example</b>
	/// </para>
	/// <para>
	/// The following sample illustrates how to decode a <seealso cref="Series"/> and is
	/// structured to decode each entry to the contained value. The sample code
	/// assumes the housed container type is an <seealso cref="ElementList"/>. Typically an
	/// application invokes the specific container type decoder for the housed type
	/// or uses a switch statement to allow for a more generic series entry decoder.
	/// This example uses the same <seealso cref="DecodeIterator"/> when calling the content's
	/// decoder method. An application could optionally use a new
	/// <seealso cref="DecodeIterator"/> by setting encData on a new iterator. To simplify the
	/// sample, some error handling is omitted.
	/// 
	/// </para>
	/// <para>
	/// <ul class="blockList">
	/// <li class="blockList">
	/// 
	/// <pre>
	/// // decode contents into the series
	/// retVal = series.Decode(encIter);
	/// if (retVal &gt;= CodecReturnCode.SUCCESS)
	/// {
	///     // create single series entry and reuse while decoding each entry
	///     SeriesEntry seriesEntry = new SeriesEntry();
	/// 
	///     // if summary data is present, invoking decoder for that type (instead of
	///     // DecodeEntry) indicates to ETA that user wants to decode summary data
	///     if ((series.Flags &amp; SeriesFlags.HAS_STANDARD_DATA) != 0)
	///     {
	///         // summary data is present. Its type should be that of
	///         // series.containerType
	///         ElementList elementList = new ElementList();
	///         retVal = elementList.DecodeElementList(decIter, 0);
	/// 
	///         // Continue decoding element entries. See example for ElementList
	///     }
	/// 
	///     // decode each series entry until there are no more left
	///     while ((retVal = seriesEntry.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
	///     {
	///         if (retVal &lt; CodecReturnCode.SUCCESS)
	///         {
	///             // error condition - decoding failure tends to be unrecoverable
	///         }
	///         else
	///         {
	///             ElementList elementList = new ElementList();
	///             retVal = elementList.DecodeElementList(decIter, 0);
	/// 
	///             // Continue decoding element entries. See example for ElementList
	///         }
	///     }
	/// }
	/// else
	/// {
	///     // error condition - decoding failure tends to be unrecoverable
	/// }
	/// 
	/// </pre>
	/// 
	/// </li>
	/// </ul>
	/// 
	/// </para>
	/// </summary>
	/// <seealso cref="SeriesEntry"/>
	/// <seealso cref="SeriesFlags"/>
	sealed public class Series : IXMLDecoder
    {
		internal readonly Buffer _encodedSetDefs = new Buffer();
		internal readonly Buffer _encodedSummaryData = new Buffer();
		internal readonly Buffer _encodedEntries = new Buffer();

        /// <summary>
        /// Creates <see cref="Series"/>.
        /// </summary>
        /// <returns> Series object
        /// </returns>
        /// <seealso cref="Series"/>
        public Series()
        {
        }

        /// <summary>
        /// Sets all members in <see cref="Series"/> to an initial value. Useful for object
        /// reuse during encoding. While decoding, <see cref="Series"/> object can be
        /// reused without using <see cref="Clear()"/>.
        /// </summary>
        public void Clear()
		{
			Flags = 0;
			ContainerType = DataTypes.CONTAINER_TYPE_MIN;
			_encodedSetDefs.Clear();
			_encodedSummaryData.Clear();
			TotalCountHint = 0;
			_encodedEntries.Clear();
		}

        /// <summary>
		/// Prepares series for encoding.
		/// 
		/// Typical use:<para />
		/// 1. Call Series.EncodeInit()<para />
		/// 2. Call SeriesEntry.Encode() or SeriesEntry.EncodeInit()..SeriesEntry.EncodeComplete()
		///    for each entry using the same buffer<para />
		/// 3. Call Series.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="summaryMaxSize"> max encoding size of summary data, if present </param>
		/// <param name="setMaxSize"> max encoding size of the set data, if present
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize)
		{
			return Encoders.EncodeSeriesInit(iter, this, summaryMaxSize, setMaxSize);
		}

        /// <summary>
        /// Complete set data encoding for a series. If both EncodeSetDefsComplete()
        /// and EncodeSummaryDataComplete() are called, EncodeSetDefsComplete() must be called first.
        /// </summary>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> True if encoding of set data was successful, false for rollback
        /// </param>
        /// <returns> <see cref="CodecReturnCode"/>
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        public CodecReturnCode EncodeSetDefsComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeSeriesSetDefsComplete(iter, success);
		}

        /// <summary>
		/// Complete summary data encoding for a series.
		/// If both EncodeSetDefsComplete() and EncodeSummaryDataComplete() are called,
		/// EncodeSetDefsComplete() must be called first.
		/// </summary>
		/// <param name="iter"> Encoding iterator </param>
		/// <param name="success"> True if encoding of summary data was successful, false for rollback
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeSummaryDataComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeSeriesSummaryDataComplete(iter, success);
		}

        /// <summary>
		/// Completes series encoding.
		/// 
		/// Typical use:<para />
		/// 1. Call Series.EncodeInit()<para />
		/// 2. Call SeriesEntry.Encode() or SeriesEntry.EncodeInit()..SeriesEntry.EncodeComplete()
		///    for each entry using the same buffer<para />
		/// 3. Call Series.EncodeComplete()<para />
		/// </summary>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="success"> If true - successfully complete the aggregate,
		///                if false - remove the aggregate from the buffer.
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeSeriesComplete(iter, success);
		}

        /// <summary>
		/// Decode Series.
		/// </summary>
		/// <param name="iter"> decode iterator
		/// </param>
		/// <returns> <see cref="CodecReturnCode"/>
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeSeries(iter, this);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.SERIES, null, null, null, iter);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.SERIES, null, dictionary, null, iter);
        }

        /// <summary>
		/// Checks the presence of the Set Definition presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if exists; false if does not exist </returns>
		public bool CheckHasSetDefs()
		{
			return ((Flags & SeriesFlags.HAS_SET_DEFS) > 0 ? true : false);
		}

        /// <summary>
		/// Checks the presence of the Summary Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		/// <returns> true - if exists; false if does not exist </returns>
		public bool CheckHasSummaryData()
		{
			return ((Flags & SeriesFlags.HAS_SUMMARY_DATA) > 0 ? true : false);
		}

        /// <summary>
        /// Checks the presence of the Total Count Hint presence flag.<br />
        /// <br />
        /// Flags may also be bulk-get via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        /// <returns> true - if exists; false if does not exist </returns>
        public bool CheckHasTotalCountHint()
		{
			return ((Flags & SeriesFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false);
		}

        /// <summary>
        /// Applies the local Set Definition presence flag.<br />
        /// <br />
        /// Flags may also be bulk-set via <see cref="Flags"/>.
        /// </summary>
        /// <seealso cref="Flags"/>
        public void ApplyHasSetDefs()
		{
			Flags = (Flags | SeriesFlags.HAS_SET_DEFS);
		}

        /// <summary>
		/// Applies the Summary Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		public void ApplyHasSummaryData()
		{
			Flags = (Flags | SeriesFlags.HAS_SUMMARY_DATA);
		}

        /// <summary>
		/// Applies the Total Count Hint presence flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		public void ApplyHasTotalCountHint()
		{
			Flags = (Flags | SeriesFlags.HAS_TOTAL_COUNT_HINT);
		}

        /// <summary>
		/// Gets or sets all the flags applicable to this series.
		/// Must be in the range of 0 - 255.
		/// </summary>
		public SeriesFlags Flags { get; set; }

        /// <summary>
        /// Gets or sets the <see cref="DataTypes"/> enumeration value that describes the container type
        /// of each <see cref="SeriesEntry"/>'s payload. containerType must be from the
        /// <see cref="DataTypes"/> enumeration in the range <see cref="DataTypes.CONTAINER_TYPE_MIN"/>
        /// to 255.
        /// </summary>
        public int ContainerType { get; set; }

        /// <summary>
		/// Gets or sets encoded set definitions, if any, contained in the message. If populated,
		/// these definitions correspond to data contained within this <seealso cref="Series"/>
		/// 's entries and are used for encoding or decoding their contents.
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
		/// Gets or sets encoded summary data. If populated, summary data contains information
		/// that applies to every entry encoded in the <seealso cref="Series"/> (e.g., currency
		/// type). The container type of summary data should match the containerType
		/// specified on the <seealso cref="Series"/>.
		/// </summary>
		public Buffer EncodedSummaryData
		{
            get
            {
                return _encodedSummaryData;
            }

            set
            {
                (_encodedSummaryData).CopyReferences(value);
            }
		}

        /// <summary>
        /// Gets or sets an approximate total number of entries associated with this stream.
        /// This is typically used when multiple <see cref="Series"/> containers are spread
        /// across multiple parts of a refresh message. The totalCountHint provides an
        /// approximation of the total number of entries sent across all series on all
        /// parts of the refresh message. This information is useful when determining the
        /// amount of resources to allocate for caching or displaying all expected entries.
        /// Must be in the range of 0 - 1073741823.
        /// </summary>
        public int TotalCountHint { get; set; }

        /// <summary>
		/// All encoded key-value pair encoded data, if any, contained in the message.
		/// This refers to encoded <see cref="Series"/> payload data.
		/// </summary>
		public Buffer EncodedEntries
		{
            get
            {
                return _encodedEntries;
            }
		}
	}
}