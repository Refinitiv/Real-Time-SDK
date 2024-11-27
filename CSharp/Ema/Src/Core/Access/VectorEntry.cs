/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// VectorEntry represents an entry of Vector.<br/>
    /// VectorEntry associates entry's position, action, permission information, data and its data type.
    /// </summary>
    /// <remarks>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// This class is designed to efficiently perform setting and extracting of Vector and its content.<br/>
    /// Objects of this class are not cache-able.<br/>
    /// </remarks>
    public sealed class VectorEntry : Entry
    {
        internal Eta.Codec.VectorEntry m_rsslVectorEntry;
        private EmaBuffer m_permissionBuffer = new EmaBuffer();

        /// <summary>
        /// Indicates presence of PermissionData.
        /// </summary>
        public bool HasPermissionData { get => m_rsslVectorEntry.CheckHasPermData(); }

        internal VectorEntry() 
        {
            m_rsslVectorEntry = new Eta.Codec.VectorEntry();
        }
        
        /// <summary>
        /// Returns the VectorAction value as a string format.
        /// </summary>
        /// <returns>string representing VectorEntry action</returns>
        public string VectorActionAsString() { return VectorAction.VectorActionToString(Action); }
        
        /// <summary>
        /// Returns position of the entry
        /// </summary>
        /// <returns>entry's position</returns>
        public long Position { get => m_rsslVectorEntry.Index; }
        
        /// <summary>
        /// Returns the current action on the OMM data.
        /// </summary>
        /// <returns>VectorEntry Action</returns>
        public int Action { get => VectorAction.EtaVectorActionToInt(m_rsslVectorEntry.Action); }

        /// <summary>
        /// Returns PermissionData.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing permission information</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasPermissionData"/> returns false</exception>
        public EmaBuffer PermissionData 
        {
            get
            {
                if (!m_rsslVectorEntry.CheckHasPermData())
                {
                    throw new OmmInvalidUsageException("Attempt to access Permission Data while VectorEntry doesn't have it.");
                }
                m_permissionBuffer.Clear();
                var span = new Span<byte>(m_rsslVectorEntry.PermData.Data().Contents, m_rsslVectorEntry.PermData.Position, m_rsslVectorEntry.PermData.Length);
                m_permissionBuffer.Append(span);
                return m_permissionBuffer;
            }
        }

        internal CodecReturnCode Decode(DecodeIterator decodeIterator)
        {
            m_rsslVectorEntry.Clear();
            return m_rsslVectorEntry.Decode(decodeIterator);
        }

        /// <summary>
        /// Clears current VectorEntry instance
        /// </summary>
        public void Clear()
        {
            m_rsslVectorEntry.Clear();
            m_permissionBuffer.Clear();
            ClearLoad();
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="VectorEntry"/> object.</returns>
        public override string ToString()
        {
            if (Load == null)
                return $"{NewLine}ToString() method could not be used for just encoded object.{NewLine}";

            m_toString.Length = 0;
            m_toString.Append("VectorEntry ")
                    .Append(" action=\"").Append(VectorActionAsString()).Append("\"")
                    .Append(" index=\"").Append(Position);

            if (HasPermissionData)
            {
                m_toString.Append("\" permissionData=\"").Append(PermissionData).Append("\"");
                Utilities.AsHexString(m_toString, PermissionData).Append("\"");
            }

            m_toString.Append("\" dataType=\"").Append(DataType.AsString(Load.DataType)).Append("\"").AppendLine();
            m_toString.Append(Load.ToString(1));
            Utilities.AddIndent(m_toString, 0).Append("VectorEntryEnd").AppendLine();

            return m_toString.ToString();
        }
    }
}
