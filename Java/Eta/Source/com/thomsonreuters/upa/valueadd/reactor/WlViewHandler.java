package com.thomsonreuters.upa.valueadd.reactor;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;

import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.ViewTypes;

public class WlViewHandler 
{	
	private ElementList elementList = CodecFactory.createElementList();
	private ElementEntry elementEntry = CodecFactory.createElementEntry();
	private Array viewArray = CodecFactory.createArray();
	private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
	private Int tempInt = CodecFactory.createInt();
	private UInt tempUInt = CodecFactory.createUInt();	
    private Buffer elementNameBuf = CodecFactory.createBuffer();
    Watchlist _watchlist;
	
    LinkedList<ArrayList<Integer>> _viewFieldIdListPool = new LinkedList<ArrayList<Integer>>();
    LinkedList<ArrayList<String>> _viewElementNameListPool = new LinkedList<ArrayList<String>>();
    LinkedList<LinkedList<WlView>> _mergedViewsPool = new LinkedList<LinkedList<WlView>>();
    LinkedList<LinkedList<WlView>> _committedViewsPool = new LinkedList<LinkedList<WlView>>();
    LinkedList<LinkedList<WlView>> _newViewsPool = new LinkedList<LinkedList<WlView>>();   
    LinkedList<HashMap<Integer, Integer>> _viewFieldIdCountMapPool = new LinkedList<HashMap<Integer, Integer>>();
    LinkedList<HashMap<String, Integer>> _viewElementNameCountMapPool = new LinkedList<HashMap<String, Integer>>();

    WlViewHandler(Watchlist watchlist)
    {
    	_watchlist = watchlist;
    }
            
