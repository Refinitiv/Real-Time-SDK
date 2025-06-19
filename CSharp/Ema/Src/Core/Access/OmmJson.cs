/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// OmmJson represents JSON data format in Omm.
    /// </summary>
    /// <remarks>
    /// Method <see cref="StructuredTextData.GetString"/> can have performance decrease, since it will trigger garbage collection.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of JSON and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public sealed class OmmJson : StructuredTextData
    {
        /// <summary>
        /// Constructor for OmmJson class
        /// </summary>
        public OmmJson()
        {
            ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
            m_dataType = Access.DataType.DataTypes.JSON;
        }

        /// <summary>
        /// Clears the OmmJson instance.
        /// </summary>
        /// <returns>Reference to current <see cref="OmmJson"/> object.</returns>
        public OmmJson Clear()
        {
            Clear_All();
            return this;
        }

        /// <summary>
        /// Specifies Set.
        /// </summary>
        /// <param name="value">specifies JSON data using string</param>
        /// <returns>Reference to current <see cref="OmmJson"/> object.</returns>
        public new OmmJson SetString(string value)
        {
            base.SetString(value);
            return this;
        }

        /// <summary>
        /// Encodes specified EmaBuffer containing JSON data.
        /// </summary>
        /// <param name="value"> specifies JSON data using <see cref="EmaBuffer"/></param>
        /// <returns>Reference to current <see cref="OmmJson"/> object.</returns>
        public new OmmJson SetBuffer(EmaBuffer value)
        {
            base.SetBuffer(value);
            return this;
        }
    }
}