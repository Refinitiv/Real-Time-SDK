package com.thomsonreuters.upa.valueadd.reactor;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.apache.http.HttpStatus;
import org.json.JSONArray;

import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.SessionMgntState;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.State;

class RestClient implements Runnable, RestCallback {
	
	static final String EDP_RT_TRANSPORT = "transport";
	static final String EDP_RT_DATAFORMAT = "dataformat";
	static final String EDP_RT_TRANSPORT_PROTOCOL_WEBSOCKET = "websocket";
	static final String EDP_RT_TRANSPORT_PROTOCOL_TCP = "tcp";
	static final String EDP_RT_DATAFORMAT_PROTOCOL_RWF = "rwf";
	static final String EDP_RT_DATAFORMAT_PROTOCOL_JSON2 = "tr_json2";
	
	static final String EDP_RT_SD_SERVICES = "services";
	static final String EDP_RT_SD_ENDPOINT = "endpoint";
	static final String EDP_RT_SD_PORT = "port";
	static final String EDP_RT_SD_LOCATION = "location";
	static final String EDP_RT_SD_DATAFORMAT = "dataFormat";
	static final String EDP_RT_SD_PROVIDER = "provider";
	static final String EDP_RT_SD_TRANSPORT = "transport";
	
	RestReactor _restReactor;
	
	private RestReactorOptions _restReactorOptions;
	private ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
	
	private List<ReactorServiceEndpointInfo> _reactorServiceEndpointInfoList;
    
    RestClient ( RestReactorOptions restReactorOpt, ReactorErrorInfo errorInfo )
    {    	
    	_restReactorOptions = restReactorOpt;
    	
    	_restReactor = new RestReactor(_restReactorOptions, errorInfo);
    	
    	_reactorServiceEndpointInfoList = new ArrayList<ReactorServiceEndpointInfo>();
    	
    	new Thread(this).start();
    }
    
    int getAuthAccessTokenInfo(RestAuthOptions authOptions, RestConnectOptions connectOptions, ReactorAuthTokenInfo authTokenInfo,
    		boolean isBlocking, ReactorErrorInfo errorInfo)
    {    
    	int ret = ReactorReturnCodes.SUCCESS;
    	
    	try
    	{	
    		if(isBlocking)
    		{
    			ret = _restReactor.submitAuthRequestBlocking(authOptions, connectOptions, 
    					authTokenInfo, errorInfo);
    		}
    		else
    		{
    			
    			ret = _restReactor.submitAuthRequest(authOptions, connectOptions, 
    					authTokenInfo, errorInfo);
    		}
    	}
    	catch(IOException e)
    	{
    		return RestReactor.populateErrorInfo(errorInfo,   				
    				ReactorReturnCodes.FAILURE,
    				"RestClient.getAuthAccessTokenInfo", 
    				"Failed to send REST request. exception: " 
    						+  RestReactor.getExceptionCause(e));
    	}
    	
    	return ret;
    }
    
    int getServiceDiscovery(RestConnectOptions connectOptions, ReactorAuthTokenInfo authTokenInfo, boolean isBlocking, 
    		List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList, ReactorErrorInfo errorInfo)
    {    	
    	RestRequest restRequest = createRestRequestForServiceDiscovery(connectOptions.transport(), connectOptions.dataFormat());    	
    	int ret = ReactorReturnCodes.SUCCESS;
    	
    	
    	try
    	{
    		if(isBlocking)
    		{
    			ret = _restReactor.submitServiceDiscoveryRequestBlocking(restRequest, connectOptions, authTokenInfo, reactorServiceEndpointInfoList, errorInfo);
    		}
    		else
    		{
    			ret = _restReactor.submitRequestForServiceDiscovery(restRequest, connectOptions, authTokenInfo, reactorServiceEndpointInfoList, errorInfo);
    		}
    	}
    	catch (IOException e) 
    	{
    		return RestReactor.populateErrorInfo(errorInfo,   				
    				ReactorReturnCodes.FAILURE,
    				"RestClient.connectBlocking", 
    				"Failed to send REST request. exception: " 
    						+  RestReactor.getExceptionCause(e));
    	}
    	
    	return ret;
    }
    
	@Override
	public int RestResponseCallback(RestResponse response, RestEvent event)
	{
		ReactorChannel reactorChannel = (ReactorChannel)event.resultClosure().userSpecObj();
		switch (event.eventType())
		{
		case RestEventTypes.COMPLETED:
		{
			if (response.jsonObject() != null)
			{
				parseServiceDiscovery(response, reactorChannel.reactorServiceEndpointInfoList());
				
				if (reactorChannel != null)
				{
					reactorChannel.sessionMgntState(ReactorChannel.SessionMgntState.RECEIVED_ENDPOINT_INFO);
					
					if(reactorChannel.state() == State.EDP_RT)
					{
						reactorChannel.state(State.EDP_RT_DONE);
					}
				}
			}			
		}
		break;	
		case RestEventTypes.FAILED:
		case RestEventTypes.STOPPED:
			
			if(reactorChannel != null)
			{
				String errorText = "failed to get endpoints from the service discovery"; // Default error status text if data body is not defined
				if (response.jsonObject() != null)
				{
					errorText = "Failed REST request for the service discovery. Text: " + response.jsonObject().toString();
				}
					
				event.errorInfo().error().text("Failed REST request from HTTP status code " + response.statusCode() + ". Text: " +  errorText);
					
				reactorChannel.reactor().populateErrorInfo(_errorInfo, ReactorReturnCodes.FAILURE, "RestClient.RestResponseCallback", 
							"Failed REST request with text: " + errorText);
					
				reactorChannel.reactor().sendChannelWarningEvent(reactorChannel, _errorInfo);
				
				if(reactorChannel.state() == State.EDP_RT)
				{
					reactorChannel.setEDPErrorInfo(_errorInfo);
					
					if(event.eventType() == RestEventTypes.STOPPED)
					{
						reactorChannel.sessionMgntState(SessionMgntState.STOP_QUERYING_SERVICE_DISCOVERY);
					}
					else
					{
						reactorChannel.sessionMgntState(SessionMgntState.REQ_FAILURE_FOR_SERVICE_DISCOVERY);
					}
					
					reactorChannel.state(State.EDP_RT_FAILED);
				}
				
				return ReactorReturnCodes.FAILURE;
			}
		
			break;
		default:
			break;
		}
		return ReactorReturnCodes.SUCCESS;
	}
	
