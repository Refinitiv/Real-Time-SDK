/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

public class ReactorChannelStatsImpl implements ReactorChannelStats {

	private int _pingsReceived = 0;
	private int _pingsSent = 0;
	private int _bytesWritten = 0;
	private int _uncompressedBytesWritten = 0;
	private int _bytesRead = 0;
	private int _uncompressedBytesRead = 0;
	
	ReactorChannelStatsImpl()
    {
    }
	
	@Override
	public int pingsReceived() {
		return _pingsReceived;
	}

	@Override
	public int pingsSent() {
		return _pingsSent;
	}

	@Override
	public int bytesWritten() {
		return _bytesWritten;
	}

	@Override
	public int uncompressedBytesWritten() {
		return _uncompressedBytesWritten;
	}

	@Override
	public int bytesRead() {
		return _bytesRead;
	}

	@Override
	public int uncompressedBytesRead() {
		return _uncompressedBytesRead;
	}

	@Override
	public void pingsReceived(int pingsReceived) {
		_pingsReceived = pingsReceived;
	}

	@Override
	public void pingsSent(int pingsSent) {
		_pingsSent = pingsSent;
	}

	@Override
	public void bytesWritten(int bytesWritten) {
		_bytesWritten = bytesWritten;
	}

	@Override
	public void uncompressedBytesWritten(int uncompressedBytesWritten) {
		_uncompressedBytesWritten = uncompressedBytesWritten;
	}

	@Override
	public void bytesRead(int bytesRead) {
		_bytesRead = bytesRead;
	}

	@Override
	public void uncompressedBytesRead(int uncompressedBytesRead) {
		_uncompressedBytesRead = uncompressedBytesRead;
	}

	@Override
	public void clear() {
		_pingsReceived = 0;
		_pingsSent = 0;
		_bytesWritten = 0;
		_uncompressedBytesWritten = 0;
		_bytesRead = 0;
		_uncompressedBytesRead = 0;
	}
}
