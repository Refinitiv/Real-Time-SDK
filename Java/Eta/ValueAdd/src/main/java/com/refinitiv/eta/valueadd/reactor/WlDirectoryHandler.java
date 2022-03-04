/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.LinkedList;
import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceFlags;

class WlDirectoryHandler implements WlHandler
{
    final long ALL_FILTERS = Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE |
            Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.DATA |
            Directory.ServiceFilterFlags.LINK | Directory.ServiceFilterFlags.LOAD;
    
    Watchlist _watchlist;
    WlStream _stream;
    DirectoryRequest _directoryRequest;
    DirectoryRefresh _directoryRefresh;
    DirectoryStatus _directoryStatus;
    DirectoryUpdate _directoryUpdate, _directoryUpdateCopy;
    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    ReactorSubmitOptions _submitOptions = ReactorFactory.createReactorSubmitOptions();
    Msg _tempMsg = CodecFactory.createMsg();
    StatusMsg _statusMsg;
    Buffer _tempBuffer;
    WlServiceCache _serviceCache;
    RefreshMsg	_tempRefreshMsg;
    DirectoryStatus _tempDirectoryStatus;
    UpdateMsg _tempUpdateMsg;
    boolean _receivedRefresh;
    boolean _roleDirectoryRequestAdded;
    int _directoryStreamId;
    boolean _hasPendingRequest;

    // pool of Service 
    LinkedList<Service> _servicePool = new LinkedList<Service>();
    
    // pool of DirectoryRefresh messages
    LinkedList<DirectoryRefresh> _directoryRefreshPool = new LinkedList<DirectoryRefresh>();
    
    // flag for dispatching requests
    boolean _requestDispatchFlag;
    
    WlInteger _tempWlInteger = ReactorFactory.createWlInteger();
    
    WlDirectoryHandler(Watchlist watchlist)
    {
        _watchlist = watchlist;
        _serviceCache = new WlServiceCache(_watchlist);
        _directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        _directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        _directoryRequest.filter(ALL_FILTERS);
        _directoryRequest.applyStreaming();
        _directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
        _directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        _directoryStatus = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        _directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
        _directoryStatus.applyHasState();
        _directoryStatus.state().code(StateCodes.NONE);
        _directoryStatus.state().text(_tempBuffer);
        _directoryUpdate = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        _directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
        _directoryUpdateCopy = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        _directoryUpdateCopy.rdmMsgType(DirectoryMsgType.UPDATE);
        _statusMsg = (StatusMsg)CodecFactory.createMsg();
        _statusMsg.msgClass(MsgClasses.STATUS);
        _statusMsg.domainType(DomainTypes.SOURCE);
        _statusMsg.applyHasState();
        _statusMsg.state().code(StateCodes.NONE);
        _statusMsg.state().text(_tempBuffer);       
        _tempBuffer = CodecFactory.createBuffer();
        _tempBuffer.data("");
        _tempRefreshMsg = (RefreshMsg)CodecFactory.createMsg();
        _tempDirectoryStatus = (DirectoryStatus)DirectoryMsgFactory.createMsg();
        _tempUpdateMsg = (UpdateMsg)CodecFactory.createMsg();
        _receivedRefresh = false;
        _roleDirectoryRequestAdded = false;
        _hasPendingRequest = false;
        
        // get next id for directory stream from watchlist
        _directoryStreamId = _watchlist.nextStreamId();

        // create stream
        _stream = ReactorFactory.createWlStream();
        _stream.handler(this);
        _stream.watchlist(_watchlist);
        _stream.streamId(_directoryStreamId);
        _stream.domainType(DomainTypes.SOURCE);
    }

    @Override
    public int submitRequest(WlRequest wlRequest, RequestMsg requestMsg, boolean isReissue, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        // Save streamInfo
        wlRequest.streamInfo().serviceName(submitOptions.serviceName());
        wlRequest.streamInfo().userSpecObject(submitOptions.requestMsgOptions().userSpecObj());

        // If not reissue, add watchlist Request to stream userRequestList
        if (!isReissue)
        {
        	_stream.userRequestList().add(wlRequest);
			wlRequest.stream(_stream);
        }
        
        // Queue request message for assembly and dispatch only if the requestMsg wants a refresh and we have a refresh message
        if (!wlRequest.requestMsg().checkNoRefresh() && !_serviceCache._serviceList.isEmpty())
        {
            if (_requestDispatchFlag == false)
            {
                // trigger dispatch only for first add to list, and only if the directory refresh was received
                _watchlist.reactor().sendWatchlistDispatchNowEvent(_watchlist.reactorChannel());
                
                _requestDispatchFlag = true;
            }
        }
        	
        wlRequest.state(WlRequest.State.PENDING_REFRESH);
        
        return ret;
    }

