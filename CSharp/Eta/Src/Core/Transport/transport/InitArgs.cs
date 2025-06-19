/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// ETA Initialize Arguments used in the <see cref="Transport.Initialize(InitArgs, out Error)"/> call.
    /// 
    /// <seealso cref="Transport"/>
    /// </summary>
    sealed public class InitArgs
    {
        /// <summary>
        /// The default constructor to clear all options to default values.
        /// </summary>
        public InitArgs()
        {
            Clear();
        }

        /// <summary>
        /// If locking is <c>true</c>, the global locking will be used by <see cref="Transport"/>.
        /// </summary>
        /// <value>The global locking</value>
        public bool GlobalLocking;

        /// <summary>
        /// Clears InitArgs options
        /// </summary>
        public void Clear()
        {
            GlobalLocking = false;
        }
    }
}
