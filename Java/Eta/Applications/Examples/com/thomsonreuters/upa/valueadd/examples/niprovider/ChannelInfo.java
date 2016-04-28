package com.thomsonreuters.upa.valueadd.examples.niprovider;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.examples.common.CacheInfo;
import com.thomsonreuters.upa.valueadd.examples.common.ConnectionArg;
import com.thomsonreuters.upa.valueadd.reactor.NIProviderRole;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;

/*
* Contains information associated with each open channel
* in the value add NIProvider.
*/
class ChannelInfo
{
	ConnectionArg connectionArg;
	ReactorConnectOptions connectOptions = ReactorFactory.createReactorConnectOptions();
	ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
	NIProviderRole niproviderRole = ReactorFactory.createNIProviderRole();

	StreamIdWatchList itemWatchList = new StreamIdWatchList();

	MarketPriceHandler marketPriceHandler; 
	MarketByOrderHandler marketByOrderHandler; 

	int fieldDictionaryStreamId = 0;
	int enumDictionaryStreamId = 0;

	DecodeIterator dIter = CodecFactory.createDecodeIterator();
	Msg responseMsg = CodecFactory.createMsg();

	LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
	Service serviceInfo = DirectoryMsgFactory.createService();
	ReactorChannel reactorChannel;

	List<String> mpItemList = new ArrayList<String>();
	List<String> mppsItemList = new ArrayList<String>();
	List<String> mboItemList = new ArrayList<String>();
	List<String> mbopsItemList = new ArrayList<String>();
	
	CacheInfo cacheInfo = new CacheInfo();


	{
	    connectOptions.connectionList().add(connectInfo);
	    loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	}
}
