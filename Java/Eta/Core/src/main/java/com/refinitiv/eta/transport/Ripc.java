package com.refinitiv.eta.transport;

/* Defines constants related to the RIPC protocol */
class Ripc
{
    static final byte COMP_BITMAP_SIZE = 1; /* Current number of bytes in the compression bitmap */

    /* RIPC message flags */
    static class Flags
    {
        /* The presence of this flag indicates the message has optional (a.k.a. "extended" flags) */
        static final int HAS_OPTIONAL_FLAGS = 0x01;

        /* The presence of this flag indicates a packed message. */
        static final int PACKING = 0x10;

        /* The presence of this flag indicates a compressed message. */
        static final int COMPRESSION = 0x04;

        /* The presence of this flag indicates a compressed fragmented message. */
        static final int COMP_FRAGMENT = 0x08;

        /* The presence of this flag indicates that the server has forced compression for this connection. */
        static final int FORCE_COMPRESSION = 0x80;

        /* Optional RIPC message flags */
        static class Optional
        {
            /* The presence of this flag indicates the message is a Connection Acknowledgment. */
            static final int CONNECT_ACK = 0x01;

            /* The presence of this flag indicates the message is a Connection NAK. */
            static final int CONNECT_NAK = 0x02;

            /* The presence of this flag indicates the message is a continuation of a fragmented message. */
            static final int FRAGMENT = 0x04;

            /* The presence of this flag indicates the message contains a fragment header (a.k.a. "the start of a fragmented message") */
            static final int FRAGMENT_HEADER = 0x08;
        }
    }

    /* Offsets in a RIPC message */
    static class Offsets
    {
        /* This "rwfUInt8" (offset from the start of the message) contains the RIPC message flag. */
        static final int MSG_FLAG = 2;

        /* This "rwfUInt8" (offset from the start of the message) contains the RIPC message extended flags. */
        static final int EXTENDED_FLAGS = MSG_FLAG + 1;

        /* This "rwfUnit32" (offset from the start of the message) contains the entire length of the *data* in the fragmented message. */
        static final int FRAGMENTED_MSG_LENGTH = EXTENDED_FLAGS + 1;

        /* This is the length of the fragId in RIPC13 and beyond, which is two bytes. */
        static final int FRAGMENT_ID_LENGTH_RIPC13 = 2;

        /* This is the length of the fragId in pre-RIPC13, which is one byte. */
        static final int FRAGMENT_ID_LENGTH_RIPC12 = 1;

        /* This "rwfUInt8 (pre RIPC13)" or "rwfUInt16 (RIPC13 and beyond)"
         * (offset from the start of a message) containing a fragmented header
         * (the first part of a fragmented message) contains the ID of the fragment.
         */
        static final int FRAGMENT_HEADER_FRAGMENT_ID = FRAGMENTED_MSG_LENGTH + 4;

        /* This "rwfUInt8 (pre RIPC13)" or "rwfUInt16 (RIPC13 and beyond)"
         * (offset from the start of a message) containing the continuation of a
         * fragmented message contains the ID of the fragment.
         */
        static final int FRAGMENT_ID = EXTENDED_FLAGS + 1;

        /* This byte (offset from the start of a packed message) is where it's data begins. */
        static final int PACKED_MSG_DATA = 2;
    }

    /* Length properties of RIPC messages */
    static class Lengths
    {
        /* The number of bytes in RIPC message header */
        static final int HEADER = 3;

        /* The number of bytes in the first fragment header, which includes
         * the RIPC header(3), the Extended flags(1) and the total message length(4).
         */
        static final int FIRST_FRAGMENT_WITHOUT_FRAGID = 8;

        /* The number of bytes in the additional fragment header, which includes the RIPC header(3) and the extended flags(1). */
        static final int ADDITIONAL_FRAGMENT_WITHOUT_FRAGID = 4;
    }

    /* The connection versions of the RIPC protocol. */
    static class ConnectionVersions
    {
        static final int VERSION14 = 0x0017;
        static final int VERSION13 = 0x0016;
        static final int VERSION12 = 0x0015;
        static final int VERSION11 = 0x0014;
    }

    /* The RIPC versions of the RIPC protocol. */
    static class RipcVersions
    {
        static final int VERSION14 = 9;
        static final int VERSION13 = 8;
        static final int VERSION12 = 7;
        static final int VERSION11 = 6;
    }

    /* Defines client to server and server to client pings. */
    static class SessionFlags
    {
        /* consumer sends pings to provider */
        static final int CLIENT_TO_SERVER_PING = 0x1;
        
        /* provider sends pings to consumer */
        static final int SERVER_TO_CLIENT_PING = 0x2;
    }

    /* Internal compression type definitions.
     * These values are defined from the public CompTypes.
     * LZ4 is currently not defined on the public interface.
     */
    static class CompressionTypes
    {
        public static final int NONE = 0;
        public static final int ZLIB = 1;
        public static final int LZ4 = 2;
        public static final int MAX_DEFINED = LZ4;
    }
}