	@Override
	public int RestErrorCallback(RestEvent event, String errorText)
	{
		ReactorChannel reactorChannel = (ReactorChannel)event.resultClosure().userSpecObj();
		
		RestReactor.populateErrorInfo(event.errorInfo(),
                ReactorReturnCodes.FAILURE,
                "RestHandler.failed", "Failed REST request for the service discovery. Text: " + errorText);
		
		reactorChannel.sessionMgntState(SessionMgntState.REQ_FAILURE_FOR_SERVICE_DISCOVERY);
		
		reactorChannel.reactor().sendChannelWarningEvent(reactorChannel, event.errorInfo());
		if (reactorChannel.state() == State.EDP_RT)
		{
			reactorChannel.state(State.EDP_RT_FAILED);
			
			/* Retry when failed to send a request to service discovery */
			if ( getServiceDiscovery(reactorChannel.restConnectOptions(), reactorChannel.tokenSession().authTokenInfo(), false,
    				reactorChannel.reactorServiceEndpointInfoList(), event.errorInfo()) != ReactorReturnCodes.SUCCESS)
			{
				return ReactorReturnCodes.FAILURE;
			}
		}

		return ReactorReturnCodes.SUCCESS;
	}	
	
	final static RestRequest createRestRequestForServiceDiscovery(int transport, int dataformat)
	{
    	RestRequest restRequest = new RestRequest();    	
    	
    	HashMap<String,String> map = new HashMap<>();    	
    	switch (transport)
    	{
    	case ReactorDiscoveryTransportProtocol.RD_TP_TCP:
    		map.put(EDP_RT_TRANSPORT, EDP_RT_TRANSPORT_PROTOCOL_TCP);
    		break;
    	case ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET:
    		map.put(EDP_RT_TRANSPORT, EDP_RT_TRANSPORT_PROTOCOL_WEBSOCKET);   		
    		break;
    		default:
    			break;
    	}

    	switch(dataformat)
    	{
    	case ReactorDiscoveryDataFormatProtocol.RD_DP_RWF:
    		map.put(EDP_RT_DATAFORMAT, EDP_RT_DATAFORMAT_PROTOCOL_RWF);
    		break;
    	case ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2:
    		map.put(EDP_RT_DATAFORMAT, EDP_RT_DATAFORMAT_PROTOCOL_JSON2);
    		break;
    	default:
    		break;
    	}
	
		restRequest.queryParameter(map);
		
		return restRequest;
	}

	@Override
	public void run()
	{
		if (_restReactor == null || _restReactor.isShutdown())
			return;
		
		ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    	if (_restReactor.dispatch(errorInfo) != ReactorReturnCodes.SUCCESS)
    	{
    		_restReactor.shutdown(errorInfo);
    	}
	}
	
	void shutdown()
	{
		ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();		
		if (_restReactor != null || !_restReactor.isShutdown())
		{
			_restReactor.shutdown(errorInfo);
		}
	}

	public List<ReactorServiceEndpointInfo> reactorServiceEndpointInfo()
	{
		return _reactorServiceEndpointInfoList;
	}
	
	final static void parseServiceDiscovery(RestResponse response, List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList)
	{
		if (response.statusCode() == HttpStatus.SC_OK && response.jsonObject() != null)
		{	
			JSONArray arr = response.jsonObject().getJSONArray(EDP_RT_SD_SERVICES);
			reactorServiceEndpointInfoList.clear();
	
			for (int i = 0; i < arr.length(); i++) 
			{
				ReactorServiceEndpointInfo serviceInfo = new ReactorServiceEndpointInfo();
	
				serviceInfo._endPoint = arr.getJSONObject(i).opt(EDP_RT_SD_ENDPOINT).toString();
				serviceInfo._port = arr.getJSONObject(i).opt(EDP_RT_SD_PORT).toString();
				serviceInfo._provider = arr.getJSONObject(i).opt(EDP_RT_SD_PROVIDER).toString();
				serviceInfo._transport = arr.getJSONObject(i).opt(EDP_RT_SD_TRANSPORT).toString();
	
				for (int l = 0; l < arr.getJSONObject(i).getJSONArray(EDP_RT_SD_LOCATION).length(); l++)
				{
					serviceInfo._locationList.add (arr.getJSONObject(i).getJSONArray(EDP_RT_SD_LOCATION).get(l).toString());
				}
				
				if (arr.getJSONObject(i).has(EDP_RT_SD_DATAFORMAT))
				{
					for (int l = 0; l < arr.getJSONObject(i).getJSONArray(EDP_RT_SD_DATAFORMAT).length(); l++)
					{
						serviceInfo._dataFormatList.add (arr.getJSONObject(i).getJSONArray(EDP_RT_SD_DATAFORMAT).get(l).toString());
					}
				}
				
				reactorServiceEndpointInfoList.add(serviceInfo);
			}
		}
	}
}
