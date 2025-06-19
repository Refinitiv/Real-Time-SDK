/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

namespace LSEG.Eta.ValueAdd.Reactor
{
    internal class WlPostTimeoutInfo
    {
        private IPostMsg m_PostMsg = (IPostMsg)new Msg();

        internal IPostMsg PostMsg
        {
            get => m_PostMsg;
            set
            {
                value.Copy(m_PostMsg, CopyMsgFlags.ALL_FLAGS);
            }
        }

        internal WlTimeoutTimer? Timer { get; set; }

        /// <summary>
        /// Clears the object for re-use.
        /// </summary>
        internal void Clear()
        {
            m_PostMsg.Clear();
            Timer?.ReturnToPool();
            Timer = null;
        }
    }
}