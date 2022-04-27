/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.LimitedVaPool;
import com.refinitiv.eta.valueadd.common.VaPool;

import java.io.OutputStream;

/**
 * Factory for Reactor package objects.
 */
public class ReactorFactory
{
    static VaPool _reactorChannelPool = new VaPool(true);
    static VaPool _rdmLoginMsgEventPool = new VaPool(true);
    static VaPool _rdmDirectoryMsgEventPool = new VaPool(true);
    static VaPool _rdmDictionaryMsgEventPool = new VaPool(true);
    static LimitedVaPool _reactorMsgEventPool = new LimitedVaPool(true);
    static LimitedVaPool _reactorChannelEventPool = new LimitedVaPool(true);
    static VaPool _reactorAuthTokenEventPool = new VaPool(true);
    static VaPool _reactorServiceEndpointEventPool = new VaPool(true);
    static VaPool _reactorOAuthCredentialEventPool = new VaPool(true);
    static VaPool _queueMsgEventPool = new VaPool(true);
    static LimitedVaPool _tunnelStreamMsgEventPool = new LimitedVaPool(true);
    static LimitedVaPool _tunnelStreamStatusEventPool = new LimitedVaPool(true);
    static LimitedVaPool _workerEventPool = new LimitedVaPool(true);
    static VaPool _watchlistPool = new VaPool(true);
    static VaPool _postTimeoutInfoPool = new VaPool(true);
    static VaPool _itemAggregationKeyPool = new VaPool(true);
    static VaPool _wlStreamPool = new VaPool(true);
    static VaPool _wlRequestPool = new VaPool(true);
    static VaPool _wlServicePool = new VaPool(true);
    static VaPool _wlItemGroupPool = new VaPool(true);
    static VaPool _wlIntegerPool = new VaPool(true);
    static VaPool _wlViewPool = new VaPool(true);
    static VaPool _packedBufferPool = new VaPool(true);

    private static final int DEFAULT_POOL_LIMIT = -1;
    /**
     * Instantiates a new reactor factory.
     */
    private ReactorFactory()
    {
        throw new AssertionError();
    }


    /**
     * Sets maximum number of events in _reactorMsgEventPoolLimit, if value is negative then amount of events is unlimited 
     * @param reactorMsgEventPoolLimit value to set
     */
    public static void setReactorMsgEventPoolLimit(int reactorMsgEventPoolLimit) {
        ReactorFactory._reactorMsgEventPool.setLimit(reactorMsgEventPoolLimit > 0 ? reactorMsgEventPoolLimit : DEFAULT_POOL_LIMIT);
    }

    /**
     * Sets maximum number of events in _reactorChannelEventPoolLimit, if value is negative then amount of events is unlimited 
     * @param reactorChannelEventPoolLimit value to set
     */
    public static void setReactorChannelEventPoolLimit(int reactorChannelEventPoolLimit) {
        ReactorFactory._reactorChannelEventPool.setLimit(reactorChannelEventPoolLimit > 0 ? reactorChannelEventPoolLimit : DEFAULT_POOL_LIMIT);
    }

    /**
     * Sets maximum number of events in _workerEventPoolLimit, if value is negative then amount of events is unlimited 
     * @param workerEventPoolLimit value to set
     */
    public static void setWorkerEventPoolLimit(int workerEventPoolLimit) {
        ReactorFactory._workerEventPool.setLimit(workerEventPoolLimit > 0 ? workerEventPoolLimit : DEFAULT_POOL_LIMIT);
    }

    /**
     * Sets maximum number of events in _tunnelStreamMsgEventPoolLimit, if value is negative then amount of events is unlimited 
     * @param tunnelStreamMsgEventPoolLimit value to set
     */
    public static void setTunnelStreamMsgEventPoolLimit(int tunnelStreamMsgEventPoolLimit) {
        ReactorFactory._tunnelStreamMsgEventPool.setLimit(tunnelStreamMsgEventPoolLimit > 0 ? tunnelStreamMsgEventPoolLimit : DEFAULT_POOL_LIMIT);
    }

