using LSEG.Ema.Access;
using LSEG.Ema.Domain.Internal;
using System;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Directory Update. Used by an OMM Provider to provide updates about available services.
    /// </summary>
    public sealed class DirectoryUpdateMsg : DirectoryMsgWithPayload<UpdateMsg, DirectoryUpdateMsg>, ICloneable
    {
        private OptionalField<long> m_SequenceNumber = new();
        private bool m_DoNotCache;
        private bool m_DoNotConflate;

        /// <summary>
        /// Gets the presence of sequence number field.
        /// </summary>
        public bool HasSequenceNumber => m_SequenceNumber.HasValue;

        /// <summary>
        /// Gets sequence number of this message.
        /// </summary>
        public long SequenceNumber() => m_SequenceNumber.Value;
        /// <summary>
        /// Sets sequence number of this message.
        /// </summary>
        public DirectoryUpdateMsg SequenceNumber(long value)
        {
            m_SequenceNumber.Value = value;
            return this;
        }

        /// <summary>
        /// Indicates the presence of the filter field.
        /// </summary>
        public bool HasFilter { get; private set; }

        /// <inheritdoc />
        public override DirectoryFilters Filter()
        {
            if (!HasFilter)
                throw new OmmInvalidUsageException($"{nameof(Filter)} element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return base.Filter();
        }

        /// <inheritdoc />
        public override DirectoryUpdateMsg Filter(DirectoryFilters value)
        {
            base.Filter(value);
            HasFilter = true;
            return this;
        }

        /// <summary>
        /// Gets the presence of do not cache flag.
        /// </summary>
        public bool DoNotCache() => m_DoNotCache;
        /// <summary>
        /// Sets the presence of do not cache flag.
        /// </summary>
        public DirectoryUpdateMsg DoNotCache(bool value)
        {
            m_DoNotCache = value;
            return this;
        }

        /// <summary>
        /// Gets the presence of do not conflate flag.
        /// </summary>
        public bool DoNotConflate() => m_DoNotConflate;
        /// <summary>
        /// Sets the presence of do not conflate flag.
        /// </summary>
        public DirectoryUpdateMsg DoNotConflate(bool value)
        {
            m_DoNotConflate = value;
            return this;
        }

        /// <inheritdoc />
        public override DirectoryUpdateMsg Clear()
        {
            base.Clear();
            m_SequenceNumber.Clear();
            HasFilter = false;
            m_DoNotCache = false;
            m_DoNotConflate = false;
            return this;
        }

        /// <summary>
        /// Creates a copy of this instance.
        /// </summary>
        /// <returns>A new <see cref="DirectoryUpdateMsg"/> instance that is a copy of this instance.</returns>
        public DirectoryUpdateMsg Clone() => new DirectoryUpdateMsg().CopyFrom(this);

        /// <inheritdoc />
        object ICloneable.Clone() => Clone();

        /// <inheritdoc />
        protected override void DecodeFrom(UpdateMsg message)
        {
            if (message.HasFilter)
            {
                Filter((DirectoryFilters)message.Filter());
            }
            if (message.HasSeqNum)
            {
                SequenceNumber(message.SeqNum());
            }
            DoNotCache(message.DoNotCache());
            DoNotConflate(message.DoNotConflate());

            if (message.Payload().DataType != DataType.DataTypes.MAP)
                return;
            m_ServiceList.DecodeFrom(message.Payload().Map());
        }

        /// <inheritdoc />
        protected override void EncodeTo(UpdateMsg message)
        {
            message.DomainType(DomainType);
            if (HasSequenceNumber)
            {
                message.SeqNum(SequenceNumber());
            }
            if (HasFilter)
            {
                message.Filter((long)Filter());
            }
            message.DoNotCache(DoNotCache());
            message.DoNotConflate(DoNotConflate());

            m_Payload.Clear();
            m_ServiceList.EncodeTo(m_Payload);
            message.Payload(m_Payload.Complete());
            m_Payload.Clear();
        }

        /// <inheritdoc />
        protected override void CopyFromInternal(DirectoryUpdateMsg source)
        {
            base.CopyFromInternal(source);
            m_SequenceNumber.CopyFrom(source.m_SequenceNumber);
            HasFilter = source.HasFilter;
            m_DoNotCache = source.m_DoNotCache;
            m_DoNotConflate = source.m_DoNotConflate;
        }

        /// <inheritdoc />
        protected override void AppendToString(StringBuilder builder, int indent)
        {
            base.AppendToString(builder, indent);
            if (HasSequenceNumber)
            {
                builder.AddIndent(indent).AppendLine($"SequenceNumber: {SequenceNumber()}");
            }
            builder
                .AddIndent(indent).AppendLine($"DoNotCache: {DoNotCache()}")
                .AddIndent(indent).AppendLine($"DoNotConflate: {DoNotConflate()}");
        }
    }
}
