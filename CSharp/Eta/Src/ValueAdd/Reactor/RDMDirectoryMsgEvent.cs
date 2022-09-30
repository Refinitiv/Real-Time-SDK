/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.ValueAdd.Rdm;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Event provided to RDMDirectoryMsgCallback methods.
    /// </summary>
    public class RDMDirectoryMsgEvent : ReactorMsgEvent
    {
        public DirectoryMsg? DirectoryMsg { get; set; }

        public override void Clear()
        {
            base.Clear();
            DirectoryMsg = null;
        }

        public override void ReturnToPool()
        {
            DirectoryMsg = null;
            base.ReturnToPool();
        }
    }
}
