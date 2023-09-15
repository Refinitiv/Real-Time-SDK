/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// Represents container that has no data
    /// </summary>
    public sealed class NoData : ComplexType
    {
        /// <summary>
        ///  Gets the <see cref="DataType.DataTypes"/>, which is the type of Omm data.
        /// </summary>
        public override int DataType => Access.DataType.DataTypes.NO_DATA;

        internal override CodecReturnCode Decode(DecodeIterator dIter)
        {
            return CodecReturnCode.SUCCESS;
        }

        internal override CodecReturnCode Decode(int majorVersion, int minorVersion, Buffer body, DataDictionary? dictionary, object? localDb)
        {
            return CodecReturnCode.SUCCESS;
        }

        internal override string ToString(int indent)
        {
            m_ToString.Length = 0;
            Utilities.AddIndent(m_ToString, indent).Append("NoData").AppendLine();
            Utilities.AddIndent(m_ToString, indent).Append("NoDataEnd").AppendLine();

            return m_ToString.ToString();
        }
    }
}