    /**
     * Sets maximum number of events in _tunnelStreamStatusEventPoolLimit, if value is negative then amount of events is unlimited 
     * @param tunnelStreamStatusEventPoolLimit value to set
     */
    public static void setTunnelStreamStatusEventPoolLimit(int tunnelStreamStatusEventPoolLimit) {
        ReactorFactory._tunnelStreamStatusEventPool.setLimit(tunnelStreamStatusEventPoolLimit > 0 ? tunnelStreamStatusEventPoolLimit : DEFAULT_POOL_LIMIT);
    }

    /**
     * Create a {@link Reactor}, including all necessary internal memory and
     * threads. Once the RsslReactor is created, use
     * {@link Reactor#connect(ReactorConnectOptions, ReactorRole, ReactorErrorInfo)}
     * or {@link Reactor#accept(com.refinitiv.eta.transport.Server, ReactorAcceptOptions,
     * ReactorRole, ReactorErrorInfo)} to create new {@link ReactorChannel}.
     * Options are passed in via the {@link ReactorOptions}.
     *
     * @param options the options
     * @param errorInfo the error info
     * @return a Reactor object or null. If null, check errorInfo for additional
     *         information regarding the failure
     */
    public static Reactor createReactor(ReactorOptions options, ReactorErrorInfo errorInfo)
    {
        if (errorInfo == null)
        {
            System.out.println("ReactorFactory.createReactor: ReactorErrorInfo cannot be null, reactor not created.");
            return null;
        }

        Reactor reactor = new Reactor(options, errorInfo);
        if (!reactor.isShutdown())
            return reactor;
        else
            return null;
    }

    /**
     * Create {@link ReactorErrorInfo}.
     *
     * @return {@link ReactorErrorInfo} object
     *
     * @see ReactorErrorInfo
     */
    public static ReactorErrorInfo createReactorErrorInfo()
    {
        return new ReactorErrorInfo();
    }

    /**
     * Create {@link ReactorOptions}.
     *
     * @return {@link ReactorOptions} object
     *
     * @see ReactorOptions
     */
    public static ReactorOptions createReactorOptions()
    {
        return new ReactorOptions();
    }

    /**
     * Create {@link ReactorChannelStats}.
     *
     * @return {@link ReactorChannelStats} object
     *
     * @see ReactorChannelStats
     */
    public static ReactorChannelStats createReactorChannelStats()
    {
        return new ReactorChannelStatsImpl();
    }

    /**
     * Create {@link ReactorConnectOptions}.
     *
     * @return {@link ReactorConnectOptions} object
     *
     * @see ReactorConnectOptions
     */
    public static ReactorConnectOptions createReactorConnectOptions()
    {
        return new ReactorConnectOptions();
    }

    /**
     * Create {@link ReactorAcceptOptions}.
     *
     * @return {@link ReactorAcceptOptions} object
     *
     * @see ReactorAcceptOptions
     */
    public static ReactorAcceptOptions createReactorAcceptOptions()
    {
        return new ReactorAcceptOptions();
    }

    /**
     * Create {@link ReactorDispatchOptions}.
     *
     * @return {@link ReactorDispatchOptions} object
     *
     * @see ReactorDispatchOptions
     */
    public static ReactorDispatchOptions createReactorDispatchOptions()
    {
        return new ReactorDispatchOptions();
    }

    /**
     * Create {@link ReactorSubmitOptions}.
     *
     * @return {@link ReactorSubmitOptions} object
     *
     * @see ReactorSubmitOptions
     */
    public static ReactorSubmitOptions createReactorSubmitOptions()
    {
        return new ReactorSubmitOptions();
    }

