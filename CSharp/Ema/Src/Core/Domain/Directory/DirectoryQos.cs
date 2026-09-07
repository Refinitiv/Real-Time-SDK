/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Internal;
using System;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// DirectoryQos represents Quality of Service (QoS) information.
    /// </summary>
    public sealed class DirectoryQos : ICloneable
    {
        private readonly OmmQos m_OmmQos = new();
        private bool m_IsDirty;
        private uint m_Rate;
        private uint m_Timeliness;

        /// <summary>
        /// Initializes a new instance of the <see cref="DirectoryQos"/> class.
        /// </summary>
        public DirectoryQos()
        {
            Clear();
        }

        /// <summary>
        /// Gets rate value.
        /// </summary>
        /// <returns>Returns the rate value.</returns>
        public uint Rate() => m_Rate;
        /// <summary>
        /// Sets rate value.
        /// </summary>
        /// <param name="rate">the rate</param>
        /// <returns>Returns this <see cref="DirectoryQos"/> instance to allow method chaining.</returns>
        public DirectoryQos Rate(uint rate)
        {
            m_Rate = rate;
            m_IsDirty = true;
            return this;
        }

        /// <summary>
        /// Gets timeliness.
        /// </summary>
        /// <returns>Returns the timeliness value.</returns>
        public uint Timeliness() => m_Timeliness;
        /// <summary>
        /// Sets timeliness.
        /// </summary>
        /// <param name="timeliness">the timeliness</param>
        /// <returns>Returns this <see cref="DirectoryQos"/> instance to allow method chaining.</returns>
        public DirectoryQos Timeliness(uint timeliness)
        {
            m_Timeliness = timeliness;
            m_IsDirty = true;
            return this;
        }

        /// <summary>
        /// Clears <see cref="DirectoryQos"/>. Useful for object reuse.
        /// </summary>
        /// <returns>Returns this <see cref="DirectoryQos"/> instance to allow method chaining.</returns>
        public DirectoryQos Clear()
        {
            m_Rate = 0;
            m_Timeliness = 0;
            m_IsDirty = true;
            return this;
        }

        /// <summary>
        /// Copies the state from the specified source instance to the current instance, replacing any existing data.
        /// </summary>
        /// <remarks>This method clears the current instance before copying the state from the source.</remarks>
        /// <param name="source">The instance from which to copy state. Must not be null.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryQos CopyFrom(DirectoryQos source)
        {
            m_Rate = source.m_Rate;
            m_Timeliness = source.m_Timeliness;
            m_IsDirty = true;
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryQos"/> instance that is a copy of this instance.</returns>
        public DirectoryQos Clone() => new DirectoryQos().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override string ToString() => ToOmmQos().ToString();

        internal OmmQos ToOmmQos()
        {
            if (m_IsDirty)
            {
                m_OmmQos.Populate(m_Timeliness, m_Rate);
                m_IsDirty = false;
            }
            return m_OmmQos;
        }
    }
}
