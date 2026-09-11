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
    /// A Directory request message is encoded and sent by OMM consumer applications.
    /// A consumer can request information about all services by omitting serviceId
    /// information, or specify a serviceId to request information about only that
    /// service.
    /// </summary>
    public sealed class DirectoryRequestMsg : DirectoryMsg<RequestMsg, DirectoryRequestMsg>, ICloneable
    {
        private OptionalField<ushort> m_ServiceId = new("ServiceId element");
        private OptionalField<string> m_ServiceName = new("ServiceName element");
        private bool m_InitialImage;
        private bool m_InterestAfterRefresh;

        /// <summary>
        /// Initializes a new instance of the DirectoryRequestMsg class.
        /// </summary>
        public DirectoryRequestMsg()
        {
            Clear();
        }

        /// <summary>
        /// Gets the ID of the service to request the directory from.
        /// </summary>
        /// <returns>A 16-bit unsigned integer representing the service identifier.</returns>
        public ushort ServiceId() => m_ServiceId.Value;
        /// <summary>
        /// Sets the ID of the service to request the directory from.
        /// </summary>
        /// <param name="value">A 16-bit unsigned integer representing the service identifier.</param>
        /// <returns>The current instance of <see cref="DirectoryRequestMsg"/> to support method chaining.</returns>
        public DirectoryRequestMsg ServiceId(ushort value)
        {
            m_ServiceId.Value = value;
            return this;
        }

        /// <summary>
        /// Gets the Name of the service to request the directory from.
        /// </summary>
        /// <returns>A string representing the service name.</returns>
        public string ServiceName() => m_ServiceName.Value;
        /// <summary>
        /// Sets the Name of the service to request the directory from.
        /// </summary>
        /// <param name="value">A string representing the service name.</param>
        /// <returns>The current instance of <see cref="DirectoryRequestMsg"/> to support method chaining.</returns>
        public DirectoryRequestMsg ServiceName(string value)
        {
            if (string.IsNullOrEmpty(value))
                throw new OmmInvalidUsageException("ServiceName cannot be null or empty", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_ServiceName.Value = value;
            return this;
        }

        /// <summary>
        /// Checks the presence of the serviceId field.
        /// </summary>
        public bool HasServiceId => m_ServiceId.HasValue;
        /// <summary>
        /// Checks the presence of the serviceName field.
        /// </summary>
        public bool HasServiceName => m_ServiceName.HasValue;

        /// <summary>
        /// Gets or sets the presence of initial image flag.
        /// </summary>
        /// <returns>True if the initial image flag is set; otherwise, false.</returns>
        public bool InitialImage() => m_InitialImage;
        /// <summary>
        /// Sets the presence of initial image flag.
        /// </summary>
        /// <param name="value">True to set the initial image flag; otherwise, false.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRequestMsg InitialImage(bool value)
        {
            m_InitialImage = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of interest after refresh flag.
        /// </summary>
        /// <returns>True if the interest after refresh flag is set; otherwise, false.</returns>
        public bool InterestAfterRefresh() => m_InterestAfterRefresh;
        /// <summary>
        /// Sets the presence of interest after refresh flag.
        /// </summary>
        /// <param name="value">True to set the interest after refresh flag; otherwise, false.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryRequestMsg InterestAfterRefresh(bool value)
        {
            m_InterestAfterRefresh = value;
            return this;
        }

        /// <inheritdoc />
        protected override void DecodeFrom(RequestMsg message)
        {
            if (message.HasFilter)
            {
                Filter((DirectoryFilters)message.Filter());
            }
            if (message.HasServiceId)
            {
                if (!(message.ServiceId() is >= 0 and <= ushort.MaxValue))
                    throw new OmmInvalidUsageException("Invalid ServiceId value", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                ServiceId((ushort)message.ServiceId());
            }
            if (message.HasServiceName)
            {
                ServiceName(message.ServiceName());
            }
            m_InitialImage = message.InitialImage();
            m_InterestAfterRefresh = message.InterestAfterRefresh();
        }

        /// <inheritdoc />
        protected override void EncodeTo(RequestMsg message)
        {
            message.DomainType(DomainType);
            message.Filter((long)Filter());
            if (m_ServiceId.HasValue)
            {
                message.ServiceId(m_ServiceId.Value);
            }
            if (m_ServiceName.HasValue)
            {
                message.ServiceName(m_ServiceName.Value);
            }
            message.InitialImage(m_InitialImage);
            message.InterestAfterRefresh(m_InterestAfterRefresh);
        }

        /// <inheritdoc />
        protected override void CopyFromInternal(DirectoryRequestMsg source)
        {
            base.CopyFromInternal(source);
            m_ServiceId.CopyFrom(source.m_ServiceId);
            m_ServiceName.CopyFrom(source.m_ServiceName);
            m_InitialImage = source.m_InitialImage;
            m_InterestAfterRefresh = source.m_InterestAfterRefresh;
        }

        /// <inheritdoc />
        public override DirectoryRequestMsg Clear()
        {
            base.Clear();
            m_ServiceId.Clear();
            m_ServiceName.Clear();
            m_InitialImage = true;
            m_InterestAfterRefresh = true;
            Filter(DirectoryFilters.ALL);
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryRequestMsg"/> instance that is a copy of this instance.</returns>
        public DirectoryRequestMsg Clone() => new DirectoryRequestMsg().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        protected override void AppendToString(StringBuilder builder, int indent)
        {
            base.AppendToString(builder, indent);
            if (m_ServiceId.HasValue)
            {
                builder.AddIndent(indent).AppendLine($"ServiceId: {m_ServiceId}");
            }

            if (m_ServiceName.HasValue)
            {
                builder.AddIndent(indent).AppendLine($"ServiceName: {m_ServiceName}");
            }

            builder
                .AddIndent(indent).AppendLine($"InitialImage: {m_InitialImage}")
                .AddIndent(indent).AppendLine($"InterestAfterRefresh: {m_InterestAfterRefresh}");
        }
    }
}
