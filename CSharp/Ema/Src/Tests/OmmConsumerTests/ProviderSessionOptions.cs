/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{
    internal class ProviderSessionOptions
    {
        /// <summary>
        ///  Type of connection the session will use.
        /// </summary>
        public ConnectionType ConnectionType { get; set; }

        public int ProtocolType { get; set; }

        /** pingTimeout the consumer and provider will use. */
        public TimeSpan PingTimeout { get; set; } = TimeSpan.FromSeconds(60);

        /** Sets whether a second default directory stream will be setup.
         *
         * If set to true, either the consumer's reactor role must have a preset directory
         * request, or its watchlist is enabled (or both). */
        public bool SetupSecondDefaultDirectoryStream { get; set; }

        /** openWindow the provider will use on its service. By default, value is -1 and no
         * OpenWindow is defined. */
        public long OpenWindow { get; set; } = -1;

        public int NumOfGuaranteedBuffers { get; set; } = 50;

        /* Overrides the default system send buffer size */
        public int SysSendBufSize = 0;

        /* Overrides the default system receive buffer size */
        public int SysRecvBufSize = 0;

        public Eta.Transports.CompressionType CompressionType { get; set; } = Eta.Transports.CompressionType.NONE;

        public bool SendLoginReject { get; set; } = false;

        public bool SendDirectoryResp { get; set; } = true;

        public bool SendMarketDataItemResp { get; set; } = true;

        public bool SendMarketDataItemUpdate { get; set; } = true;

        public bool SendMarketDataItemReject { get; set; } = false;
        public bool SendDictionaryResp { get; set; } = false;
    }
}
