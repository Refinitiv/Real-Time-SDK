/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access
{
    /// <summary>
    /// IOmmProviderEvent encapsulates item identifiers.
    /// </summary>
    /// <remarks>
    /// <para>
    /// IOmmProviderEvent is used to convey item identifiers and OmmProvider to application.<br/>
    /// </para>
    /// <para>
    /// IOmmProviderEvent is read only interface. It is used for item identification only.
    /// </para>
    /// </remarks>
    public interface IOmmProviderEvent
    {
        /// <summary>
        /// Gets a unique item identifier (a.k.a., item handle) associated by EMA with an open item stream.
        /// Item identifier is returned from all OmmProvider RegisterClient() methods.
        /// </summary>
        /// <value>The item identifier or handle.</value>
        public long Handle { get; }

        /// <summary>
        /// Returns a unique client identifier (a.k.a., client handle) associated by EMA with a connected client.
        /// </summary>
        public long ClientHandle { get; }

        /// <summary>
        /// Gets an identifier (a.k.a., closure) associated with an open stream by provider application.
        /// Application associates the closure with an open item stream on OmmProvider RegisterClient() methods
        /// </summary>
        /// <value>The closure object.</value>
        public object? Closure { get; }

        /// <summary>
        /// Gets <see cref="OmmProvider"/> instance for this event.
        /// </summary>
        /// <value>The reference to <see cref="OmmProvider"/>.</value>
        public OmmProvider Provider { get; }

        /// <summary>
        /// Returns the channel information for the channel associated with this event.
        /// </summary>
        /// <returns>The channel information.</returns>
        public ChannelInformation ChannelInformation();
    }
}
