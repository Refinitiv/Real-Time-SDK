/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.valueadd.common.VaNode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;

public class WlView extends VaNode
{
	WlViewHandler _viewHandler;
	int _viewType;
	int _elemCount;
	ArrayList<Integer> _fieldIdList;
	ArrayList<String> _elementNameList;
	State _state = State.NEW;

	enum State
    {
        NEW,
        MERGED,
        COMMITTED
    }

        
	boolean _aggregated; 
	LinkedList<WlView> _newViews;
	LinkedList<WlView> _mergedViews;
	LinkedList<WlView> _committedViews;
	HashMap<Integer, Integer> _viewFieldIdCountMap;
	HashMap<String, Integer> _viewElementNameCountMap;

	
	public WlViewHandler viewHandler()
	{
		return _viewHandler;
	}

	public void viewHandler(WlViewHandler viewHandler) 
	{
		_viewHandler = viewHandler;
	}

	public int viewType()
	{
		return _viewType;
	}

	public void viewType(int viewType) 
	{
		this._viewType = viewType;
	}

	public int elemCount()
	{
		return _elemCount;
	}

	public void elemCount(int elemCount)
	{
		this._elemCount = elemCount;
	}

	public boolean aggregated()
	{
		return _aggregated;
	}

	public void aggregated(boolean aggregated)
	{
		this._aggregated = aggregated;
	}
	
	public ArrayList<Integer> fieldIdList()
	{
		return _fieldIdList;
	}

	public void fieldIdList(ArrayList<Integer> fieldIdList)
	{
		_fieldIdList = fieldIdList;
	}

	public ArrayList<String> elementNameList()
	{
		return _elementNameList;
	}

	public void elementNameList(ArrayList<String> elementNameList)
	{
		_elementNameList = elementNameList;
	}
	
	public State state()
	{
		return _state;
	}

	public void state(State state)
	{
		_state = state;
	}

	public LinkedList<WlView> newViews() 
	{
		return _newViews;
	}

	public void newViews(LinkedList<WlView> newViews)
	{
		_newViews = newViews;
	}

	public LinkedList<WlView> mergedViews()
	{
		return _mergedViews;
	}

	public void mergedViews(LinkedList<WlView> mergedViews)
	{
		_mergedViews = mergedViews;
	}

	public LinkedList<WlView> committedViews() 
	{
		return _committedViews;
	}

	public void committedViews(LinkedList<WlView> committedViews)
	{
		_committedViews = committedViews;
	}

	void clear()
	{
		_elemCount = 0;
	}
	
	@Override
	public void returnToPool()
	{
		/* Clear out all pooled elements */
		_viewFieldIdCountMap = null;
		_viewElementNameCountMap = null;
		_newViews = null;
		_mergedViews = null;
		_committedViews = null;
		_fieldIdList = null;
		_elementNameList = null;

    		super.returnToPool();
    	}
}