	@Override
    public int submitMsg(WlRequest wlRequest, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret;
        
        switch (msg.msgClass())
        {
            case MsgClasses.CLOSE:
                // remove watchlist request form userRequestList
                _stream.userRequestList().remove(wlRequest);
                
                // close watchlist request
                _watchlist.closeWlRequest(wlRequest);
     
                break;	
            case MsgClasses.GENERIC:
                if (_stream.state().streamState() == StreamStates.OPEN)
                {
                    boolean resetServiceId = false;
                    
                    // replace service id if message submitted with service name 
                    if (submitOptions.serviceName() != null)
                    {
                        if (!((GenericMsg)msg).checkHasMsgKey())
                        {
                            return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                          ReactorReturnCodes.INVALID_USAGE,
                                                                          "WlDirectoryHandler.submitMsg",
                                                                          "Generic message submitted with service name but no message key.");
                            
                        }
                        
                        if ((ret = _watchlist.changeServiceNameToID(((GenericMsg)msg).msgKey(), submitOptions.serviceName(), errorInfo)) < ReactorReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        
                        // set resetServiceId flag
                        resetServiceId = true;
                    }
                    
                    // replace stream id with aggregated stream id
				    msg.streamId(wlRequest.stream().streamId());
                    
	                // send message
	                ret = _stream.sendMsg(msg, submitOptions, errorInfo);
	                
	                // reset service id if necessary
	                if (resetServiceId)
	                {
	                    ((GenericMsg)msg).msgKey().flags(((GenericMsg)msg).msgKey().flags() & ~MsgKeyFlags.HAS_SERVICE_ID);
	                    ((GenericMsg)msg).msgKey().serviceId(0);
	                }              
	                
	                // return if send message not successful
	                if (ret < ReactorReturnCodes.SUCCESS)
	                {
	                    return ret;
	                }
                }
                else
                {
                    // cannot submit generic message when stream is not open
                    return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                                  ReactorReturnCodes.INVALID_USAGE,
                                                                  "WlDirectoryHandler.submitMsg",
                                                                  "Cannot submit GenericMsg when stream not in open state.");
                }
                break;
            default:
                return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.FAILURE,
                                                              "WlDirectoryHandler.submitMsg",
                                                              "Invalid message class (" + msg.msgClass() + ") submitted to Watchlist directory handler");
        }

        return ReactorReturnCodes.SUCCESS;
    }
	
    public void fillDirectoryRefreshFromRequestMsg(DirectoryRefresh directoryRefresh, RequestMsg requestMsg)
    {
	        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
	        
	        directoryRefresh.streamId(requestMsg.streamId());
            if (requestMsg.msgKey().checkHasFilter())
            	directoryRefresh.filter(requestMsg.msgKey().filter());
            else
            	directoryRefresh.filter(_directoryRefresh.filter());

	        directoryRefresh.applySolicited();
	        directoryRefresh.state().dataState(_directoryRefresh.state().dataState());
	        directoryRefresh.state().streamState(_directoryRefresh.state().streamState());
	        
	        if (requestMsg.msgKey().checkHasServiceId())
	        {
	        	directoryRefresh.applyHasServiceId();
	        	directoryRefresh.serviceId(requestMsg.msgKey().serviceId());
	        }
    }
	
    private void fillDirectoryRefreshServiceListFromCache(DirectoryRefresh directoryRefresh, String serviceName) {
    	Service service = null;
    	if (_servicePool.isEmpty())
		{
			service = DirectoryMsgFactory.createService();
		}
    	else
    	{
    		service = _servicePool.poll();
    		service.clear();
    	}
    	
    	if (serviceName != null)
    	{
    		if (_serviceCache.service(serviceName) != null)
    		{
    			_serviceCache.service(serviceName).rdmService().copy(service);
    			directoryRefresh.serviceList().add(service);
    			setFilterFlagsRefresh(directoryRefresh.filter(), directoryRefresh.serviceList().get(0));
    		}
    	}
    	else if (directoryRefresh.checkHasServiceId())
    	{
    		if (_serviceCache.service(directoryRefresh.serviceId()) != null)
    		{
    			_serviceCache.service(directoryRefresh.serviceId()).rdmService().copy(service);
    			directoryRefresh.serviceList().add(service);
    			setFilterFlagsRefresh(directoryRefresh.filter(), directoryRefresh.serviceList().get(0));
    		}
    	}
    	else
    	{
    		for (int i = 0; i < serviceList().size(); ++i)
    		{
    	    	if (_servicePool.isEmpty())
    			{
    	    		service = DirectoryMsgFactory.createService();
    			}
    	    	else
    	    	{
    	    		service = _servicePool.poll();
    	    		service.clear();
    	    	}
    	    	
    			serviceList().get(i).rdmService().copy(service);
    			setFilterFlagsRefresh(directoryRefresh.filter(), service);
    			directoryRefresh.serviceList().add(service);
    		}
    	}
	}
    
    void fillDirectoryUpdateFromRequestMsg(DirectoryUpdate directoryUpdate, WlRequest wlRequest)
    {
        directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
        
        directoryUpdate.streamId(wlRequest.requestMsg().streamId());
        if (!directoryUpdate.checkHasFilter())
        	directoryUpdate.applyHasFilter();
        if (wlRequest.requestMsg().msgKey().checkHasFilter())
        {
          long filter = wlRequest.requestMsg().msgKey().filter();
          if(filter == 0)
          {
            directoryUpdate.filter(ALL_FILTERS);
          }
          else
            directoryUpdate.filter(filter);
        }
        else
        {
          long filter = _directoryUpdate.filter();
          if(filter == 0)
          {
            directoryUpdate.filter(ALL_FILTERS);
          }
          else
            directoryUpdate.filter(filter);
        }
        
        if (wlRequest.requestMsg().msgKey().checkHasServiceId())
        {
        	directoryUpdate.applyHasServiceId();
        	directoryUpdate.serviceId(wlRequest.requestMsg().msgKey().serviceId());
        }
        else if (wlRequest.streamInfo().serviceName() != null)
        {
        	int serviceId = _serviceCache.serviceId(wlRequest.streamInfo().serviceName());
        	if (serviceId >= 0)
        	{
            	directoryUpdate.applyHasServiceId();
            	directoryUpdate.serviceId(serviceId);        		
        	}
        }
    }
	
    void fillDirectoryUpdateServiceListFromUpdateMsgServices(DirectoryUpdate directoryUpdate, List<Service> services)
    {
    	Service service = null;
   		for (int i = 0; i < services.size(); ++i)
		{
   			if ((directoryUpdate.checkHasServiceId() && directoryUpdate.serviceId() == services.get(i).serviceId()) ||
   				!directoryUpdate.checkHasServiceId())
   			{
		    	if (_servicePool.isEmpty())
				{
		    		service = DirectoryMsgFactory.createService();
				}
		    	else
		    	{
		    		service = _servicePool.poll();
		    		service.clear();
		    	}
		    	
		    	services.get(i).copy(service);
				int ret = setFilterFlagsUpdate(directoryUpdate.filter(), service, services.get(i));
				if (services.get(i).action() == MapEntryActions.DELETE ||
					services.get(i).action() == MapEntryActions.ADD)
				{
					directoryUpdate.serviceList().add(service);
				}
				else // UPDATE
				{
					if (ret > 0 || directoryUpdate.filter() == 0)
					{
						directoryUpdate.serviceList().add(service);
					}
					else
					{
						// service shouldn't be added to directory update
						_servicePool.add(service);
					}
				}
   			}
		}
	}

    void setFilterFlagsRefresh(long filter, Service service)
    {
		// One service selected
		if (service.checkHasInfo() == true && 
			((filter & Directory.ServiceFilterFlags.INFO) == 0))
		{
	        service.flags(service.flags() & ~ServiceFlags.HAS_INFO);
		}

		if (service.checkHasData() == true && 
			((filter & Directory.ServiceFilterFlags.DATA) == 0))
		{
		    service.flags(service.flags() & ~ServiceFlags.HAS_DATA);
		}
			
		
		if ((service.groupStateList().size() > 0) && 
			((filter & Directory.ServiceFilterFlags.GROUP) == 0)) 
		{
			service.groupStateList().clear(); // Remove group
		}

		if (service.checkHasLink() == true && 
			((filter & Directory.ServiceFilterFlags.LINK) == 0))
		{
			service.flags(service.flags() & ~ServiceFlags.HAS_LINK);
		}

		if (service.checkHasLoad() == true && 
			((filter & Directory.ServiceFilterFlags.LOAD) == 0))
		{
			service.flags(service.flags() & ~ServiceFlags.HAS_LOAD);
		}

		if (service.checkHasState() == true && 
			((filter & Directory.ServiceFilterFlags.STATE) == 0))
		{
			service.flags(service.flags() & ~ServiceFlags.HAS_STATE);
		}
    }

    int setFilterFlagsUpdate(long filter, Service service, Service serviceReceived)
    {
        int retNumFilters = 0;
        // One service selected
        if (serviceReceived.checkHasInfo() && 
                ((filter & Directory.ServiceFilterFlags.INFO) != 0)) // Apply flag
        {
            service.applyHasInfo();
            retNumFilters++;
        }
        else
        {
        	service.flags(service.flags() & ~Directory.ServiceFilterFlags.INFO);
        }

        if (serviceReceived.checkHasData() && 
                ((filter & Directory.ServiceFilterFlags.DATA) != 0))
        {
            service.applyHasData();
            retNumFilters++;
        }
        else
        {
        	service.flags(service.flags() & ~Directory.ServiceFilterFlags.DATA);
        }
        
        if ((serviceReceived.groupStateList().size() > 0) && 
                ((filter & Directory.ServiceFilterFlags.GROUP) != 0))
        {
            retNumFilters++;
        }
        else
        {
        	service.flags(service.flags() & ~Directory.ServiceFilterFlags.GROUP);
        	service.groupStateList().clear();
        }

        if (serviceReceived.checkHasLink() && 
                ((filter & Directory.ServiceFilterFlags.LINK) != 0))
        {
            service.applyHasLink();
            retNumFilters++;
        }
        else
        {
        	service.flags(service.flags() & ~Directory.ServiceFilterFlags.LINK);
        }

        if (serviceReceived.checkHasLoad() && 
                ((filter & Directory.ServiceFilterFlags.LOAD) != 0))
        {
            service.applyHasLoad();
            retNumFilters++;
        }
        else
        {
        	service.flags(service.flags() & ~Directory.ServiceFilterFlags.LOAD);
        }

        if (serviceReceived.checkHasState() && 
                ((filter & Directory.ServiceFilterFlags.STATE) != 0))
        {
            service.applyHasState();
            retNumFilters++;
        }
        else
        {
        	service.flags(service.flags() & ~Directory.ServiceFilterFlags.STATE);
        }
        
        return retNumFilters;
    }
    
    int getResultingFilter(long userFilter, long responseFilter)
    {
        int resultingFilter = 0;
        if (responseFilter != 0)
        {
	        if (((responseFilter & Directory.ServiceFilterFlags.INFO) != 0) && 
	            ((userFilter & Directory.ServiceFilterFlags.INFO) != 0)) // Apply flag
	        {
	            resultingFilter |= Directory.ServiceFilterFlags.INFO;
	        }
	
	        if (((responseFilter & Directory.ServiceFilterFlags.DATA) != 0) && 
	            ((userFilter & Directory.ServiceFilterFlags.DATA) != 0))
	        {
	            resultingFilter |= Directory.ServiceFilterFlags.DATA;
	        }
	        
	        if (((responseFilter & Directory.ServiceFilterFlags.GROUP) != 0) && 
	            ((userFilter & Directory.ServiceFilterFlags.GROUP) != 0))
	        {
	            resultingFilter |= Directory.ServiceFilterFlags.GROUP;
	        }
	
	        if (((responseFilter & Directory.ServiceFilterFlags.LINK) != 0) && 
	            ((userFilter & Directory.ServiceFilterFlags.LINK) != 0))
	        {
	            resultingFilter |= Directory.ServiceFilterFlags.LINK;
	        }
	
	        if (((responseFilter & Directory.ServiceFilterFlags.LOAD) != 0) && 
	            ((userFilter & Directory.ServiceFilterFlags.LOAD) != 0))
	        {
	            resultingFilter |= Directory.ServiceFilterFlags.LOAD;
	        }
	
	        if (((responseFilter & Directory.ServiceFilterFlags.STATE) != 0) && 
	            ((userFilter & Directory.ServiceFilterFlags.STATE) != 0))
	        {
	            resultingFilter |= Directory.ServiceFilterFlags.STATE;
	        }
        }
        else
        {
        	resultingFilter = (int)userFilter;
        }
        
       return resultingFilter;
    }

    @Override
    public int readMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        assert (_stream == wlStream);
        assert (msg.streamId() == _directoryRequest.streamId());
        
        int ret = ReactorReturnCodes.SUCCESS;
        
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            	if (!_receivedRefresh)
            		ret = readRefreshMsg(wlStream, dIter, msg, errorInfo);
            	else
            		ret = readRefreshMsgAsUpdate(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.STATUS:
                ret =  readStatusMsg(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.UPDATE:
                ret =  readUpdateMsg(wlStream, dIter, msg, errorInfo);
                break;
            case MsgClasses.GENERIC:
            	ret = readGenericMsg(wlStream, dIter, msg, errorInfo);
            	break;
            default:
                ret = _watchlist.reactor().populateErrorInfo(errorInfo,
                                                              ReactorReturnCodes.FAILURE,
                                                              "WlDirectoryHandler.readMsg",
                                                              "Invalid message class (" + msg.msgClass() + ") received by Watchlist directory handler");
                break;
        }

        // handle any state transition
        if (ret == ReactorReturnCodes.SUCCESS)
        {
            switch (wlStream.state().streamState())
            { 
                case StreamStates.CLOSED_RECOVER:
                case StreamStates.CLOSED:
                case StreamStates.REDIRECTED:
                	handleClose(wlStream, errorInfo);
                    break;
                case StreamStates.OPEN:
                    if (wlStream.state().dataState() == DataStates.OK && msg.msgClass() == MsgClasses.REFRESH)
                    {
                        if (_watchlist.reactorChannel().state() != ReactorChannel.State.READY)
                        {
                            // call back user with CHANNEL_READY
                            ret = _watchlist.reactor().sendAndHandleChannelEventCallback("WlDirectoryHandler.readRefreshMsg",
                                                                                         ReactorChannelEventTypes.CHANNEL_READY,
                                                                                         _watchlist.reactorChannel(), errorInfo);
    
                            if (ret == ReactorReturnCodes.SUCCESS)
                            {
                                _watchlist.reactorChannel().state(ReactorChannel.State.READY);
                                _watchlist.reactorChannel().clearAccessTokenForV2();;
                                // notify item handler that directory stream is open
                                ret = _watchlist.itemHandler().directoryStreamOpen();
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        return ret;
    }
    
    void handleClose(WlStream wlStream, ReactorErrorInfo errorInfo)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
     	   
    	WlRequest usrRequest = null;
    	Service service = null;

        closeDirectoryStream();
        
    	_directoryUpdate.clear();
    	_directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
    	
    	for (usrRequest = requestList.poll(); usrRequest!= null; usrRequest = requestList.poll())
    	{    	 
    		_directoryUpdate.streamId(usrRequest.requestMsg().streamId());
    		
        	for (int i = 0; i < serviceList().size(); ++i) 
        	{
        		// Only add services that the user requested (or all services if zero) to the serviceList of the update
        		if (serviceList().get(i).rdmService().serviceId() == usrRequest.requestMsg().msgKey().serviceId() ||
        				usrRequest.requestMsg().msgKey().serviceId() == 0)
        		{
                	if (_servicePool.isEmpty())
            		{
            			service = DirectoryMsgFactory.createService();
            		}
                	else
                	{
                		service = _servicePool.poll();
                		service.clear();
                	}
            		serviceList().get(i).rdmService().copy(service);
            		service.action(MapEntryActions.DELETE);
            		_directoryUpdate.serviceList().add(service);
        		}
        	}
    		
    		_tempUpdateMsg.clear();
    		_watchlist.convertRDMToCodecMsg(_directoryUpdate, _tempUpdateMsg);
    		
    		// use filter from user request
    		int returnFilter = getResultingFilter(usrRequest.requestMsg().msgKey().filter(), _directoryUpdate.filter());
            if (_tempUpdateMsg.checkHasMsgKey())
            	_tempUpdateMsg.msgKey().filter(returnFilter);
            if (_directoryUpdate.checkHasFilter())
            	_directoryUpdate.filter(returnFilter);

            _tempWlInteger.value(_tempUpdateMsg.streamId());
    		if ((callbackUser("WlDirectoryHandler.handleClose", _tempUpdateMsg, _directoryUpdate, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
    		{
    			break;
    		}
    	}
    	
    	for (int i = 0; i < _directoryUpdate.serviceList().size(); ++i)
    	{
        	_servicePool.add(_directoryUpdate.serviceList().get(i));	
    	}
    	_serviceCache.clearCache(false);
    }
    
    void deleteAllServices(WlStream wlStream, boolean isChannelDown, ReactorErrorInfo errorInfo)
    {
    	LinkedList<WlRequest> requestList = wlStream.userRequestList();
     	   
    	WlRequest usrRequest = null;
    	Service service = null;

        _stream.channelDown();
        if (_stream.state().streamState() == StreamStates.OPEN)
        {
            _stream.state().clear();
            _stream.state().streamState(StreamStates.CLOSED_RECOVER);
            _stream.state().dataState(DataStates.SUSPECT);                
            // remove this stream from watchlist table
            _watchlist.streamIdtoWlStreamTable().remove(_stream.tableKey());

            _directoryUpdate.clear();
            _directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);

            for (int j = 0; j < requestList.size(); j++)
            {    	 
                usrRequest = requestList.get(j);
                usrRequest.state(WlRequest.State.PENDING_REFRESH);
                _directoryUpdate.streamId(usrRequest.requestMsg().streamId());

                for (int i = 0; i < serviceList().size(); ++i) 
                {
                    // Only add services that the user requested (or all services if zero) to the serviceList of the update
                    if (serviceList().get(i).rdmService().serviceId() == usrRequest.requestMsg().msgKey().serviceId() ||
                            usrRequest.requestMsg().msgKey().serviceId() == 0)
                    {
                        if (_servicePool.isEmpty())
                        {
                            service = DirectoryMsgFactory.createService();
                        }
                        else
                        {
                            service = _servicePool.poll();
                            service.clear();
                        }
                        serviceList().get(i).rdmService().copy(service);
                        service.action(MapEntryActions.DELETE);
                        _directoryUpdate.serviceList().add(service);
                    }
                }

                _tempUpdateMsg.clear();
                _watchlist.convertRDMToCodecMsg(_directoryUpdate, _tempUpdateMsg);

        		// use filter from user request
                int returnFilter = getResultingFilter(usrRequest.requestMsg().msgKey().filter(), _directoryUpdate.filter());
                if (_tempUpdateMsg.checkHasMsgKey())
                	_tempUpdateMsg.msgKey().filter(returnFilter);
                if (_directoryUpdate.checkHasFilter())
                	_directoryUpdate.filter(returnFilter);

                _tempWlInteger.value(_tempUpdateMsg.streamId());
                if ((callbackUser("WlDirectoryHandler.handleClose", _tempUpdateMsg, _directoryUpdate, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                {
                    break;
                }
                
                _directoryUpdate.clear();
                _directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
            }

            for (int i = 0; i < _directoryUpdate.serviceList().size(); ++i)
            {
                _servicePool.add(_directoryUpdate.serviceList().get(i));	
            }
            _serviceCache.clearCache(isChannelDown);
        }
    }
    
    /* Reads a refresh message. */
    int readRefreshMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorCallbackReturnCodes.SUCCESS;
        
        // make sure refresh complete flag is set
        // directory handler doesn't handle multi-part directory refreshes
        if (!((RefreshMsg)msg).checkRefreshComplete())
        {
           return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                         ReactorReturnCodes.FAILURE,
                                                         "WlDirectoryHandler.readRefreshMsg",
                                                         "Watchlist doesn't handle multi-part directory refresh.");
        }

        // notify stream that response received if solicited
        if (((RefreshMsg)msg).checkSolicited())
        {
            wlStream.responseReceived();
        }

        // convert to rdm directory refresh and save
        _directoryRefresh.decode(dIter, msg);
    
        if (((RefreshMsg)msg).checkClearCache())
        {
            // clear service cache
            _serviceCache.clearCache(false);

            // if refresh is unsolicited, notify item handler all services deleted
            if (!((RefreshMsg)msg).checkSolicited())
            {
                _watchlist.directoryHandler().deleteAllServices(false);
            }
        }
        
        // set state from directory refresh
        _directoryRefresh.state().copy(wlStream.state());
        
        if (_directoryRefresh.state().streamState() == StreamStates.CLOSED_RECOVER)
        {
            _directoryRefresh.state().streamState(StreamStates.OPEN);
            _directoryRefresh.state().dataState(DataStates.SUSPECT);
            ((RefreshMsg)msg).state().streamState(StreamStates.OPEN);
            ((RefreshMsg)msg).state().dataState(DataStates.SUSPECT);
        }
        
        /* Pass service list to service cache for processing. */
        List<Service> serviceList = _directoryRefresh.serviceList();
        if (serviceList != null)
        {
            ret = _serviceCache.processServiceList(serviceList, msg, errorInfo);
        }
        
        if (ret == ReactorCallbackReturnCodes.SUCCESS)
        {
        	// fanout refresh message to user requests associated with the stream
            for (int i = 0; i < wlStream.userRequestList().size(); i++)
            {
                WlRequest wlRequest = wlStream.userRequestList().get(i);

                // only fanout to those whose state is PENDING_REFRESH or unsolicited
                if (!((RefreshMsg)msg).checkSolicited() ||
                    wlRequest.state() == WlRequest.State.PENDING_REFRESH)
                {
                    DirectoryRefresh newDirectoryRefresh = null;
                    if (!_directoryRefreshPool.isEmpty())
                    {
                    	newDirectoryRefresh = _directoryRefreshPool.poll();
                        newDirectoryRefresh.clear();
                    }
                    else
                    {
                    	newDirectoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
                    }
                    
                    // We have our Refresh, set state to open
                    wlRequest.state(WlRequest.State.OPEN);
                    
                    fillDirectoryRefreshFromRequestMsg(newDirectoryRefresh, wlRequest.requestMsg());
                    
                    fillDirectoryRefreshServiceListFromCache(newDirectoryRefresh, wlRequest.streamInfo()._serviceName);
                    
                    _tempRefreshMsg.clear();
                    _watchlist.convertRDMToCodecMsg(newDirectoryRefresh, _tempRefreshMsg);
                    
                    // use filter from user request
                    int returnFilter = getResultingFilter(wlRequest.requestMsg().msgKey().filter(), newDirectoryRefresh.filter());
                    if (_tempRefreshMsg.checkHasMsgKey())
                    	_tempRefreshMsg.msgKey().filter(returnFilter);
                   	newDirectoryRefresh.filter(returnFilter);
                    
                    // callback user
                    _tempWlInteger.value(_tempRefreshMsg.streamId());
                    if ((ret = callbackUser("WlDirectoryHandler.readRefreshMsg", _tempRefreshMsg, newDirectoryRefresh, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                     {
                        // put Directory Refresh services back into pool since we are finished with them
                        for (int j = 0; j < newDirectoryRefresh.serviceList().size(); ++j)
                        {
                            _servicePool.add(newDirectoryRefresh.serviceList().get(j));
                        }

                        // Put back in pool since we are finished with it
                        _directoryRefreshPool.add(newDirectoryRefresh);

                    	// Break out of loop
                    	break;
                    }
                    
                    // put Directory Refresh services back into pool since we are finished with them
                    for (int j = 0; j < newDirectoryRefresh.serviceList().size(); ++j)
                    {
                    	_servicePool.add(newDirectoryRefresh.serviceList().get(j));
                    }
                    
                    // Put back in pool since we are finished with it
                    _directoryRefreshPool.add(newDirectoryRefresh);
                }
            }
        }
        
        _receivedRefresh = true;

        return ret;
    }
    
    /* Reads a refresh message but fanout as update because stream is already up. */
    int readRefreshMsgAsUpdate(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorCallbackReturnCodes.SUCCESS;
        
        // make sure refresh complete flag is set
        // directory handler doesn't handle multi-part directory refreshes
        if (!((RefreshMsg)msg).checkRefreshComplete())
        {
           return _watchlist.reactor().populateErrorInfo(errorInfo,
                                                         ReactorReturnCodes.FAILURE,
                                                         "WlDirectoryHandler.readRefreshMsg",
                                                         "Watchlist doesn't handle multi-part directory refresh.");
        }

        // notify stream that response received if solicited
        if (((RefreshMsg)msg).checkSolicited())
        {
            wlStream.responseReceived();
        }

        // convert to rdm directory refresh and save
        _directoryRefresh.decode(dIter, msg);
    
        if (((RefreshMsg)msg).checkClearCache())
        {
            // clear service cache
            _serviceCache.clearCache(false);

            // if refresh is unsolicited, notify item handler all services deleted
            if (!((RefreshMsg)msg).checkSolicited())
            {
                _watchlist.directoryHandler().deleteAllServices(false);
            }
        }
        
        // set state from directory refresh
        _directoryRefresh.state().copy(wlStream.state());
        
        if (_directoryRefresh.state().streamState() == StreamStates.CLOSED_RECOVER)
        {
            _directoryRefresh.state().streamState(StreamStates.OPEN);
            _directoryRefresh.state().dataState(DataStates.SUSPECT);
            ((RefreshMsg)msg).state().streamState(StreamStates.OPEN);
            ((RefreshMsg)msg).state().dataState(DataStates.SUSPECT);
        }
        
        /* Pass service list to service cache for processing. */
        List<Service> serviceList = _directoryRefresh.serviceList();
        if (serviceList != null)
        {
            ret = _serviceCache.processServiceList(serviceList, msg, errorInfo);
        }
        
        if (ret == ReactorCallbackReturnCodes.SUCCESS)
        {
        	// fanout refresh message to user requests associated with the stream
            for (int i = 0; i < wlStream.userRequestList().size(); i++)
            {
                WlRequest wlRequest = wlStream.userRequestList().get(i);

                // only fanout to those whose state is PENDING_REFRESH or unsolicited
                if (!((RefreshMsg)msg).checkSolicited() ||
                    wlRequest.state() == WlRequest.State.PENDING_REFRESH)
                {
                    DirectoryRefresh newDirectoryRefresh = null;
                    if (!_directoryRefreshPool.isEmpty())
                    {
                    	newDirectoryRefresh = _directoryRefreshPool.poll();
                        newDirectoryRefresh.clear();
                    }
                    else
                    {
                    	newDirectoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
                    }
                    
                    // We have our Refresh, set state to open
                    wlRequest.state(WlRequest.State.OPEN);
                    
                    fillDirectoryRefreshFromRequestMsg(newDirectoryRefresh, wlRequest.requestMsg());
                    
                    fillDirectoryRefreshServiceListFromCache(newDirectoryRefresh, wlRequest.streamInfo()._serviceName);
 
                    // Turn the Directory Refresh into a Directory Update before sending
                    _directoryUpdate.clear();
                    _directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
                    _directoryUpdate.serviceList().addAll(newDirectoryRefresh.serviceList());
                    _directoryUpdate.applyHasFilter();
                    _directoryUpdate.filter(newDirectoryRefresh.filter());
                    _directoryUpdate.streamId(newDirectoryRefresh.streamId());
                    _directoryUpdate.applyHasServiceId();
                    _directoryUpdate.serviceId(newDirectoryRefresh.serviceId());
                    _directoryUpdate.flags(newDirectoryRefresh.flags());
                    
                    _tempUpdateMsg.clear();
                    _watchlist.convertRDMToCodecMsg(_directoryUpdate, _tempUpdateMsg);
                    
                    // use filter from user request
                    int returnFilter = getResultingFilter(wlRequest.requestMsg().msgKey().filter(), _directoryUpdate.filter());
                    if (_tempUpdateMsg.checkHasMsgKey())
                    	_tempUpdateMsg.msgKey().filter(returnFilter);
                    if (_directoryUpdate.checkHasFilter())
                    	_directoryUpdate.filter(returnFilter);

                    // callback user
                    _tempWlInteger.value(_tempUpdateMsg.streamId());
                    if ((ret = callbackUser("WlDirectoryHandler.readRefreshMsgAsUpdate", _tempUpdateMsg, _directoryUpdate, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                     {
                        // put Directory Refresh services back into pool since we are finished with them
                        for (int j = 0; j < newDirectoryRefresh.serviceList().size(); ++j)
                        {
                            _servicePool.add(newDirectoryRefresh.serviceList().get(j));
                        }

                        // Put back in pool since we are finished with it
                        _directoryRefreshPool.add(newDirectoryRefresh);

                    	// Break out of loop
                    	break;
                    }
                    
                    // put Directory Refresh services back into pool since we are finished with them
                    for (int j = 0; j < newDirectoryRefresh.serviceList().size(); ++j)
                    {
                    	_servicePool.add(newDirectoryRefresh.serviceList().get(j));
                    }
                    
                    // Put back in pool since we are finished with it
                    _directoryRefreshPool.add(newDirectoryRefresh);
                }
            }
        }

        return ret;
    }

    /* Reads a status message. */
    int readStatusMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
    	int ret = ReactorCallbackReturnCodes.SUCCESS;
        // convert to rdm directory status and save
        ret = _directoryStatus.decode(dIter, msg);

        // notify stream that response received
        wlStream.responseReceived();
        
        if (((StatusMsg)msg).checkClearCache())
        {
            // clear service cache
            _serviceCache.clearCache(false);
            
            // if stream state is OPEN, notify item handler all services deleted
            if (_directoryStatus.state().streamState() == StreamStates.OPEN)
            {
                _watchlist.directoryHandler().deleteAllServices(false);
            }
        }
        
        if (_directoryStatus.checkHasState())
        {
        	_directoryStatus.state().copy(wlStream.state());
        }

        return ret;
    }

    /* Reads an update message. */
    int readUpdateMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorCallbackReturnCodes.SUCCESS;
        
        // convert to rdm directory update and save
        _directoryUpdate.clear();
        _directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
        ret = _directoryUpdate.decode(dIter, msg);
        
        if (ret == ReactorCallbackReturnCodes.SUCCESS)
        {
            /* Pass service list to service cache for processing. */
            List<Service> serviceList = _directoryUpdate.serviceList();
            if (serviceList != null)
            {
                ret = _serviceCache.processServiceList(serviceList, msg, errorInfo);
                if (ret < ReactorCallbackReturnCodes.SUCCESS)
        		{
        			return ret;
        		}
            }
        }
        else
        {
        	return ret;
        }
        
        // fanout update message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = wlStream.userRequestList().get(i);
            
            // only fanout to those whose state is PENDING_REFRESH OR OPEN
            if (wlRequest.state() == WlRequest.State.PENDING_REFRESH || 
            		wlRequest.state() == WlRequest.State.OPEN)
            {
        		// Find services they want and keep them on the list of services for the update
                _directoryUpdateCopy.clear();
                fillDirectoryUpdateFromRequestMsg(_directoryUpdateCopy, wlRequest);

                fillDirectoryUpdateServiceListFromUpdateMsgServices(_directoryUpdateCopy, _directoryUpdate.serviceList());

        		// fanout only if we have a service in the directoryUpdate
        		if (_directoryUpdateCopy.serviceList().size() > 0)
        		{
                    // update filter in message to resulting filter

                    int returnFilter = getResultingFilter(_directoryUpdateCopy.filter(), _directoryUpdate.filter());
                    _directoryUpdateCopy.filter(returnFilter);
            		
                    _tempUpdateMsg.clear();
                    _watchlist.convertRDMToCodecMsg(_directoryUpdateCopy, _tempUpdateMsg);

                    // callback user
                    _tempWlInteger.value(_tempUpdateMsg.streamId());
                    if ((ret = callbackUser("WlDirectoryHandler.readUpdateMsg", _tempUpdateMsg, _directoryUpdateCopy, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                     {
                        // put Directory Update services back into pool since we are finished with them
                        for (int j = 0; j < _directoryUpdateCopy.serviceList().size(); ++j)
                        {
                        	_servicePool.add(_directoryUpdateCopy.serviceList().get(j));
                        }

                        // break out of loop for error
                        return ret;
                    }
        		}

                // put Directory Update services back into pool since we are finished with them
                for (int j = 0; j < _directoryUpdateCopy.serviceList().size(); ++j)
                {
                	_servicePool.add(_directoryUpdateCopy.serviceList().get(j));
                }
            }
        }

        return ret;
    }
    
    /* Reads an generic message. */
    int readGenericMsg(WlStream wlStream, DecodeIterator dIter, Msg msg, ReactorErrorInfo errorInfo)
    {
    	int ret = ReactorCallbackReturnCodes.SUCCESS;
    	
    	// fanout generic message to user requests associated with the stream
        for (int i = 0; i < wlStream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = wlStream.userRequestList().get(i);
            
            // only fanout to those whose state is PENDING_REFRESH, PENDING_COMPLETE_REFRESH or OPEN
            if (wlRequest.state() == WlRequest.State.PENDING_REFRESH ||
                wlRequest.state() == WlRequest.State.PENDING_COMPLETE_REFRESH ||
                wlRequest.state() == WlRequest.State.OPEN)
            {
            	// update stream id in message to that of user request
                msg.streamId(wlRequest.requestMsg().streamId());
                
                // callback user
                if ((ret = _watchlist.reactor().sendAndHandleDefaultMsgCallback("WLDirectoryHandler.readGenericMsg",
                        _watchlist.reactorChannel(),
                        null,
                        msg,
                        (wlRequest != null ? wlRequest.streamInfo() : null),
                        errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                {
                    // break out of loop for error
                    break;
                }
            }
        }
     
        return ret;
    }

    /* Dispatch all streams for the handler. */
    int dispatch(ReactorErrorInfo errorInfo)
    {
    	int ret = ReactorReturnCodes.SUCCESS;

		 if (_stream != null && _hasPendingRequest) 
		 {
	        _hasPendingRequest = false;
	        _tempMsg.clear();
            _watchlist.convertRDMToCodecMsg(_directoryRequest, _tempMsg);
            ret = _stream.sendMsg(_tempMsg, _submitOptions, errorInfo);
			if (ret != ReactorReturnCodes.SUCCESS)
            {
            	return ret;
            }
		 }
		 
	     
        if (!_serviceCache._serviceList.isEmpty())
        {
	        DirectoryRefresh newDirectoryRefresh = null;
	        if (!_directoryRefreshPool.isEmpty())
	        {
	        	newDirectoryRefresh = _directoryRefreshPool.poll();
	            newDirectoryRefresh.clear();
	        }
	        else
	        {
	        	newDirectoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
	        }
	        
            // fanout refresh message to user requests associated with the stream
            for (int i = 0; i < _stream.userRequestList().size(); i++)
            {
                WlRequest wlRequest = _stream.userRequestList().get(i);
	                
	            // only fanout to those whose state is PENDING_REFRESH
                if (wlRequest.state() == WlRequest.State.PENDING_REFRESH)
	            {
                    wlRequest.state(WlRequest.State.OPEN);
	
                    newDirectoryRefresh.clear();
                    fillDirectoryRefreshFromRequestMsg(newDirectoryRefresh, wlRequest.requestMsg());
	                	
                    fillDirectoryRefreshServiceListFromCache(newDirectoryRefresh, wlRequest.streamInfo()._serviceName);
	                	
                    _tempRefreshMsg.clear();
                    _watchlist.convertRDMToCodecMsg(newDirectoryRefresh, _tempRefreshMsg);
                    
                    // use filter from user request
                    int returnFilter = getResultingFilter(wlRequest.requestMsg().msgKey().filter(), newDirectoryRefresh.filter());
                    if (_tempRefreshMsg.checkHasMsgKey())
                    	_tempRefreshMsg.msgKey().filter(returnFilter);
                    newDirectoryRefresh.filter(returnFilter);
	            // callback user
                    _tempWlInteger.value(_tempRefreshMsg.streamId());
                    if ((ret = callbackUser("WlDirectoryHandler.dispatch", _tempRefreshMsg, newDirectoryRefresh, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                    {
	                        // put Directory Refresh services back into pool since we are finished with them
	                        for (int j = 0; j < newDirectoryRefresh.serviceList().size(); ++j)
	                        {
	                        	_servicePool.add(newDirectoryRefresh.serviceList().get(j));
	                        }
	                        
	                    	_directoryRefreshPool.add(newDirectoryRefresh);
	                    	
	                    	return ret;
	                }
	            }
	            
	            // put Directory Refresh services back into pool since we are finished with them
	            for (int j = 0; j < newDirectoryRefresh.serviceList().size(); ++j)
	            {
	            	_servicePool.add(newDirectoryRefresh.serviceList().get(j));
	            }
	            
	        	_directoryRefreshPool.add(newDirectoryRefresh);

	        	_requestDispatchFlag = false;
	        	
	        	if (ret != ReactorReturnCodes.SUCCESS)
	        	{
	        		return ret;
	        	}
	        }
        }
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    /* Handles login stream open event. */
    int loginStreamOpen(ReactorErrorInfo errorInfo)
    {
        _stream.handler(this);
        _stream.watchlist(_watchlist);
        _stream.domainType(DomainTypes.SOURCE);

        _stream.tableKey(ReactorFactory.createWlInteger());
        _directoryRequest.streamId(_directoryStreamId);
        _stream.tableKey().value(_directoryRequest.streamId());
        _watchlist.streamIdtoWlStreamTable().put(_stream.tableKey(), _stream);
        
        if (_watchlist.role()._directoryRequest != null && !_roleDirectoryRequestAdded)
        {
            WlInteger wlInteger = ReactorFactory.createWlInteger();
            wlInteger.value(_watchlist.role().rdmDirectoryRequest().streamId());
        	if (!_watchlist.streamIdtoWlRequestTable().containsKey(wlInteger))
        	{
            	// User has enabled initDefaultRDMDirectoryRequest
                WlRequest wlRequest = ReactorFactory.createWlRequest();
                _tempMsg.clear();
                _watchlist.convertRDMToCodecMsg(_watchlist.role().rdmDirectoryRequest(), _tempMsg);
                wlRequest.requestMsg().clear();
                _tempMsg.copy(wlRequest.requestMsg(), CopyMsgFlags.ALL_FLAGS);
                wlRequest.handler(this);
                wlRequest.tableKey(wlInteger);
                _watchlist.streamIdtoWlRequestTable().put(wlInteger, wlRequest);
                // Go immediately into Refresh Complete Pending state because we do not use Pending Request
                wlRequest.state(WlRequest.State.PENDING_REFRESH);
            	_stream.userRequestList().add(wlRequest);	
				wlRequest.stream(_stream);
            	_roleDirectoryRequestAdded = true;
        	}
        	else
        	{
        	    wlInteger.returnToPool();
        	}
        }
        
        // send directory request for all services and filters
        if (_stream.state().streamState() != StreamStates.OPEN && _stream.state().streamState() != StreamStates.CLOSED && !_stream.requestPending())
        {
            _tempMsg.clear();
            _watchlist.convertRDMToCodecMsg(_directoryRequest, _tempMsg);
            int ret = _stream.sendMsg(_tempMsg, _submitOptions, errorInfo);
            if (ret < ReactorReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        return ReactorReturnCodes.SUCCESS;
    }
    
    /* Handles login stream closed event. */
    int loginStreamClosed()
    {
        int ret = ReactorReturnCodes.SUCCESS;
        
        if (_stream.state().streamState() == StreamStates.OPEN)
        {
            if (_watchlist.loginHandler().wlStream().state().streamState() == StreamStates.CLOSED_RECOVER || _watchlist.reactorChannel().enableSessionManagement())
            {
                // login stream in close recover state
                _stream.state().clear();
                _stream.state().streamState(StreamStates.CLOSED_RECOVER);
                _stream.state().dataState(DataStates.SUSPECT);
                
                // clear service cache
                _serviceCache.clearCache(false);
            }
            
            // close stream if login stream is closed
            else if (_watchlist.loginHandler().wlStream().state().streamState() == StreamStates.CLOSED)
            {                
            	// clear service cache
                _serviceCache.clearCache(false);

                closeDirectoryStream();
            }
        }
        
        return ret;
    }
    
    /* Handles channel up event. */
    void channelUp(ReactorErrorInfo errorInfo)
    {
        if (_stream != null)
        {
            _stream.channelUp();
        }
    }

    /* Handles channel down event. */
    void deleteAllServices(boolean isChannelDown)
    {
        if (_stream != null)
        {
            deleteAllServices(_stream, isChannelDown, _errorInfo);
        }
    }
    
    /* Fans out a status message to the application. */
    int fanoutStatus()
    {
    	int ret = ReactorReturnCodes.SUCCESS;
        // set state to closed recover if current state isn't closed
        if (_stream.state().streamState() != StreamStates.CLOSED)
        {
            // call back user with directory status of OPEN/SUSPECT 
            _statusMsg.state().streamState(StreamStates.OPEN);
            _statusMsg.state().dataState(DataStates.SUSPECT);
           
            _directoryStatus.state().streamState(StreamStates.OPEN);
            _directoryStatus.state().dataState(DataStates.SUSPECT);
        }
        else // closed, call back user with directory status of CLOSED/SUSPECT 
        {
            _statusMsg.state().streamState(StreamStates.CLOSED);
            _statusMsg.state().dataState(DataStates.SUSPECT);

            _directoryStatus.state().streamState(StreamStates.CLOSED);
            _directoryStatus.state().dataState(DataStates.SUSPECT);
        }

     // fanout status message to user requests associated with the stream
        for (int i = 0; i < _stream.userRequestList().size(); i++)
        {
            WlRequest wlRequest = _stream.userRequestList().get(i);
            
            // only fanout to those whose state is PENDING_REFRESH or OPEN
            if (wlRequest.state() == WlRequest.State.PENDING_REFRESH ||
                wlRequest.state() == WlRequest.State.OPEN)
            {
                // Set streamId and filter to that of the userRequest
            	_statusMsg.streamId(wlRequest.requestMsg().streamId());
            	_directoryStatus.streamId(wlRequest.requestMsg().streamId());
            	
            	// use filter from user request
            	_statusMsg.applyHasMsgKey();
            	_statusMsg.msgKey().filter(wlRequest.requestMsg().msgKey().filter());
            	_directoryStatus.applyHasFilter();
            	_directoryStatus.filter(wlRequest.requestMsg().msgKey().filter());
            	
                // callback user
                _tempWlInteger.value(_statusMsg.streamId());
                if ((ret = callbackUser("WlDirectoryHandler.fanoutStatus", _statusMsg, _directoryStatus, _watchlist.streamIdtoWlRequestTable().get(_tempWlInteger), _errorInfo)) < ReactorCallbackReturnCodes.SUCCESS)
                 {
                	// Break out of loop
                	break;
                }
            }
        }

        return ret;
    }
    
    @Override
    public int callbackUser(String location, Msg msg, MsgBase rdmMsg, WlRequest wlRequest, ReactorErrorInfo errorInfo)
    {
    	
        int ret = ReactorReturnCodes.SUCCESS;

        ret = _watchlist.reactor().sendAndHandleDirectoryMsgCallback(location,
                                                                 _watchlist.reactorChannel(),
                                                                 null,
                                                                 msg,
                                                                 (DirectoryMsg)rdmMsg,
                                                                 (wlRequest != null ? wlRequest.streamInfo() : null),
                                                                 errorInfo);

        if (ret == ReactorCallbackReturnCodes.RAISE)
        {
            ret = _watchlist.reactor().sendAndHandleDefaultMsgCallback(location,
                                                                       _watchlist.reactorChannel(),
                                                                       null,
                                                                       msg,
                                                                       (wlRequest != null ? wlRequest.streamInfo() : null),
                                                                       errorInfo);
        }

        return ret;
    }
    
    @Override
    public void addPendingRequest(WlStream wlStream)
    {
    	_hasPendingRequest = true;
    }
    
    /* Returns the list of services from the service cache. */
    LinkedList<WlService> serviceList()
    {
    	return _serviceCache._serviceList; 
    }

    @Override
    public int requestTimeout(WlStream wlStream, ReactorErrorInfo errorInfo)
    {
        // fanout status to user
        fanoutStatus();
        
        // re-send directory request
        _tempMsg.clear();
        _watchlist.convertRDMToCodecMsg(_directoryRequest, _tempMsg);
        return _stream.sendMsg(_tempMsg, _submitOptions, errorInfo);
    }
    
    /* Retrieve service id from service name. */
    int serviceId(String serviceName)
    {
        return _serviceCache.serviceId(serviceName);
    }

    /* Retrieve service name from service id. */
    String serviceName(int serviceId)
    {
        return _serviceCache.serviceName(serviceId);
    }

    /* Retrieve service by service name. */
    WlService service(String serviceName)
    {
        return _serviceCache.service(serviceName);
    }

    /* Retrieve service by service id. */
    WlService service(int serviceId)
    {
        return _serviceCache.service(serviceId);
    }

    /* Clear state of watchlist directory handler for re-use. */
    void clear()
    {
        // this handler is still associated with same watchlist so don't set watchlist to null
        _stream.clear();
        _directoryRefresh.clear();
        _directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        _directoryStatus.clear();
        _directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
        _directoryStatus.applyHasState();
        _directoryStatus.state().code(StateCodes.NONE);
        _directoryStatus.state().text(_tempBuffer);
        _directoryUpdate.clear();
        _directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
        _directoryUpdateCopy = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        _directoryUpdateCopy.rdmMsgType(DirectoryMsgType.UPDATE);
        _submitOptions.clear();
        _tempMsg.clear();
        _statusMsg.clear();
        _statusMsg.msgClass(MsgClasses.STATUS);
        _statusMsg.domainType(DomainTypes.SOURCE);
        _statusMsg.applyHasState();
        _statusMsg.state().code(StateCodes.NONE);
        _statusMsg.state().text(_tempBuffer);       
        _tempBuffer.clear();
        _tempBuffer.data("");
        _tempMsg.clear();
        _errorInfo.clear();
        _serviceCache.clear();
        _tempRefreshMsg.clear();
        _directoryRefreshPool.clear();
        _tempDirectoryStatus.clear();
        _tempUpdateMsg.clear();
        _requestDispatchFlag = false;
        _receivedRefresh = false;
        _roleDirectoryRequestAdded = false;
        _hasPendingRequest = false;
    }

    /* Close directory stream (but don't repool it so that we keep the application's requests) */
    private void closeDirectoryStream()
    {
        // set state to closed
        _stream.state().clear();
        _stream.state().streamState(StreamStates.CLOSED);
        _stream.state().dataState(DataStates.SUSPECT);                
        // remove this stream from watchlist table
        _watchlist.streamIdtoWlStreamTable().remove(_stream.tableKey());
        _stream.tableKey().returnToPool();
        _stream.tableKey(null);
    }
}
