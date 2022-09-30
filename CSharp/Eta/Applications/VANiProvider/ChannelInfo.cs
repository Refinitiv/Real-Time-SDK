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

namespace Refinitiv.Eta.ValueAdd.VANiProvider
{
    /// <summary>
    /// Contains information associated with each open channel 
    /// in the ValueAdd NIProvider.
    /// </summary>
    public class ChannelInfo
    {
		public ConnectionArg? ConnectionArg;
		public ReactorConnectOptions ConnectOptions = new ReactorConnectOptions();
		public ReactorConnectInfo ConnectInfo = new ReactorConnectInfo();
		public NIProviderRole NiProviderRole = new NIProviderRole();

		public StreamIdWatchList ItemWatchList = new StreamIdWatchList();

		public MarketPriceHandler? MarketPriceHandler;
		public MarketByOrderHandler? MarketByOrderHandler;

		public int fieldDictionaryStreamId = 0;
		public int enumDictionaryStreamId = 0;

		public DecodeIterator DecodeIter = new DecodeIterator();
		public Msg responseMsg = new Msg();

		public LoginRefresh LoginRefresh = new LoginRefresh();
		public Service ServiceInfo = new Service();
		public ReactorChannel? ReactorChannel;

		public List<string> MpItemList = new List<string>();
		public List<string> MppsItemList = new List<string>();
		public List<string> MboItemList = new List<string>();
		public List<string> MbopsItemList = new List<string>();

		public long LoginReissueTime; // represented by epoch time in milliseconds
		public bool CanSendLoginReissue;

		public ChannelInfo() 
		{
			ConnectOptions.ConnectionList.Add(ConnectInfo);
		}
	}
}
