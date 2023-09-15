/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Consumer watchlist options.
 *
 */
public class ConsumerWatchlistOptions
{
    private boolean _enableWatchlist;
    private ReactorChannelEventCallback _channelOpenCallback;
    private int _itemCountHint;
    private boolean _obeyOpenWindow = true;
    private int  _maxOutstandingPosts = 100000;
    private int _postAckTimeout = 15000;
    private int _requestTimeout = 15000; 
    private boolean _enableWarmStandby = false;
    
    /**
     * Is the watchlist enabled.
     * 
     * @return is the watchlist enabled
     */
    public boolean enableWatchlist()
    {
        return _enableWatchlist;
    }
    
    /**
     * Enables the watchlist.
     *
     * @param enableWatchlist the enable watchlist
     */
    public void enableWatchlist(boolean enableWatchlist)
    {
        _enableWatchlist = enableWatchlist;
    }

    /**
     * Callback function that is provided when a channel is first opened by Reactor.connect.
     * This is only allowed when a watchlist is enabled and is optional.
     * 
     * @return the channel open callback
     */
    public ReactorChannelEventCallback channelOpenCallback()
    {
        return _channelOpenCallback;
    }

    /**
     * Callback function that is provided when a channel is first opened by Reactor.connect.
     * This is only allowed when a watchlist is enabled and is optional.
     *
     * @param channelOpenCallback the channel open callback
     */
    public void channelOpenCallback(ReactorChannelEventCallback channelOpenCallback)
    {
        _channelOpenCallback = channelOpenCallback;
    }

    /**
     * Set to the number of items the application expects to request.
     * 
     * @return the item count hint
     */
    public int itemCountHint()
    {
        return _itemCountHint;
    }

    /**
     * Set to the number of items the application expects to request.
     *
     * @param itemCountHint the item count hint
     */
    public void itemCountHint(int itemCountHint)
    {
        _itemCountHint = itemCountHint;
    }
    
    /**
     * Return if warm standby is enabled.
     * 
     * @return enableWarmStandby boolean telling if warm standby is enabled
     */
    public boolean enableWarmStandby()
    {
        return _enableWarmStandby;
    }
    
    void enableWarmStandby(boolean enableWarmStandby)
    {
    	_enableWarmStandby = enableWarmStandby;
    }

    /**
     * Controls whether item requests obey the OpenWindow provided by a service.
     *
     * @return whether or not item requests obey OpenWindow
     */
    public boolean obeyOpenWindow()
    {
        return _obeyOpenWindow;
    }

    /**
     * Controls whether item requests obey the OpenWindow provided by a service.
     *
     * @param obeyOpenWindow whether or not item requests obey OpenWindow
     */
    public void obeyOpenWindow(boolean obeyOpenWindow)
    {
        _obeyOpenWindow = obeyOpenWindow;
    }

    /**
     * Sets the maximum number of on-stream post acknowledgments that may be outstanding for the channel.
     * 
     * @return the max outstanding posts
     */
    public int maxOutstandingPosts()
    {
        return _maxOutstandingPosts;
    }

    /**
     * Sets the maximum number of on-stream post acknowledgments that may be outstanding for the channel.
     *
     * @param maxOutstandingPosts the max outstanding posts
     */
    public void maxOutstandingPosts(int maxOutstandingPosts)
    {
        _maxOutstandingPosts = maxOutstandingPosts;
    }

    /**
     * Time a stream will wait for acknowledgment of an on-stream post, in milliseconds.
     * 
     * @return the post acknowledgment timeout
     */
    public int postAckTimeout()
    {
        return _postAckTimeout;
    }

    /**
     * Time a stream will wait for acknowledgment of an on-stream post, in milliseconds.
     *
     * @param postAckTimeout the post ack timeout
     */
    public void postAckTimeout(int postAckTimeout)
    {
        _postAckTimeout = postAckTimeout;
    }
    
    /**
     * Time a requested stream will wait for a response from the provider, in milliseconds.
     * 
     * @return the request timeout
     */
    public int requestTimeout()
    {
        return _requestTimeout;
    }

    /**
     * Time a requested stream will wait for a response from the provider, in milliseconds.
     *
     * @param requestTimeout the request timeout
     */
    public void requestTimeout(int requestTimeout)
    {
        _requestTimeout = requestTimeout;
    }
        
    
    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
        _enableWatchlist = false;
        _channelOpenCallback = null;
        _itemCountHint = 0;
        _obeyOpenWindow = true;
        _maxOutstandingPosts = 100000;
        _postAckTimeout = 15000;
        _requestTimeout = 15000;
    }

    /*
     * Performs a deep copy from a specified ConsumerWatchlistOptions into this ConsumerWatchlistOptions.
     */
    void copy(ConsumerWatchlistOptions watchlistOptions)
    {
        _enableWatchlist = watchlistOptions.enableWatchlist();
        _channelOpenCallback = watchlistOptions.channelOpenCallback();
        _itemCountHint = watchlistOptions.itemCountHint();
        _obeyOpenWindow = watchlistOptions.obeyOpenWindow();
        _maxOutstandingPosts = watchlistOptions.maxOutstandingPosts();
        _postAckTimeout = watchlistOptions.postAckTimeout();
        _requestTimeout = watchlistOptions.requestTimeout();
    }
}
