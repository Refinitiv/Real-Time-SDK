/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.Codec
{
    /// <summary>
	/// The <see cref="Vector"/> is a uniform container type of index-value pair entries.
	/// Each entry, known as a <see cref="VectorEntry"/>, contains an index, correlating
	/// to the entry's position in the information stream and value. A
	/// <see cref="Vector"/> can contain zero to N entries (zero entries indicates an empty
	/// <see cref="Vector"/>).
	/// </summary>
	/// <remarks>
	/// 
	/// <para>
	/// <b>Vector Encoding Example</b>
	/// </para>
	/// <para>
	/// The following sample demonstrates how to encode a <see cref="Vector"/> containing
	/// <see cref="Series"/> values. The example encodes three <see cref="VectorEntry"/> values
	/// as well as summary data:</para>
	/// <ul>
	/// <li>The first entry is encoded from an unencoded series</li>
	/// <li>
	/// The second entry is encoded from a buffer containing a pre-encoded series and
	/// has perm data</li>
	/// <li>The third is a clear action type with no payload.</li>
	/// </ul>
	/// <para>
	/// This example demonstrates error handling for the initial encode method. To
	/// simplify the example, additional error handling is omitted (though it should
	/// be performed).
	/// </para>
	/// 
	/// <code>
	/// // populate vector structure prior to call to vector.EncodeInit()
	/// 
	/// // indicate that summary data and a total count hint will be encoded
	/// vector.Flags = VectorFlags.HAS_SUMMRAY_DATA | VectorFlags.HAS_TOTAL_COUNT | VectorFlags.HAS_PER_ENTRY_PERM_DATA;
	/// 
	/// // populate containerType and total count hint
	/// vector.ContainerType = DataTypes.SERIES;
	/// vector.TotalCountHint = 3;
	/// 
	/// // begin encoding of vector - assumes that encIter is already populated with
	/// // buffer and version information, store return value to determine success or
	/// // failure
	/// 
	/// if ((retVal = vector.EncodeInit(encIter, 50, 0)) &lt; CodecReturnCode.SUCCESS)
	/// {
	///     // error condition - switch our success value to false so we can roll back
	///     success = false;
	/// }
	/// else
	/// {
	///     // vector init encoding was successful
	///     // create a single VectorEntry and Series and reuse for each entry
	///     VectorEntry vectorEntry = new VectorEntry();
	///     Series series = new Series();
	/// 
	///     // encode expected summary data, init for this was done by
	///     // vector.EncodeInit - this type should match vector.containerType
	///     {
	///         // now encode nested container using its own specific encode methods
	///         // begin encoding of series - using same encIterator as vector
	/// 
	///         retVal = series.EncodeInit(encIter, 0, 0);
	///         {
	///             // ----- Continue encoding series entries, see Series encoding
	///             // example -----
	/// 
	///             // Complete nested container encoding
	///             retVal = series.EncodeComplete(encIter, success);
	///         }
	/// 
	///         // complete encoding of summary data. If any series entry encoding
	///         // failed, success is false
	///         retVal = vector.EncodeSummaryComplete(encIter, success);
	/// 
	///         // FIRST Vector Entry: encode entry from unencoded data. Approx. encoded
	///         // length 90 bytes
	/// 
	///         //populate index and action, no perm data on this entry
	///         vectorEntry.Index = 1;
	///         vectorEntry.Flags = VectorEntryFlags.NONE;
	///         vectorEntry.Action = VectorEntryActions.UPDATE;
	///         retVal = vectorEntry.EncodeInit(encIter, 90);
	/// 
	///         //encode contained series - this type should match vector.containerType
	///         {
	///              //now encode nested container using its own specific encode methods
	///              //clear, then begin encoding of series - using same encIterator as vector
	///              series.Clear();
	///              retVal = series.EncodeInit(encIter, 0, 0);
	/// 
	///              //-----Continue encoding series entries. See example in Series ---
	///              retVal = series.EncodeComplete(encIter, success);    
	///         }
	/// 
	///         //SECOND Vector Entry: encode entry from pre-encoded buffer containing an encoded Series
	///         //assuming pEncSeries Buffer contains the pre-encoded payload with data and length populated
	///         //and pPermData contains permission data information
	/// 
	///         vectorEntry.Index = 2;
	///         //by passing permData on an entry, the map encoding functionality will implicitly set the
	///         //Vector.HAS_PER_ENTRY_PERM flag
	///         vectorEntry.Flags = VectorEntryFlags.HAS_PERM_DATA;
	///         vectorEntry.Action = VectorEntryActions.SET;
	///         vectorEntry.PermData = permData;
	///         vectorEntry.EncodedData(encSeries);
	/// 
	///         retVal = vectorEntry.Encode(encIter);
	/// 
	///         //THIRD Vector Entry: encode entry with clear action, no payload on clear
	///         //Should clear entry for safety, this will set flags to NONE 
	///         vectorEntry.Clear();
	///         vectorEntry.Index = 3;
	///         vectorEntry.Action = VectorEntryFlags.CLEAR;
	/// 
	///         retVal = vectorEntry.encode(encIter);
	/// }
	/// 
	/// //complete vector encoding.  If success parameter is true, this will finalize encoding.
	/// //If success parameter is false, this will roll back encoding prior to vector.encodeInit();
	///  retVal = vector.EncodeComplete(encIter, success);
	/// </code>
	/// 
	/// <para>
	/// <b>Vector decoding example</b>
	/// </para>
	/// <para>
	/// The following sample illustrates how to decode a <see cref="Vector"/> and is
	/// structured to decode each entry to the contained value. This sample code
	/// assumes the housed container type is a <see cref="Series"/>. Typically an
	/// application would invoke the specific container type decoder for the housed
	/// type or use a switch statement to allow a more generic series entry decoder.
	/// This example uses the same <see cref="DecodeIterator"/> when calling the content's
	/// decoder method. Optionally, an application could use a new
	/// <see cref="DecodeIterator"/> by setting the encData on a new iterator. To simplify
	/// the sample, some error handling is omitted.
	/// </para>
	/// 
	/// <code>
	/// //decode contents into the vector
	/// if((retVal = vector.Decode(decIter) >= CodecReturnCode.SUCCESS)
	/// {
	///      //create single vector entry and reuse while decoding each entry
	///      VectorEntry vectorEntry = new Vectory();
	/// 
	///      //if summary data is present, invoking decoder for that type (instead of DecodeEntry)
	///      //indicates to ETA that user wants to decode summary data 
	///      if(vector.Flags AND; VectorFlags.HAS_SUMMARY_DATA)
	///      {
	///          //summary data is present.  Its type should be that of vector.containerType
	///          Series series = new Series();
	///          retVal = series.Decode(decIter, series);
	/// 
	///           //Continue decoding series entries, see Series decoding example
	///      }
	/// 
	///      //decode each vector entry until there are no more left  
	///      while((retVal = vectorEntry.Decode(decIter)) != CodecReturnCode.END_OF_CONTAINER)
	///      {
	///          if(retVal &lt;  CodecReturnCode.SUCCESS)
	///          {
	///              //decoding failure tends to be unrecoverable 
	///          }
	///          else
	///          {
	///              Series series = new Series();
	///              retVal = series.Decode(decIter);
	/// 
	///              //Continue decoding series entries, see Series decoding example
	///          }
	///      }
	/// }
	/// else
	/// {
	///      //decoding failure tends to be unrecoverable
	/// }
	/// </code>
	/// 
	/// </remarks>
	/// <seealso cref="VectorEntry"/>
	/// <seealso cref="VectorFlags"/>
	sealed public class Vector : IXMLDecoder
    {
		internal int _containerType = DataTypes.CONTAINER_TYPE_MIN;
		internal readonly Buffer _encodedSetDefs = new Buffer();
		internal readonly Buffer _encodedSummaryData = new Buffer();
		internal int _totalCountHint;

        /// <summary>
        /// Creates <see cref="Vector"/>.
        /// </summary>
        /// <seealso cref="Vector"/>
        public Vector()
        {
            EncodedEntries = new Buffer();
        }

		/// <summary>
		/// Sets all members in <see cref="Vector"/> to an initial value. Useful for object
		/// reuse during encoding. While decoding, <see cref="Vector"/> object can be
		/// reused without using <see cref="Clear()"/>.
		/// </summary>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void Clear()
		{
		    Flags = 0;
			_containerType = DataTypes.CONTAINER_TYPE_MIN;
			_encodedSetDefs.Clear();
			_encodedSummaryData.Clear();
			_totalCountHint = 0;
			EncodedEntries.Clear();
		}

		/// <summary>
		/// Prepares vector for encoding.
		/// </summary>
		/// <remarks>
		/// 
		/// <para>Typical use:</para>
		/// <list type="number">
		/// <item> Call <see cref="Vector.EncodeInit(EncodeIterator, int, int)"/></item>
		/// <item> Call <see cref="VectorEntry.Encode(EncodeIterator)"/> or
		///   <see cref="VectorEntry.EncodeInit(EncodeIterator, int)"/>..<see cref="VectorEntry.EncodeComplete(EncodeIterator, bool)"/>
		///   for each entry using the same buffer</item>
		/// <item> Call <see cref="Vector.EncodeComplete(EncodeIterator, bool)"/></item>
		/// </list>
		/// </remarks>
		/// <param name="iter"> The encoder iterator. </param>
		/// <param name="summaryMaxSize"> max encoding size of summary data, if present </param>
		/// <param name="setMaxSize"> max encoding size of the set data, if present
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="EncodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeInit(EncodeIterator iter, int summaryMaxSize, int setMaxSize)
		{
			return Encoders.EncodeVectorInit(iter, this, summaryMaxSize, setMaxSize);
		}

        /// <summary>
        /// Complete set data encoding for a vector.
		/// </summary>
        /// <remarks>
        /// If both <see cref="EncodeSetDefsComplete(EncodeIterator, bool)"/> and <see cref="EncodeSummaryDataComplete(EncodeIterator, bool)"/>
        /// are called, <see cref="EncodeSetDefsComplete(EncodeIterator, bool)"/> must be called first.
        /// </remarks>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> <c>true</c> if encoding of set data was successful, <c>false</c> for rollback
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeSetDefsComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeVectorSetDefsComplete(iter, success);
		}

        /// <summary>
        /// Complete summary data encoding for a vector.
        /// </summary>
		/// <remarks>
        /// If both <see cref="EncodeSetDefsComplete(EncodeIterator, bool)"/> and <see cref="EncodeSummaryDataComplete(EncodeIterator, bool)"/>
        /// are called, <see cref="EncodeSetDefsComplete(EncodeIterator, bool)"/> must be called first.
		/// </remarks>
        /// <param name="iter"> Encoding iterator </param>
        /// <param name="success"> <c>true</c> if encoding of summary data was successful, <c>false</c> for rollback
        /// </param>
        /// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
        /// </returns>
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeSummaryDataComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeVectorSummaryDataComplete(iter, this, success);
		}

        /// <summary>
        /// Completes vector encoding.
        /// </summary>
        /// <remarks>
        /// 
        /// <para>Typical use:</para>
        /// <list type="number">
        /// <item> Call <see cref="Vector.EncodeInit(EncodeIterator, int, int)"/></item>
        /// <item> Call <see cref="VectorEntry.Encode(EncodeIterator)"/> or
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
        /// <seealso cref="EncodeIterator"/>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode EncodeComplete(EncodeIterator iter, bool success)
		{
			return Encoders.EncodeVectorComplete(iter, success, this);
		}

		/// <summary>
		/// Decode Vector.
		/// </summary>
		/// <param name="iter"> decode iterator
		/// </param>
		/// <returns> <c>CodecReturnCode.SUCCESS</c> upon successful completion.
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public CodecReturnCode Decode(DecodeIterator iter)
		{
			return Decoders.DecodeVector(iter, this);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.VECTOR, null, null, null, iter);
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
            return DecodersToXML.DecodeDataTypeToXML(DataTypes.VECTOR, null, dictionary, null, iter);
        }


		/// <summary>
		/// Checks the presence of the local Set Definition presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if present; <c>false</c> - if not present
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasSetDefs()
		{
			return (Flags & VectorFlags.HAS_SET_DEFS) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Summary Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if present; <c>false</c> - if not present
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasSummaryData()
		{
			return (Flags & VectorFlags.HAS_SUMMARY_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Per Entry Permission Data presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if present; <c>false</c> - if not present
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasPerEntryPermData()
		{
			return (Flags & VectorFlags.HAS_PER_ENTRY_PERM_DATA) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Total Count Hint presence flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if present; <c>false</c> - if not present
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckHasTotalCountHint()
		{
			return (Flags & VectorFlags.HAS_TOTAL_COUNT_HINT) > 0 ? true : false;
		}

		/// <summary>
		/// Checks the presence of the Supports Sorting indication flag.<br />
		/// <br />
		/// Flags may also be bulk-get via <see cref="Flags"/>.
		/// </summary>
		/// <returns> <c>true</c> - if present; <c>false</c> - if not present
		/// </returns>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public bool CheckSupportsSorting()
		{
			return (Flags & VectorFlags.SUPPORTS_SORTING) > 0 ? true : false;
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
			Flags = Flags | VectorFlags.HAS_SET_DEFS;
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
			Flags = Flags | VectorFlags.HAS_SUMMARY_DATA;
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
			Flags = Flags | VectorFlags.HAS_PER_ENTRY_PERM_DATA;
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
			Flags = Flags | VectorFlags.HAS_TOTAL_COUNT_HINT;
		}

		/// <summary>
		/// Applies the Supports Sorting indication flag.<br />
		/// <br />
		/// Flags may also be bulk-set via <see cref="Flags"/>.
		/// </summary>
		/// <seealso cref="Flags"/>
		[MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
		public void ApplySupportsSorting()
		{
			Flags = (Flags | VectorFlags.SUPPORTS_SORTING);
		}

        /// <summary>
        /// Gets or sets all the flags applicable to this vector.
        /// Must be in the range of 0 - 255.
        /// </summary>
        public VectorFlags Flags { get; set; }

        /// <summary>
        /// Gets or sets <see cref="DataTypes"/> enumeration value that describes the container type
        /// of each <see cref="VectorEntry"/>'s payload. containerType must be from the
        /// <see cref="DataTypes"/> enumeration in the range <see cref="DataTypes.CONTAINER_TYPE_MIN"/> to 255.
        /// </summary>
        public int ContainerType
        {
            get
            {
                return _containerType;
            }

            set
            {
                Debug.Assert(value >= DataTypes.CONTAINER_TYPE_MIN && value <= DataTypes.LAST,
                "containerType must be from the DataTypes enumeration in the range CONTAINER_TYPE_MIN to LAST.");

                _containerType = value;
            }
        }

        /// <summary>
		/// Gets or sets encoded set definitions. If populated, these definitions correspond to
		/// data contained within this Vector's entries and are used to encode or
		/// decode their contents.
		/// </summary>
		public Buffer EncodedSetDefs
        {
            get
            {
                return _encodedSetDefs;
            }

            set
            {
                Debug.Assert(value != null, "encodedSetDefs must be non-null");

                (_encodedSetDefs).CopyReferences(value);
            }
        }

        /// <summary>
		/// Raw encoded summary data. If populated, summary data contains information
		/// that applies to every entry encoded in the <see cref="Vector"/> (e.g. currency
		/// type). The container type of summary data must match the containerType
		/// specified on the <see cref="Vector"/>. If encodedSummaryData is populated while
		/// encoding, contents are used as pre-encoded summary data.
		/// </summary>
		public Buffer EncodedSummaryData
        {
            get
            {
                return _encodedSummaryData;
            }

            set
            {
                Debug.Assert(value != null, "encodedSummaryData must be non-null");

                (_encodedSummaryData).CopyReferences(value);
            }
        }

        /// <summary>
		/// Indicates the approximate total number of entries sent across all vectors
		/// on all parts of the refresh message. totalCountHint is typically used when
		/// multiple <see cref="Vector"/> containers are spread across multiple parts of a
		/// refresh message.Such information helps in determining the amount of resources
		/// to allocate for caching or displaying all expected entries.
		/// Must be in the range of 0 - 1073741823.
		/// </summary>
		public int TotalCountHint
		{
            get
            {
                return _totalCountHint;
            }

            set
            {
                Debug.Assert(value >= 0 && value <= 1073741823, "totalCountHint is out of range (0-1073741823)");

                _totalCountHint = value;
            }
		}

        /// <summary>
		/// Gets encoded index-value pair encoded data contained in the message.
		/// This would refer to encoded <see cref="Vector"/> payload.
		/// </summary>
		public Buffer EncodedEntries { get; internal set; }
	}
}