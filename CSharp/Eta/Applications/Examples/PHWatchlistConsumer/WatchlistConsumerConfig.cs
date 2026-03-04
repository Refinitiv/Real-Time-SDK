/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using Microsoft.Extensions.Primitives;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

[ProgName("PHWatchlistConsumer")]
public partial class WatchlistConsumerConfig : CommandLine
{
    private const string DEFAULT_SERVICE_NAME = "DIRECT_FEED";

    private const string DEFAULT_ITEM_NAME = "TRI.N";

    private const string DEFAULT_CONFIG_FILE = "Config.json";

    private const int DEFAULT_RUNTIME = 600;

    private const int MAX_ITEMS = 128;

    private const int ITEMS_MIN_STREAM_ID = 5;

    private readonly List<ItemInfo> m_ItemList = new();

    private readonly ReactorConnectOptions m_ReactorConnectOpts = new();

    private readonly List<string> m_Errors = new();

    public IReadOnlyList<string> Errors => m_Errors;

    /// <summary>
    /// Watchlist consumer config item info.
    /// </summary>
    public class ItemInfo
    {
        public State State { get; set; } = new();

        public bool IsBatchStream { get; set; }

        public bool IsPrivateStream { get; set; }

        public bool SymbolListData { get; set; }

        public int StreamId { get; set; }

        public string? Name { get; set; }

        public string? ServiceName { get; set; }

        public DomainType Domain { get; set; }
    }

    public void AddItem(string itemName, string serviceName, DomainType domainType)
    {
        if (ItemList.Count >= MAX_ITEMS)
        {
            m_Errors.Add($"Config Error: Example only supports up to {MAX_ITEMS} items ");
            return;
        }
        ItemInfo itemInfo = new()
        {
            Domain = domainType,
            Name = itemName,
            ServiceName = serviceName,
            StreamId = ITEMS_MIN_STREAM_ID + ItemList.Count
        };
        m_ItemList.Add(itemInfo);
    }

    public ItemInfo? GetItemInfo(int streamId)
    {
        if (streamId > 0)
        {
            if (streamId >= ITEMS_MIN_STREAM_ID && streamId < ItemList.Count + ITEMS_MIN_STREAM_ID)
                return ItemList[streamId - ITEMS_MIN_STREAM_ID];
            else
                return null;
        }
        else
        {
            return null;
        }
    }

    public WatchlistConsumerConfig() => Init();

    private void Init()
    {
        m_ReactorConnectOpts.Clear();
        try
        {
            AddCommandLineArgs();
            ParseArgs();
            if (MarketPriceItemNames?.Count > 0)
                ParseItems(MarketPriceItemNames, DomainType.MARKET_PRICE);

            if (MarketByOrderItemNames?.Count > 0)
                ParseItems(MarketByOrderItemNames, DomainType.MARKET_BY_ORDER);

            if (MarketByPriceItemNames?.Count > 0)
                ParseItems(MarketByPriceItemNames, DomainType.MARKET_BY_PRICE);

            if (m_ItemList.Count == 0)
            {
                AddItem(DEFAULT_ITEM_NAME, ServiceName!, DomainType.MARKET_PRICE);
            }
        }
        catch (Exception exception)
        {
            m_Errors.Add("Error loading command line arguments:\t");
            m_Errors.Add(exception.Message);
        }

        try
        {
            if (ConfigJson is not null)
            {
                string json = File.ReadAllText(ConfigJson!);
                var jsonSerializerSettings = new JsonSerializerSettings
                {
                    ContractResolver = new CamelCasePropertyNamesContractResolver()
                };
                Config = JsonConvert.DeserializeObject<Json>(json, jsonSerializerSettings);
            }
        }
        catch (Exception exception)
        {
            m_Errors.Add($"Error loading {nameof(ConfigJson)}:");
            m_Errors.Add(exception.Message);
        }
    }

    [Option("s", DEFAULT_SERVICE_NAME, "Default service name for requests. This will be used for dictionary requests as well as any items that do not have a default service specified in -mp, -mbo, or -mbp.")]
    public string? ServiceName { get; private set; }

    [Option("u", "Login user name. Default is system user name.", Aliases = new string[] { "uname" })]
    public string? UserName { get; private set; }

    [Option("passwd", "Password for the user name.")]
    public string? Password { get; private set; }

    [Option("runTime", DEFAULT_RUNTIME, "Program runtime in seconds", Aliases = new string[] { "runtime" })]
    public int Runtime { get; private set; }

    [Option("x", "Provides XML tracing of messages.")]
    public bool EnableXmlTracing { get; private set; }

    [Option("clientId", "Specifies the client Id for Refinitiv login V2, or specifies a unique ID with login V1 for applications making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.")]
    public string? ClientId { get; private set; }

    [Option("clientSecret", "Specifies the associated client Secret with a provided clientId for V2 logins.")]
    public string? ClientSecret { get; private set; }

    [Option("tokenURLV2", "Specifies the token URL for V2 token oauthclientcreds grant type.")]
    public string? TokenUrlV2 { get; private set; }

    [Option("tokenScope", "", "Specifies the token scope.")]
    public string? TokenScope { get; private set; }

