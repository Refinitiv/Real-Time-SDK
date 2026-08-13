/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.codec;


import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;

/**
 * Interface for dumping ETA message buffer contents to XML format for tracing and debugging.
 */
public interface XmlTraceDump {

    /**
     * Dumps the contents of a transport buffer to XML format.
     *
     * @param channel the channel associated with the buffer
     * @param protocolType the protocol type (RWF or JSON)
     * @param buffer the transport buffer containing data to dump
     * @param dataDictionary the data dictionary used for decoding, may be null
     * @param msgBuilder the StringBuilder to append the XML output to
     * @param error error information, populated in event of failure
     *
     * @return {@link CodecReturnCodes#SUCCESS} if successful, or a failure code
     */
    int dumpBuffer(Channel channel, int protocolType, TransportBuffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error);

    /**
     * Dumps the contents of a buffer to XML format using specified version information.
     *
     * @param majorVersion the major version of the RWF protocol
     * @param minorVersion the minor version of the RWF protocol
     * @param protocolType the protocol type (RWF or JSON)
     * @param buffer the buffer containing data to dump
     * @param dataDictionary the data dictionary used for decoding, may be null
     * @param msgBuilder the StringBuilder to append the XML output to
     * @param error error information, populated in event of failure
     *
     * @return {@link CodecReturnCodes#SUCCESS} if successful, or a failure code
     */
    int dumpBuffer(int majorVersion, int minorVersion, int protocolType, Buffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, Error error);
}
