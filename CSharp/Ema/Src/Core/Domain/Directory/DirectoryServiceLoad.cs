/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using System;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service Load. Contains information provided by the Source Directory Load filter.
    /// </summary>
    public sealed class DirectoryServiceLoad : DirectoryServiceFilter<ElementList, DirectoryServiceLoad>, ICloneable
    {
        private OptionalField<ulong> m_OpenLimit = new();
        private OptionalField<ulong> m_OpenWindow = new();
        private OptionalField<ushort> m_LoadFactor = new();

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_LOAD_ID;

        /// <summary>
        /// The maximum number of items the Consumer is allowed to open from this service.
        /// </summary>
        /// <returns>Returns the open limit.</returns>
        public ulong OpenLimit() => m_OpenLimit.Value;
        /// <summary>
        /// The maximum number of items the Consumer is allowed to open from this service.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceLoad OpenLimit(ulong value)
        {
            m_OpenLimit.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the openLimit field.
        /// </summary>
        public bool HasOpenLimit => m_OpenLimit.HasValue;


        /// <summary>
        /// The maximum number of items the Consumer may have outstanding
        /// (i.e. waiting for a RefreshMsg) from this service.
        /// </summary>
        /// <returns>Returns the open window.</returns>
        public ulong OpenWindow() => m_OpenWindow.Value;
        /// <summary>
        /// The maximum number of items the Consumer may have outstanding
        /// (i.e. waiting for a RefreshMsg) from this service.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceLoad OpenWindow(ulong value)
        {
            m_OpenWindow.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the openWindow field.
        /// </summary>
        public bool HasOpenWindow => m_OpenWindow.HasValue;

        /// <summary>
        /// The load factor - a number indicating the current workload of
        /// the source providing the data.
        /// </summary>
        /// <returns>Returns the load factor.</returns>
        public ushort LoadFactor() => m_LoadFactor.Value;
        /// <summary>
        /// The load factor - a number indicating the current workload of
        /// the source providing the data.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceLoad LoadFactor(ushort value)
        {
            m_LoadFactor.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the loadFactor field.
        /// </summary>
        public bool HasLoadFactor => m_LoadFactor.HasValue;

        /// <inheritdoc />
        public override DirectoryServiceLoad Clear()
        {
            base.Clear();
            m_OpenLimit.Clear();
            m_OpenWindow.Clear();
            m_LoadFactor.Clear();
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceLoad"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceLoad Clone() => new DirectoryServiceLoad().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override DirectoryServiceLoad CopyFrom(DirectoryServiceLoad source)
        {
            base.CopyFrom(source);
            m_OpenLimit.CopyFrom(source.m_OpenLimit);
            m_OpenWindow.CopyFrom(source.m_OpenWindow);
            m_LoadFactor.CopyFrom(source.m_LoadFactor);
            return this;
        }

        internal override void AppendToString(StringBuilder sb, int indent)
        {
            base.AppendToString(sb, indent);
            if (HasOpenLimit)
            {
                sb.AddIndent(indent).AppendLine($"OpenLimit: {m_OpenLimit.Value}");
            }
            if (HasOpenWindow)
            {
                sb.AddIndent(indent).AppendLine($"OpenWindow: {m_OpenWindow.Value}");
            }
            if (HasLoadFactor)
            {
                sb.AddIndent(indent).AppendLine($"LoadFactor: {m_LoadFactor.Value}");
            }
        }

        internal override void DecodeFrom(ElementList elementList)
        {
            Clear();

            foreach (var elementEntry in elementList)
            {
                var elementName = elementEntry.Name;
                switch (elementName)
                {
                    case EmaRdm.ENAME_OPEN_LIMIT:
                        var openLimit = elementEntry.UIntValue();
                        OpenLimit(openLimit);
                        break;
                    case EmaRdm.ENAME_OPEN_WINDOW:
                        var openWindow = elementEntry.UIntValue();
                        OpenWindow(openWindow);
                        break;
                    case EmaRdm.ENAME_LOAD_FACT:
                        var loadFactor = elementEntry.UIntValue();
                        if (loadFactor > ushort.MaxValue)
                            throw new OmmInvalidUsageException(
                                $"Invalid element value {loadFactor} of {elementName}",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        LoadFactor((ushort)loadFactor);
                        break;
                    default:
                        break;
                }
            }
        }

        internal override void EncodeTo(ElementList elementList)
        {
            if (HasOpenWindow)
            {
                elementList.AddUInt(EmaRdm.ENAME_OPEN_WINDOW, m_OpenWindow.Value);
            }

            if (HasOpenLimit)
            {
                elementList.AddUInt(EmaRdm.ENAME_OPEN_LIMIT, m_OpenLimit.Value);
            }

            if (HasLoadFactor)
            {
                elementList.AddUInt(EmaRdm.ENAME_LOAD_FACT, m_LoadFactor.Value);
            }
        }
    }
}