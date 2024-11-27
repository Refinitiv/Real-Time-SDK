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
using System.Numerics;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// MapEntry represents an entry of Map.
    /// </summary>
    /// <remarks>
    /// MapEntry associates entry's key, permission information, action, data and its data type.<br/>
    /// Objects of this class are intended to be short lived or rather transitional.<br/>
    /// Objects of this class are not cache-able.
    /// </remarks>
    public sealed class MapEntry : Entry
    {
        internal Eta.Codec.MapEntry m_rsslMapEntry;
        internal Key m_key;
        private EmaBuffer m_permissionBuffer;
        
        /// <summary>
        /// Indicates presence of PermissionData.
        /// </summary>
        public bool HasPermissionData { get => m_rsslMapEntry.CheckHasPermData(); }

        internal MapEntry(EmaObjectManager objectManager) 
        { 
            m_rsslMapEntry = new Eta.Codec.MapEntry();
            m_key = new Key(objectManager);
            m_permissionBuffer = new EmaBuffer();
        }
        
        /// <summary>
        /// Returns the MapAction value as a string format.
        /// </summary>
        /// <returns>string containing string representation of MapAction</returns>
        public string MapActionAsString() 
        { 
            return MapAction.MapActionToString(Action); 
        }
        
        /// <summary>
        /// Returns the contained key Data based on the key DataType.
        /// </summary>
        /// <returns><see cref="Key"/> class reference to contained entry's Key object.</returns>
        public Key Key 
        {
            get => m_key; 
        }
        
        /// <summary>
        /// Returns the current action on the OMM data.
        /// </summary>
        public int Action 
        { 
            get => MapAction.EtaMapActionToInt(m_rsslMapEntry.Action);
        }
        
        /// <summary>
        /// Returns PermissionData.
        /// </summary>
        /// <returns><see cref="EmaBuffer"/> containing permission information</returns>
        /// <exception cref="OmmInvalidUsageException">Thrown if <see cref="HasPermissionData"/> returns false</exception>
        public EmaBuffer PermissionData 
        {
            get
            {
                if (!m_rsslMapEntry.CheckHasPermData())
                {
                    throw new OmmInvalidUsageException("Attempt to access Permission Data while MapEntry doesn't have it.");
                }
                m_permissionBuffer.Clear();
                var span = new Span<byte>(m_rsslMapEntry.PermData.Data().Contents, m_rsslMapEntry.PermData.Position, m_rsslMapEntry.PermData.Length);
                m_permissionBuffer.Append(span);
                return m_permissionBuffer;
            }
        }

        internal CodecReturnCode Decode(DecodeIterator decodeIterator, int primitiveKeyType)
        {
            var ret = m_rsslMapEntry.Decode(decodeIterator, null);
            if (ret < CodecReturnCode.SUCCESS || ret == CodecReturnCode.END_OF_CONTAINER)
            {
                return ret;
            }
            return m_key.DecodeKey(decodeIterator.MajorVersion(), 
                decodeIterator.MinorVersion(), 
                m_rsslMapEntry.EncodedKey, 
                primitiveKeyType);
        }

        /// <summary>
        /// Clears current MapEntry instance.
        /// </summary>
        public void Clear()
        {
            m_permissionBuffer.Clear();
            m_rsslMapEntry.Clear();
            m_key.Clear();
            if (m_errorString != null) 
                m_errorString!.Clear();
            Load = null;
        }


        /// <summary>
        /// Provides string representation of the current instance
        /// </summary>
        /// <returns>string representing current <see cref="MapEntry"/> object.</returns>
        public override string ToString()
        {
            if (Load == null)
                return $"{NewLine}ToString() method could not be used for just encoded object.{NewLine}";

            m_toString.Length = 0;
            m_toString.Append("MapEntry ")
                    .Append(" action=\"").Append(MapActionAsString()).Append("\"")
                    .Append(" value=\"").Append(Key.ToString());

            if (HasPermissionData)
            {
                m_toString.Append("\" permissionData=\"").Append(PermissionData).Append("\"");
                Utilities.AsHexString(m_toString, PermissionData).Append("\"");
            }

            m_toString.Append("\" dataType=\"").Append(DataType.AsString(Load.DataType)).Append("\"").AppendLine();
            m_toString.Append(Load.ToString(1));
            Utilities.AddIndent(m_toString, 0).Append("MapEntryEnd").AppendLine();

            return m_toString.ToString();
        }
    }
}
