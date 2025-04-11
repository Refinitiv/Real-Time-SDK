/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.consumer;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.examples.common.CacheInfo;
import com.refinitiv.eta.valueadd.examples.common.ConnectionArg;
import com.refinitiv.eta.valueadd.reactor.ConsumerRole;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.TunnelStream;

/*
 * Contains information associated with each open channel
 * in the value add Consumer.
 */
class ChannelInfo
{
    ConnectionArg connectionArg;
    ReactorConnectOptions connectOptions = ReactorFactory.createReactorConnectOptions();
    ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
    ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
    StreamIdWatchList itemWatchList = new StreamIdWatchList();
    MarketPriceHandler marketPriceHandler = new MarketPriceHandler(itemWatchList);
    MarketByOrderHandler marketByOrderHandler = new MarketByOrderHandler(itemWatchList);
    MarketByPriceHandler marketByPriceHandler = new MarketByPriceHandler(itemWatchList);
    PostHandler postHandler = new PostHandler();
    SymbolListHandler symbolListHandler = new SymbolListHandler();
    YieldCurveHandler yieldCurveHandler = new YieldCurveHandler(itemWatchList);
    DataDictionary dictionary;
    int fieldDictionaryStreamId = 0;
    int enumDictionaryStreamId = 0;

    boolean shouldOffStreamPost = false;
    boolean shouldOnStreamPost = false;
    boolean shouldEnableEncrypted = false;
    boolean shouldEnableHttp = false;
    boolean shouldEnableWebsocket = false;
    Buffer postItemName = CodecFactory.createBuffer();

    DecodeIterator dIter = CodecFactory.createDecodeIterator();
    Msg responseMsg = CodecFactory.createMsg();

    LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    boolean hasServiceInfo = false;
    Service serviceInfo = DirectoryMsgFactory.createService();
    ReactorChannel reactorChannel;

    List<String> mpItemList = new ArrayList<>();
    List<String> mppsItemList = new ArrayList<>();
    List<String> mboItemList = new ArrayList<>();
    List<String> mbopsItemList = new ArrayList<>();
    List<String> mbpItemList = new ArrayList<>();
    List<String> mbppsItemList = new ArrayList<>();
    List<String> ycItemList = new ArrayList<>();
    List<String> ycpsItemList = new ArrayList<>();
    List<String> slItemList = new ArrayList<>();

    // streams items are non-recoverable, it is not sent again after
    // recovery
    boolean mppsRequestSent = false;
    boolean mbopsRequestSent = false;
    boolean mbppsRequestSent = false;
    boolean ycpsRequestSent = false;

    boolean requestsSent = false; // flag to track if we already made item request(s)

    CacheInfo cacheInfo = new CacheInfo();

    boolean tunnelStreamOpenSent = false; // flag to track if we already made a tunnel stream open request

    // assume one tunnel stream per ReactorChannel
    TunnelStream tunnelStream;
    boolean hasTunnelStreamServiceInfo = false;
    Service tsServiceInfo = DirectoryMsgFactory.createService();
    boolean isTunnelStreamUp;

    long loginReissueTime; // represented by epoch time in milliseconds
    boolean canSendLoginReissue;

    boolean isChannelClosed;

    {
        connectOptions.connectionList().add(connectInfo);
        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
    }
}
