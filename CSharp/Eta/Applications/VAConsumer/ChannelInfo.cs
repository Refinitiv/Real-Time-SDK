/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Example.VACommon;
using Refinitiv.Eta.ValueAdd.Rdm;
using Refinitiv.Eta.ValueAdd.Reactor;

namespace Refinitiv.Eta.ValueAdd.Consumer
{
    /// <summary>
    /// Contains information associated with each open channel
    /// in the value add Consumer.
    /// </summary>
    public class ChannelInfo
    {
        internal ConnectionArg ConnectionArg;
        internal ReactorConnectOptions ConnectOptions = new();
        internal ReactorConnectInfo ConnectInfo = new();
        internal ConsumerRole ConsumerRole = new();
        internal StreamIdWatchList ItemWatchList = new();
        internal MarketPriceHandler MarketPriceHandler;
        internal MarketByOrderHandler MarketByOrderHandler;
        internal MarketByPriceHandler MarketByPriceHandler;
        internal PostHandler PostHandler = new();
        internal SymbolListHandler SymbolListHandler = new();
        internal YieldCurveHandler YieldCurveHandler;
        internal DataDictionary Dictionary = new();
        internal int FieldDictionaryStreamId = 0;
        internal int EnumDictionaryStreamId = 0;

        internal bool ShouldOffStreamPost = false;
        internal bool ShouldOnStreamPost = false;
        internal bool ShouldEnableEncrypted = false;
        internal Codec.Buffer PostItemName = new();

        internal DecodeIterator DecodeIter = new();
        internal Msg ResponseMsg = new();

        internal LoginRefresh LoginRefresh = new LoginRefresh();
        internal bool HasServiceInfo = false;
        internal Service ServiceInfo = new();
        internal ReactorChannel? ReactorChannel;

        internal List<string> mpItemList = new();
        internal List<string> mppsItemList = new();
        internal List<string> mboItemList = new();
        internal List<string> mbopsItemList = new();
        internal List<string> mbpItemList = new();
        internal List<string> mbppsItemList = new();
        internal List<string> ycItemList = new();
        internal List<string> ycpsItemList = new();
        internal List<string> slItemList = new();

        // streams items are non-recoverable, it is not sent again after
        // recovery
        internal bool mppsRequestSent = false;
        internal bool mbopsRequestSent = false;
        internal bool mbppsRequestSent = false;
        internal bool ycpsRequestSent = false;

        /// <summary>
        /// Flag to track if we already made item request(s)
        /// </summary>
        internal bool RequestsSent = false;

        internal System.DateTime LoginReissueTime;
        internal bool CanSendLoginReissue;

        public ChannelInfo(ConnectionArg connectionArg)
        {
            MarketPriceHandler = new MarketPriceHandler(ItemWatchList);
            MarketByOrderHandler = new MarketByOrderHandler(ItemWatchList);
            MarketByPriceHandler = new MarketByPriceHandler(ItemWatchList);
            YieldCurveHandler = new YieldCurveHandler(ItemWatchList);
            ConnectionArg = connectionArg;

            ConnectOptions.ConnectionList.Add(ConnectInfo);
        }
    }
}
