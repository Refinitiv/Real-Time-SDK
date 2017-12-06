package com.thomsonreuters.upa.valueadd.reactor;

import java.util.ArrayList;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.valueadd.common.VaNode;

/* Watchlist user request that contains user request information. */
class WlRequest extends VaNode
{
    RequestMsg _requestMsg = (RequestMsg)CodecFactory.createMsg();  
    WlHandler _handler;
    WlStream _stream;
    WatchlistStreamInfo _streamInfo = new WatchlistStreamInfo();
    State _state = State.PENDING_REQUEST;
    boolean _hasStaticQos;
    boolean _hasBehaviour;
    int _symbolListFlags;
    boolean _providerDriven;
    boolean _initialResponseReceived;
    boolean _hasServiceId;
    long _serviceId;
    WlView _view;
    ArrayList<Integer> _viewFieldIdList;
    ArrayList<String> _viewElementNameList;
    int _viewElemCount;    
    int _viewType;
    int _viewAction;
    boolean _reissue_hasChange = false;
    boolean _reissue_hasViewChange = false;
       
    Qos _matchedQos = CodecFactory.createQos();
    
    WlInteger _tableKey;
    
    WlRequest()
    {
        _requestMsg.msgClass(MsgClasses.REQUEST);
        _providerDriven = false;
        _initialResponseReceived = false;
        _hasServiceId = false;
        _serviceId = 0;
        _reissue_hasChange = false;
    }
    
    /* The state of the watchlist request. */
    enum State
    {
        RETURN_TO_POOL,
        PENDING_REQUEST,
        REFRESH_PENDING,
        REFRESH_VIEW_PENDING, 
        REFRESH_COMPLETE_PENDING, // multi-part
        OPEN
    }
    
    /* Returns the request message. */
    RequestMsg requestMsg()
    {
        return _requestMsg;
    }
    
    /* Handler associated with request. */
    WlHandler handler()
    {
        return _handler;
    }
    
    /* Set the handler associated with request. */
    void handler(WlHandler handler)
    {
        _handler = handler;
    }

    /* Stream associated with request. */
    WlStream stream()
    {
        return _stream;
    }
    
    /* Set the stream associated with request. */
    void stream(WlStream stream)
    {
        _stream = stream;
    }

    /* Returns whether or not request has a static Qos. */
    boolean hasStaticQos()
    {
        return _hasStaticQos;
    }

    /* Sets whether or not request has a static Qos. If this is set,
     * then the matchedQos is the static Qos. */
    void hasStaticQos(boolean hasStaticQos)
    {
        _hasStaticQos = hasStaticQos;
    }

    /* Matched Qos of the request. This is the static Qos if
     * hasStaticQos is set. */
    Qos matchedQos()
    {
        return _matchedQos;
    }
    
    /* Stream information for the stream. */
    WatchlistStreamInfo streamInfo()
    {
        return _streamInfo;
    }

    /* Sets the state of the user request. */
    void state(State state)
    {
        _state = state;
    }

    /* Returns the state of the user request. */
    State state()
    {
        return _state;
    }

    /*
     * return whether has symbolList data stream behavior    
     */
    public boolean hasBehaviour()
    {
		return _hasBehaviour;
	}
    /*
     * set symbolList data stream behavior flag
     */
	public void hasBehaviour(boolean hasBehaviour)
	{
		_hasBehaviour = hasBehaviour;
	}

	/*
	 * return symbolList data flags
	 */
	public int symbolListFlags()
	{
		return _symbolListFlags;
	}

	/*
	 *  set symbolList data stream flags
	 */
	public void symbolListFlags(int symbolListFlags)
	{
		_symbolListFlags = symbolListFlags;
	}

	/*
     * return whether the request is provider driven    
     */
    public boolean providerDriven()
    {
		return _providerDriven;
	}
    /*
     * set request as provider driven
     */
	public void providerDriven(boolean providerDriven)
	{
		_providerDriven = providerDriven;
	}

	/*
     * Indicates whether the request knows its service ID.
     */
    public boolean hasServiceId()
    {
		return _hasServiceId;
	}

    /*
     * Set serviceID of request.
     */
	public void serviceId(long serviceId)
	{
        _serviceId = serviceId;
        _hasServiceId = true;
	}

    /*
     * Return serviceId request is using.
     */
	public long serviceId()
	{
        return _serviceId;
	}

    /*
     * Unset serviceID of request.
     */
	public void unsetServiceId()
	{
        _serviceId = 0;
        _hasServiceId = false;
	}

	/*
     * return whether the request has received its initial response.
     * Mainly intended for provider driven streams to determine whether a msgKey may need to be added.
     */
    public boolean initialResponseReceived()
    {
		return _initialResponseReceived;
	}

    /*
     * Set whether the request has received its initial response.
     */
	public void initialResponseReceived(boolean initialResponseReceived)
	{
		_initialResponseReceived = initialResponseReceived;
	}
		
	/*
	 * return type of view
	 */	
    public int viewType() 
    {
		return _viewType;
	}

	/*
	 * set type of view
	 */	
	public void viewType(int viewType)
	{
		this._viewType = viewType;
	}

	/*
	 * return view action
	 */		
	public int viewAction()
	{
		return _viewAction;
	}

	/*
	 * set view action
	 */	
	public void viewAction(int viewAction) 
	{
		this._viewAction = viewAction;
	}

	void tableKey(WlInteger tableKey)
    {
        _tableKey = tableKey;
    }
    
    WlInteger tableKey()
    {
        return _tableKey;
    }
    
    WlView view()
    {
    	return _view;
    }
    
    void view(WlView view)
    {
    	_view = view;
    }
      
	public ArrayList<Integer> viewFieldIdList() 
	{
		return _viewFieldIdList;
	}

	public void viewFieldIdList(ArrayList<Integer> viewFieldIdList)
	{
		_viewFieldIdList = viewFieldIdList;
	}

	public ArrayList<String> viewElementNameList()
	{
		return _viewElementNameList;
	}

	public void viewElementNameList(ArrayList<String> viewElementNameList)
	{
		_viewElementNameList = viewElementNameList;
	}

	int viewElemCount()
	{
		return _viewElemCount;
	}

	void viewElemCount(int viewElemCount)
	{
		_viewElemCount = viewElemCount;
	}
    
	/* Clears the object for re-use. */
    void clear()
    {
        assert (_state == State.RETURN_TO_POOL);
        
        _requestMsg.clear();
        _requestMsg.msgClass(MsgClasses.REQUEST);
        _handler = null;
        _stream = null;
        _streamInfo.clear();
        _state = State.PENDING_REQUEST;
        _hasStaticQos = false;
        _matchedQos.clear();
        _hasBehaviour = false;
        _providerDriven = false;
        _serviceId = 0;
        _hasServiceId = false;
        _initialResponseReceived = false;
        _symbolListFlags = 0;
        _tableKey = null;
        _viewElemCount = 0;
        _viewType = 0;
    }
}