	WlView viewCreate(ArrayList<Integer> fieldIdList, ArrayList<String> elementNameList, int elemCount, int viewType, ReactorErrorInfo errorInfo)
	{
		switch(viewType)
		{		
			case ViewTypes.FIELD_ID_LIST:
			{
				boolean hasDuplicateFields = false;
				WlView wlView = ReactorFactory.createWlView();
				wlView.viewHandler(this);
				wlView.viewType(viewType);
			    wlView.elemCount(elemCount);
			    wlView.state(WlView.State.NEW);
				wlView._fieldIdList = fieldIdList;
				
				if (wlView._fieldIdList != null && wlView.elemCount() > 0 )
				{
					Collections.sort(wlView._fieldIdList);
					if ( elemCount > 0 && wlView._fieldIdList.get(0).equals(0) )
						hasDuplicateFields = true;
					for (int i = 1; i < wlView._fieldIdList.size(); i++)
					{
						if (!wlView._fieldIdList.get(i).equals(0) &&
                           wlView._fieldIdList.get(i).equals(wlView._fieldIdList.get(i-1)))
						{
							wlView._fieldIdList.set(i, 0);
							hasDuplicateFields = true;
						}							   
					}
				
					if (hasDuplicateFields)
					{
						for (int i = 0; i < wlView._elemCount;)
						{
							if (wlView._fieldIdList.get(i).equals(0))
							{
								wlView._fieldIdList.set(i, wlView._fieldIdList.get(wlView._elemCount-1));
								wlView._elemCount --;
							}
							else
								++i;
						}						
					
						Collections.sort(wlView._fieldIdList.subList(0, wlView._elemCount));
					}
					return wlView;				
				}			
				else
				{
				    wlView.elemCount(0);
				    return wlView;
				}
			}
			case ViewTypes.ELEMENT_NAME_LIST:
			{
				boolean hasDuplicateFields = false;
				WlView wlView = ReactorFactory.createWlView();
				wlView.viewHandler(this);
				wlView.viewType(viewType);
			    wlView.elemCount(elemCount);   
			    wlView.state(WlView.State.NEW);
				wlView._elementNameList = elementNameList;
				
				if (wlView._elementNameList != null && wlView.elemCount() > 0 )
				{
					Collections.sort(wlView._elementNameList);

					for (int i = 1; i < wlView._elementNameList.size(); i++)
					{
						if (wlView._elementNameList.get(i) != null &&
                           wlView._elementNameList.get(i).equals(wlView._elementNameList.get(i-1)))
						{
							wlView._elementNameList.set(i, null);
							hasDuplicateFields = true;
						}							   
					}				
					if (hasDuplicateFields)
					{
						for (int i = 0; i < wlView._elemCount;)
						{
							if (wlView._elementNameList.get(i) == null)
							{
								wlView._elementNameList.set(i, wlView._elementNameList.get(wlView._elemCount-1));
								wlView._elemCount --;
							}
							else
								++i;
						}						
					}					
					Collections.sort(wlView._elementNameList.subList(0, wlView._elemCount));
					return wlView;				
				}			
				else
				{
				    wlView.elemCount(0);
				    return wlView;
				}	
			}
			default:			
			{
				_watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.FAILURE, "ViewHandder",
						"Invalid ViewType  <" + viewType + ">");
					
				return null;		
			}// case			
		}// switch		
	}
	
	WlView aggregateViewCreate(WlView view, ReactorErrorInfo errorInfo)
	{
		WlView aggView = ReactorFactory.createWlView();
		aggView.aggregated(true);
		aggView.viewHandler(this);

		aggView._newViews = _newViewsPool.poll();
		if (aggView._newViews == null) aggView._newViews = new LinkedList<WlView>();
		aggView._mergedViews = _mergedViewsPool.poll();
		if (aggView._mergedViews == null) aggView._mergedViews = new LinkedList<WlView>();
		aggView._committedViews = _committedViewsPool.poll();
		if (aggView._committedViews == null) aggView._committedViews = new LinkedList<WlView>();
		
		if ( view.viewType() == ViewTypes.FIELD_ID_LIST)
		{
			aggView._viewFieldIdCountMap = _viewFieldIdCountMapPool.poll();
			if (aggView._viewFieldIdCountMap == null) aggView._viewFieldIdCountMap = new HashMap<Integer, Integer>();
			aggView._viewFieldIdCountMap.clear();
			for (Integer fieldId : view.fieldIdList().subList(0, view.elemCount()))
			{
				aggView._viewFieldIdCountMap.put(fieldId,  1);
			}
		}
		else if ( view.viewType() == ViewTypes.ELEMENT_NAME_LIST)
		{
			aggView._viewElementNameCountMap = _viewElementNameCountMapPool.poll();
			if (aggView._viewElementNameCountMap == null) aggView._viewElementNameCountMap = new HashMap<String, Integer>();
			aggView._viewElementNameCountMap.clear();
			for (String elementName : view.elementNameList().subList(0, view.elemCount()))
			{
				aggView._viewElementNameCountMap.put(elementName,  1);
			}
		}
		aggView.mergedViews().add(view);
		view.state(WlView.State.MERGED);
		
		switch(view.viewType())
		{		
		    // first item in the stream aggview
			case ViewTypes.FIELD_ID_LIST:
			{
				aggView.viewHandler(this);
				aggView.viewType(view.viewType());
			    aggView.elemCount(view.elemCount());
				aggView._fieldIdList = _viewFieldIdListPool.poll();
				if (aggView._fieldIdList == null) aggView._fieldIdList = new ArrayList<Integer>();				
				aggView._fieldIdList.addAll(view.fieldIdList());
				break;
			}
			case ViewTypes.ELEMENT_NAME_LIST:
			{
				aggView.viewHandler(this);
				aggView.viewType(view.viewType());
			    aggView.elemCount(view.elemCount());
				aggView._elementNameList = _viewElementNameListPool.poll();
				if (aggView._elementNameList == null) aggView._elementNameList = new ArrayList<String>();	
				aggView._elementNameList.addAll(view.elementNameList());
				break;
			}
			default:
				_watchlist.reactor().populateErrorInfo(errorInfo,
						ReactorReturnCodes.FAILURE, "ViewHandder",
						"Invalid ViewType, cannot create aggregate view  <" + view.viewType() + ">");
					
				return null;		
		}
		
 				
		return aggView;
	}
	
	int aggregateViewAdd(WlView aggView, WlView view, ReactorErrorInfo errorInfo)
	{
		if(view.viewType() != aggView.viewType())
		{
			_watchlist.reactor().populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE, "ItemHandder",
					"ViewType mismatch, cannot add to aggregated view");
			return CodecReturnCodes.FAILURE;
		}
						
		aggView._newViews.add(view);
		return CodecReturnCodes.SUCCESS;
	}
		
	int aggregateViewMerge(RequestMsg streamRequestMsg, WlView aggView)
	{	
	   	for (WlView view = aggView.newViews().poll(); view!= null; view = aggView.newViews().poll())
    	{		
			switch(view.viewType())
			{		
				case ViewTypes.FIELD_ID_LIST:
				{
					ArrayList<Integer> aggViewFieldIdList = aggView.fieldIdList();
				
					if (aggViewFieldIdList == null)
					{
						aggViewFieldIdList = _viewFieldIdListPool.poll();
						if (aggViewFieldIdList == null ) aggViewFieldIdList = new ArrayList<Integer>();
						else 
							aggViewFieldIdList.clear();
							
						aggView._elemCount = 0;
					}
				
					for (int i = 0; i < view.fieldIdList().subList(0, view.elemCount()).size(); i++)
					{					
						Integer fid = view.fieldIdList().get(i);
						int index = Collections.binarySearch(aggViewFieldIdList.subList(0, aggView._elemCount), fid);
						if (index >= 0)
						{
							Integer count = aggView._viewFieldIdCountMap.get(fid);
							aggView._viewFieldIdCountMap.put(fid, ++count);
						}
						else
						{
							if (aggView.elemCount() < aggViewFieldIdList.size())
							{
								aggViewFieldIdList.set(aggView._elemCount, fid);
							}
							else 
								aggViewFieldIdList.add(fid);

							aggView._viewFieldIdCountMap.put(fid, 1);
							aggView._elemCount++;				    		
						}
					}
					Collections.sort(aggViewFieldIdList.subList(0, aggView._elemCount));
               
					break;
				}
				case ViewTypes.ELEMENT_NAME_LIST:
				{
					ArrayList<String> aggViewElementNameList = aggView.elementNameList();
					
					if (aggViewElementNameList == null)
					{
						aggViewElementNameList = _viewElementNameListPool.poll();
						if (aggViewElementNameList == null ) aggViewElementNameList = new ArrayList<String>();
						else 
							aggViewElementNameList.clear();
						aggView._elemCount = 0;
					}
				
					for (int i = 0; i < view.elementNameList().subList(0, view.elemCount()).size(); i++)
					{						
					    String elementName = view.elementNameList().get(i);
						int index = Collections.binarySearch(aggViewElementNameList.subList(0, aggView._elemCount), elementName);
						if (index >= 0)
						{
							Integer count = aggView._viewElementNameCountMap.get(elementName);
							aggView._viewElementNameCountMap.put(elementName, ++count);
						}
						else
						{
							if (aggView.elemCount() < aggViewElementNameList.size())
							{
								aggViewElementNameList.set(aggView._elemCount, elementName);
							}
							else 
								aggViewElementNameList.add(elementName);

							aggView._viewElementNameCountMap.put(elementName, 1);
							aggView._elemCount++;				    		
						}
					}
					Collections.sort(aggViewElementNameList.subList(0, aggView._elemCount));               						
				}
			}// switch
			aggView.mergedViews().add(view);
			view.state(WlView.State.MERGED);
    	}
		return CodecReturnCodes.SUCCESS;
	}	

	
	int removeRequestView(WlStream wlStream, WlRequest wlRequest, ReactorErrorInfo errorInfo)
	{
		if (!wlRequest.requestMsg().checkHasView())
		{			
			// need to re-evaluate the wlStream request views
			// as wlStream._userRequestList size will decrease 
			wlStream._pendingViewChange = true;
			return CodecReturnCodes.SUCCESS;
		}
					
		WlView view = wlRequest.view();
				
		wlStream._requestsWithViewCount--;
	
		WlView aggView = wlStream.aggregateView();
		
		if (view.state() == WlView.State.NEW)
		{
			wlStream.aggregateView().newViews().remove(view);
			return CodecReturnCodes.SUCCESS;
		}
		else if (view.state() == WlView.State.MERGED)
		{
			wlStream.aggregateView().mergedViews().remove(view);
			
		}
		else if (view.state() == WlView.State.COMMITTED)
		{
			wlStream.aggregateView().committedViews().remove(view);
		}
		
		if(view.state() == WlView.State.MERGED || view.state() == WlView.State.COMMITTED)
		{
			switch(view.viewType())
			{		
				case ViewTypes.FIELD_ID_LIST:
				{
					ArrayList<Integer> aggViewFieldIdList = aggView.fieldIdList();
								
					for (int i = 0; i < view.fieldIdList().subList(0, view.elemCount()).size(); i++)
					{						
						Integer fid = view.fieldIdList().get(i);
						int index = Collections.binarySearch(aggViewFieldIdList.subList(0, aggView._elemCount), fid);
						if ( index >= 0) 
						{
					    	Integer count = aggView._viewFieldIdCountMap.get(fid);
					    	aggView._viewFieldIdCountMap.put(fid, --count);
						}
						else
						{
							_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ViewHandder",
									"Aggregate View cannot remove a non-existent field id  <" + fid + ">");
							 
						}
					}
					if(view.state() == WlView.State.MERGED)
					{
					// so if the fid commited, cannot remove, just for cleanup, in case needed again 
						boolean needResort = false;
						for (int i = 0; i < aggViewFieldIdList.subList(0, aggView.elemCount()).size(); i++)
						{						
							if (aggView._viewFieldIdCountMap.get(aggViewFieldIdList.get(i)) == 0 ) 
							{
								aggViewFieldIdList.set(i, aggViewFieldIdList.get(aggView.elemCount()-1));
								--aggView._elemCount;
								needResort = true;
							}
						}
					
						if (needResort)
							Collections.sort(aggViewFieldIdList.subList(0, aggView._elemCount));					
					}
					break;
				}
			
				case ViewTypes.ELEMENT_NAME_LIST:
				{
					ArrayList<String> aggViewElementNameList = aggView.elementNameList();
								
					for (int i = 0; i < view.elementNameList().subList(0, view.elemCount()-1).size(); i++)
					{						
						String elementName = view.elementNameList().get(i);
						int index = Collections.binarySearch(aggViewElementNameList.subList(0, aggView._elemCount-1), elementName);
						if ( index >= 0) 
						{
					    	Integer count = aggView._viewElementNameCountMap.get(elementName);
					    	aggView._viewElementNameCountMap.put(elementName, --count);
						}
						else
						{
							_watchlist.reactor().populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE, "ViewHandder",
									"Aggregate View cannot remove a non-existent elementName  <" + elementName + ">");
						}
					}
					if(view.state() == WlView.State.MERGED)
					{
						// so if the fid commited, cannot remove then, just for cleanup, in case needed again 
						boolean needResort = false;
						for (int i = 0; i < aggViewElementNameList.subList(0, aggView.elemCount()-1).size(); i++)
						{						
							if (aggView._viewElementNameCountMap.get(aggViewElementNameList.get(i)) == 0 ) 
							{
								aggViewElementNameList.set(i, aggViewElementNameList.get(aggView.elemCount()-1));
								--aggView._elemCount;
								needResort = true;
							}
						}
					
						if (needResort)
							Collections.sort(aggViewElementNameList.subList(0, aggView._elemCount-1));					
					}
		        
					break;
				}
		
			}		
		}
		return CodecReturnCodes.SUCCESS;
	}
	
	
	void aggregateViewCommit(WlView aggView)
	{			
	   	for (WlView view = aggView.mergedViews().poll(); view!= null; view = aggView.mergedViews().poll())
    	{			   		
	   		aggView.committedViews().add(view);
	   	    view.state(WlView.State.COMMITTED);
    	}

		switch(aggView.viewType())
		{		
			case ViewTypes.FIELD_ID_LIST:
			{
				ArrayList<Integer> aggViewFieldIdList = aggView.fieldIdList();

				boolean needResort = false;
				for (int i = 0; i < aggViewFieldIdList.subList(0, aggView.elemCount()).size(); i++)
				{						
					if (aggView._viewFieldIdCountMap.get(aggViewFieldIdList.get(i)) == 0 ) 
					{
						aggViewFieldIdList.set(i, aggViewFieldIdList.get(aggView.elemCount()-1));
						--aggView._elemCount;
						needResort = true;
					}
				}
	
				if (needResort)
					Collections.sort(aggViewFieldIdList.subList(0, aggView._elemCount));
				break;
			}
			case ViewTypes.ELEMENT_NAME_LIST:
			{
				ArrayList<String> aggViewElementNameList = aggView.elementNameList();
					
				boolean needResort = false;
				for (int i = 0; i < aggViewElementNameList.subList(0, aggView.elemCount()).size(); i++)
				{						
					if (aggView._viewElementNameCountMap.get(aggViewElementNameList.get(i)) == 0 ) 
					{
						aggViewElementNameList.set(i, aggViewElementNameList.get(aggView.elemCount()-1));
						--aggView._elemCount;
						needResort = true;
					}
				}
				
				if (needResort)
					Collections.sort(aggViewElementNameList.subList(0, aggView._elemCount));				
				
				break;
			}
			default:
				break;							
		}	   	
	}	
	
	int encodeViewRequest(EncodeIterator encodeIter, WlView aggView)
	{
		elementList.clear();
		elementList.applyHasStandardData();

		int ret = elementList.encodeInit(encodeIter, null, 0);
				
		switch(aggView.viewType())
		{		
			case ViewTypes.FIELD_ID_LIST:
			{
		
				elementEntry.clear();
				elementEntry.name(ElementNames.VIEW_TYPE);
				elementEntry.dataType(DataTypes.UINT);
				tempUInt.value(ViewTypes.FIELD_ID_LIST);
				ret = elementEntry.encode(encodeIter, tempUInt);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}	
		
				elementEntry.clear();
				elementEntry.name(ElementNames.VIEW_DATA);
				elementEntry.dataType(DataTypes.ARRAY);
				if ((ret = elementEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}

				viewArray.primitiveType(DataTypes.INT);
				viewArray.itemLength(2);

				if ((ret = viewArray.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}	

				for (Integer viewField : aggView.fieldIdList().subList(0, aggView.elemCount()))
				{

					if (aggView._viewFieldIdCountMap.get(viewField) == null || aggView._viewFieldIdCountMap.get(viewField) == 0 ) continue;
								
					arrayEntry.clear();
					tempInt.value(viewField);
					ret = arrayEntry.encode(encodeIter, tempInt);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						return ret;
					}	
				}
				ret = viewArray.encodeComplete(encodeIter, true);

				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}	

				ret = elementEntry.encodeComplete(encodeIter, true);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}
				ret = elementList.encodeComplete(encodeIter, true);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
			    }	
				break;
			}
			case ViewTypes.ELEMENT_NAME_LIST:
			{
				elementEntry.clear();
				elementEntry.name(ElementNames.VIEW_TYPE);
				elementEntry.dataType(DataTypes.UINT);
				tempUInt.value(ViewTypes.ELEMENT_NAME_LIST);
				ret = elementEntry.encode(encodeIter, tempUInt);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}	
		
				elementEntry.clear();
				elementEntry.name(ElementNames.VIEW_DATA);
				elementEntry.dataType(DataTypes.ARRAY);
				if ((ret = elementEntry.encodeInit(encodeIter, 0)) < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}

				viewArray.primitiveType(DataTypes.ASCII_STRING);
