package com.refinitiv.eta.transport;

class WSocketOptsImpl implements WSocketOpts {
	
	private String _protocolList = "";
	private long _maxMsgSize = 61440;
	private HttpCallback httpCallback;

	@Override
	public void protocols(String protocolList) {
		_protocolList = protocolList.toLowerCase();
	}

	@Override
	public String protocols() {
		return _protocolList;
	}

	@Override
	public void maxMsgSize(long maxMsgSize) {
		_maxMsgSize = maxMsgSize;
	}

	@Override
	public long maxMsgSize() {
		return _maxMsgSize;
	}

	@Override
	public void httpCallback(HttpCallback httpCallback) {
		this.httpCallback = httpCallback;
	}

	@Override
	public HttpCallback httpCallback() {
		return httpCallback;
	}

	void copy(WSocketOptsImpl destOpts)
	{
		destOpts.protocols(_protocolList);
		destOpts.maxMsgSize(_maxMsgSize);
	}

	@Override
	public void clear() {
		this._protocolList = "";
		_maxMsgSize = 61440L;
		httpCallback = null;
	}

	@Override
    public String toString()
    {
        return "WSocketOpts" + "\n" + 
               "\t\tprotocols: " + _protocolList + 
               "\t\tmaxMsgSize: " + _maxMsgSize;
    }
}
