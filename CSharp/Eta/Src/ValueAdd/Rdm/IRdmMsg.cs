/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Common interface for admin domain messages
    /// </summary>
    public interface IRdmMsg
    {
        /// <summary>
        /// The domain type of the message
        /// </summary>
        public Eta.Rdm.DomainType DomainType { get; }
        /// <summary>
        /// Message StreamId
        /// </summary>
        public int StreamId { get; set; }
        /// <summary>
        /// Encodes the contents of the message using the provided EncodeIterator
        /// </summary>
        /// <param name="encodeIter"><see cref="EncodeIterator"/> instance</param>
        /// <returns><see cref="CodecReturnCode"/> value representing the status of the operation</returns>
        public CodecReturnCode Encode(EncodeIterator encodeIter);
        /// <summary>
        /// Decodes the provided message
        /// </summary>
        /// <param name="decodeIter"><see cref="DecodeIterator"/> instance</param>
        /// <param name="msg">the message to be decoded</param>
        /// <returns><see cref="CodecReturnCode"/> value indicating the status of the operation</returns>
        public CodecReturnCode Decode(DecodeIterator decodeIter, IMsg msg);
    }
}
