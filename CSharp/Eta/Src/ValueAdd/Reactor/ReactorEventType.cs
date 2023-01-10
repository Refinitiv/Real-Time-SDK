/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal enum ReactorEventType
    {
        /// <summary>
        /// Unknown event.
        /// </summary>
        INIT = 0,

        /// <summary>
        /// Channel event.
        /// </summary>
        CHANNEL = 1,

        /// <summary>
        /// Reactor-related event.
        /// </summary>
        REACTOR = 2,

        /// <summary>
        /// Flushing needs to start or has finished.
        /// </summary>
        FLUSH = 3,

        /// <summary>
        /// A timer event.
        /// </summary>
        TIMER = 4
    }
}