    /**
     * Create {@link TunnelStreamOpenOptions}.
     *
     * @return {@link TunnelStreamOpenOptions} object
     *
     * @see TunnelStreamOpenOptions
     */
    public static TunnelStreamOpenOptions createTunnelStreamOpenOptions()
    {
        return new TunnelStreamOpenOptions();
    }

    /**
     * Create a ConsumerRole.
     *
     * @return {@link ConsumerRole} object
     *
     * @see ConsumerRole
     */
    public static ConsumerRole createConsumerRole()
    {
        return new ConsumerRole();
    }

    /**
     * Create a ProviderRole.
     *
     * @return {@link ProviderRole} object
     *
     * @see ProviderRole
     */
    public static ProviderRole createProviderRole()
    {
        return new ProviderRole();
    }

    /**
     * Create a NIProviderRole.
     *
     * @return {@link NIProviderRole} object
     *
     * @see NIProviderRole
     */
    public static NIProviderRole createNIProviderRole()
    {
        return new NIProviderRole();
    }

    /**
     * Create {@link ReactorChannelInfo}.
     *
     * @return {@link ReactorChannelInfo} object
     *
     * @see ReactorChannelInfo
     */
    public static ReactorChannelInfo createReactorChannelInfo()
    {
        return new ReactorChannelInfo();
    }

    /**
     * Create {@link ReactorConnectInfo}.
     *
     * @return {@link ReactorConnectInfo} object
     *
     * @see ReactorConnectInfo
     */
    public static ReactorConnectInfo createReactorConnectInfo()
    {
        return new ReactorConnectInfo();
    }

    /**
     * Create {@link TunnelStreamAcceptOptions}.
     *
     * @return {@link TunnelStreamAcceptOptions} object
     *
     * @see TunnelStreamAcceptOptions
     */
    public static TunnelStreamAcceptOptions createTunnelStreamAcceptOptions()
    {
        return new TunnelStreamAcceptOptions();
    }

    /**
     * Create {@link TunnelStreamRejectOptions}.
     *
     * @return {@link TunnelStreamRejectOptions} object
     *
     * @see TunnelStreamRejectOptions
     */
    public static TunnelStreamRejectOptions createTunnelStreamRejectOptions()
    {
        return new TunnelStreamRejectOptions();
    }

    /**
     * Create {@link TunnelStreamSubmitOptions}.
     *
     * @return {@link TunnelStreamSubmitOptions} object
     *
     * @see TunnelStreamSubmitOptions
     */
    public static TunnelStreamSubmitOptions createTunnelStreamSubmitOptions()
    {
        return new TunnelStreamSubmitOptions();
    }

    /**
     * Create {@link ClassOfService}.
     *
     * @return {@link ClassOfService} object
     *
     * @see ClassOfService
     */
    public static ClassOfService createClassOfService()
    {
        return new ClassOfService();
    }

    /**
     * Creates {@link ReactorServiceDiscoveryOptions}.
     *
     * @return {@link ReactorServiceDiscoveryOptions} object
     *
     * @see ReactorServiceDiscoveryOptions
     */
    public static ReactorServiceDiscoveryOptions createReactorServiceDiscoveryOptions()
    {
        return new ReactorServiceDiscoveryOptions();
    }

    /**
     * Creates {@link TunnelStreamInfo}
     *
     * @return the tunnel info
     * @see TunnelStreamInfo
     */
    public static TunnelStreamInfo createTunnelStreamInfo()
    {
        return new TunnelStreamInfoImpl();
    }

    /**
     * Creates {@link ReactorOAuthCredential}.
     *
     * @return {@link ReactorOAuthCredential} object
     *
     * @see ReactorOAuthCredential
     */
    public static ReactorOAuthCredential createReactorOAuthCredential()
    {
        return new ReactorOAuthCredential();
    }

    /**
     * Creates {@link ReactorOAuthCredentialRenewal}.
     *
     * @return {@link ReactorOAuthCredentialRenewal} object
     *
     * @see ReactorOAuthCredentialRenewal
     */
    public static ReactorOAuthCredentialRenewal createReactorOAuthCredentialRenewal()
    {
        return new ReactorOAuthCredentialRenewal();
    }

