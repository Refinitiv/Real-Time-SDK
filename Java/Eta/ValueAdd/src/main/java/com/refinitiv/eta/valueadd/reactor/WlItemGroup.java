/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.HashMap;
import java.util.LinkedList;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.valueadd.common.VaNode;

/* Watchlist Item Group ID of a group of streams. */
class WlItemGroup extends VaNode
{
	WlService _wlService;
	Buffer _groupId;
	LinkedList<WlStream> _openStreamList;
	
    /* Table of watchlist streams, by watchlist stream id. Improves lookup when adding WlStreams to existing group. */
    HashMap<WlInteger,WlStream> _streamIdToItemGroupTable;
	
	WlItemGroup()
	{
		_openStreamList = new LinkedList<WlStream>();
		_streamIdToItemGroupTable = new HashMap<WlInteger, WlStream>();
	}
	
	void wlService(WlService service)
	{
		_wlService = service;
	}
	
	WlService wlService()
	{
		return _wlService;
	}
	
	void groupId(Buffer groupId)
	{
		_groupId = groupId;
	}
	
	Buffer groupId()
	{
		return _groupId;
	}
	
	LinkedList<WlStream> openStreamList()
	{
		return _openStreamList;
	}

	HashMap<WlInteger,WlStream> streamIdToItemGroupTable()
    {
        return _streamIdToItemGroupTable;
    }
	
	void clear()
	{
		_wlService = null;
		_groupId = null;
		_openStreamList.clear();
		_streamIdToItemGroupTable.clear();
	}
	
	@Override
	public void returnToPool()
	{
		_wlService = null;
		_groupId = null;
    	
    		super.returnToPool();
	}
}
