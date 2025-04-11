/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

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
/*
 * Contains information associated with each open channel
 * in the value add Consumer.
 */
class TestChannelInfo
{
    ReactorConnectOptions connectOptions = ReactorFactory.createReactorConnectOptions();
    ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
    ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
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

    List<String> mpItemList = new ArrayList<String>();
    List<String> mppsItemList = new ArrayList<String>();
    List<String> mboItemList = new ArrayList<String>();
    List<String> mbopsItemList = new ArrayList<String>();
    List<String> mbpItemList = new ArrayList<String>();
    List<String> mbppsItemList = new ArrayList<String>();
    List<String> ycItemList = new ArrayList<String>();
    List<String> ycpsItemList = new ArrayList<String>();
    List<String> slItemList = new ArrayList<String>();

    // streams items are non-recoverable, it is not sent again after
    // recovery
    boolean mppsRequestSent = false;
    boolean mbopsRequestSent = false;
    boolean mbppsRequestSent = false;
    boolean ycpsRequestSent = false;

    boolean requestsSent = false; // flag to track if we already made item request(s)

    boolean tunnelStreamOpenSent = false; // flag to track if we already made a tunnel stream open request

    // assume one tunnel stream per ReactorChannel
    TunnelStream tunnelStream;
    boolean hasTunnelStreamServiceInfo = false;
    Service tsServiceInfo = DirectoryMsgFactory.createService();
    boolean isTunnelStreamUp;

    long loginReissueTime; // represented by epoch time in milliseconds
    boolean canSendLoginReissue;

    {
        connectOptions.connectionList().add(connectInfo);
        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
    }
}