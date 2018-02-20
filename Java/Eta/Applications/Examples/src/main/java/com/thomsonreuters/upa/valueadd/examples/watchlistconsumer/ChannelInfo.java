package com.thomsonreuters.upa.valueadd.examples.watchlistconsumer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.examples.common.ConnectionArg;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerRole;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStream;

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
    
    // assume one tunnel stream/substream per ReactorChannel
    TunnelStream tunnelStream;
    boolean isTunnelStreamUp;
    boolean isQueueStreamUp;
    
    // service for queue messaging
    boolean hasQServiceInfo = false;
    Service qServiceInfo = DirectoryMsgFactory.createService();
    
	long loginReissueTime; // represented by epoch time in milliseconds
	boolean canSendLoginReissue;

    {
    	connectOptions.connectionList().add(connectInfo);
    	loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
    }
}
