/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using System.Text;

namespace LSEG.Ema.Domain.Directory
{
    /// <summary>
    /// The RDM Service Filter. Contains information provided by the Source Directory filter.
    /// </summary>
    /// <typeparam name="T">The type of the complex data associated with this filter.</typeparam>
    /// <typeparam name="TSelf">The type of the derived filter class.</typeparam>
    public abstract class DirectoryServiceFilter<T, TSelf>
        where T : ComplexType
        where TSelf : DirectoryServiceFilter<T, TSelf>
    {
        private readonly StringBuilder m_ToString = new();
        private DirectoryFilterAction m_Action = DirectoryFilterAction.SET;

        /// <summary>
        /// Action associated with this service filter.
        /// </summary>
        /// <returns>The action associated with this service filter.</returns>
        public DirectoryFilterAction Action() => m_Action;
        /// <summary>
        /// Action associated with this service filter.
        /// </summary>
        /// <param name="value">The action to set for this service filter.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public TSelf Action(DirectoryFilterAction value)
        {
            if (!System.Enum.IsDefined(value))
                throw new OmmInvalidUsageException($"Invalid action value: {value}", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            m_Action = value;
            return (TSelf)this;
        }

        /// <summary>
        /// Clears the current contents of the message and prepares it for re-use.
        /// </summary>
        /// <returns>The current instance to support method chaining.</returns>
        public virtual TSelf Clear()
        {
            m_Action = DirectoryFilterAction.SET;
            return (TSelf)this;
        }

        /// <summary>
        /// Copies the state from the specified source instance to the current instance, replacing any existing data.
        /// </summary>
        /// <remarks>This method clears the current instance before copying the state from the source.</remarks>
        /// <param name="source">The instance from which to copy state. Must not be null.</param>
        /// <returns>The current instance to support method chaining.</returns>
        public virtual TSelf CopyFrom(TSelf source)
        {
            Clear();
            m_Action = source.m_Action;
            return (TSelf)this;
        }

        internal abstract void DecodeFrom(T data);
        internal abstract void EncodeTo(T data);

        /// <summary>
        /// Populated by <see cref="Rdm.EmaRdm"/>
        /// </summary>
        public abstract int FilterId { get; }

        /// <inheritdoc />
        public override string ToString()
        {
            m_ToString.Clear();
            AppendToString(m_ToString, 0);
            return m_ToString.ToString();
        }

        internal virtual void AppendToString(StringBuilder sb, int indent)
        {
            sb
                .AddIndent(indent).AppendLine($"FilterId: {FilterId}")
                .AddIndent(indent).AppendLine($"Action: {m_Action}");
        }
    }

    /// <summary>
    /// Represents filter entry action.
    /// </summary>
    public enum DirectoryFilterAction
    {
        /// <summary>
        /// Indicates a partial change of the current Omm data.
        /// </summary>
        UPDATE = FilterAction.UPDATE,
        /// <summary>
        /// Indicates to specify or replace the current Omm data.
        /// </summary>
        SET = FilterAction.SET,
        /// <summary>
        /// Indicates to unset the current Omm data.
        /// </summary>
        CLEAR = FilterAction.CLEAR,
    }
}
