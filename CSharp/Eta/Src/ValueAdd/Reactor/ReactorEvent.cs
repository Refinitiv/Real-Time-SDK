/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */


using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Base class for Reactor's events
    /// </summary>
    public class ReactorEvent : VaNode
    {
        /// <summary>
        /// Gets the ReactorChannel associated with this event.
        /// </summary>
        public ReactorChannel? ReactorChannel { get; internal set; }

        /// <summary>
        /// Gets the ReactorErrorInfo associated with this event.
        /// </summary>
        public ReactorErrorInfo ReactorErrorInfo { get; internal set; }

        internal ReactorEventType Type { get; set; }

        /// <summary>
        /// Creates <see cref="ReactorEvent"/>
        /// </summary>
        protected ReactorEvent()
        {
            ReactorErrorInfo = new ReactorErrorInfo();
        }

        /// <summary>
        /// Returns this object back to its pool.
        /// </summary>
        public override void ReturnToPool()
        {
            ReactorChannel = null;
            base.ReturnToPool();
        }

        /// <summary>
        /// Returns a String representation of this object.
        /// </summary>
        /// <returns>a String representation of this object</returns>
        public override string ToString()
        {
            return (ReactorChannel != null ? ReactorChannel.ToString() : "ReactorChannel is null" + ", " + ReactorErrorInfo.ToString());
        }
    }
}
