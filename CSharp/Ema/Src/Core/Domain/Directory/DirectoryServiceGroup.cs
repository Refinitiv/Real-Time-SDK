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
    ///  The RDM Service Group State. Contains information provided by the Source Directory Group filter.
    /// </summary>
    public sealed class DirectoryServiceGroup : DirectoryServiceFilter<ElementList, DirectoryServiceGroup>, ICloneable
    {
        private StateField m_Status = new();
        private readonly EmaBuffer m_Group = new();
        private readonly EmaBuffer m_MergedToGroup = new();

        /// <inheritdoc />
        public override int FilterId => EmaRdm.SERVICE_GROUP_ID;

        /// <summary>
        /// Returns status to be applied to all items whose ItemGroup matches the Group element.
        /// </summary>
        /// <returns></returns>
        public OmmState Status() => m_Status.Value();

        /// <summary>
        /// Sets status to be applied to all items whose ItemGroup matches the Group element.
        /// </summary>
        public DirectoryServiceGroup Status(OmmState value)
        {
            m_Status.Value(value);
            return this;
        }

        /// <summary>
        /// Sets status to be applied to all items whose ItemGroup matches the Group element.
        /// </summary>
        public DirectoryServiceGroup Status(int streamState, int dataState, int statusCode, string statusText)
        {
            m_Status.Value(streamState, dataState, statusCode, statusText);
            return this;
        }

        /// <summary>
        /// Gets flag that indicates presence of the status field.
        /// </summary>
        public bool HasStatus => m_Status.HasValue;

        /// <summary>
        /// Group for this service with the user specified buffer.
        /// </summary>
        public EmaBuffer Group() => m_Group;
        /// <summary>
        /// Group for this service with the user specified buffer.
        /// </summary>
        public DirectoryServiceGroup Group(EmaBuffer value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("Group cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Group.CopyFrom(value.Buffer);
            return this;
        }

        /// <summary>
        /// Group for this service with the user specified buffer.
        /// </summary>
        public EmaBuffer MergedToGroup()
        {
            if (!HasMergedToGroup)
                throw new OmmInvalidUsageException("MergedToGroup element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_MergedToGroup;
        }
        /// <summary>
        /// Group for this service with the user specified buffer.
        /// </summary>
        public DirectoryServiceGroup MergedToGroup(EmaBuffer value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("MergedToGroup cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_MergedToGroup.CopyFrom(value.Buffer);
            HasMergedToGroup = true;
            return this;
        }

        /// <summary>
        /// Gets the the presence of the mergedToGroup field.
        /// </summary>
        public bool HasMergedToGroup { get; private set; }

        /// <inheritdoc />
        public override DirectoryServiceGroup Clear()
        {
            base.Clear();
            m_Status.Clear();
            m_Group.Clear();
            m_MergedToGroup.Clear();
            HasMergedToGroup = false;
            return this;
        }

        /// <inheritdoc />
        public override DirectoryServiceGroup CopyFrom(DirectoryServiceGroup source)
        {
            base.CopyFrom(source);
            m_Status.CopyFrom(source.m_Status);
            m_Group.CopyFrom(source.m_Group.Buffer);
            m_MergedToGroup.CopyFrom(source.m_MergedToGroup.Buffer);
            HasMergedToGroup = source.HasMergedToGroup;
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryServiceGroup"/> instance that is a copy of this instance.</returns>
        public DirectoryServiceGroup Clone() => new DirectoryServiceGroup().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        internal override void AppendToString(StringBuilder sb, int indent)
        {
            base.AppendToString(sb, indent);
            if (HasStatus)
            {
                sb.AddIndent(indent).AppendLine($"Status: {m_Status}");
            }
            sb.AddIndent(indent).AppendLine("Group:");
            m_Group.AppendToString(sb, indent + 1);
            if (HasMergedToGroup)
            {
                sb.AddIndent(indent).AppendLine("MergedToGroup:");
                m_MergedToGroup.AppendToString(sb, indent + 1);
            }
        }

        internal override void DecodeFrom(ElementList elementList)
        {
            Clear();

            var foundGroup = false;
            foreach (var elementEntry in elementList)
            {
                switch (elementEntry.Name)
                {
                    case EmaRdm.ENAME_GROUP:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var group = elementEntry.OmmBufferValue().Value;
                            Group(group);
                        }
                        foundGroup = true;
                        break;
                    case EmaRdm.ENAME_MERG_TO_GRP:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            var mergedToGroup = elementEntry.OmmBufferValue().Value;
                            MergedToGroup(mergedToGroup);
                        }
                        break;
                    case EmaRdm.ENAME_STATUS:
                        if (elementEntry.Code != Data.DataCode.BLANK)
                        {
                            OmmState status = elementEntry.OmmStateValue();
                            if (status.StreamState is OmmState.StreamStates.OPEN or OmmState.StreamStates.CLOSED_RECOVER)
                            {
                                Status(status);
                            }
                            else
                            {
                                Clear();
                                throw new OmmInvalidUsageException(
                                    $"Invalid element value \"{status}\" of {elementEntry.Name} element",
                                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            if (!foundGroup)
            {
                Clear();
                throw new OmmInvalidUsageException($"{EmaRdm.ENAME_GROUP} element is absent", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }
        }

        internal override void EncodeTo(ElementList elementList)
        {
            elementList.AddBuffer(EmaRdm.ENAME_GROUP, m_Group);

            if (HasMergedToGroup)
            {
                elementList.AddBuffer(EmaRdm.ENAME_MERG_TO_GRP, m_MergedToGroup);
            }

            if (HasStatus)
            {
                var ommState = m_Status.Value();
                elementList.AddState(
                    EmaRdm.ENAME_STATUS,
                    ommState.StreamState, ommState.DataState, ommState.StatusCode, ommState.StatusText);
            }

        }
    }
}