/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Options to use when submitting a <see cref="IRequestMsg"/>.
    /// Only used whan a watchlist is enabled.
    /// </summary>
    public sealed class ReactorRequestMsgOptions
    {
        /// <summary>
        /// Gets or sets user-specified object to return as the application receives events 
        /// related to this request.
        /// </summary>
        /// <value>the user-specified object</value>
        public object? UserSpecObj { get; set; }

        /// <summary>
        /// Clears to default for reuse.
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            UserSpecObj = null;
        }
    }
}
