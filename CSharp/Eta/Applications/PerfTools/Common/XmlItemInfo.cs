/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// Contains information about an item in the item list
    /// </summary>
    public class XmlItemInfo
    {
        /// <summary>
        /// Domain Type of the item
        /// </summary>
        public int DomainType { get; set; }

        /// <summary>
        /// Name of the item
        /// </summary>
        public string? Name { get; set; }

        /// <summary>
        /// Whether the item is to be used for consumer posting
        /// </summary>
        public bool IsPost { get; set; }

        /// <summary>
        /// Whether the item is to be used for consumer to send generic msgs
        /// </summary>
        public bool IsGenMsg { get; set; }

        /// <summary>
        /// Whether the item is to be used to carry latency information
        /// </summary>
        public bool IsSnapshot { get; set; }
    }
}
