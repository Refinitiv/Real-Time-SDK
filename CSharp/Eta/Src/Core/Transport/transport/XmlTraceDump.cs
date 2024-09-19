/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System.Text;
using LSEG.Eta.Codec;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Helper class that provides some useful functionality for dumping data to XML
    /// </summary>
    sealed public class XmlTraceDump
    {

        DecodeIterator xmlDumpIterator = new DecodeIterator();
        Eta.Codec.Buffer xmlDumpBuffer = new Eta.Codec.Buffer();
        Msg xmlDumpMsg = new Msg();

        /// <summary>
        /// Dumps data in RWF format to XML string
        /// </summary>
        /// <param name="channel"><see cref="IChannel"/> instance</param>
        /// <param name="protocolType">Protocol Type</param>
        /// <param name="buffer">The <see cref="ITransportBuffer"/> instance that contains input data</param>
        /// <param name="dataDictionary"><see cref="DataDictionary"/> instance used to decode the data</param>
        /// <param name="msgBuilder"><see cref="StringBuilder"/> instance that will contain output XML</param>
        /// <param name="error">A reference that will point to an <see cref="Error"/> instance with error informaiton in case of failure of dump operation</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode DumpBuffer(IChannel channel, int protocolType, ITransportBuffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, out Error error)
        {
            if (channel == null)
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Input Channel must not be null.\n");
                return TransportReturnCode.FAILURE;
            }

            if (buffer == null)
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Input TransportBuffer must not be null.\n");
                return TransportReturnCode.FAILURE;
            }

            if (buffer.Length() <= 0)
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Buffer of length zero cannot be dumped.\n");
                return TransportReturnCode.FAILURE;
            }

            System.String result;
            if (Codec.Codec.RWF_PROTOCOL_TYPE == protocolType)
            {
                xmlDumpIterator.Clear();
                xmlDumpIterator.SetBufferAndRWFVersion(buffer, channel.MajorVersion, channel.MinorVersion);
                result = xmlDumpMsg.DecodeToXml(xmlDumpIterator, dataDictionary);
            } else
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Unsupported protocol type: " + protocolType + "\n");
                return TransportReturnCode.FAILURE;
            }

            msgBuilder.Append(result);
            error = null;
            return TransportReturnCode.SUCCESS;
        }

        /// <summary>
        /// Dumps data in RWF format to XML string
        /// </summary>
        /// <param name="majorVersion">RWF major version</param>
        /// <param name="minorVersion">RWF minor version</param>
        /// <param name="protocolType">Protocol Type</param>
        /// <param name="buffer">The <see cref="Buffer"/> instance that contains input data</param>
        /// <param name="dataDictionary"><see cref="DataDictionary"/> instance used to decode the data</param>
        /// <param name="msgBuilder"><see cref="StringBuilder"/> instance that will contain output XML</param>
        /// <param name="error">A reference that will point to an <see cref="Error"/> instance with error informaiton in case of failure of dump operation</param>
        /// <returns><see cref="TransportReturnCode"/></returns>
        public TransportReturnCode DumpBuffer(int majorVersion, int minorVersion, int protocolType, Buffer buffer, DataDictionary dataDictionary, StringBuilder msgBuilder, out Error error)
        {
            if (buffer == null)
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Input TransportBuffer must not be null.\n");
                return TransportReturnCode.FAILURE;
            }

            if (buffer.GetLength() <= 0)
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Buffer of length zero cannot be dumped.\n");
                return TransportReturnCode.FAILURE;
            }

            System.String result;
            if (Codec.Codec.RWF_PROTOCOL_TYPE == protocolType)
            {
                xmlDumpIterator.Clear();
                xmlDumpIterator.SetBufferAndRWFVersion(buffer, majorVersion, minorVersion);
                result = xmlDumpMsg.DecodeToXml(xmlDumpIterator, dataDictionary);
            }
            else
            {
                error = new Error(errorId: TransportReturnCode.FAILURE, text: "Unsupported protocol type: " + protocolType + "\n");
                return TransportReturnCode.FAILURE;
            }

            msgBuilder.Append(result);
            error = null;
            return TransportReturnCode.SUCCESS;
        }
    }
}
