/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.ValueAdd.Reactor
{

    /// <summary>
    /// Event provided to <see cref="IRDMLoginMsgCallback"/> methods.
    /// </summary>
    /// <seealso cref="ReactorMsgEvent"/>
    sealed public class RDMLoginMsgEvent : ReactorMsgEvent
    {
        internal RDMLoginMsgEvent() : base()
        {
        }

        /// <summary>
        /// Gets LoginMsg associated with this message event.
        /// </summary>
        public LoginMsg? LoginMsg { get; internal set; }
    }
}