    [Option("serviceDiscoveryURL", "Specifies the service discovery URL.")]
    public string? ServiceDiscoveryURL { get; private set; }

    [Option("mp", "For each occurrence, requests item using Market Price domain.")]
    public List<string>? MarketPriceItemNames { get; private set; }

    [Option("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.")]
    public List<string>? MarketByOrderItemNames { get; private set; }

    [Option("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.")]
    public List<string>? MarketByPriceItemNames { get; private set; }

    [Option("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.")]
    public string? AuthenticationToken { get; private set; }

    [Option("ax", "", "Specifies the Authentication Extended information.")]
    public string? AuthenticationExtended { get; private set; }

    [Option("aid", "", "Specifies the Application ID.")]
    public string? ApplicationId { get; private set; }

    [Option("audience", "", "Optionally specifies the audience used with V2 JWT logins")]
    public string? Audience { get; private set; }

    [Option("jwkFile", "Specifies the file location containing the JWK encoded private key for V2 logins.")]
    public string? JwkFile { get; private set; }

    [Option("configJson", DEFAULT_CONFIG_FILE, "Json config file path", Aliases = new string[] { "config" })]
    public string? ConfigJson { get; private set; }

    [Option("restProxyHost", "Proxy server host name for REST requests")]
    public string? RestProxyHost { get; private set; }

    [Option("restProxyPort", "Proxy port number for REST requests")]
    public string? RestProxyPort { get; private set; }

    [Option("restProxyUsername", "User Name on proxy server for REST requests")]
    public string? RestProxyUsername { get; private set; }

    [Option("restProxyPasswd", "Password on proxy server for REST requests")]
    public string? RestProxyPassword { get; private set; }

    [Option("reconnectAttemptLimit", -1, "Specifies the maximum number of reconnection attempts", Aliases = new[] { "reconnectCount", "reconnectLimit" })]
    public int ReconnectAttempLimit { get; private set; }

    [Option("reconnectMinDelay", 500, "Specifies minimum delay (in milliseconds) between reconnection attempts")]
    public int ReconnectMinDelay { get; private set; }

    [Option("reconnectMaxDelay", 6000, "Specifies maximum delay (in milliseconds) between reconnection attempts")]
    public int ReconnectMaxDelay { get; private set; }

    [Option("ioctlInterval", 0, "Specifies time interval (in second) before IOCtl function is invoked. O indicates that function won't be invoked. Default is 0")]
    public int IoctlInterval { get; private set; }

    [Option("fallBackInterval", 0, "Specifies time interval (in second) in application before Ad Hoc Fallback function is invoked. O indicates that function won't be invoked. Default is 0", Aliases = new[] { "fallbackInterval" })]
    public int FallBackInterval { get; private set; }

    [Option("ioctlEnablePH", "Enables Preferred host feature. Possible values are true/false")]
    public bool? IoctlEnablePH { get; private set; }

    [Option("ioctlConnectListIndex", "Specifies the preferred host as the index in the connection list")]
    public int? IoctlConnectListIndex { get; private set; }

    [Option("ioctlDetectionTimeInterval", "Specifies time interval (in second) to switch over to a preferred host. 0 indicates that the detection time interval is disabled")]
    public int? IoctlDetectionTimeInterval { get; private set; }

    [Option("ioctlDetectionTimeSchedule", "Specifies Cron time format to switch over to a preferred host")]
    public string? IoctlDetectionTimeSchedule { get; private set; }

    [Option("restEnableLog", false, "Enables REST logging message")]
    public bool EnableRestLogging { get; private set; }

    [Option("restLogFileName", "Specifies REST logging file stream name")]
    public string? RestLogFileName { get; private set; }

    internal Json? Config { get; private set; }

    public int ItemCount => ItemList.Count;

    public IReadOnlyList<ItemInfo> ItemList => m_ItemList;

    public bool IoctlOverridesConfigPreferredHost =>
        (IoctlEnablePH is not null) ||
        (IoctlConnectListIndex is not null && IoctlConnectListIndex != Config?.PreferredHost?.ConnectListIndex) ||
        (IoctlDetectionTimeInterval is not null && IoctlDetectionTimeInterval != Config?.PreferredHost?.DetectionTimeInterval) ||
        (IoctlDetectionTimeSchedule is not null && IoctlDetectionTimeSchedule != Config?.PreferredHost?.DetectionTimeSchedule);

    private void ParseItems(List<string> itemNames, DomainType domain)
    {
        foreach (string itemName in itemNames)
        {
            StringTokenizer tokenizer = new(itemName, new[] { ':' });
            string name;
            string serviceName;
            var segments = tokenizer.ToArray();
            if (segments.Length == 1 && segments[0].HasValue)
            {
                AddItem(segments[0].Value!, ServiceName!, domain);
            }
            else if (segments.Length == 2 && segments[0].HasValue && segments[1].HasValue)
            {
                serviceName = segments[0].Value!;
                name = segments[1].Value!;
                AddItem(name, serviceName, domain);
            }
            else
            {
                m_Errors.Add($"Invalid item name {itemName}.  This needs to be either <item_name> or <service_name>:<item_name");
            }
        }
    }
}
