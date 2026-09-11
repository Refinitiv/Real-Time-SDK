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
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Directory Status. Used by an OMM Provider to indicate changes to the
    /// Directory stream.
    /// </summary>
    public sealed class DirectoryStatusMsg : DirectoryMsg<StatusMsg, DirectoryStatusMsg>, ICloneable
    {
        private readonly StateField m_State = new();
        private readonly EmaBuffer m_PermissionData = new();
        private bool m_ClearCache;
        private OptionalField<ushort> m_ServiceId = new("ServiceId element");

        /// <summary>
        /// Gets or sets presence of state field.
        /// </summary>
        public bool HasState => m_State.HasValue;

        /// <summary>
        /// Gets or sets presence of permission data field.
        /// </summary>
        public bool HasPermissionData { get; private set; }

        /// <summary>
        /// Gets or sets presence of filter field.
        /// </summary>
        public bool HasFilter { get; private set; }

        /// <summary>
        /// Gets a value indicating whether a service id is present.
        /// </summary>
        public bool HasServiceId => m_ServiceId.HasValue;

        /// <inheritdoc />
        public override DirectoryFilters Filter()
        {
            if (!HasFilter)
                throw new OmmInvalidUsageException($"{nameof(Filter)} element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return base.Filter();
        }

        /// <inheritdoc />
        public override DirectoryStatusMsg Filter(DirectoryFilters value)
        {
            base.Filter(value);
            HasFilter = true;
            return this;
        }

        /// <summary>
        /// Gets the unique identifier for the service associated with this instance.
        /// </summary>
        /// <returns>A 16-bit unsigned integer representing the service identifier.</returns>
        public ushort ServiceId() => m_ServiceId.Value;
        /// <summary>
        /// Sets the unique identifier for the service associated with this instance.
        /// </summary>
        /// <param name="value">A 16-bit unsigned integer representing the service identifier.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryStatusMsg ServiceId(ushort value)
        {
            m_ServiceId.Value = value;
            return this;
        }

        /// <summary>
        /// Gets clear cache flag.
        /// </summary>
        /// <returns>Returns true if the clear cache flag is set, false otherwise.</returns>
        public bool ClearCache() => m_ClearCache;
        /// <summary>
        /// Sets clear cache flag.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryStatusMsg ClearCache(bool value)
        {
            m_ClearCache = value;
            return this;
        }

        /// <summary>
        /// Gets state for the directory status message.
        /// </summary>
        /// <returns>Returns the state for the directory status message.</returns>
        public OmmState State() => m_State.Value();

        /// <summary>
        /// Sets state for the directory status message.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryStatusMsg State(OmmState value)
        {
            m_State.Value(value);
            return this;
        }

        /// <summary>
        /// Sets state for the directory status message.
        /// </summary>
        /// <param name="streamState">the stream state to set</param>
        /// <param name="dataState">the data state to set</param>
        /// <param name="statusCode">the status code to set</param>
        /// <param name="statusText">the status text to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryStatusMsg State(int streamState, int dataState, int statusCode, string statusText)
        {
            m_State.Value(streamState, dataState, statusCode, statusText);
            return this;
        }

        /// <summary>
        /// Gets permission data associated with all contents on the stream.
        /// </summary>
        /// <returns>Returns the permission data associated with all contents on the stream.</returns>
        public EmaBuffer PermissionData()
        {
            if (!HasPermissionData)
                throw new OmmInvalidUsageException("PermissionData element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return m_PermissionData;
        }

        /// <summary>
        /// Sets permission data associated with all contents on the stream.
        /// </summary>
        /// <param name="value">the value to set</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryStatusMsg PermissionData(EmaBuffer value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("PermissionData cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_PermissionData.CopyFrom(value.Buffer);
            HasPermissionData = true;
            return this;
        }

        /// <inheritdoc />
        protected override void DecodeFrom(StatusMsg message)
        {
            if (message.HasFilter)
            {
                Filter((DirectoryFilters)message.Filter());
            }
            if (message.HasState)
            {
                State(message.State());
            }
            if (message.HasPermissionData)
            {
                PermissionData(message.PermissionData());
            }
            if (message.HasServiceId)
            {
                ServiceId(message.GetServiceId());
            }
            ClearCache(message.ClearCache());
        }

        /// <inheritdoc />
        protected override void EncodeTo(StatusMsg message)
        {
            message.DomainType(DomainType);
            if (HasFilter)
            {
                message.Filter((int)Filter());
            }
            if (HasState)
            {
                var state = m_State.Value();
                message.State(state.StreamState, state.DataState, state.StatusCode, state.StatusText);
            }
            if (HasPermissionData)
            {
                message.PermissionData(m_PermissionData);
            }
            if (HasServiceId)
            {
                message.ServiceId(ServiceId());
            }
            message.ClearCache(m_ClearCache);
        }

        /// <inheritdoc />
        public override DirectoryStatusMsg Clear()
        {
            base.Clear();
            m_State.Clear();
            m_PermissionData.Clear();
            HasPermissionData = false;
            m_ClearCache = false;
            HasFilter = false;
            m_ServiceId.Clear();
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryStatusMsg"/> instance that is a copy of this instance.</returns>
        public DirectoryStatusMsg Clone() => new DirectoryStatusMsg().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        protected override void CopyFromInternal(DirectoryStatusMsg source)
        {
            base.CopyFromInternal(source);
            m_State.CopyFrom(source.m_State);
            m_PermissionData.CopyFrom(source.m_PermissionData.Buffer);
            HasPermissionData = source.HasPermissionData;
            m_ClearCache = source.m_ClearCache;
            HasFilter = source.HasFilter;
            m_ServiceId.CopyFrom(source.m_ServiceId);
        }

        /// <inheritdoc />
        protected override void AppendToString(StringBuilder builder, int indent)
        {
            base.AppendToString(builder, indent);
            if (HasState)
            {
                builder.AddIndent(indent).AppendLine($"State: {m_State}");
            }
            if (HasPermissionData)
            {
                builder.AddIndent(indent).AppendLine("PermissionData:");
                m_PermissionData.AppendToString(builder, indent + 1);
            }
            builder.AddIndent(indent).AppendLine($"ClearCache: {m_ClearCache}");
            builder.AddIndent(indent).AppendLine($"ServiceId: {m_ServiceId}");
        }
    }
}
