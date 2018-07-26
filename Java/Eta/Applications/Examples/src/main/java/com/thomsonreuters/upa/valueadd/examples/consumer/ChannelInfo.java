package com.thomsonreuters.upa.valueadd.examples.consumer;

import java.util.ArrayList;
import java.util.List;

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
import com.thomsonreuters.upa.valueadd.examples.common.CacheInfo;
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
    
    CacheInfo cacheInfo = new CacheInfo();
    
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
