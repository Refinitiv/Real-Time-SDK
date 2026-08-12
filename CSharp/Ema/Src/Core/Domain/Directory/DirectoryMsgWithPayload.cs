using LSEG.Ema.Access;
using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Internal;
using System;
using System.Collections.Generic;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Directory Base Message with payload, that can be transformed into list of RDM Services.
    /// </summary>
    /// <typeparam name="TMessage">Type of message to be encoded/decoded.</typeparam>
    /// <typeparam name="TSelf">Type used for fluent interfaces.</typeparam>
    public abstract class DirectoryMsgWithPayload<TMessage, TSelf> : DirectoryMsg<TMessage, TSelf>
        where TMessage : Msg, new()
        where TSelf : DirectoryMsgWithPayload<TMessage, TSelf>
    {
        /// <summary>
        /// Payload for RDM directory message.
        /// </summary>
        protected readonly Map m_Payload = new();
        internal readonly DirectoryServiceList m_ServiceList = new();
        private readonly FluentListBuilder<DirectoryService> m_ServiceListBuilder;

        /// <summary>
        /// Initializes a new instance of the DirectoryMsgWithPayload class.
        /// </summary>
        protected DirectoryMsgWithPayload()
        {
            m_ServiceListBuilder = new(m_ServiceList.Value);
        }

        /// <summary>
        /// Gets service entries into the directory message. This object's
        /// Service elements will be set to Service elements from list in the
        /// parameter passed in.
        /// </summary>
        public IList<DirectoryService> ServiceList() => m_ServiceList.Value;
        /// <summary>
        /// Sets service entries into the directory message. This object's
        /// Service elements will be set to Service elements from list in the
        /// parameter passed in.
        /// </summary>
        public TSelf ServiceList(IList<DirectoryService> value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("ServiceList must be non-null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            m_ServiceList.Value = value;
            return (TSelf)this;
        }
        /// <summary>
        /// Sets service entries into the directory message. This object's
        /// Service elements will be set to Service elements from list in the
        /// parameter passed in.
        /// </summary>
        public TSelf ServiceList(Action<IFluentListBuilder<DirectoryService>> buildAction)
        {
            if (buildAction == null)
                throw new OmmInvalidUsageException($"{nameof(buildAction)} must be non-null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            buildAction(m_ServiceListBuilder);
            return (TSelf)this;
        }

        /// <inheritdoc />
        public override TSelf Clear()
        {
            base.Clear();
            m_Payload.Clear();
            m_ServiceList.Clear();
            return (TSelf)this;
        }

        /// <inheritdoc />
        protected override void CopyFromInternal(TSelf source)
        {
            base.CopyFromInternal(source);
            m_ServiceList.CopyFrom(source.m_ServiceList);
        }

        /// <inheritdoc />
        protected override void AppendToString(StringBuilder builder, int indent)
        {
            base.AppendToString(builder, indent);
            builder
                .AddIndent(indent).AppendLine("ServiceList:");
            m_ServiceList.AppendToString(builder, indent + 1);
        }
    }
}
