package com.rtsdk.eta.valueadd.examples.niprovider;

import java.util.ArrayList;
import java.util.List;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.Service;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.rtsdk.eta.valueadd.examples.common.CacheInfo;
import com.rtsdk.eta.valueadd.examples.common.ConnectionArg;
import com.rtsdk.eta.valueadd.reactor.NIProviderRole;
import com.rtsdk.eta.valueadd.reactor.ReactorChannel;
import com.rtsdk.eta.valueadd.reactor.ReactorConnectInfo;
import com.rtsdk.eta.valueadd.reactor.ReactorConnectOptions;
import com.rtsdk.eta.valueadd.reactor.ReactorFactory;

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

	long loginReissueTime; // represented by epoch time in milliseconds
	boolean canSendLoginReissue;

	{
	    connectOptions.connectionList().add(connectInfo);
	    loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	}
}
