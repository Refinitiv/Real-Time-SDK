/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Rdm;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Event provided to <see cref="IDirectoryMsgCallback"/> methods.
    /// </summary>
    public class RDMDirectoryMsgEvent : ReactorMsgEvent
    {
        /// <summary>
        /// Gets DirectoryMsg associated with this message event.
        /// </summary>
        public DirectoryMsg? DirectoryMsg { get; internal set; }

        /// <summary>
        /// Clears to default values.
        /// </summary>
        public override void Clear()
        {
            base.Clear();
            DirectoryMsg = null;
        }

        /// <summary>
        /// Returns the object back to the pool.
        /// </summary>
        public override void ReturnToPool()
        {
            DirectoryMsg = null;
            base.ReturnToPool();
        }
    }
}
