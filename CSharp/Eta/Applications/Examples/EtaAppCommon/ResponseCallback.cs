/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Callback interface for processing a response message.
    /// </summary>
    public interface ResponseCallback
    {
        /// <summary>
        /// Process the response message.
        /// </summary>
        /// <param name="session">Current <see cref="ChannelSession"/> instance</param>
        /// <param name="transportBuffer"><see cref="ITransportBuffer"/> instance that contains the response message.</param>
        void ProcessResponse(ChannelSession session, ITransportBuffer transportBuffer);
    }
}
