/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using LSEG.Eta.Codec;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class ItemRequestInfo
    {
        public int StreamId { get; set; }

        /// <summary>
        /// Gets itema name
        /// </summary>
        public Buffer ItemName { get; private set; } = new Buffer();

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
            IsStreamingRequest = false;
            IsPrivateStreamRequest = false;
            IncludeKeyInUpdates = false;
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
            IsStreamingRequest = false;
            IsPrivateStreamRequest = false;
            IncludeKeyInUpdates = false;
            MsgKey.Clear();
            ReactorChannel = null;
            IsInUse = false;
            DomainType = 0;
        }
    }
}
