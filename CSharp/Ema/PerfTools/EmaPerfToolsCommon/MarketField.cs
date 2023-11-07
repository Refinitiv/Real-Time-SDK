/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.PerfTools.Common
{
    /// <summary>
    /// Represents one field in a fieldList (used by MarketPrice and MarketByOrder messages). 
    /// Stores the FieldID and dataType of the desired data using a FieldEntry, 
    /// as well as the value to use.
    /// </summary>
    public class MarketField
    {
        public int FieldId;                // The market field id.
        public int LoadType;               // The market field load type.
        public string? Value;               // The market field init value.
        public QosWrapper? Qos;
        public StateWrapper? State;
        public bool Blank;
        public object? FieldEntry;

        /// <summary>
        /// Instantiates a new market field.
        /// </summary>
        /// <param name="fieldId">the market field id</param>
        /// <param name="loadType">the market field load type</param>
        /// <param name="value">the market field init value</param>
        /// <param name="qosWrapper">Qos helper object</param>
        /// <param name="stateWrapper">State helper object</param>
        public MarketField(int fieldId, int loadType, string value, QosWrapper qosWrapper, StateWrapper stateWrapper)
        {
            FieldId = fieldId;
            LoadType = loadType;
            Value = value;
            Qos = qosWrapper;
            State = stateWrapper;
            Blank = value == null || value.Length == 0;
        }

        public MarketField() { }
    }
}
