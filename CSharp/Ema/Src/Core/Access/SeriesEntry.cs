﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// SeriesEntry represents an entry of Series.
    /// <p>SeriesEntry associates entry's data and its data type.</p>
    /// 
    /// Objects of this class are intended to be short lived or rather transitional.
    /// This class is designed to efficiently perform setting and extracting of Map and its content.
    /// Objects of this class are not cache-able.
    /// </summary>
    public sealed class SeriesEntry : Entry
    {
        internal Eta.Codec.SeriesEntry m_rsslSeriesEntry;

        internal SeriesEntry() 
        {
            m_rsslSeriesEntry = new Eta.Codec.SeriesEntry();
        }

        internal CodecReturnCode Decode(DecodeIterator decodeIterator)
        {
            m_rsslSeriesEntry.Clear();
            return m_rsslSeriesEntry.Decode(decodeIterator);
        }

        /// <summary>
        /// Clears current SeriesEntry instance
        /// </summary>
        public void Clear()
        {
            m_rsslSeriesEntry.Clear();
            ClearLoad();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="SeriesEntry"/> instance.</returns>
        public override string ToString()
        {
            if (Load == null)
                return "\nDecoding of just encoded object in the same application is not supported\n";

            m_toString.Length = 0;

            m_toString.Append("SeriesEntry dataType=\"").Append(DataType.AsString(Load.DataType)).Append("\"").AppendLine();
            m_toString.Append(Load.ToString(1));
            Utilities.AddIndent(m_toString, 0).Append("SeriesEntryEnd").AppendLine();

            return m_toString.ToString();
        }
    }
}
