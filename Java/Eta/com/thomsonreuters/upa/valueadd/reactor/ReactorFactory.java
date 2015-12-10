package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.valueadd.common.VaPool;

/**
 * Factory for Reactor package objects.
 */
public class ReactorFactory
{
    static VaPool _reactorChannelPool = new VaPool(true);
    static VaPool _rdmLoginMsgEventPool = new VaPool(true);
    static VaPool _rdmDirectoryMsgEventPool = new VaPool(true);
    static VaPool _rdmDictionaryMsgEventPool = new VaPool(true);
    static VaPool _reactorMsgEventPool = new VaPool(true);
    static VaPool _reactorChannelEventPool = new VaPool(true);
    static VaPool _queueMsgEventPool = new VaPool(true);
    static VaPool _tunnelStreamMsgEventPool = new VaPool(true);
    static VaPool _tunnelStreamStatusEventPool = new VaPool(true);
    static VaPool _workerEventPool = new VaPool(true);
    static VaPool _watchlistPool = new VaPool(true);
    static VaPool _postTimeoutInfoPool = new VaPool(true);
    static VaPool _itemAggregationKeyPool = new VaPool(true);
    static VaPool _wlStreamPool = new VaPool(true);
    static VaPool _wlRequestPool = new VaPool(true);
    static VaPool _wlServicePool = new VaPool(true);
    
    private ReactorFactory()
    {
        throw new AssertionError();
    }

    /**
     * Create a {@link Reactor}, including all necessary internal memory and
     * threads. Once the RsslReactor is created, use
     * {@link Reactor#connect(ReactorConnectOptions, ReactorRole, ReactorErrorInfo)}
     * or {@link Reactor#accept(com.thomsonreuters.upa.transport.Server, ReactorAcceptOptions,
     * ReactorRole, ReactorErrorInfo)} to create new {@link ReactorChannel}.
     * Options are passed in via the {@link ReactorOptions}.
     * 
     * @param options
     * @param errorInfo
     * 
     * @return a Reactor object or null. If null, check errorInfo for additional
     *         information regarding the failure
     */
    public static Reactor createReactor(ReactorOptions options, ReactorErrorInfo errorInfo)
    {
        if (errorInfo == null)
        {
            System.out.println("ReactoryFactor.createReactor: ReactorErrorInfo cannot be null, reactor not created.");
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

    static TunnelStreamAuthInfo createTunnelStreamAuthInfo()
    {
        return new TunnelStreamAuthInfo();
    }

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
}
