/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    /// <summary>
    /// Item request information.
    /// </summary>
    public class ItemRequestInfo
    {
        /// <summary>
        /// Gets or sets stream ID
        /// </summary>
        public int StreamId { get; set; }

        /// <summary>
        /// Gets itema name
        /// </summary>
        public Buffer ItemName { get; private set; } = new Buffer();

        /// <summary>
        /// Gets or sets item info
        /// </summary>
        public ItemInfo? ItemInfo { get; set; }

        /// <summary>
        /// Gets or sets whether if is streaming request
        /// </summary>
        public bool IsStreamingRequest { get; set; }

        /// <summary>
        /// Gets or sets whether if is private stream request
        /// </summary>
        public bool IsPrivateStreamRequest { get; set; }

        /// <summary>
        /// Gets or sets whether include key in updates
        /// </summary>
        public bool IncludeKeyInUpdates { get; set; }

        /// <summary>
        /// Gets message key
        /// </summary>
        public IMsgKey MsgKey { get; set; } = new MsgKey();

        /// <summary>
        /// Gets or sets <c>Channel</c>
        /// </summary>
        public IChannel? Channel { get; set; }

        /// <summary>
        /// Gets or sets <c>ReactorChannel</c>
        /// </summary>
        public ReactorChannel? ReactorChannel { get; set; }

        /// <summary>
        /// Gets or sets if is in use.
        /// </summary>
        public bool IsInUse { get; set; }

        /// <summary>
        /// Gets or sets domain type
        /// </summary>
        public int DomainType { get; set; }

        /// <summary>
        /// Instantiates a new item request info.
        /// </summary>
        public ItemRequestInfo()
        {
            StreamId = 0;
            ItemInfo = null;
            IsStreamingRequest = false;
            IsPrivateStreamRequest = false;
            IncludeKeyInUpdates = false;
            Channel = null;
            ReactorChannel = null;
            IsInUse = false;
            DomainType = 0;
        }

        /// <summary>
        /// Clears to default values
        /// </summary>
        public void Clear()
        {
            StreamId = 0;
            ItemName.Clear();
            ItemInfo = null;
            IsStreamingRequest = false;
            IsPrivateStreamRequest = false;
            IncludeKeyInUpdates = false;
            MsgKey.Clear();
            Channel = null;
            ReactorChannel = null;
            IsInUse = false;
            DomainType = 0;
        }
    }
}