    /**
     * Creates {@link ReactorOAuthCredentialRenewalOptions}.
     *
     * @return {@link ReactorOAuthCredentialRenewalOptions} object
     *
     * @see ReactorOAuthCredentialRenewalOptions
     */
    public static ReactorOAuthCredentialRenewalOptions createReactorOAuthCredentialRenewalOptions()
    {
        return new ReactorOAuthCredentialRenewalOptions();
    }

    /**
     * Creates {@link ReactorJsonConverterOptions}.
     *
     * @return {@link ReactorJsonConverterOptions} object
     *
     * @see ReactorJsonConverterOptions
     */
    public static ReactorJsonConverterOptions createReactorJsonConverterOptions()
    {
        return new ReactorJsonConverterOptions();
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the tunnel stream auth info
     */
    static TunnelStreamAuthInfo createTunnelStreamAuthInfo()
    {
        return new TunnelStreamAuthInfo();
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the reactor channel
     */
    static ReactorChannel createReactorChannel()
    {
        ReactorChannel reactorChannel = (ReactorChannel)_reactorChannelPool.poll();
        if(reactorChannel == null)
        {
            reactorChannel = new ReactorChannel();
            _reactorChannelPool.updatePool(reactorChannel);
        }
        else
        {
            reactorChannel.clear();
        }
        return reactorChannel;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the RDM login msg event
     */
    static RDMLoginMsgEvent createRDMLoginMsgEvent()
    {
        RDMLoginMsgEvent rdmLoginMsgEvent = (RDMLoginMsgEvent)_rdmLoginMsgEventPool.poll();
        if(rdmLoginMsgEvent == null)
        {
            rdmLoginMsgEvent = new RDMLoginMsgEvent();
            _rdmLoginMsgEventPool.updatePool(rdmLoginMsgEvent);
        }
        else
        {
            rdmLoginMsgEvent.clear();
        }
        return rdmLoginMsgEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the RDM directory msg event
     */
    static RDMDirectoryMsgEvent createRDMDirectoryMsgEvent()
    {
        RDMDirectoryMsgEvent rdmDirectoryMsgEvent = (RDMDirectoryMsgEvent)_rdmDirectoryMsgEventPool.poll();
        if(rdmDirectoryMsgEvent == null)
        {
            rdmDirectoryMsgEvent = new RDMDirectoryMsgEvent();
            _rdmDirectoryMsgEventPool.updatePool(rdmDirectoryMsgEvent);
        }
        else
        {
            rdmDirectoryMsgEvent.clear();
        }
        return rdmDirectoryMsgEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the RDM dictionary msg event
     */
    static RDMDictionaryMsgEvent createRDMDictionaryMsgEvent()
    {
        RDMDictionaryMsgEvent rdmDictionaryMsgEvent = (RDMDictionaryMsgEvent)_rdmDictionaryMsgEventPool.poll();
        if(rdmDictionaryMsgEvent == null)
        {
            rdmDictionaryMsgEvent = new RDMDictionaryMsgEvent();
            _rdmDictionaryMsgEventPool.updatePool(rdmDictionaryMsgEvent);
        }
        else
        {
            rdmDictionaryMsgEvent.clear();
        }
        return rdmDictionaryMsgEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the reactor msg event
     */
    static ReactorMsgEvent createReactorMsgEvent()
    {
        ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)_reactorMsgEventPool.poll();
        if(reactorMsgEvent == null)
        {
            reactorMsgEvent = new ReactorMsgEvent();
            _reactorMsgEventPool.updatePool(reactorMsgEvent);
        }
        else
        {
            reactorMsgEvent.clear();
        }
        return reactorMsgEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the reactor channel event
     */
    static ReactorChannelEvent createReactorChannelEvent()
    {
        ReactorChannelEvent reactorChannelEvent = (ReactorChannelEvent)_reactorChannelEventPool.poll();
        if(reactorChannelEvent == null)
        {
            reactorChannelEvent = new ReactorChannelEvent();
            _reactorChannelEventPool.updatePool(reactorChannelEvent);
        }
        else
        {
            reactorChannelEvent.clear();
        }
        return reactorChannelEvent;
    }

    /**
     * Creates a new ReactorAuthTokenEvent object.
     *
     * @return the reactor Auth token event
     */
    static ReactorAuthTokenEvent createReactorAuthTokenEvent()
    {
        ReactorAuthTokenEvent reactorAuthTokenEvent = (ReactorAuthTokenEvent)_reactorAuthTokenEventPool.poll();
        if(reactorAuthTokenEvent == null)
        {
            reactorAuthTokenEvent = new ReactorAuthTokenEvent();
            _reactorAuthTokenEventPool.updatePool(reactorAuthTokenEvent);
        }
        else
        {
            reactorAuthTokenEvent.clear();
        }
        return reactorAuthTokenEvent;
    }

    /**
     * Creates a new ReactorServiceEndpointEvent object.
     *
     * @return the reactor service endpoint event
     */
    static ReactorServiceEndpointEvent createReactorServiceEndpointEvent()
    {
        ReactorServiceEndpointEvent reactorServiceEndpointEvent = (ReactorServiceEndpointEvent)_reactorServiceEndpointEventPool.poll();
        if(reactorServiceEndpointEvent == null)
        {
            reactorServiceEndpointEvent = new ReactorServiceEndpointEvent();
            _reactorServiceEndpointEventPool.updatePool(reactorServiceEndpointEvent);
        }
        else
        {
            reactorServiceEndpointEvent.clear();
        }
        return reactorServiceEndpointEvent;
    }

    /**
     * Creates a new ReactorOAuthCredentialEvent object.
     *
     * @return the reactor OAuth credential event
     */
    static ReactorOAuthCredentialEvent createReactorOAuthCredentialEvent()
    {
        ReactorOAuthCredentialEvent reactorOAuthCredentialEvent = (ReactorOAuthCredentialEvent)_reactorOAuthCredentialEventPool.poll();
        if(reactorOAuthCredentialEvent == null)
        {
            reactorOAuthCredentialEvent = new ReactorOAuthCredentialEvent();
            _reactorOAuthCredentialEventPool.updatePool(reactorOAuthCredentialEvent);
        }
        else
        {
            reactorOAuthCredentialEvent.clear();
        }

        return reactorOAuthCredentialEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the tunnel stream status event
     */
    public static TunnelStreamStatusEvent createTunnelStreamStatusEvent()
    {
        TunnelStreamStatusEvent tunnelStreamStatusEvent = (TunnelStreamStatusEvent)_tunnelStreamStatusEventPool.poll();
        if(tunnelStreamStatusEvent == null)
        {
            tunnelStreamStatusEvent = new TunnelStreamStatusEvent();
            _tunnelStreamStatusEventPool.updatePool(tunnelStreamStatusEvent);
        }
        else
        {
            tunnelStreamStatusEvent.clear();
        }
        return tunnelStreamStatusEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the tunnel stream msg event
     */
    static TunnelStreamMsgEvent createTunnelStreamMsgEvent()
    {
        TunnelStreamMsgEvent tunnelStreamMsgEvent = (TunnelStreamMsgEvent)_tunnelStreamMsgEventPool.poll();
        if(tunnelStreamMsgEvent == null)
        {
            tunnelStreamMsgEvent = new TunnelStreamMsgEvent();
            _tunnelStreamMsgEventPool.updatePool(tunnelStreamMsgEvent);
        }
        else
        {
            tunnelStreamMsgEvent.clear();
        }
        return tunnelStreamMsgEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the tunnel stream queue msg event
     */
    static TunnelStreamQueueMsgEvent createQueueMsgEvent()
    {
        TunnelStreamQueueMsgEvent queueMsgEvent = (TunnelStreamQueueMsgEvent)_queueMsgEventPool.poll();
        if(queueMsgEvent == null)
        {
            queueMsgEvent = new TunnelStreamQueueMsgEvent();
            _queueMsgEventPool.updatePool(queueMsgEvent);
        }
        else
        {
            queueMsgEvent.clear();
        }
        return queueMsgEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the worker event
     */
    static WorkerEvent createWorkerEvent()
    {
        WorkerEvent workerEvent = (WorkerEvent)_workerEventPool.poll();
        if(workerEvent == null)
        {
            workerEvent = new WorkerEvent();
            _workerEventPool.updatePool(workerEvent);
        }
        else
        {
            workerEvent.clear();
        }
        return workerEvent;
    }

    /**
     * Creates a new Reactor object.
     *
     * @param reactorChannel the reactor channel
     * @param consumerRole the consumer role
     * @return the watchlist
     */
    static Watchlist createWatchlist(ReactorChannel reactorChannel, ConsumerRole consumerRole)
    {
        Watchlist watchlist = (Watchlist)_watchlistPool.poll();
        if(watchlist == null)
        {
            watchlist = new Watchlist(reactorChannel, consumerRole);
            _watchlistPool.updatePool(watchlist);
        }
        else
        {
            watchlist.clear();
            watchlist.reactorChannel(reactorChannel);
            watchlist.role(consumerRole);
        }
        return watchlist;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl post timeout info
     */
    static WlPostTimeoutInfo createWlPostTimeoutInfo()
    {
        WlPostTimeoutInfo postTimeoutInfo = (WlPostTimeoutInfo)_postTimeoutInfoPool.poll();
        if(postTimeoutInfo == null)
        {
            postTimeoutInfo = new WlPostTimeoutInfo();
            _postTimeoutInfoPool.updatePool(postTimeoutInfo);
        }
        else
        {
            postTimeoutInfo.clear();
        }
        return postTimeoutInfo;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl item aggregation key
     */
    static WlItemAggregationKey createWlItemAggregationKey()
    {
        WlItemAggregationKey itemAggregationKey = (WlItemAggregationKey)_itemAggregationKeyPool.poll();
        if(itemAggregationKey == null)
        {
            itemAggregationKey = new WlItemAggregationKey();
            _itemAggregationKeyPool.updatePool(itemAggregationKey);
        }
        else
        {
            itemAggregationKey.clear();
        }
        return itemAggregationKey;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl stream
     */
    static WlStream createWlStream()
    {
        WlStream wlStream = (WlStream)_wlStreamPool.poll();
        if(wlStream == null)
        {
            wlStream = new WlStream();
            _wlStreamPool.updatePool(wlStream);
        }
        else
        {
            wlStream.clear();
        }
        return wlStream;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl request
     */
    static WlRequest createWlRequest()
    {
        WlRequest wlRequest = (WlRequest)_wlRequestPool.poll();
        if(wlRequest == null)
        {
            wlRequest = new WlRequest();
            _wlRequestPool.updatePool(wlRequest);
        }
        else
        {
            wlRequest.clear();
        }
        return wlRequest;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl service
     */
    static WlService createWlService()
    {
        WlService wlService = (WlService)_wlServicePool.poll();
        if(wlService == null)
        {
            wlService = new WlService();
            _wlServicePool.updatePool(wlService);
        }
        else
        {
            wlService.clear();
        }
        return wlService;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl item group
     */
    static WlItemGroup createWlItemGroup()
    {
        WlItemGroup wlItemGroup = (WlItemGroup)_wlItemGroupPool.poll();
        if (wlItemGroup == null)
        {
            wlItemGroup = new WlItemGroup();
            _wlItemGroupPool.updatePool(wlItemGroup);
        }
        else
        {
            wlItemGroup.clear();
        }
        return wlItemGroup;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl integer
     */
    static WlInteger createWlInteger()
    {
        WlInteger wlInteger = (WlInteger)_wlIntegerPool.poll();
        if (wlInteger == null)
        {
            wlInteger = new WlInteger();
            _wlIntegerPool.updatePool(wlInteger);
        }
        else
        {
            wlInteger.clear();
        }
        return wlInteger;
    }

    /**
     * Creates a new Reactor object.
     *
     * @return the wl view
     */
    static WlView createWlView()
    {
        WlView wlView = (WlView)_wlViewPool.poll();
        if(wlView == null)
        {
            wlView = new WlView();
            _wlViewPool.updatePool(wlView);
        }
        else
        {
            wlView.clear();
        }
        return wlView;
    }
    
    /**
     * Creates a new ReactorPackedBuffer object or get it from the pool.
     *
     * @return the ReactorPackedBuffer
     */
    static ReactorPackedBuffer createPackedBuffer()
    {
    	ReactorPackedBuffer packedBuffer = (ReactorPackedBuffer)_packedBufferPool.poll();
    	
    	if(packedBuffer == null)
    	{
    		packedBuffer = new ReactorPackedBuffer();
    		_packedBufferPool.updatePool(packedBuffer);
    	}
    	else
    	{
    		packedBuffer.clear();
    	}
    	
    	return packedBuffer;
    }

    /**
     * Creates a new ReactorDebugger instance that will use the provided OutputStream to write messages.
     * In case the OutputStream instance is null, a ByteArrayOutputStream will be created with the capacity set to
     * {@link ReactorDebuggerOptions#DEFAULT_CAPACITY}
     * @param stream the OutputStream instance into which the debugging messages will be written
     * @return ReactorDebugger instance
     */
    static ReactorDebugger createReactorDebugger(OutputStream stream)
    {
        return new ReactorDebuggerImpl(stream);
    }

    /**
     * Creates a new ReactorDebugger instance that will use the provided OutputStream to write messages.
     * In case the OutputStream instance is null, a ByteArrayOutputStream will be created with the capacity set to
     * the provided capacity value
     * @param stream the OutputStream instance into which the debugging messages will be written
     * @param capacity the capacity that will be used when creating ByteArrayOutputStream in case the provided OutputStream is null.
     *                 This value determines the total size of the debug messages that can be stored by the debugger
     *                 until {@link Reactor#getDebuggingInfo()} is called.
     *                 If it is smaller than {@link ReactorDebuggerOptions#DEFAULT_CAPACITY}, the default capacity will be used.
     *                 If it is equal to {@link ReactorDebuggerOptions#NO_LIMIT_SET}, the debugger will not check whether the capacity
     *                 was exceeded when writing the message.
     * @return ReactorDebugger instance
     */
    static ReactorDebugger createReactorDebugger(OutputStream stream, int capacity)
    {
        return new ReactorDebuggerImpl(stream, capacity);
    }

    /**
     * Creates a new ReactorDebugger instance that will use a ByteArrayOutputStream instance to write messages.
     * The ByteArrayOutputStream will be created with the capacity set to the provided capacity value. The user application has to call
     * {@link Reactor#getDebuggingInfo()} method to get the debugging output
     * @param capacity the capacity that will be used when creating ByteArrayOutputStream in case the provided OutputStream is null.
     *                 This value determines the total size of the debug messages that can be stored by the debugger
     *                 until {@link Reactor#getDebuggingInfo()} is called.
     *                 If it is smaller than {@link ReactorDebuggerOptions#DEFAULT_CAPACITY}, the default capacity will be used.
     *                 If it is equal to {@link ReactorDebuggerOptions#NO_LIMIT_SET}, the debugger will not check whether the capacity
     *                 was exceeded when writing the message.
     * @return ReactorDebugger instance
     */
    static ReactorDebugger createReactorDebugger(int capacity)
    {
        return new ReactorDebuggerImpl(capacity);
    }
}
