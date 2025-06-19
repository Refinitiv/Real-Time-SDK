/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Represents one MarketPrice message.
    /// </summary>
    public class MarketPriceMsg
    {
        /// <summary>
        /// List of fields
        /// </summary>
        public MarketField[] FieldEntries { get; set; }

        /// <summary>
        /// Number of fields in list
        /// </summary>
        public int FieldEntryCount { get; set; }

        /// <summary>
        /// Estimated size of payload
        /// </summary>
        public int EstimatedContentLength { get; set; }

        /// <summary>
        /// Count for field entry array
        /// </summary>
        public int ArrayCount { get; set; }

        public MarketPriceMsg(int count)
        {
            ArrayCount = count;
            FieldEntries = new MarketField[count];
        }
    }
}
