/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;

namespace Refinitiv.Eta.PerfTools.Common
{
    /// <summary>
    /// Represents one field in a fieldList (used by MarketPrice and MarketByOrder messages). 
    /// Stores the FieldID and dataType of the desired data using a FieldEntry, 
    /// as well as the value to use.
    /// </summary>
    public class MarketField
    {
        /// <summary>
        /// The market field entry
        /// </summary>
        public FieldEntry FieldEntry { get; set; } = new FieldEntry();

        /// <summary>
        /// The market field value
        /// </summary>
        public object? Value { get; set; }
    }
}
