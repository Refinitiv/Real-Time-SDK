/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

class WSocketOptsImpl implements WSocketOpts {
	
	private String _protocolList = "";
	private long _maxMsgSize = 61440;
	private HttpCallback _httpCallback;

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
		_httpCallback = httpCallback;
	}

	@Override
	public HttpCallback httpCallback() {
		return _httpCallback;
	}

	void copy(WSocketOptsImpl destOpts)
	{
		destOpts.protocols(_protocolList);
		destOpts.maxMsgSize(_maxMsgSize);
		destOpts.httpCallback(_httpCallback);
	}

	@Override
	public void clear() {
		this._protocolList = "";
		_maxMsgSize = 61440L;
		_httpCallback = null;
	}

	@Override
    public String toString()
    {
        return "WSocketOpts" + "\n" + 
               "\t\tprotocols: " + _protocolList + 
               "\t\tmaxMsgSize: " + _maxMsgSize +
               "\t\thttpCallback: " + (_httpCallback != null);
    }
}
