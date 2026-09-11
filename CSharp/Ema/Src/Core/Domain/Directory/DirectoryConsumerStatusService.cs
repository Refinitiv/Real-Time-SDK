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
    /// Information about how a Consumer is using a particular service with regard to
    /// Source Mirroring.
    /// </summary>
    public sealed class DirectoryConsumerStatusService : ICloneable
    {
        private readonly StringBuilder m_ToString = new();
        private OptionalField<WarmStandbyDirectoryServiceType> m_WarmStandbyMode;
        private ushort m_ServiceId;
        private DirectoryMapAction m_Action = DirectoryMapAction.ADD;
        private OptionalField<SourceMirroringMode> m_SourceMirroringMode;

        /// <summary>
        /// ID of the service this status concerns.
        /// </summary>
        /// <returns>The ID of the service this status concerns.</returns>
        public ushort ServiceId() => m_ServiceId;
        /// <summary>
        /// ID of the service this status concerns.
        /// </summary>
        /// <param name="value">The ID of the service to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryConsumerStatusService ServiceId(ushort value)
        {
            m_ServiceId = value;
            return this;
        }

        /// <summary>
        /// Action associated with this status.
        /// </summary>
        /// <returns>The action associated with this status.</returns>
        public DirectoryMapAction Action() => m_Action;
        /// <summary>
        /// Action associated with this status.
        /// </summary>
        /// <param name="value">The action to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryConsumerStatusService Action(DirectoryMapAction value)
        {
            m_Action = value;
            return this;
        }

        /// <summary>
        /// Gets a value indicating whether a source mirroring mode is specified.
        /// </summary>
        public bool HasSourceMirroringMode => m_SourceMirroringMode.HasValue;

        /// <summary>
        /// The Source Mirroring Mode for this service.
        /// </summary>
        /// <returns>The Source Mirroring Mode for this service.</returns>
        public SourceMirroringMode SourceMirroringMode() => m_SourceMirroringMode.Value;
        /// <summary>
        /// The Source Mirroring Mode for this service.
        /// </summary>
        /// <param name="value">The Source Mirroring Mode to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryConsumerStatusService SourceMirroringMode(SourceMirroringMode value)
        {
            m_SourceMirroringMode.Value = value;
            return this;
        }

        /// <summary>
        /// Gets a value indicating whether a warm standby mode is specified.
        /// </summary>
        public bool HasWarmStandbyMode => m_WarmStandbyMode.HasValue;

        /// <summary>
        /// The Warm Standby Mode for this service.
        /// </summary>
        /// <returns>The Warm Standby Mode for this service.</returns>
        public WarmStandbyDirectoryServiceType WarmStandbyMode() => m_WarmStandbyMode.Value;
        /// <summary>
        /// The Warm Standby Mode for this service.
        /// </summary>
        /// <param name="value">The Warm Standby Mode to set.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryConsumerStatusService WarmStandbyMode(WarmStandbyDirectoryServiceType value)
        {
            m_WarmStandbyMode.Value = value;
            return this;
        }

        /// <summary>
        /// Resets object to its initial state.
        /// </summary>
        /// <returns>The current instance to support method chaining.</returns>
        public DirectoryConsumerStatusService Clear()
        {
            m_ServiceId = 0;
            m_Action = DirectoryMapAction.ADD;
            m_SourceMirroringMode.Clear();
            m_WarmStandbyMode.Clear();
            return this;
        }

        /// <summary>
        /// Decodes the current instance from the specified element list.
        /// </summary>
        /// <param name="elementList">The element list containing the data to decode into this instance.</param>
        /// <returns>The current instance of the DirectoryConsumerStatusService to support method chaining.</returns>
        internal DirectoryConsumerStatusService DecodeFrom(ElementList elementList)
        {
            var foundSourceMirroringMode = false;
            var foundWarmStandbyMode = false;
            foreach (var element in elementList)
            {
                switch (element.Name)
                {
                    case EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE:
                        m_SourceMirroringMode.Value = (SourceMirroringMode)element.UIntValue();
                        if (!System.Enum.IsDefined(m_SourceMirroringMode.Value))
                            throw new OmmInvalidUsageException(
                                $"Invalid element value {m_SourceMirroringMode} of {element.Name}",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        foundSourceMirroringMode = true;
                        break;
                    case EmaRdm.ENAME_WARMSTANDBY_MODE:
                        m_WarmStandbyMode.Value = (WarmStandbyDirectoryServiceType)element.UIntValue();
                        if (!System.Enum.IsDefined(m_WarmStandbyMode.Value))
                            throw new OmmInvalidUsageException(
                                $"Invalid element value {m_WarmStandbyMode} of {element.Name}",
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        foundWarmStandbyMode = true;
                        break;
                    default:
                        // Ignore unknown elements
                        break;
                }
            }

            if (!foundSourceMirroringMode && !foundWarmStandbyMode)
                throw new OmmInvalidUsageException(
                    $"Both {EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE} and {EmaRdm.ENAME_WARMSTANDBY_MODE} element are absent",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            return this;
        }

        /// <summary>
        /// Encodes the current consumer status settings into the specified element list.
        /// </summary>
        /// <param name="elementList">The element list to which the consumer status information is added. Cannot be null.</param>
        /// <returns>The current instance of the DirectoryConsumerStatusService to support method chaining.</returns>
        internal DirectoryConsumerStatusService EncodeTo(ElementList elementList)
        {
            if (m_SourceMirroringMode.HasValue)
            {
                elementList.AddUInt(EmaRdm.ENAME_CONS_SOURCE_MIROR_MODE, (ulong)m_SourceMirroringMode.Value);
            }

            if (m_WarmStandbyMode.HasValue)
            {
                elementList.AddUInt(EmaRdm.ENAME_WARMSTANDBY_MODE, (ulong)m_WarmStandbyMode.Value);
            }

            return this;
        }

        /// <summary>
        /// Copies the values from the specified source instance to the current instance.
        /// </summary>
        /// <remarks>This method overwrites the current instance's values with those from the
        /// specified source. The source and target instances must be of the same type.</remarks>
        /// <param name="source">The source instance from which to copy values. Cannot be null.</param>
        /// <returns>The current instance of the DirectoryConsumerStatusService to support method chaining.</returns>
        public DirectoryConsumerStatusService CopyFrom(DirectoryConsumerStatusService source)
        {
            m_ServiceId = source.m_ServiceId;
            m_Action = source.m_Action;
            m_SourceMirroringMode = source.m_SourceMirroringMode;
            m_WarmStandbyMode = source.m_WarmStandbyMode;
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryConsumerStatusService"/> instance that is a copy of this instance.</returns>
        public DirectoryConsumerStatusService Clone() => new DirectoryConsumerStatusService().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        public override string ToString()
        {
            m_ToString.Clear();
            AppendToString(m_ToString, 0);
            return m_ToString.ToString();
        }

        internal void AppendToString(StringBuilder sb, int indent)
        {
            sb.AddIndent(indent).AppendLine("ConsumerStatusService:");
            indent++;
            sb
                .AddIndent(indent).AppendLine($"ServiceId: {m_ServiceId}")
                .AddIndent(indent).AppendLine($"Action: {m_Action}")
                .AddIndent(indent).AppendLine($"SourceMirroringMode: {m_SourceMirroringMode}")
                .AddIndent(indent).AppendLine($"WarmStandbyMode: {m_WarmStandbyMode}");
            indent--;
            sb.AddIndent(indent).AppendLine("EndConsumerStatusService");
        }
    }

    /// <summary>
    /// Indicates how the downstream component is using the service.
    /// </summary>
    public enum SourceMirroringMode
    {
        /// <summary>
        /// The downstream device is using the data from this service, and is not
        /// receiving it from any other service.
        /// </summary>
        ACTIVE_NO_STANDBY,
        /// <summary>
        /// The downstream device is using the data from this service, but is also
        /// getting it from another service.
        /// </summary>
        ACTIVE_WITH_STANDBY,
        /// <summary>
        /// The downstream device is getting data from this service, but is actually
        /// using data from another service.
        /// </summary>
        STANDBY
    }

    /// <summary>
    /// Indicates the warm standby service type
    /// </summary>
    public enum WarmStandbyDirectoryServiceType
    {
        /// <summary>
        /// Indicates that the provider for this service is the active server.
        /// </summary>
        ACTIVE,
        /// <summary>
        /// Indicates that the provider for this service is the standby server.
        /// </summary>
        STANDBY
    }
}