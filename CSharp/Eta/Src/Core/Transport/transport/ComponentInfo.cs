/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System.Diagnostics;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// Connected Component Information, used to identify components from across the connection.
    /// </summary>
    /// <seealso cref="ChannelInfo"/>
    sealed public class ComponentInfo
    {
        /// <summary>
        /// Gets the buffer holding the component version
        /// </summary>
        public Buffer ComponentVersion { get; internal set; } = new Buffer();

        /// <summary>
        /// Creates a clone of <see cref="ComponentInfo"/>
        /// </summary>
        /// <returns>A deep copy <see cref="ComponentInfo"/></returns>
        internal ComponentInfo Clone()
        {
            ComponentInfo cloned = new ComponentInfo();

            cloned.CommentVersion(ComponentVersion);

            return cloned;
        }

        /// <summary>
        /// This method will copy the specified component version into the internal component version
        /// </summary>
        /// <param name="componentVersion">The component version to copy</param>
        /// <returns></returns>
        internal TransportReturnCode CommentVersion(Buffer componentVersion)
        {
            Debug.Assert(componentVersion != null);

            int len = componentVersion.Length;
            ByteBuffer backingBuffer = new ByteBuffer(len);

            TransportReturnCode ret = (TransportReturnCode)componentVersion.Copy(backingBuffer);
            if (ret != TransportReturnCode.SUCCESS)
                return TransportReturnCode.FAILURE;

            ComponentVersion.Data(backingBuffer, 0, len);

            return TransportReturnCode.SUCCESS;
        }
    }
}
