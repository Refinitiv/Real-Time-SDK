/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;

namespace LSEG.Eta.Transports.Internal
{
    // The connection versions of the RIPC protocol.
    internal enum ConnectionsVersions
    {
        VERSION14 = 0x0017,
        VERSION13 = 0x0016,
        VERSION12 = 0x0015,
        VERSION11 = 0x0014
    }

    // The RIPC versions of the RIPC protocol.
    internal enum RipcVersions
    {
        VERSION14 = 9,
        VERSION13 = 8,
        VERSION12 = 7,
        VERSION11 = 6
    }

    internal struct RipcVersionInfo
    {
        public const int CurrentVersion = (int)ConnectionsVersions.VERSION13;
        public const int MaxUserMsgSize = 8192;
        public const byte PingTimeout = 0xFF;
        public const byte MajorVersion = 14;
        public const byte MinorVersion = 0;

        public const int ComponentVersionStringLenMax = 253;
    }

    [Flags]
    internal enum RipcFlags : byte
    {
        NONE = 0x00,

        // The presence of this flag indicates the message has optional (a.k.a. "extended" flags)
        HAS_OPTIONAL_FLAGS = 0x01,

        // The presence of this flag indicates a normal message.
        DATA = 0x02,

        // The presence of this flag indicates a compressed message.
        COMPRESSION = 0x04,

        // The presence of this flag indicates a compressed fragmented message.
        COMP_FRAGMENT = 0x08,

        // The presence of this flag indicates that the server has forced compression for this connection.
        FORCE_COMPRESSION = 0x80,

        // The presence of this flag indicates a packed message.
        PACKING = 0x10
    }

    [Flags]
    internal enum RipcOpCode : byte
    {
        // The presence of this flag indicates the message is a Connection Acknowledgment.
        CONNECT_ACK = 0x01,

        // The presence of this flag indicates the message is a Connection NAK.
        CONNECT_NAK = 0x02,

        // The presence of this flag indicates the message is a continuation of a fragmented message.
        FRAGMENT = 0x04,

        // The presence of this flag indicates the message contains a fragment header (a.k.a. "the start of a fragmented message")
        FRAGMENT_HEADER = 0x08
    }

    internal enum Unused : byte
    {

    }

    internal enum RipcSessionFlags : byte
    {
        NONE = 0x00,
        CLIENT_TO_SERVER_PING = 0x01,
        SERVER_TO_CLIENT_PING = 0x02
    }

    internal class RipcCompressionTypes
    {
        public const byte NONE = 0;
        public const byte ZLIB = 1;
        public const byte LZ4 = 2;
        public const byte MAX_DEFINED = LZ4;
    }

    internal class RipcOffsets
    {
        /* This represents offset for message length or the RIPC message flag.*/
        public const int MSG_FLAG = 2;

        /* This represents offset for the RIPC message extended flags. */
        public const int EXTENDED_FLAGS = MSG_FLAG + 1;

        /* This represents offset for the entire length of the payload in the fragmented message. */
        public const int FRAGMENTED_MSG_LENGTH = EXTENDED_FLAGS + 1;

        /* This is the length of Flag ID in RIPC13 and later, which is two bytes.*/
        public const int FRAGMENTED_ID_LENGTH_RIPC13 = 2;

        /* This is the length of Flag ID in pre RIPC13, which is one byte. */
        public const int FRAGMENTED_ID_LENGTH_RIPC12 = 1;

        /* This represents offset containing a fragmented header(the first part of a fragmented message) contains the ID of the fragment */
        public const int FRAGMENT_HEADER_FRAGMENT_ID = FRAGMENTED_MSG_LENGTH + 4;

        /* This represents offset containing the continuation of a fragmented message contains the ID of the fragment*/
        public const int FRAGMENT_ID = EXTENDED_FLAGS + 1;

        /* This represents offset where data begins for a packed message.*/
        public const int PACKED_MSG_DATA = 2;
    }

    internal class RipcLengths
    {
        /* The number of byes in RIPC message header*/
        public const int HEADER = 3;

        /* The number of bytes in the first fragment header, which includes the RIPC header(2), the Extended flags(1)
         * and the total message length(4).
         */
        public const int FIRST_FRAGMENT_WITHOUT_FRAGID = 8;

        /* The number of bytes in the additional fragment header, which include the RIPC header(3) and the extended flags(1) */
        public const int ADDITIONAL_FRAGMENT_WITHOUT_FRAGID = 4;
    }

}
