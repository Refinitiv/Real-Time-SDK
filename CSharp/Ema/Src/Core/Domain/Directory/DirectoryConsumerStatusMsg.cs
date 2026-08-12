using LSEG.Ema.Access;
using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The Directory Consumer Status is sent by OMM Consumer applications to inform
    /// a service of how it is being used for Source Mirroring. This message is
    /// primarily informational.
    /// </summary>
    public sealed class DirectoryConsumerStatusMsg : DirectoryMsg<GenericMsg, DirectoryConsumerStatusMsg>, ICloneable
    {
        private OptionalField<long> m_SequenceNumber = new("SequenceNumber element");
        private readonly List<DirectoryConsumerStatusService> m_ConsumerServiceStatusList = new();
        private readonly FluentListBuilder<DirectoryConsumerStatusService> m_ConsumerServiceStatusListBuilder;
        private readonly Map m_Payload = new();

        /// <summary>
        /// Initializes a new instance of the <see cref="DirectoryConsumerStatusMsg"/> class.
        /// </summary>
        public DirectoryConsumerStatusMsg()
        {
            m_ConsumerServiceStatusListBuilder = new(m_ConsumerServiceStatusList);
        }

        /// <inheritdoc />
        public override DirectoryFilters Filter() => throw new OmmInvalidUsageException("Filter isn't used for this type of message", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        /// <inheritdoc />
        public override DirectoryConsumerStatusMsg Filter(DirectoryFilters value) => throw new OmmInvalidUsageException("Filter isn't used for this type of message", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

        /// <summary>
        /// The name of the RDM message.
        /// </summary>
        public string Name => EmaRdm.ENAME_CONS_STATUS;

        /// <summary>
        /// The list of Consumer Service Status elements.
        /// </summary>
        public IList<DirectoryConsumerStatusService> ConsumerServiceStatusList() => m_ConsumerServiceStatusList;
        /// <summary>
        /// The list of Consumer Service Status elements.
        /// </summary>
        public DirectoryConsumerStatusMsg ConsumerServiceStatusList(IList<DirectoryConsumerStatusService> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("consumerServiceStatusList can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_ConsumerServiceStatusList.Clear();
            m_ConsumerServiceStatusList.AddRange(value);
            return this;
        }
        /// <summary>
        /// The list of Consumer Service Status elements.
        /// </summary>
        public DirectoryConsumerStatusMsg ConsumerServiceStatusList(Action<IFluentListBuilder<DirectoryConsumerStatusService>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            buildAction(m_ConsumerServiceStatusListBuilder);
            return this;
        }

        /// <summary>
        /// Sequence number of this message.
        /// </summary>
        public long SequenceNumber() => m_SequenceNumber.Value;
        /// <summary>
        /// Sequence number of this message.
        /// </summary>
        public DirectoryConsumerStatusMsg SequenceNumber(long value)
        {
            m_SequenceNumber.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates presence of the sequence number field.
        /// </summary>
        public bool HasSequenceNumber => m_SequenceNumber.HasValue;

        /// <inheritdoc />
        public override DirectoryConsumerStatusMsg Clear()
        {
            base.Clear();
            m_ConsumerServiceStatusList.Clear();
            m_SequenceNumber.Clear();
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryConsumerStatusMsg"/> instance that is a copy of this instance.</returns>
        public DirectoryConsumerStatusMsg Clone() => new DirectoryConsumerStatusMsg().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        protected override void DecodeFrom(GenericMsg message)
        {
            if (message.Name() != Name)
                throw new OmmInvalidUsageException(
                    $"Unexpected message name: \"{message.Name()}\". Expected: \"{Name}\"",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            if (message.Payload().DataType != DataType.DataTypes.MAP)
                throw new OmmInvalidUsageException(
                    $"Unexpected payload data type: {DataType.AsString(message.Payload().DataType)}. Expected: {DataType.AsString(DataType.DataTypes.MAP)}",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            if (message.HasSeqNum)
            {
                m_SequenceNumber.Value = message.SeqNum();
            }
            foreach (var mapEntry in message.Payload().Map())
            {
                if (mapEntry.Key.DataType != DataType.DataTypes.UINT)
                    throw new OmmInvalidUsageException(
                        $"Unexpected map entry key type: {DataType.AsString(mapEntry.Key.DataType)}. Expected: {DataType.AsString(DataType.DataTypes.UINT)}",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                if (mapEntry.LoadType != DataType.DataTypes.ELEMENT_LIST && // ADD and UPDATE
                    mapEntry.LoadType != DataType.DataTypes.NO_DATA) // DELETE
                    continue;

                var serviceId = mapEntry.GetServiceId();
                var service = m_ConsumerServiceStatusList.FirstOrDefault(x => x.ServiceId() == serviceId);
                if (service == null)
                {
                    service = new DirectoryConsumerStatusService();
                    m_ConsumerServiceStatusList.Add(service);
                }
                if (mapEntry.Action != MapAction.DELETE)
                {
                    service.DecodeFrom(mapEntry.ElementList());
                }
                service.Action((DirectoryMapAction)mapEntry.Action);
                service.ServiceId(serviceId);
            }
        }

        /// <inheritdoc />
        protected override void EncodeTo(GenericMsg message)
        {
            message.DomainType(DomainType);
            message.Name(Name);
            if (m_SequenceNumber.HasValue)
            {
                message.SeqNum(m_SequenceNumber.Value);
            }

            m_Payload.Clear();
            m_Payload.KeyType(DataType.DataTypes.UINT);
            m_Payload.TotalCountHint(m_ConsumerServiceStatusList.Count);
            var elementList = new ElementList();
            foreach (var service in m_ConsumerServiceStatusList)
            {
                service.EncodeTo(elementList);
                m_Payload.AddKeyUInt(service.ServiceId(), (int)service.Action(), elementList.Complete());
                elementList.Clear();
            }

            message.Payload(m_Payload.Complete());
            m_Payload.Clear();
        }

        /// <inheritdoc />
        protected override void CopyFromInternal(DirectoryConsumerStatusMsg source)
        {
            base.CopyFromInternal(source);
            m_ConsumerServiceStatusList.CopyFrom(source.m_ConsumerServiceStatusList);
            m_SequenceNumber.CopyFrom(source.m_SequenceNumber);
        }

        /// <inheritdoc />
        protected override void AppendToString(StringBuilder builder, int indent)
        {
            builder.AddIndent(indent).AppendLine($"DomainType: {Utilities.RdmDomainAsString(DomainType)}");
            builder.AddIndent(indent).AppendLine("ConsumerServiceStatusList:");
            m_ConsumerServiceStatusList.AppendToString(builder, indent + 1,
                (sb, service, indent) => service.AppendToString(sb, indent));
            if (HasSequenceNumber)
            {
                builder.AddIndent(indent).AppendLine($"SequenceNumber: {m_SequenceNumber}");
            }
        }
    }
}
