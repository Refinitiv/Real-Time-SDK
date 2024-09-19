/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Linq;
using System.Text.RegularExpressions;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Encryption protocol types specified with the <see cref="ConnectionType.ENCRYPTED"/> connection type.
    /// </summary>
    [Flags]
    public enum EncryptionProtocolFlags
    {
        /// <summary>
        /// Allows the operating system to choose the best protocol to use, and to block protocols that are not secure. 
        /// Unless your app has a specific reason not to, you should use this field
        /// </summary>
        ENC_NONE = 0x00,

        /// <summary>
        /// Encryption using TLSv1.2 protocol
        /// </summary>
        ENC_TLSV1_2 = 0x04,

        /// <summary>
        /// Encryption using TLSv1.3 protocol
        /// </summary>
        ENC_TLSV1_3 = 0x08
    }

    /// <summary>
    /// Extension class for enumeration of EncryptionProtocolFlags.
    /// </summary>
    public static class EncryptionProtocolFlagsExtension
    {
        private static readonly Regex EncryptionProtocolFlagsRegex = new(
            "((tls){0,1}(v){0,1}1.(?<ver>[23]))?((;){0,1})",
                RegexOptions.Compiled | RegexOptions.IgnoreCase);

        /// <summary>
        /// Parses a string value to EncryptionProtocolFlags.
        /// </summary>
        /// <param name="str">Input string(e.g. TlsV1.3 or TlsV1.2;TlsV1.3).</param>
        /// <param name="value">Result of parsing.</param>
        /// <returns>true if successful;otherwise return false.</returns>
        public static bool TryParse(string str, out EncryptionProtocolFlags value)
        {
            value = EncryptionProtocolFlags.ENC_NONE;
            foreach(Match m in EncryptionProtocolFlagsRegex.Matches(str).Cast<Match>())
            {
                if (m.Success)
                {
                    var verGroup = m.Groups["ver"];
                    if (verGroup.Success)
                    {
                        int ordinalVersionNumber = int.Parse(verGroup.Value);
                        value |= ordinalVersionNumber switch
                        {
                            2 => EncryptionProtocolFlags.ENC_TLSV1_2,
                            3 => EncryptionProtocolFlags.ENC_TLSV1_3,
                            _ => EncryptionProtocolFlags.ENC_NONE,
                        };
                    }
                }
            }

            return value != EncryptionProtocolFlags.ENC_NONE;
        }
    }
}
