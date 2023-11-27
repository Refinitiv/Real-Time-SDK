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
    /// Base class for Reactor's roles
    /// </summary>
    public abstract class ReactorRole
    {
        /// <summary>
        /// Gets the role type
        /// </summary>
        /// <value><see cref="ReactorRoleType"/></value>
        public ReactorRoleType Type { get; protected set; }

        /// <summary>
        /// Gets or sets the <see cref="IReactorChannelEventCallback"/> associated with this role.
        /// Handles channel events.
        /// </summary>
        /// <remarks>
        /// Must be provided for all roles.
        /// </remarks>
        public IReactorChannelEventCallback? ChannelEventCallback { get; set; }

        /// <summary>
        /// The <see cref="IDefaultMsgCallback"/> associated with this role. 
        /// Handles message events that aren't handled by a specific domain callback.
        /// </summary>
        /// <remarks>
        /// Must be provided for all roles.
        /// </remarks>
        public IDefaultMsgCallback? DefaultMsgCallback { get; set; }

        internal ReactorMsgEvent DefaultReactorMsgEvent = new ReactorMsgEvent();

        /// <summary>
        /// Create <see cref="ReactorRole"/>
        /// </summary>
        protected ReactorRole()
        {
        }

        /// <summary>
        /// Returns a String representation of this object.
        /// </summary>
        /// <returns>a String representation of this object</returns>
        public override string ToString()
        {
            return "ReactorRole: " + Type.ToString();
        }

        /// <summary>
        /// Performs a deep copy from a specified ReactorRole into this ReactorRole.
        /// </summary>
        /// <param name="role">The ReactorRole to copy from</param>
        internal void Copy(ReactorRole role)
        {
            ChannelEventCallback = role.ChannelEventCallback;
            DefaultMsgCallback = role.DefaultMsgCallback;
        }

    }
}
