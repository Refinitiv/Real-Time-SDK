///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.valueadd.reactor;

import java.util.ArrayList;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.Qos;
import com.rtsdk.eta.codec.QosRates;
import com.rtsdk.eta.codec.QosTimeliness;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.directory.Service;

public class Provider extends TestReactorComponent implements ProviderCallback, TunnelStreamListenerCallback, TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback
{

	public Provider(TestReactor testReactor)
	{
		super(testReactor);
		_reactorRole = ReactorFactory.createProviderRole();
	}
	
    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        return _testReactor.handleChannelEvent(event);
    }

    @Override
    public int defaultMsgCallback(ReactorMsgEvent event)
    {
        return _testReactor.handleDefaultMsgEvent(event);
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        return _testReactor.handleLoginMsgEvent(event);
    }

    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        return _testReactor.handleDirectoryMsgEvent(event);
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        return _testReactor.handleDictionaryMsgEvent(event);
    }
	
	/* A default service that can be used when setting up a connection. */
	private static Service _defaultService;
	
	/* A second default service that can be used when setting up a connection. */
	private static Service _defaultService2;
	
	static
	{
		_defaultService = DirectoryMsgFactory.createService();
		_defaultService.clear();
		_defaultService.applyHasInfo();
		_defaultService.applyHasState();
		_defaultService.serviceId(1);

		_defaultService.info().serviceName().data("DEFAULT_SERVICE");

		_defaultService.info().capabilitiesList(new ArrayList<Long>());
		_defaultService.info().capabilitiesList().add((long)DomainTypes.DICTIONARY);
		_defaultService.info().capabilitiesList().add((long)DomainTypes.MARKET_PRICE);
		_defaultService.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_ORDER);
		_defaultService.info().capabilitiesList().add((long)DomainTypes.SYMBOL_LIST);
		_defaultService.info().capabilitiesList().add((long)DomainTypes.SYSTEM);

		_defaultService.info().applyHasQos();
		_defaultService.info().qosList(new ArrayList<Qos>());
        Qos qos = CodecFactory.createQos();
        qos.clear();
        qos.timeliness(QosTimeliness.DELAYED);
        qos.rate(QosRates.JIT_CONFLATED);
        qos.dynamic(false);
        qos.timeInfo(0);
        qos.rateInfo(0);
        _defaultService.info().qosList().add(qos);
        qos.clear();
        qos.timeliness(QosTimeliness.REALTIME);
        qos.rate(QosRates.TICK_BY_TICK);
        qos.dynamic(false);
        qos.timeInfo(0);
        qos.rateInfo(0);
        _defaultService.info().qosList().add(qos);
        
        _defaultService.state().applyHasAcceptingRequests();
        _defaultService.state().acceptingRequests(1);
        _defaultService.state().serviceState(1);

		_defaultService2 = DirectoryMsgFactory.createService();
		_defaultService2.clear();
		_defaultService2.applyHasInfo();
		_defaultService2.applyHasState();
		_defaultService2.serviceId(2);

		_defaultService2.info().serviceName().data("DEFAULT_SERVICE2");

		_defaultService2.info().capabilitiesList(new ArrayList<Long>());
		_defaultService2.info().capabilitiesList().add((long)DomainTypes.DICTIONARY);
		_defaultService2.info().capabilitiesList().add((long)DomainTypes.MARKET_PRICE);
		_defaultService2.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_ORDER);
		_defaultService2.info().capabilitiesList().add((long)DomainTypes.SYMBOL_LIST);
		_defaultService2.info().capabilitiesList().add((long)DomainTypes.SYSTEM);

		_defaultService2.info().applyHasQos();
		_defaultService2.info().qosList(new ArrayList<Qos>());
        qos.clear();
        qos.timeliness(QosTimeliness.DELAYED);
        qos.rate(QosRates.JIT_CONFLATED);
        qos.dynamic(false);
        qos.timeInfo(0);
        qos.rateInfo(0);
        _defaultService2.info().qosList().add(qos);
        qos.clear();
        qos.timeliness(QosTimeliness.REALTIME);
        qos.rate(QosRates.TICK_BY_TICK);
        qos.dynamic(false);
        qos.timeInfo(0);
        qos.rateInfo(0);
        _defaultService2.info().qosList().add(qos);
        
        _defaultService2.state().applyHasAcceptingRequests();
        _defaultService2.state().acceptingRequests(1);
        _defaultService2.state().serviceState(1);
	}
	
	static final Service defaultService()
	{
		return _defaultService;
	}

	static final Service defaultService2()
	{
		return _defaultService2;
	}

	@Override
    public int defaultMsgCallback(TunnelStreamMsgEvent event)
    {
        return _testReactor.handleTunnelStreamMsgEvent(event);
    }

    @Override
    public int statusEventCallback(TunnelStreamStatusEvent event)
    {
        return _testReactor.handleTunnelStreamStatusEvent(event);
    }

    @Override
    public int listenerCallback(TunnelStreamRequestEvent event)
    {
        return _testReactor.handleTunnelStreamRequestEvent(event);
    }
}