//				viewArray.itemLength(0);   or what

				if ((ret = viewArray.encodeInit(encodeIter)) < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}	

				for (String elementName : aggView.elementNameList())
				{
					if (aggView._viewElementNameCountMap.get(elementName) == null || aggView._viewElementNameCountMap.get(elementName) == 0 ) continue;

					arrayEntry.clear();					
		            elementNameBuf.data(elementName);
		            ret = arrayEntry.encode(encodeIter, elementNameBuf);
					
					if (ret < CodecReturnCodes.SUCCESS)
					{
						return ret;
					}	
				}
				ret = viewArray.encodeComplete(encodeIter, true);

				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}	

				ret = elementEntry.encodeComplete(encodeIter, true);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
				}
				ret = elementList.encodeComplete(encodeIter, true);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					return ret;
			    }	
				break;			
			}
			default:
				break;
				
		}
		return CodecReturnCodes.SUCCESS;
	}	
	
	void aggregateViewDestroy(WlView aggView)
	{
		_viewFieldIdCountMapPool.add(aggView._viewFieldIdCountMap);	
		_viewElementNameCountMapPool.add(aggView._viewElementNameCountMap);	
		
		_newViewsPool.add(aggView._newViews);
		_mergedViewsPool.add(aggView._mergedViews);
		_committedViewsPool.add(aggView._committedViews);
		
		_viewFieldIdListPool.add(aggView._fieldIdList);
		_viewElementNameListPool.add(aggView._elementNameList);
		aggView.returnToPool();
	}
}
