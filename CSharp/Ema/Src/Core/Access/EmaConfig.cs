/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// This class contains enumerations and other configuration values used in the EMA configuration classes.
    /// </summary>
    public static class EmaConfig
    {
        /// <summary>
        /// This enumeration defines the connection types available for the Channel ConnectionType configuration elements.
        /// </summary>
        public class ConnectionTypeEnum
        {
            /// <summary>
            /// This connection type uses the TCP-based ETA Socket connection protocol.
            /// </summary>
            public const int SOCKET = 0;

            /// <summary>
            /// This connection type uses a TLS encrypted connection.  Additional protocol configuration is defined in EncryptedProtocolType.
            /// </summary>
            public const int ENCRYPTED = 1;

            /// <summary>
            /// max defined connectionType
            /// </summary>
            public const int MAX_DEFINED = ENCRYPTED;
        }

        /// <summary>
        /// This enumeration defines the TLS encryption protocol versions used by the Channel EncryptedProtocolType configuration element.
        /// </summary>
        public class EncryptedTLSProtocolFlags
        {
            /// <summary>
            /// Allows the operating system to choose the best protocol to use, and to block protocols that are not secure. 
            /// Unless your app has a specific reason not to, you should use this field
            /// </summary>
            public const uint NONE = 0;

            /// <summary>
            /// This connection uses the TLS 1.2 protocol to handle key exchange and encryption.
            /// </summary>
            public const uint TLSv1_2 = 0x04;

            /// <summary>
            /// This connection uses the TLS 1.3 protocol to handle key exchange and encryption.
            /// </summary>
            public const uint TLSv1_3 = 0x08;

            /// <summary>
            /// This connection will attempt configurable protocols to handle key exchange and encryption.
            /// </summary>
            public const uint TLS_ALL = (TLSv1_2 |TLSv1_3);
        }

        /// <summary>
        /// This enumeration defines the compression algorithms available for the Channel CompressionType configuration elements.
        /// </summary>
        public class CompressionTypeEnum
        {
            /// <summary>
            /// No compression is desired on the connection.
            /// </summary>
            public const int NONE = 0;

            /// <summary>
            /// Use of zlib compression is desired on the connection. Zlib, an open
            /// source utility, employs a variation of the LZ77 algorithm while
            /// compressing and decompressing data.
            /// </summary>
            public const int ZLIB = 1;

            /// <summary>
            /// Use of lz4 compression is desired on the connection. Lz4 is a lossless 
            /// data compression algorithm that is focused on compression and decompression speed.
            /// It belongs to the LZ77 family of byte-oriented compression schemes.
            /// </summary>
            public const int LZ4 = 2;

            /// <summary>
            /// Max defined Compression type.
            /// </summary>
            public const int MAX_DEFINED = LZ4;
        }

        /// <summary>
        /// This enumeration defines the Dictionary Loading mode for the Dictionary DictionaryType configuration elements.
        /// </summary>
        public class DictionaryTypeEnum
        {
            /// <summary>
            /// Load the dictionary from a file, using the RdmFieldDictionaryFileName and EnumTypeDefFileName configuration elements.
            /// Default file names are RDMFieldDictionary and enumtype.def, respectively.
            /// </summary>
            public const int FILE = 0;
            /// <summary>
            /// Request the dictionary from the upstream provider once connected. Item names are defined by RdmFieldDictionaryItemName and EnumTypeDefItemName configuration elements.
            /// Default item names are "RWFFld" 
            /// </summary>
            public const int CHANNEL = 1;
            /// <summary>
            /// Max defined Dictionary loading mode type.
            /// </summary>
            public const int MAX_DEFINED = CHANNEL;
        }

        /// <summary>
        /// This enumeration defines the logging severity for the Logger LoggerSeverity configuration elements.
        /// </summary>
        public class LoggerLevelEnum
        {
            /// <summary>
            /// The EMA Logger will log verbose or trace events in addition to any below log events.
            /// </summary>
            public const int TRACE = 0;

            /// <summary>
            /// The EMA Logger will log informative events in addition to any below log events.
            /// </summary>
            public const int INFO = 2;

            /// <summary>
            /// The EMA Logger will log warning events in addition to any below log events.
            /// </summary>
            public const int WARNING = 3;

            /// <summary>
            /// The EMA Logger will log error events.
            /// </summary>
            public const int ERROR = 4;

            /// <summary>
            /// The EMA Logger will not log anything.
            /// </summary>
            public const int OFF = 5;

            /// <summary>
            /// Max defined Logger severity type.
            /// </summary>
            public const int MAX_DEFINED = OFF;
        }

        /// <summary>
        /// This enumeration defines logger type for the Logger LoggerType configuration elements.
        /// </summary>
        public class LoggerTypeEnum
        {
            /// <summary>
            /// Configure the EMA Logger to output to a file.
            /// </summary>
            public const int FILE = 0;

            /// <summary>
            /// Configure the EMA Logger to output to the standard out stream.
            /// </summary>
            public const int STDOUT = 1;

            /// <summary>
            /// Max defined Logger type.
            /// </summary>
            public const int MAX_DEFINED = STDOUT;
        }
}
}
