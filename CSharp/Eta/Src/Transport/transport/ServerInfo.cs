/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace Refinitiv.Eta.Transports.Interfaces
{
    /// <summary>
    /// ETA Server Info returned by the <see cref="IServer.Info(ServerInfo, out Error)"/> call.
    /// </summary>
    public class ServerInfo
    {
        /// <summary>
        /// The default constructor to clear to default values.
        /// </summary>
        public ServerInfo()
        {
            Clear();
        }

        /// <summary>
        /// Gets the number of currently used shared pool buffers across all channels
        /// connected to the <see cref="IServer"/>
        /// </summary>
        /// <returns>The current buffer usage</returns>
        public int CurrentBufferUsage { get; internal set; }

        /// <summary>
        /// Gets the maximum achieved number of used shared pool buffers across all users
        /// connected to the <see cref="IServer"/>. This value can be reset through the use
        /// of , also described in this section.
        /// </summary>
        /// <returns>The peak buffer usage</returns>
        public int PeakBufferUsage { get; internal set; }

        /// <summary>
        /// Clears ETA Server Info
        /// </summary>
        public void Clear()
        {
            CurrentBufferUsage = 0;
            PeakBufferUsage = 0;
        }

        /// <summary>
        /// The string representation of this object
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            return $"ServerInfo\n\nCurrentBufferUsage: {CurrentBufferUsage}\n" +
                $"\nPeakBufferUsage: {PeakBufferUsage}\n";
        }
    }
}
