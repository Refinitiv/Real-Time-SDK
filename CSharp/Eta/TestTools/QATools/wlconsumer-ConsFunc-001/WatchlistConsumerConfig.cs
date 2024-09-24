/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;

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

    // APIQA:
    // eventCounters keeps track of all information from any -e1/e2/e3 etc arguments
    private List<EventCounter> m_EventCounters = new();
    private const int DEFAULT_SINGLE_OPEN = 1;
    private const int DEFAULT_ALLOW_SUSPECT = 1;
    // breakpoint is an indicator of the split in the list of -mp items in which
    // those before the breakpoint are all requested initially,
    // and those after breakpoint are requested after some number of events has
    // occurred (which should be specified by the -e1, -e2, or -e3 arguments)
    private int m_Breakpoint;
    // maxInitialStreamId is the stream Id of the last item reuqested intially
    private int m_MaxInitialStreamId;

    // APIQA: allMPItems is a list of all mp item arguments, whereas itemList
    // only contains those that are requested intially, and as new items are
    // requested via requestNewItem the item information from allMPItems is used
    // to make a new ItemInfo object which is then added to itemList
    private readonly List<ItemArg> m_AllMPItems = new();

    // APIQA: EventCounter class created to keep track of -e1/e2/e3 etc
    // arguments; a EventCounter object is created for each -e1/e2/e3 command
    // line argument
    public class EventCounter
    {
        public EventCounter(int eventType, int delay, int startIdx, int endIdx, int reissueType, int reissueViewId)
        {
            EventType = eventType;
            Delay = delay;
            StartIdx = startIdx;
            EndIdx = endIdx;
            ReissueType = reissueType;
            ReissueViewId = reissueViewId;
        }

        public int EventType { get; set; }
        public int StartIdx { get; set; }
        public int ReissueViewId { get; set; }

        /// <summary>
        /// ReissueType and reissueViewId are applicable only to reissue even "-e4", the reissue types are:
        /// 1 View
        ///  2 Pause
        ///  3 Resume
        /// (For example the command line argument: -e4 6::1:1,V3 
        /// the reissue type is View internally stored as 1, and the reissueViewId is 3.)
        /// For event types other than -e4, the value is 0 for reissueType.
        /// </summary>
        public int ReissueType { get; set; }
        public int EndIdx { get; set; }

        /// <summary>
        /// delay is how many instances of the specific event are to pass until
        /// specified items are requested
        /// </summary>
        public int Delay { get; set; }

        public string PrintEventCounter() =>
            "eventtype " + EventType + "  dalay " + Delay + "   sIdx " + StartIdx + "    eIdx " + EndIdx + "    reissueType " + ReissueType;
    }

    // END APIQA

    /// <summary>
    /// Watchlist consumer config item info.
    /// </summary>
    public class ItemInfo
    {
        public int Domain { get; set; }
        public string? Name { get; set; }
        public int StreamId { get; set; }
        public State State { get; set; } = new State();
        public bool SymbolListData { get; set; }
        public bool IsPrivateStream { get; set; }
        public bool IsBatchStream { get; set; }
        public bool IsView { get; set; }
        public bool IsSnapshot { get; set; }
        public bool IsMsgKeyInUpdates { get; set; }
        public int ViewId { get; set; }
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
                    // APIQA: interface of addItem changed
                    AddItem(itemArg.ItemName, (int)itemArg.Domain, itemArg.SymbolListData, itemArg.EnablePrivateStream, itemArg.EnableView, itemArg.ViewId, itemArg.EnableSnapshot, itemArg.EnableMsgKeyInUpdates);
                }
            }
        }
        return true;
    }

    //AddEventCounter parses -e1/e2/e3 argument eg. -e2 5::2:6
    //the '5' in the above example is the amount of occurance of event 1 you want to happen until requesting specified items
    //'2' is the index of the first mp item in list of mp items to request once 5 event1's have occured
    //'6' is the index of the last mp item in list of mp items to request once 5 event 1's have occured
    //doing -e1 7::2 (not specifying an end index) will result in only item indexed 2 to be requested, not a range
    // NOTE: On -e4 event 4, Please read the following on how the event 4 is used:
    // Example arguments: -h 48.22.4.1 -p 14002 -s ELEKTRON_DD  -mp GOOG.O -mp TRI.N  -e2 5::1 -e4 10::1:1,P -x
    //  Notes: Make sure that -e2 event is prior to using -e4 because the way the counter works is unless an -e2
    //         event opens up a new item the re-issue will not work.
    // In the above example "-e4 10::1:1,P" would mean after -e2 opened the item "TRI.N" the "-e4 10::1:1,P" means:
    //      -e4 -Reissue event
    //      10::1:1,P send reissue after 10 updates on item indexed 1 which is 'TRI.N' in the above example. (because end index is also 1 '1:1'.
    //      ',' after comma the reissue action V3 -View change V3, P pause, R resume. In the above example reissue action is P i.E pause
    private void AddEventCounter(string varName, string value, int numMPItems)
    {
        int eType = 9;
        string eTypeString = varName.Split("e")[1];

        static void HandleInvalidInput()
        {
            Console.WriteLine("Error loading command line arguments:\t");
            Console.WriteLine("Invalid event input");
            Environment.Exit((int)CodecReturnCode.FAILURE);
        }

        if (!int.TryParse(eTypeString, out eType))
        {
            HandleInvalidInput();
        }

        int delayNum = 0;
        string[] partsOfValue = value.Split("::");

        if (!int.TryParse(partsOfValue[0], out delayNum))
        {
            HandleInvalidInput();
        }

        int sIdx = 0; int eIdx = numMPItems;
        string reissueTypeStr = "";
        int reissueType = 0;
        int reissueViewId = 0;

        if (partsOfValue[1].Contains(":"))
        {
            try
            {
                sIdx = int.Parse(partsOfValue[1].Split(":")[0]);
            }
            catch
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.WriteLine("Invalid startIdx input");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }



            if (!int.TryParse(partsOfValue[0], out delayNum))
            {
                HandleInvalidInput();
            }

            try
            {
                if (partsOfValue[1].Contains(",") == false)
                    eIdx = int.Parse(partsOfValue[1].Split(":")[1]) + 1;
                else
                {
                    eIdx = int.Parse(partsOfValue[1].Split(":")[1].Split(",")[0]);
                }
            }
            catch (Exception)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.WriteLine("Invalid endIdx input");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
            if (eIdx > numMPItems)
            {
                Console.WriteLine("Invalid end index to event range entered. Default will set to last item.");
                eIdx = numMPItems;
            }
        }
        else
        {
            try
            {
                sIdx = int.Parse(partsOfValue[1]);
            }
            catch (Exception)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.WriteLine("Invalid startIdx input");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
            eIdx = sIdx + 1;
        }
        if (partsOfValue[1].Contains(","))
        {
            if (sIdx == eIdx)
                eIdx = sIdx + 1;
            try
            {
                reissueTypeStr = partsOfValue[1].Split(",")[1];
                if (reissueTypeStr.Contains("V"))
                {
                    reissueType = 1; // View
                    reissueViewId = int.Parse(reissueTypeStr.Split("V")[1]);
                }
                else if (reissueTypeStr.Contains("P"))
                {
                    reissueType = 2; // Pause
                }
                else if (reissueTypeStr.Contains("R"))
                {
                    reissueType = 3; // Resume
                }
            }
            catch (Exception)
            {
                Console.Error.WriteLine("Error loading command line arguments:\t");
                Console.WriteLine("Invalid startIdx input");
                Environment.Exit((int)CodecReturnCode.FAILURE);
            }
        }



        EventCounter e = new(eType, delayNum, sIdx, eIdx, reissueType, reissueViewId);
        m_EventCounters.Add(e);
    }

    public List<EventCounter> GetEventCounters(int eventType)
    {
        List<EventCounter> eCounters = new();
        foreach (EventCounter eCounter in m_EventCounters)
        {
            if (eCounter.EventType == eventType)
            {
                eCounters.Add(eCounter);
            }
        }
        return eCounters;
    }
    public List<EventCounter> GetEventCounters()
    {
        return m_EventCounters;
    }


    public int GetViewId(int i)
    {
        return m_AllMPItems[i].ViewId;
    }

    public string? GetNameOfItem(int i)
    {
        return m_AllMPItems[i].ItemName;
    }

    // APIQA: interface of addItem changed
    public void AddItem(string itemName, int domainType, bool isSymbolList, bool isPrivate, bool isView, int viewId, bool isSnapshot, bool isMsgKeyInUpdates)
    {
        if (ItemList.Count >= MAX_ITEMS)
        {
            Console.WriteLine($"Config Error: Example only supports up to {MAX_ITEMS} items ");
            Environment.Exit(-1);
        }
        ItemInfo itemInfo = new()
        {
            Domain = domainType,
            IsMsgKeyInUpdates = isMsgKeyInUpdates,
            IsPrivateStream = isPrivate,
            IsSnapshot = isSnapshot,
            IsView = isView,
            ViewId = viewId,
            Name = itemName,
            StreamId = ITEMS_MIN_STREAM_ID + m_ItemList.Count,
            SymbolListData = isSymbolList
        };
        itemInfo.Name = itemName;
        itemInfo.SymbolListData = isSymbolList;
        itemInfo.ViewId = viewId;
        // END APIQA
        itemInfo.StreamId = ITEMS_MIN_STREAM_ID + m_ItemList.Count;
        Console.WriteLine("------------APIQA: " + itemInfo.Name + ": StreamID = " + itemInfo.StreamId + ", PrivateFlag = " + itemInfo.IsPrivateStream + ", SnapShotFlag= "
                + itemInfo.IsSnapshot + ", ViewFlag= " + itemInfo.IsView + ", ViewId= " + itemInfo.ViewId + ", MsgKeyInUpdatesFlag= " + itemInfo.IsMsgKeyInUpdates);

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
            if (itemNames != null)
            {
                int numMPItems = itemNames.Count;

                // APIQA:an EventCounter object is made to represent every
                // 'event' (e1, e2, e3, e4, e5) argument
                itemNames = Values("e1");
                if (itemNames != null && itemNames.Count > 0)
                {
                    foreach (var item in itemNames)
                    {
                        AddEventCounter("e1", item, numMPItems);
                    }
                }

                itemNames = Values("e2");
                if (itemNames != null && itemNames.Count > 0)
                {
                    foreach (var item in itemNames!)
                    {
                        AddEventCounter("e2", item, numMPItems);
                    }
                }

                itemNames = Values("e3");
                if (itemNames != null && itemNames.Count > 0)
                {
                    foreach (var item in itemNames!)
                    {
                        AddEventCounter("e3", item, numMPItems);
                    }
                }
                itemNames = Values("e4");
                if (itemNames != null && itemNames.Count > 0)
                {
                    foreach (var item in itemNames!)
                    {
                        AddEventCounter("e4", item, numMPItems);
                    }
                }
                itemNames = Values("e5");
                if (itemNames != null && itemNames.Count > 0)
                {
                    foreach (var item in itemNames!)
                    {
                        AddEventCounter("e5", item, numMPItems);
                    }
                }
                itemNames = Values("mp");

                // breakpoint is an index in the list of mp arguments sepcified
                // all items before the breakpoint will be requested intially,
                // and ones after are assumed to be requested after some numebr
                // of events specified by an -e1/e2/e3 argument
                int bPoint = DetermineBreakpoint(itemNames!);

                /*APIQA: all market price items are added to itemNames
                *handleMarketPriceArgs handles adding the correct items (those that will be requested intitially) to itemList
                *and puts all mp items in a different List called allMPItems, which is indexed in the same order that the 
                *items appeared in command line arguments
                */
                HandleMarketPriceArgs(itemNames!, bPoint, itemList);
            }

            // END APIQA

            itemNames = Values("mbo");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.MARKET_BY_ORDER, false, false, false, false, 0, false, itemList);

            itemNames = Values("mbp");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.MARKET_BY_PRICE, false, false, false, false, 0, false, itemList);

            itemNames = Values("yc");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.YIELD_CURVE, false, false, false, false, 0, false, itemList);

            itemNames = Values("sl");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.SYMBOL_LIST, false, false, false, false, 0, false, itemList);

            itemNames = Values("sld");
            if (itemNames != null && itemNames.Count > 0)
                ParseItems(itemNames, DomainType.SYMBOL_LIST, false, true, false, false, 0, false, itemList);

            if (itemList.Count == 0 && !HasArg("tunnel"))
            {
                ItemArg itemArg = new(DomainType.MARKET_PRICE, DEFAULT_ITEM_NAME, false);
                itemList.Add(itemArg);
            }

            connectionArg.ItemList = itemList;
            ConnectionList.Add(connectionArg);

            m_MaxInitialStreamId = ITEMS_MIN_STREAM_ID + itemList.Count;

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

    // APIQA

    public bool EnablePostMultipart => BoolValue("postmultipart");

    public bool TestOnlyLoginClose => BoolValue("testOnlyLoginClose");
    public bool ReqItemBeforeLogin => BoolValue("reqItemBeforeLogin");
    public bool LoginCloseAfterLoginStatus => BoolValue("loginCloseAfterLoginStatus");
    public bool LoginPauseAndResume => BoolValue("loginPauseAndResume");
    public bool ReissueDirEvery5Updates => BoolValue("reissueDirEvery5Updates");
    public int ReissueDirWithSID => IntValue("reissueDirWithSID");
    public int ReqDirWithSID => IntValue("reqDirWithSID");
    public bool HasReqQos => BoolValue("hasReqQos");
    public bool QosDynamic => BoolValue("qosDynamic");
    public int QosRate => IntValue("qosRate");
    public int QosRateInfo => IntValue("qosRateInfo");
    public int QosTimeliness => IntValue("qosTimeliness");
    public int QosTimeInfo => IntValue("qosTimeInfo");
    public int WorstQosRate => IntValue("worstQosRate");
    public int WorstQosRateInfo => IntValue("worstQosRateInfo");
    public int WorstQosTimeliness => IntValue("worstQosTimeliness");
    public int WorstQosTimeInfo => IntValue("worstQosTimeInfo");


    public bool EnableSnapshot => BoolValue("snapshot");

    public int Runtime => IntValue("runTime");

    public bool EnableXmlTracing => BoolValue("x");

    public bool EnableSessionManagement => BoolValue("sessionMgnt");

    public string? ClientId => Value("clientId");

    public string? ClientSecret => Value("clientSecret");

    public string? TokenUrlV2 => Value("tokenURL");

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

    public int SingleOpen => IntValue("singleOpen");

    public int AllowSuspectData => IntValue("allowSuspect");

    public List<ConnectionArg> ConnectionList { get; set; } = new();

    // APIQA
    public bool IsElementView => HasArg("eView");

    // APIQA
    /// <summary>
    /// DelayIntialRequest, if specified, will delay all initial requests until after Src Dir Refresh
    /// </summary>
    public bool DelayInitialRequest => HasArg("delayInitialRequest");

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

    public List<ItemInfo> ItemList => m_ItemList;

    /// <summary>
    /// APIQA: Added more parameters
    /// </summary>
    /// <param name="itemNames"></param>
    /// <param name="domain"></param>
    /// <param name="isPrivateStream"></param>
    /// <param name="isSymbollistData"></param>
    /// <param name="isView"></param>
    /// <param name="isSnapshot"></param>
    /// <param name="viewId"></param>
    /// <param name="isMsgKeyInUpdates"></param>
    /// <param name="itemList"></param>
    private void ParseItems(List<string> itemNames, DomainType domain, bool isPrivateStream, bool isSymbollistData, bool isView, bool isSnapshot, int viewId, bool isMsgKeyInUpdates, List<ItemArg> itemList)
    {
        foreach (string itemName in itemNames)
        {
            ItemArg itemArg = new(domain, itemName, isPrivateStream);
            // APIQA:
            itemArg.EnablePrivateStream = isPrivateStream;
            itemArg.EnableSnapshot = isSnapshot;
            itemArg.EnableView = isView;
            itemArg.ViewId = viewId;
            itemArg.EnableMsgKeyInUpdates = isMsgKeyInUpdates;
            // END APIQA
            itemArg.SymbolListData = isSymbollistData;
            itemList.Add(itemArg);
        }
    }



    /*APIQA:Looks through all 'event' (e1, e2, e3) arguments and determines the breakpoint.
	*Brekapoint represent the the first index of market price items that should not be initially requested
	*ASSUMES that all market price items after that one are going to be requested after a delay as well
	*Therefore, all market price items that are to be requested intitally have to be listed before all delayed items
	*/
    private int DetermineBreakpoint(List<string> itemNames)
    {
        int bpoint = itemNames.Count;
        if (m_EventCounters.Count > 0)
        {
            for (int eventCounterIndex = 0; eventCounterIndex < m_EventCounters.Count; ++eventCounterIndex)
            {
                if (m_EventCounters[eventCounterIndex].StartIdx < bpoint)
                {
                    bpoint = m_EventCounters[eventCounterIndex].StartIdx;
                }
            }
        }
        m_Breakpoint = bpoint;
        return bpoint;
    }

    //APIQA:returns last streamId used initially
    public int LastInitialStreamId => m_MaxInitialStreamId;

    public bool IsSnapshot => BoolValue("snapshot");

    //APIQA:returns requested attribute about the ItemArg at index i in allMPItems
    public bool GetMPItemInfo(int i, string attrib)
    {
        if (i >= m_AllMPItems.Count)
        {
            Console.WriteLine("Invalid index input for getMPItem, default returning false");
            return false;
        }
        if (attrib.Equals("isSnapshot"))
        {
            return m_AllMPItems[i].EnableSnapshot;
        }
        else if (attrib.Equals("isView"))
        {
            return m_AllMPItems[i].EnableView;
        }
        else if (attrib.Equals("isPrivate"))
        {
            return m_AllMPItems[i].EnablePrivateStream;
        }
        if (attrib.Equals("isMsgKeyInUpdates"))
        {
            return m_AllMPItems[i].EnableMsgKeyInUpdates;
        }
        Console.WriteLine("Invalid attribute requested. Default returning False");
        return false;
    }

    //APIQA:itemNames contains all the values of -mp inputs (example  TRI:PSV2 which is requesting a private stream snapshot view 2 for TRI)
    //all the items get added to allMPItems but only those that will be requested initially (AKA the items are are before the 'breakpoint') will be added to itemList
    //the items are aren't added to itemList will be added when they are first requested via the requestNewItem method in WatchlistConsumer.java
    void HandleMarketPriceArgs(List<string> itemNames, int breakpoint, List<ItemArg> list)
    {
        for (int index = 0; index < itemNames.Count; ++index)
        {
            string itemName = itemNames[index];
            bool isPrivateStream = false;
            bool isView = false;
            bool isSnapshot = false;
            bool isMsgKeyInUpdates = false;
            int viewId = 0;
            string name = itemName;
            //APIQA: parse for S, P, V, K
            if (itemName.Contains(":"))
            {
                string[] splitName = itemName.Split(":");
                name = splitName[0];
                if (splitName[1].Contains("P"))
                    isPrivateStream = true;
                if (splitName[1].Contains("S"))
                    isSnapshot = true;
                if (splitName[1].Contains("K"))
                    isMsgKeyInUpdates = true;
                if (splitName[1].Contains("V"))
                {
                    isView = true;
                    try
                    {
                        viewId = int.Parse(splitName[1].Split("V")[1]);
                    }
                    catch
                    {
                        Console.Error.WriteLine("Error loading command line arguments:\t");
                        Console.WriteLine("Invalid view argument");
                        Environment.Exit((int)CodecReturnCode.FAILURE);
                    }
                }
            }
            ItemArg itemArg = new(DomainType.MARKET_PRICE, name, isPrivateStream);
            itemArg.EnableSnapshot = isSnapshot;
            itemArg.EnableMsgKeyInUpdates = isMsgKeyInUpdates;
            itemArg.EnableView = isView;
            itemArg.ViewId = viewId;
            m_AllMPItems.Add(itemArg);
            if (index < breakpoint)
            {
                list.Add(itemArg);
            }
        }
    }

    //END APIQA

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
        AddOption("query", "", "(optional) Queries Delivery Platform service discovery to get an endpoint according to a specified connection type and location.");
        AddOption("clientId", "Specifies the client Id for V2 authentication OR specifies a unique ID, also known as AppKey generated by an AppGenerator, for V1 authentication usedwhen connecting to Real-Time Optimized.");
        AddOption("clientSecret", "Specifies the associated client Secret with a provided clientId for V2 logins.");
        AddOption("jwkFile", "Specifies the file location containing the JWK encoded private key for V2 logins.");
        AddOption("tokenURL", "Specifies the token URL for V2 token oauthclientcreds grant type.");
        AddOption("tokenScope", "", "Specifies the token scope.");
        AddOption("audience", "", "Optionally specifies the audience used with V2 JWT logins");
        AddOption("serviceDiscoveryURL", "Specifies the service discovery URL.");
        AddOption("restEnableLog", false, "(optional) Enable REST logging message");
        AddOption("restLogFileName", "Set REST logging output stream");
        
        AddOption("rtt", false, "(optional) Enable RTT support in the WatchList");

        //APIQA
        AddOption("postmultipart", "Specifies that the application should attempt to send post multi part messages, default is false.");

        AddOption("singleOpen", DEFAULT_SINGLE_OPEN, "SingleOpen default set to 1, can be specified to 0 or 1");
        AddOption("allowSuspect", DEFAULT_ALLOW_SUSPECT, "Allow suspect data default set to 1, can be set to 0 or 1");
        AddOption("testOnlyLoginClose", "Only close login stream when runtime expires");
        AddOption("reqItemBeforeLogin", "Test request item befor login stream establishes");
        AddOption("loginCloseAfterLoginStatus", "Send login close message from rdmLoginMsgCallback after getting a login status message");
        AddOption("loginPauseAndResume", "Send login pause after 3 item updates and login resume after 3 src dir updates");
        AddOption("reissueDirEvery5Updates", "Send dorectory reissue every 5 updates received upto 30 updates.");
        AddOption("reissueDirWithSID", "Send dorectory reissue with specify serviceId.");
        AddOption("reqDirWithSID", "Send Directory request at the initial with specify serviceId.");
        AddOption("hasReqQos", "Enable hasQos on the request.");
        AddOption("qosDynamic", "dynamic or static qos.");
        AddOption("qosRate", "qosRate.");
        AddOption("qosTimeliness", "qosTimeliness.");
        AddOption("qosTimeInfo", "qosTimeInfo.");
        AddOption("qosRateInfo", "qosRateInfo.");
        AddOption("worstQosRate", "worstQosRate.");
        AddOption("worstQosTimeliness", "worstQosTimeliness.");
        AddOption("worstQosTimeInfo", "worstQosTimeInfo.");
        AddOption("worstQosRateInfo", "worstQosRateInfo.");

        AddOption("e1", "Request new view item(s) after specified number of Src Directory UPDATES");
        AddOption("e2", "Request new view item(s) after specified number of defaultMsg UPDATES");
        AddOption("e3", "Request new view item(s) after specified number of Channel_down_reconnecting events");
        AddOption("e4", "Reissue on item(s) after item is established and streaming");
        AddOption("e5", "Request new view item(s) after specified number of defaultMsg REFRESHES");
        AddOption("delayInitialRequest", "Delay sending initial request for items until after source directory refresh");
        AddOption("eView", "Specifies each view request using a elementlist view. Default is false.");

        //END APIQA
    }
}
