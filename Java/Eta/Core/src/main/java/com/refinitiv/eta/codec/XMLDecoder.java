/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;

/** 
 * The ETA XML Decoder. Decodes a container of binary data to XML.<BR>
 * 
 * Note that the container to be decoded must be the next piece of data
 * in line to be decoded before this method is called.<BR>
 * 
 * Calling {@link #decodeToXml(DecodeIterator)} on the message container decodes all
 * containers within the message to XML.
 * 
 * @see DataTypes
 */
public interface XMLDecoder
{
    /**
     * Decodes to XML the data next in line to be decoded with the iterator.
     * Data that require dictionary lookups are decoded to hexadecimal strings.
     * 
     * @param iter Decode iterator
     * 
     * @return The XML representation of the container
     * 
     * @see DecodeIterator
     */
    public String decodeToXml(DecodeIterator iter);

    /**
     * Decodes to XML the data next in line to be decoded with the iterator.
     * 
     * @param iter Decode iterator
     * @param dictionary Data dictionary
     * 
     * @return The XML representation of the container
     * 
     * @see DecodeIterator
     * @see DataDictionary
     */
    public String decodeToXml(DecodeIterator iter, DataDictionary dictionary);
}
