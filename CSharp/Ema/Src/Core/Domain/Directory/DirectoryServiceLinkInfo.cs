/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service Link Info. Contains information provided by the Source
    /// Directory Link filter.
    /// </summary>
    public sealed class DirectoryServiceLinkInfo : DirectoryServiceFilter<Map, DirectoryServiceLinkInfo>, ICloneable
    {
        private readonly List<DirectoryServiceLink> m_LinkList = new();
        private readonly FluentListBuilder<DirectoryServiceLink> m_LinkListBuilder;

        /// <summary>
        /// Initializes a new instance of the <see cref="DirectoryServiceLinkInfo"/> class.
        /// </summary>
        public DirectoryServiceLinkInfo()
        {
            m_LinkListBuilder = new(m_LinkList);
        }

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_LINK_ID;

        /// <summary>
        /// Link information elements - List of entries with information
        /// about upstream sources.
        /// </summary>
        /// <returns>Returns the list of directory service links.</returns>
        public IList<DirectoryServiceLink> LinkList() => m_LinkList;
        /// <summary>
        /// Link information elements - List of entries with information
        /// about upstream sources.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceLinkInfo LinkList(IList<DirectoryServiceLink> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("linkList cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_LinkList.Clear();
            m_LinkList.AddRange(value);
            return this;
        }
        /// <summary>
        /// Link information elements - List of entries with information
        /// about upstream sources.
        /// </summary>
        /// <param name="buildAction">the action to build the list</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryServiceLinkInfo LinkList(Action<IFluentListBuilder<DirectoryServiceLink>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_LinkListBuilder);
            return this;
        }

        /// <inheritdoc />
        public override DirectoryServiceLinkInfo Clear()
        {
            base.Clear();
            m_LinkList.Clear();
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceLinkInfo"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceLinkInfo Clone() => new DirectoryServiceLinkInfo().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override DirectoryServiceLinkInfo CopyFrom(DirectoryServiceLinkInfo source)
        {
            base.CopyFrom(source);
            m_LinkList.CopyFrom(source.m_LinkList);
            return this;
        }

        internal override void AppendToString(StringBuilder sb, int indent)
        {
            base.AppendToString(sb, indent);
            sb.AddIndent(indent).AppendLine("LinkList:");
            m_LinkList.AppendToString(sb, indent + 1,
                (sb, item, indent) => item.AppendToString(sb, indent));
        }

        internal override void DecodeFrom(Map map)
        {
            Clear();

            foreach (var mapEntry in map)
            {
                if (mapEntry.LoadType == DataType.DataTypes.ELEMENT_LIST &&
                        mapEntry.Key.Data.DataType == DataType.DataTypes.ASCII)
                {
                    var action  = (DirectoryFilterAction)mapEntry.Action;
                    var serviceLink = new DirectoryServiceLink();
                    if (action != DirectoryFilterAction.CLEAR)
                    {
                        serviceLink.DecodeFrom(mapEntry.ElementList());
                    }
                    serviceLink
                        .Name(mapEntry.Key.Ascii().Value)
                        .Action(action);
                    m_LinkList.Add(serviceLink);
                }
            }
        }

        internal override void EncodeTo(Map map)
        {
            var elementList = new ElementList();
            foreach (var serviceLink in m_LinkList)
            {
                serviceLink.EncodeTo(elementList);
                map.AddKeyAscii(serviceLink.Name(), (int)serviceLink.Action(), elementList.Complete());
                elementList.Clear();
            }
        }
    }
}