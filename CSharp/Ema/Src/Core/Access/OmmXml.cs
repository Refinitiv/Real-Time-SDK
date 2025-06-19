/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmXml represents XML data format in Omm.
    /// </summary>
    /// <remarks>
    /// Method <see cref="StructuredTextData.GetString"/> can have performance decrease, since it will trigger garbage collection.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of XML and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public sealed class OmmXml : StructuredTextData
    {
        /// <summary>
        /// Constructor for OmmXml class
        /// </summary>
        public OmmXml()
        {
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.XML;
        }

        /// <summary>
        /// Clears the OmmXml instance.
        /// </summary>
        /// <returns>Reference to current <see cref="OmmXml"/> object.</returns>
        public OmmXml Clear()
        {
            Clear_All();
            return this;
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies XML data using string</param>
        /// <returns>Reference to current <see cref="OmmXml"/> object.</returns>
        public new OmmXml SetString(string value)
        {
            base.SetString(value);
            return this;
        }

        /// <summary>
        /// Encodes specified EmaBuffer containing XML data.
        /// </summary>
        /// <param name="value"> specifies XML data using <see cref="EmaBuffer"/></param>
        /// <returns>Reference to current <see cref="OmmXml"/> object.</returns>
        public new OmmXml SetBuffer(EmaBuffer value)
        {
            base.SetBuffer(value);
            return this;
        }
    }
}