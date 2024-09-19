/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Codec
{
	/// <summary>
	/// The ETA XML Decoder. Decodes a container of binary data to XML.<para />
	/// 
	/// Note that the container to be decoded must be the next piece of data
	/// in line to be decoded before this method is called.<para />
	/// 
	/// Calling <see cref="DecodeToXml(DecodeIterator)"/> on the message container decodes all
	/// containers within the message to XML.
	/// </summary>
	/// <seealso cref="DataTypes"/>
	public interface IXMLDecoder
	{
		/// <summary>
		/// Decodes to XML the data next in line to be decoded with the iterator.
		/// Data that require dictionary lookups are decoded to hexadecimal strings.
		/// </summary>
		/// <param name="iter"> Decode iterator
		/// </param>
		/// <returns> The XML representation of the container
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		string DecodeToXml(DecodeIterator iter);

		/// <summary>
		/// Decodes to XML the data next in line to be decoded with the iterator.
		/// </summary>
		/// <param name="iter"> Decode iterator </param>
		/// <param name="dictionary"> Data dictionary </param>
		/// <returns> The XML representation of the container
		/// </returns>
		/// <seealso cref="DecodeIterator"/>
		/// <seealso cref="DataDictionary"/>
		string DecodeToXml(DecodeIterator iter, DataDictionary dictionary);
	}

}