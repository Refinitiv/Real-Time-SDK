/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

using LSEG.Eta.Codec;
using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

/// <summary>
/// Contains information associated with each open channel in the Watchlist Consumer.
/// </summary>
internal class ChannelInfo
{
    public ReactorConnectOptions ConnectOptions { get; set; } = new();
    public ReactorConnectInfo ConnectInfo { get; set; } = new();
    public ConsumerRole ConsumerRole { get; set; } = new();
    public PostHandler PostHandler { get; set; } = new();
    public DataDictionary Dictionary { get; set; } = new();
    public int FieldDictionaryStreamId { get; set; }
    public int EnumDictionaryStreamId { get; set; }
    public bool ShouldOffStreamPost { get; set; }
    public bool ShouldOnStreamPost { get; set; }
    public bool ShouldEnableEncrypted { get; set; }
    public EncryptionProtocolFlags EncryptionProtocol { get; set; }
    public Buffer PostItemName { get; set; } = new();
    public DecodeIterator DIter { get; set; } = new();
    public Msg ResponseMsg { get; set; } = new();
    public LoginRefresh LoginRefresh { get; set; } = new();
    public bool HasServiceInfo { get; set; }
    public Service ServiceInfo { get; set; } = new();
    public ReactorChannel? ReactorChannel { get; set; }

    /// <summary>
    /// Represented by epoch time in milliseconds.
    /// </summary>
    public System.DateTime LoginReissueTime { get; set; }

    public ConnectionArg ConnectionArg { get; set; } = new();
    public bool CanSendLoginReissue { get; set; }

    public ChannelInfo()
    {
        ConnectOptions.ConnectionList.Add(ConnectInfo);
    }
}