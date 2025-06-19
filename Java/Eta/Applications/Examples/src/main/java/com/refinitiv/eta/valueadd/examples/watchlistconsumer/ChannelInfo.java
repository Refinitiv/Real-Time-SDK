/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

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

    PostHandler postHandler = new PostHandler();
 
    DataDictionary dictionary;
    int fieldDictionaryStreamId = 0;
    int enumDictionaryStreamId = 0;
    boolean shouldOffStreamPost = false;
    boolean shouldOnStreamPost = false;
    boolean shouldEnableEncrypted = false;
    boolean shouldEnableHttp = false;
    Buffer postItemName = CodecFactory.createBuffer();
    
    DecodeIterator dIter = CodecFactory.createDecodeIterator();
    Msg responseMsg = CodecFactory.createMsg();

    LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
	boolean hasServiceInfo = false;
    Service serviceInfo = DirectoryMsgFactory.createService();
    ReactorChannel reactorChannel;
    
    boolean tunnelStreamOpenSent = false; // flag to track if we already made a tunnel stream open request
    
    // assume one tunnel stream per ReactorChannel
    TunnelStream tunnelStream;
    boolean hasTunnelStreamServiceInfo = false;
    Service tsServiceInfo = DirectoryMsgFactory.createService();
    boolean isTunnelStreamUp;
    
	long loginReissueTime; // represented by epoch time in milliseconds
	boolean canSendLoginReissue;

	ChannelInfo()
    {
    	connectOptions.connectionList().add(connectInfo);
    	loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
    }
}
