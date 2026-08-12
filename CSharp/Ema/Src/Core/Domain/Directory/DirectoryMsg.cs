using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System.Text;
using static LSEG.Ema.Access.Utilities;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Directory Base Message. This RDM directory messages may be reused or
    /// pooled in a single collection via their common
    /// interface and re-used as a different Directory message types.
    /// </summary>
    /// <typeparam name="TMessage">Type of message to be encoded/decoded.</typeparam>
    /// <typeparam name="TSelf">Type used for fluent interfaces.</typeparam>
    public abstract class DirectoryMsg<TMessage, TSelf>
        where TMessage : Msg, new()
        where TSelf : DirectoryMsg<TMessage, TSelf>
    {
        private readonly StringBuilder m_ToString = new();
        private readonly TMessage m_Message = new();
        private DirectoryFilters m_Filter;

        /// <summary>
        /// Returns the domain type of the RDM message. See <see cref="EmaRdm"/>.MMT_*.
        /// </summary>
        public int DomainType { get; } = EmaRdm.MMT_DIRECTORY;

        /// <summary>
        /// Filter indicating which filters may appear on this stream. Where
        /// possible, this should match the consumer's request.
        /// </summary>
        public virtual DirectoryFilters Filter() => m_Filter;

        /// <summary>
        /// Filter indicating which filters may appear on this stream. Where
        /// possible, this should match the consumer's request.
        /// </summary>
        public virtual TSelf Filter(DirectoryFilters value)
        {
            m_Filter = value;
            return (TSelf)this;
        }

        /// <summary>
        /// Gets message based on RDM.
        /// </summary>
        public TMessage Message()
        {
            ClearMessage();
            EncodeTo(m_Message);
            return m_Message;
        }

        /// <summary>
        /// Creates or sets message based on RDM.
        /// </summary>
        public TSelf Message(TMessage value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("Message can not be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            if (value.DomainType() != DomainType)
                throw new OmmInvalidUsageException($"Domain type must be Directory instead of {RdmDomainAsString(value.DomainType())}.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            Clear();
            DecodeFrom(value);
            return (TSelf)this;
        }

        /// <summary>
        /// Clears the current contents of the message and prepares it for re-use.
        /// </summary>
        public virtual TSelf Clear()
        {
            m_Filter = default;
            ClearMessage();
            return (TSelf)this;
        }

        /// <summary>
        /// Copies the state from the specified source instance to the current instance, replacing any existing data.
        /// </summary>
        /// <remarks>This method clears the current instance before copying the state from the source.</remarks>
        /// <param name="source">The instance from which to copy state. Must not be null.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public TSelf CopyFrom(TSelf source)
        {
            Clear();
            CopyFromInternal(source);
            return (TSelf)this;
        }

        /// <inheritdoc />
        public override string ToString()
        {
            m_ToString.Clear();
            AppendToString(m_ToString, 0);
            return m_ToString.ToString();
        }

        /// <summary>
        /// Converts <see cref="DirectoryMsg{TMessage, TSelf}"/> to corresponding <see cref="Msg"/> descendant.
        /// </summary>
        /// <param name="directoryMsg"></param>
        public static implicit operator TMessage(DirectoryMsg<TMessage, TSelf> directoryMsg) => directoryMsg.Message();

        /// <summary>
        /// Converts <see cref="DirectoryMsg{TMessage, TSelf}"/> to corresponding <see cref="Msg"/>.
        /// </summary>
        /// <param name="directoryMsg"></param>
        public static implicit operator Msg(DirectoryMsg<TMessage, TSelf> directoryMsg) => directoryMsg.Message();

        /// <summary>
        /// Decodes values from <paramref name="message"/> into current object properties.
        /// </summary>
        /// <param name="message"></param>
        protected abstract void DecodeFrom(TMessage message);
        /// <summary>
        /// Encodes values to <paramref name="message"/> from current object properties.
        /// </summary>
        /// <param name="message"></param>
        protected abstract void EncodeTo(TMessage message);

        /// <summary>
        /// Copies the state from the specified instance into the current instance.
        /// </summary>
        /// <remarks>Derived classes should implement this method to define how state is copied between
        /// instances. This method is typically used to update the current object to match the provided
        /// instance.</remarks>
        /// <param name="source">The instance from which to copy state. Must not be null.</param>
        protected virtual void CopyFromInternal(TSelf source)
        {
            m_Filter = source.m_Filter;
        }

        /// <summary>
        /// Appends a formatted string representation of current object to the specified StringBuilder,
        /// using the given indentation level.
        /// </summary>
        /// <param name="builder">The StringBuilder to which the formatted output is appended. Must not be null.</param>
        /// <param name="indent">The number of indentation levels to apply to each line of the output. Must be non-negative.</param>
        protected virtual void AppendToString(StringBuilder builder, int indent)
        {
            builder
                .AddIndent(indent).AppendLine($"Filter: {m_Filter}")
                .AddIndent(indent).AppendLine($"DomainType: {Utilities.RdmDomainAsString(DomainType)}");
        }

        private void ClearMessage()
        {
            m_Message.Clear_All();
        }
    }
}
