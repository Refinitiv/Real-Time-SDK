/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    ///  An event that has occurred to get service discovery endpoint information.
    /// </summary>
    /// <seealso cref="ReactorEvent"/>
    /// <seealso cref="ReactorServiceEndpointInfo"/>
    public class ReactorServiceEndpointEvent : ReactorEvent
    {
        internal ReactorServiceEndpointEvent()
        {
        }

        /// <summary>
        /// Gets a list of service endpoint information associated with this event.
        /// </summary>
        public List<ReactorServiceEndpointInfo>? ServiceEndpointInfoList { get; internal set; }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public void Clear()
        {
            UserSpecObject = null;
            ServiceEndpointInfoList = null;
        }

        /// <summary>
        /// Gets a closure object associated with this event.
        /// </summary>
        public object? UserSpecObject { get; internal set; }

        /// <summary>
        /// Returns this object to the pool.
        /// </summary>
        public override void ReturnToPool()
        {
            UserSpecObject = null;
            ServiceEndpointInfoList = null;

            base.ReturnToPool();
        }
    }
}
