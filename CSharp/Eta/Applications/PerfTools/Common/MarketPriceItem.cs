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
    /// Market price item data. Allows threads 
    /// to uniquely retrieve message data information from cached XML file contents.
    /// </summary>
    public class MarketPriceItem
    {
        /// <summary>
        /// Item data for message.
        /// </summary>
        public int IMsg { get; set; }
    }
}
