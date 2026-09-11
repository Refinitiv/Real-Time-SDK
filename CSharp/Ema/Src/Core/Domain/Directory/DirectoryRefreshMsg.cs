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
    /// The RDM Directory Refresh. Used by an OMM Provider to provide information
    /// about available services.
    /// </summary>
    public sealed class DirectoryRefreshMsg : DirectoryMsgWithPayload<RefreshMsg, DirectoryRefreshMsg>, ICloneable
    {
        private OptionalField<long> m_SequenceNumber = new();
        private readonly StateField m_State = new();
        private bool m_ClearCache;
        private bool m_DoNotCache;
        private bool m_Complete;
        private bool m_Solicited;
        private OptionalField<ushort> m_ServiceId = new("ServiceId element");

        /// <summary>
        /// Constructs directory refresh message with default values.
        /// </summary>
        public DirectoryRefreshMsg()
        {
            Clear();
        }

        /// <summary>
        /// Gets the presence of sequence number field.
        /// </summary>
        public bool HasSequenceNumber => m_SequenceNumber.HasValue;

        /// <summary>
        /// Gets sequence number of this message.
        /// </summary>
        /// <returns>The sequence number of this message.</returns>
        public long SequenceNumber() => m_SequenceNumber.Value;
        /// <summary>
        /// Sets sequence number of this message.
        /// </summary>
        /// <param name="value">The sequence number to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg SequenceNumber(long value)
        {
            m_SequenceNumber.Value = value;
            return this;
        }

        /// <summary>
        /// Gets or sets the presence of clear cache flag.
        /// </summary>
        /// <returns>True if the clear cache flag is set; otherwise, false.</returns>
        public bool ClearCache() => m_ClearCache;
        /// <summary>
        /// Gets or sets the presence of clear cache flag.
        /// </summary>
        /// <param name="value">True to set the clear cache flag; otherwise, false.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg ClearCache(bool value)
        {
            m_ClearCache = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of do not cache flag.
        /// </summary>
        /// <returns>True if the do not cache flag is set; otherwise, false.</returns>
        public bool DoNotCache() => m_DoNotCache;
        /// <summary>
        /// Sets the presence of do not cache flag.
        /// </summary>
        /// <param name="value">True to set the do not cache flag; otherwise, false.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg DoNotCache(bool value)
        {
            m_DoNotCache = value;
            return this;
        }

        /// <summary>
        /// Gets or sets the presence of complete flag.
        /// </summary>
        /// <returns>True if the complete flag is set; otherwise, false.</returns>
        public bool Complete() => m_Complete;
        /// <summary>
        /// Gets or sets the presence of complete flag.
        /// </summary>
        /// <param name="value">True to set the complete flag; otherwise, false.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg Complete(bool value)
        {
            m_Complete = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of solicited flag.
        /// </summary>
        /// <returns>True if the solicited flag is set; otherwise, false.</returns>
        public bool Solicited() => m_Solicited;
        /// <summary>
        /// Sets the presence of solicited flag.
        /// </summary>
        /// <param name="value">True to set the solicited flag; otherwise, false.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg Solicited(bool value)
        {
            m_Solicited = value;
            return this;
        }

        /// <summary>
        /// Gets state for the directory refresh message.
        /// </summary>
        /// <returns>The current state of the directory refresh message.</returns>
        public OmmState State() => m_State.Value();

        /// <summary>
        /// Sets state for the directory refresh message.
        /// </summary>
        /// <param name="value">The state to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg State(OmmState value)
        {
            m_State.Value(value);
            return this;
        }

        /// <summary>
        /// Sets state for the directory refresh message.
        /// </summary>
        /// <param name="streamState">The stream state to set.</param>
        /// <param name="dataState">The data state to set.</param>
        /// <param name="statusCode">The status code to set.</param>
        /// <param name="statusText">The status text to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRefreshMsg State(int streamState, int dataState, int statusCode, string statusText)
        {
            m_State.Value(streamState, dataState, statusCode, statusText);
            return this;
        }

        /// <summary>
        /// Gets a value indicating whether a service id is present.
        /// </summary>
        public bool HasServiceId => m_ServiceId.HasValue;

        /// <summary>
        /// Gets the unique identifier for the service associated with this instance.
        /// </summary>
        /// <returns>A 16-bit unsigned integer representing the service identifier.</returns>
        public ushort ServiceId() => m_ServiceId.Value;
        /// <summary>
        /// Sets the unique identifier for the service associated with this instance.
        /// </summary>
        /// <param name="value">A 16-bit unsigned integer representing the service identifier.</param>
        /// <returns>The current instance of <see cref="DirectoryRefreshMsg"/> to support method chaining.</returns>
        public DirectoryRefreshMsg ServiceId(ushort value)
        {
            m_ServiceId.Value = value;
            return this;
        }

        /// <inheritdoc />
        protected override void DecodeFrom(RefreshMsg message)
        {
            if (message.HasFilter)
            {
                Filter((DirectoryFilters)message.Filter());
            }
            if (message.HasSeqNum)
            {
                SequenceNumber(message.SeqNum());
            }
            Solicited(message.Solicited());
            Complete(message.Complete());
            ClearCache(message.ClearCache());
            DoNotCache(message.DoNotCache());

            State(message.State());

            if (message.HasServiceId)
            {
                ServiceId(message.GetServiceId());
            }

            if (message.Payload().DataType != DataType.DataTypes.MAP)
                return;

            m_ServiceList.DecodeFrom(message.Payload().Map());
        }

        /// <inheritdoc />
        protected override void EncodeTo(RefreshMsg message)
        {
            message.DomainType(DomainType);
            if (m_SequenceNumber.HasValue)
            {
                message.SeqNum(m_SequenceNumber.Value);
            }
            var ommState = m_State.Value();
            message.State(ommState.StreamState, ommState.DataState, ommState.StatusCode, ommState.StatusText);
            message.Filter((long)Filter());
            message.Solicited(m_Solicited);
            message.Complete(m_Complete);
            message.ClearCache(m_ClearCache);
            message.DoNotCache(m_DoNotCache);
            if (m_ServiceId.HasValue)
            {
                message.ServiceId(m_ServiceId.Value);
            }

            m_Payload.Clear();
            m_ServiceList.EncodeTo(m_Payload);
            message.Payload(m_Payload.Complete());
            m_Payload.Clear();
        }

        /// <inheritdoc />
        public override DirectoryRefreshMsg Clear()
        {
            base.Clear();
            m_SequenceNumber.Clear();
            m_State.Clear();
            m_State.Value(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, string.Empty);
            m_ClearCache = false;
            m_DoNotCache = false;
            m_Complete = false;
            m_Solicited = false;
            m_ServiceId.Clear();
            return this;
        }
        
        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryRefreshMsg"/> instance that is a copy of this instance.</returns>
        public DirectoryRefreshMsg Clone() => new DirectoryRefreshMsg().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        protected override void CopyFromInternal(DirectoryRefreshMsg source)
        {
            base.CopyFromInternal(source);
            m_SequenceNumber.CopyFrom(source.m_SequenceNumber);
            m_State.CopyFrom(source.m_State);
            m_ClearCache = source.m_ClearCache;
            m_DoNotCache = source.m_DoNotCache;
            m_Complete = source.m_Complete;
            m_Solicited = source.m_Solicited;
            m_ServiceId.CopyFrom(source.m_ServiceId);
        }

        /// <inheritdoc />
        protected override void AppendToString(StringBuilder builder, int indent)
        {
            base.AppendToString(builder, indent);
            builder.AddIndent(indent).AppendLine($"State: {m_State}");
            if (HasSequenceNumber)
            {
                builder.AddIndent(indent).AppendLine($"SequenceNumber: {m_SequenceNumber}");
            }
            builder
                .AddIndent(indent).AppendLine($"ClearCache: {m_ClearCache}")
                .AddIndent(indent).AppendLine($"DoNotCache: {m_DoNotCache}")
                .AddIndent(indent).AppendLine($"Complete: {m_Complete}")
                .AddIndent(indent).AppendLine($"Solicited: {m_Solicited}")
                .AddIndent(indent).AppendLine($"ServiceId: {m_ServiceId}");
        }
    }
}
