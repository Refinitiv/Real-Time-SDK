/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// IOmmConsumerEvent encapsulates item identifiers.
    /// </summary>
    /// <remarks>
    /// <para>
    /// IOmmConsumerEvent is used to convey item identifiers to application.
    /// IOmmConsumerEvent is returned through IOmmConsumerClient callback methods.
    /// </para>
    /// <para>
    /// IOmmConsumerEvent is read only. It is used for item identification only.
    /// </para>
    /// </remarks>
    /// <seealso cref="OmmConsumer"/>
    /// <seealso cref="IOmmConsumerClient"/>
    public interface IOmmConsumerEvent
    {
        /// <summary>
        /// Gets a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.
        /// Item identifier is returned from all OmmConsumer RegisterClient() methods.
        /// </summary>
        /// <value>The item identifier or handle.</value>
        public long Handle { get; }

        /// <summary>
        /// Gets an identifier (a.k.a., closure) associated with an open stream by consumer application.
        /// Application associates the closure with an open item stream on OmmConsumer RegisterClient() methods
        /// </summary>
        /// <value>The closure object.</value>
        public object? Closure { get; }

        /// <summary>
        /// Gets <see cref="OmmConsumer"/> instance for this event.
        /// </summary>
        /// <value>The reference to <see cref="OmmConsumer"/>.</value>
        public OmmConsumer Consumer { get; }

        /// <summary>
        /// Returns the channel information for the channel associated with this event.
        /// </summary>
        /// <returns>The channel information.</returns>
        public ChannelInformation ChannelInformation();
    }
}
