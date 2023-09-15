/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Example.Common;
using LSEG.Eta.Example.VACommon;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using static LSEG.Eta.Example.Common.CommandLine;

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

internal class WatchlistConsumerConfig
{
    // default server host name
    private const string DEFAULT_SRVR_HOSTNAME = "localhost";

    // default server port number
    private const string DEFAULT_SRVR_PORT_NO = "14002";

    // default service name
    private const string DEFAULT_SERVICE_NAME = "DIRECT_FEED";

    // default item name
    private const string DEFAULT_ITEM_NAME = "TRI.N";

    private const int DEFAULT_RUNTIME = 600;

    private const int MAX_ITEMS = 128;
    private const int ITEMS_MIN_STREAM_ID = 5;

    private readonly List<ItemInfo> m_ItemList = new();
    private readonly List<ItemInfo> m_ProvidedItemList = new();

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

        public int Domain { get; set; }
    }

    /// <summary>
    /// Initializes command line arguments.
    /// </summary>
    /// <param name="args">Command line arguments</param>
    /// <returns>If false then parsing problem.</returns>
    public bool Init(string[] args)
    {
        bool ret;

        if ((ret = ParseArgs(args)) == false)
        {
            return ret;
        }

        foreach (var conn in ConnectionList)
        {
            foreach (var itemArg in conn.ItemList)
            {
                if (itemArg.ItemName is not null)
                {
                    AddItem(itemArg.ItemName, (int)itemArg.Domain, itemArg.SymbolListData, itemArg.EnablePrivateStream);
                }
            }
        }
        return true;
    }

    public void AddItem(string itemName, int domainType, bool isSymbolList, bool isPrivate)
    {
        if (ItemList.Count >= MAX_ITEMS)
        {
            Console.WriteLine($"Config Error: Example only supports up to {MAX_ITEMS} items ");
            Environment.Exit(-1);
        }
        ItemInfo itemInfo = new()
        {
            Domain = domainType,
            SymbolListData = isSymbolList,
            Name = itemName,
            IsPrivateStream = isPrivate,
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
        else if (streamId < 0)
        {
            for (int i = 0; i < m_ProvidedItemList.Count; ++i)
            {
                if (m_ProvidedItemList[i].StreamId == streamId)
                    return m_ProvidedItemList[i];
            }
            return null;
        }
        else
            return null;
    }

    internal ItemInfo? AddProvidedItemInfo(int streamId, MsgKey msgKey, int domainType)
    {
        ItemInfo? item = null;

        /* Check if item is already present. */

        item = m_ProvidedItemList.Find(ii => ii.StreamId == streamId);

        /* Add new item. */
        if (item == null)
        {
            if (m_ProvidedItemList.Count == MAX_ITEMS)
            {
                Console.WriteLine("Too many provided items.\n");
                return null;
            }
            item = new ItemInfo();
            m_ProvidedItemList.Add(item);
        }

        if ((msgKey.Flags & MsgKeyFlags.HAS_NAME) > 0)
        {
            item.Name = msgKey.Name.ToString();
        }
        else
        {
            item.Name = null;
        }

        item.StreamId = streamId;
        item.Domain = domainType;

        return item;
    }

    public void RemoveProvidedItemInfo(ItemInfo item)
    {
        int i;
        bool found = false;
        for (i = 0; i < m_ProvidedItemList.Count; ++i)
        {
            if (m_ProvidedItemList[i].StreamId == item.StreamId)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            m_ProvidedItemList.RemoveAt(i);
        }
        return;
    }

    private bool ParseArgs(string[] args)
    {
        for (int i = 0; i < args.Length; i++)
        {
            if (args[i].Contains("runtime"))
            {
                args[i] = args[i].Replace("runtime", "runTime");
            }
            if (args[i].Contains("uname"))
            {
                args[i] = args[i].Replace("uname", "u");
            }
            if (args[i].Contains("connectionType"))
            {
                args[i] = args[i].Replace("connectionType", "c");
            }

        }

        AddCommandLineArgs();
        try
        {
            CommandLine.ParseArgs(args);

            ConnectionArg connectionArg = new();

            string? connectionType = Value("c");
            if (connectionType?.Equals("socket") ?? false)
            {
                connectionArg.ConnectionType = ConnectionType.SOCKET;
            }
            else if (connectionType?.Equals("encrypted") ?? false)
            {
                connectionArg.ConnectionType = ConnectionType.ENCRYPTED;
                EnableEncrypted = true;
            }

            if (HasArg("encryptedProtocolType"))
            {
                string? encryptedProtocolType = Value("encryptedProtocolType");
                if (encryptedProtocolType?.Equals("socket") ?? false)
                {
                    connectionArg.ConnectionType = ConnectionType.SOCKET;
                }
            }
            else
            {
                if (connectionArg.ConnectionType == ConnectionType.ENCRYPTED)
                {
                    throw new NotSupportedException("Use encryptedProtocolType");
                }
            }

            connectionArg.Service = ServiceName!;

            if (Value("h") == null)
            {
                if (EnableSessionManagement == false)
                {
                    connectionArg.Hostname = DEFAULT_SRVR_HOSTNAME;
                }
            }
            else
            {
                connectionArg.Hostname = Value("h") ?? string.Empty;
            }

            if (Value("p") == null)
            {
                if (EnableSessionManagement == false)
                {
                    connectionArg.Port = DEFAULT_SRVR_PORT_NO;
                }
            }
            else
            {
                connectionArg.Port = Value("p") ?? string.Empty;
            }

            connectionArg.InterfaceName = Value("if");

            List<ItemArg> itemList = new();

            List<string>? itemNames = Values("mp");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.MARKET_PRICE, false, false, itemList);

            itemNames = Values("mbo");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.MARKET_BY_ORDER, false, false, itemList);

            itemNames = Values("mbp");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.MARKET_BY_PRICE, false, false, itemList);

            itemNames = Values("yc");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.YIELD_CURVE, false, false, itemList);

            itemNames = Values("sl");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.SYMBOL_LIST, false, false, itemList);

            itemNames = Values("sld");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.SYMBOL_LIST, false, true, itemList);

            if (itemList.Count == 0 && !HasArg("tunnel"))
            {
                ItemArg itemArg = new(DomainType.MARKET_PRICE, DEFAULT_ITEM_NAME, false);
                itemList.Add(itemArg);
            }

            connectionArg.ItemList = itemList;
            ConnectionList.Add(connectionArg);

            string? value = Value("publisherInfo") ?? string.Empty;
            if (value != null)
            {
                string[] pieces = value.Split(",");

                if (pieces.Length > 1)
                {
                    PublisherId = pieces[0];
                    if(!int.TryParse(PublisherId, out var _))
                    {
                        Console.Error.WriteLine("Error loading command line arguments:\t");
                        Console.WriteLine("publisherId within publisherinfo should be an int number");
                        Console.WriteLine("Consumer exits...");
                        Environment.Exit((int)CodecReturnCode.FAILURE);
                    }
                    PublisherAddress = pieces[1];
                }
            }

        }
        catch (Exception exception)
        {
            Console.Error.WriteLine("Error loading command line arguments:\t");
            Console.Error.WriteLine(exception.Message);
            Console.Error.WriteLine();
            Console.Error.WriteLine(OptionHelpString());
            Console.WriteLine("Consumer exits...");
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        return true;
    }

    public string? ServiceName => Value("s");

    public string? UserName => Value("u");

    public string? Password => Value("passwd");

    public bool EnableView => BoolValue("view");

    public bool EnablePost => BoolValue("post");

    public bool EnableOffpost => BoolValue("offpost");

    public bool EnableSnapshot => BoolValue("snapshot");

    public int Runtime => IntValue("runTime");

    public bool EnableXmlTracing => BoolValue("x");

    public bool EnableSessionManagement => BoolValue("sessionMgnt");

    public string? ClientId => Value("clientId");

    public string? ClientSecret => Value("clientSecret");

    public string? TokenUrlV2 => Value("tokenURLV2");

    public string? TokenScope => Value("tokenScope");

    public string? ServiceDiscoveryURL => Value("serviceDiscoveryURL");

    public string? Location => Value("l");

    public bool QueryEndpoint => BoolValue("query");

    public bool EnableProxy => BoolValue("proxy");

    public string? ProxyHostname => Value("ph");

    public string? ProxyPort => Value("pp");

    public string? ProxyUsername => Value("plogin");

    public string? ProxyPassword => Value("ppasswd");

    public string? AuthenticationToken => Value("at");

    public string? AuthenticationExtended => Value("ax");

    public string? ApplicationId => Value("aid");

    public bool EnableRTT => BoolValue("rtt");

    public string? Audience => Value("audience");

    public string? JwkFile => Value("jwkFile");

    public bool EnableRestLogging => BoolValue("restEnableLog");

    public string? RestLogFileName => Value("restLogFileName");

    public List<ConnectionArg> ConnectionList { get; set; } = new();

    /// <summary>
    /// Publisher id.
    /// </summary>
    public string? PublisherId { get; set; }

    /// <summary>
    /// Publisher address.
    /// </summary>
    public string? PublisherAddress { get; set; }

    public bool EnableEncrypted { get; set; }
    public bool CacheOption { get; set; }
    public int CacheInterval { get; set; }

    public int ItemCount => ItemList.Count;

    public IReadOnlyList<ItemInfo> ItemList => m_ItemList;

    public void ParseItems(List<string> itemNames, DomainType domain, bool isPrivateStream, bool isSymbollistData, List<ItemArg> itemList)
    {
        foreach (string itemName in itemNames)
        {
            ItemArg itemArg = new()
            {
                Domain = domain,
                EnablePrivateStream = isPrivateStream,
                SymbolListData = isSymbollistData,
                ItemName = itemName
            };
            itemList.Add(itemArg);
        }
    }

    public void AddCommandLineArgs()
    {
        ProgName("WatchlistConsumer");
        AddOption("mp", "For each occurrence, requests item using Market Price domain.");
        AddOption("mbo", "For each occurrence, requests item using Market By Order domain. Default is no market by order requests.");
        AddOption("mbp", "For each occurrence, requests item using Market By Price domain. Default is no market by price requests.");
        AddOption("yc", "For each occurrence, requests item using Yield Curve domain. Default is no yield curve requests.");
        AddOption("view", "Specifies each request using a basic dynamic view. Default is false.");
        AddOption("post", "Specifies that the application should attempt to send post messages on the first requested Market Price item (i.E., on-stream). Default is false.");
        AddOption("offpost", "Specifies that the application should attempt to send post messages on the login stream (i.E., off-stream).");
        AddOption("publisherInfo", "Specifies that the application should add user provided publisher Id and publisher ipaddress when posting");
        AddOption("snapshot", "Specifies each request using non-streaming. Default is false(i.E. streaming requests.)");
        AddOption("sl", "Requests symbol list using Symbol List domain. (symbol list name optional). Default is no symbol list requests.");
        AddOption("sld", "Requests item on the Symbol List domain and data streams for items on that list.  Default is no symbol list requests.");
        AddOption("h", "Server host name");
        AddOption("p", "Server port number");
        AddOption("if", string.Empty, "Interface name");
        AddOption("s", DEFAULT_SERVICE_NAME, "Service name");
        AddOption("u", "Login user name. Default is system user name.");
        AddOption("passwd", "Password for the user name.");
        AddOption("c", "Socket", "Specifies the connection type that the connection should use. Possible values are: 'socket', 'encrypted'");
        AddOption("encryptedProtocolType", "", "Specifies the encrypted protocol type that the connection should use, if the 'encrypted' type is selected.  Possible values are 'socket'.");

        AddOption("runTime", DEFAULT_RUNTIME, "Program runtime in seconds");
        AddOption("x", "Provides XML tracing of messages.");

        AddOption("proxy", "Specifies that the application will make a connection through an HTTP proxy server, default is false");
        AddOption("ph", "", "Proxy server host name");
        AddOption("pp", "", "Proxy port number");
        AddOption("plogin", "", "User Name on proxy server");
        AddOption("ppasswd", "", "Password on proxy server");

        AddOption("at", "", "Specifies the Authentication Token. If this is present, the login user name type will be Login.UserIdTypes.AUTHN_TOKEN.");
        AddOption("ax", "", "Specifies the Authentication Extended information.");
        AddOption("aid", "", "Specifies the Application ID.");

        AddOption("sessionMgnt", "(optional) Enable Session Management in the reactor.");
        AddOption("l", "(optional) Specifies a location to get an endpoint from service endpoint information. Defaults to us-east-1.");
        AddOption("query", "", "(optional) Queries EDP service discovery to get an endpoint according to a specified connection type and location.");
        AddOption("clientId", "Specifies the client Id for Refinitiv login V2, or specifies a unique ID with login V1 for applications making the request to EDP token service, this is also known as AppKey generated using an AppGenerator.");
        AddOption("clientSecret", "Specifies the associated client Secret with a provided clientId for V2 logins.");
        AddOption("jwkFile", "Specifies the file location containing the JWK encoded private key for V2 logins.");
        AddOption("tokenURLV2", "Specifies the token URL for V2 token oauthclientcreds grant type.");
        AddOption("tokenScope", "", "Specifies the token scope.");
        AddOption("audience", "", "Optionally specifies the audience used with V2 JWT logins");
        AddOption("serviceDiscoveryURL", "Specifies the service discovery URL.");
        AddOption("restEnableLog", false, "(optional) Enable REST logging message");
        AddOption("restLogFileName", "Set REST logging output stream");
        
        AddOption("rtt", false, "(optional) Enable RTT support in the WatchList");
    }
}