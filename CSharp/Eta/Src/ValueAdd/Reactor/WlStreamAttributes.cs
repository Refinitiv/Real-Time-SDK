﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Common;
using System.Runtime.InteropServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Attributes that uniquely identify a stream. 
    /// </summary>
    internal sealed class WlStreamAttributes : VaNode, IEquatable<WlStreamAttributes>
    {
        internal GCHandle m_handle;
        /// <summary>
        /// Gets or set <see cref="Codec.Qos"/> if any.
        /// </summary>
        public Qos Qos { get; set; } = new ();

        /// <summary>
        /// Gets or sets a domain type.
        /// </summary>
        public int DomainType { get; set; }

        /// <summary>
        /// Gets or sets <see cref="IMsgKey"/>
        /// </summary>
        public IMsgKey MsgKey { get; set; } = new MsgKey();

        /* Performs a deep copy of this Object to destItemAggregationKey. */
        public void Copy(WlStreamAttributes destItemAggregationKey)
        {
            if (MsgKey != null)
            {
                MsgKey.Copy(destItemAggregationKey.MsgKey);
            }

            destItemAggregationKey.DomainType = DomainType;

            if(Qos != null)
            {
                Qos.Copy(destItemAggregationKey.Qos);
            }
        }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void Clear()
        {
            hashCodeSet = false;
            Qos.Clear();
            DomainType = 0;
        }

        public bool Equals(WlStreamAttributes? other)
        {
            if(other is null)
                return false;

            if (other == this)
                return true;

            bool compareKeyAndDomain = MsgKey.Equals(other.MsgKey) &&
                    DomainType == other.DomainType;

            if (Qos != null)
            {
                return compareKeyAndDomain &&
                    Qos.Equals(other.Qos);
            }
            else if(other.Qos != null)
            {
                return compareKeyAndDomain &&
                   other.Qos.Equals(Qos);
            }

            return compareKeyAndDomain;
        }

        public override bool Equals(object? obj)
        {
            WlStreamAttributes? wlStreamAttributes = obj as WlStreamAttributes;
            if (wlStreamAttributes == null)
                return false;
            else
                return Equals(wlStreamAttributes);
        }

        int hashCode = 0;
        bool hashCodeSet = false;

        public override int GetHashCode()
        {
            unchecked
            {
                if (!hashCodeSet)
                {
                    if (Qos != null)
                    {
                        hashCode = MsgKey.GetHashCode() ^ DomainType.GetHashCode() ^ Qos.GetHashCode();
                    }
                    else
                    {
                        hashCode = MsgKey.GetHashCode() ^ DomainType.GetHashCode();
                    }
                    hashCodeSet = true;
                }
                return hashCode;
            }          
        }

        public override void ReturnToPool()
        {
            Clear();
            base.ReturnToPool();
        }
    }
}
