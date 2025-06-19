/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// An event that has occurred on an RsslReactorChannel.
    /// </summary>
    /// <see cref="ReactorChannel"/>
    sealed public class ReactorChannelEvent : ReactorEvent
    {
        /// <summary>
        /// Gets the event type.
        /// </summary>
        /// <value><see cref="ReactorChannelEventType"/></value>
        public ReactorChannelEventType EventType { get; internal set; }

        internal ReactorChannelEvent()
        {
        }

        internal void Clear()
        {
            EventType = ReactorChannelEventType.INIT;
        }

        /// <summary>
        /// Returns a String representation of this object.
        /// </summary>
        /// <returns>a String representation of this object</returns>
        public override string ToString()
        {
            return base.ToString() + ", " + EventType;
        }
    }
}
