/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// Login RTT Flags, indicating the presence of a data member or a specific feature.
    /// </summary>
    /// <seealso cref="LoginRTT"/>
    [Flags]
    public enum LoginRTTFlags : int
    {
        /// <summary>
        /// (0x00) No flags set
        /// </summary>
        NONE = 0x00,

        /// <summary>
        /// (0x01) Indicates that this message was sent by the provider
        /// </summary>
        PROVIDER_DRIVEN = 0x01,

        /// <summary>
        /// (0x02) Indicates presence of the TCP retransmission member
        /// </summary>
        HAS_TCP_RETRANS = 0x02,

        /// <summary>
        /// (0x04) Has RoundTrip latency
        /// </summary>
        ROUND_TRIP_LATENCY = 0x04
    }
}
