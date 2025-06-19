/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// FilterEntry represents an entry of FilterList. 
    /// Objects of this class are intended to be short lived or rather transitional. 
    /// Objects of this class are not cache-able.
    /// </summary>
    public sealed class FilterEntry : Entry
    {
        internal Eta.Codec.FilterEntry m_rsslFilterEntry;
        internal EmaBuffer m_permissionBuffer = new EmaBuffer();

        internal FilterEntry()
        {
            m_rsslFilterEntry = new Eta.Codec.FilterEntry();
        }
        
        /// <summary>
        /// Indicates presence of PermissionData.
        /// </summary>
        public bool HasPermissionData { get => m_rsslFilterEntry.CheckHasPermData(); }
        
        /// <summary>
        /// Returns the FilterAction value in a string format.
        /// </summary>
        /// <returns>string containing string representation of FilterAction</returns>
        public string FilterActionAsString() 
        { 
            return FilterAction.FilterActionToString(Action); 
        }
        
        /// <summary>
        /// Returns the Id of the FilterEntry.
        /// </summary>
        /// <returns><see cref="int"/> value representing the Id</returns>
        public int FilterId { get => m_rsslFilterEntry.Id; }
       
        /// <summary>
        /// Returns the current action on the OMM data.
        /// </summary>
        public int Action { get => FilterAction.EtaFilterActionToInt(m_rsslFilterEntry.Action); }
        
        /// <summary>
        /// Returns PermissionData.
        /// </summary>
        /// <exception cref="OmmInvalidUsageException">throws the exception if <see cref="HasPermissionData"/> returns false</exception>
        public EmaBuffer PermissionData 
        {
            get
            {
                if (!m_rsslFilterEntry.CheckHasPermData())
                {
                    throw new OmmInvalidUsageException("Attempt to access Permission Data while FilterEntry doesn't have it.");
                }
                m_permissionBuffer.Clear();
                var span = new Span<byte>(m_rsslFilterEntry.PermData.Data().Contents, m_rsslFilterEntry.PermData.Position, m_rsslFilterEntry.PermData.Length);
                m_permissionBuffer.CopyFrom(span);
                return m_permissionBuffer;
            }            
        }

        internal CodecReturnCode Decode(DecodeIterator decodeIterator)
        {
            return m_rsslFilterEntry.Decode(decodeIterator);
        }

        /// <summary>
        /// Clears current FilterEntry instance
        /// </summary>
        public void Clear()
        {
            m_permissionBuffer.Clear();
            m_rsslFilterEntry.Clear();
            Load = null;
        }

        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="FilterEntry"/> object.</returns>
        public override string ToString()
        {
            if (Load == null)
                return $"{NewLine}ToString() method could not be used for just encoded object.{NewLine}";

            m_toString.Length = 0;
            m_toString.Append("FilterEntry ")
                    .Append(" action=\"").Append(FilterActionAsString()).Append("\"")
                    .Append(" filterId=\"").Append(FilterId);

            if (HasPermissionData)
            {
                m_toString.Append("\" permissionData=\"").Append(PermissionData).Append("\"");
                Utilities.AsHexString(m_toString, PermissionData).Append("\"");
            }

            m_toString.Append("\" dataType=\"").Append(DataType.AsString(Load.DataType)).Append("\"").AppendLine();
            if (Load != null)
            {
                m_toString.Append(Load.ToString(1));
            }         
            Utilities.AddIndent(m_toString, 0).Append("FilterEntryEnd").AppendLine();

            return m_toString.ToString();
        }
    }
}
