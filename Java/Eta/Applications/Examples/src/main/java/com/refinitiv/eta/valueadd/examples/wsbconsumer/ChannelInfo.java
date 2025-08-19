/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.wsbconsumer;

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
import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;

/*
 * Contains information associated with each open channel
 * in the value add Consumer.
 */
class ChannelInfo
{
	ConnectionArg connectionArg;
    ReactorConnectOptions connectOptions = null;
    ConsumerRole consumerRole = ReactorFactory.createConsumerRole();

    DataDictionary dictionary;
    int fieldDictionaryStreamId = 0;
    int enumDictionaryStreamId = 0;
    
    DecodeIterator dIter = CodecFactory.createDecodeIterator();
    Msg responseMsg = CodecFactory.createMsg();

    LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
    Service serviceInfo = DirectoryMsgFactory.createService();
    ReactorChannel reactorChannel;
    
	long loginReissueTime; // represented by epoch time in milliseconds
	boolean canSendLoginReissue;

    boolean isChannelClosed;

	ChannelInfo()
    {
    	loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
    }
}
