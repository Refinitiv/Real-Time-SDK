package com.thomsonreuters.upa.valueadd.reactor;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.json.JSONArray;

import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.State;

abstract class RestClient implements Runnable, RestCallback {
	
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
	private boolean _failedAuthReRequest;
	
	private ReactorAuthTokenInfo _authTokenInfo = new ReactorAuthTokenInfo();
	
	private RestReactorOptions _restReactorOptions;
	private RestConnectOptions _restConnectOptions;
	private RestAuthOptions _restAuthRequest;
	private ReactorErrorInfo _errorInfo;
	private String _location;
	
	private List<ReactorServiceEndpointInfo> _reactorServiceEndpointInfoList;
	
	private JSONArray _endpointConnections;
    
    RestClient ( RestReactorOptions restReactorOpt, RestConnectOptions restConnOpt, 
    		RestAuthOptions restAuthOpt, 
    		ReactorErrorInfo errorInfo )
    {
    	_failedAuthReRequest = false;
    	
    	_restReactorOptions = restReactorOpt;
    	_restConnectOptions = restConnOpt;
    	_restAuthRequest = restAuthOpt;    	
    	_errorInfo = errorInfo;
    	
    	if (_restConnectOptions.location() == null)
    		_location = new String();
    	else
    		_location = _restConnectOptions.location();
    	
    	_restReactorOptions.connectionOptions(_restConnectOptions);
    	_restReactorOptions.connectTimeout(5000);
    	_restReactorOptions.soTimeout(10000);
    	_restReactorOptions.defaultRespCallback(this);

    	_restReactorOptions.authorizationCallback(this);  
    	
    	_restReactor = new RestReactor(_restReactorOptions, errorInfo);
    	
    	_reactorServiceEndpointInfoList = new ArrayList<ReactorServiceEndpointInfo>();
    	
    	new Thread(this).start();
    }
    
    void connect(ReactorErrorInfo errorInfo)
    {
    	// Request access token
    	_restAuthRequest.username(_restConnectOptions.userName().toString());
    	_restAuthRequest.password(_restConnectOptions.password().toString());
    	if (_restConnectOptions.clientId().toString() == null)
    		_restAuthRequest.clientId(_restConnectOptions.userName().toString());
    	else
    		_restAuthRequest.clientId(_restConnectOptions.clientId().toString());    		

    	if (_restConnectOptions.location() != null)
    		_location = _restConnectOptions.location();
        
    	// submit auth request
    	_restReactor.submitAuthRequest((ReactorChannel)_restConnectOptions.userSpecObject(), _restAuthRequest, _restConnectOptions, errorInfo);
    	
    }
    
    void connectBlocking(ReactorServiceDiscoveryOptions options, boolean requestServices, ReactorErrorInfo errorInfo)
    {
    	// Request access token
    	_restAuthRequest.username(_restConnectOptions.userName().toString());
    	_restAuthRequest.password(_restConnectOptions.password().toString());
    	if (_restConnectOptions.clientId().toString() == null)
    		_restAuthRequest.clientId(_restConnectOptions.userName().toString());
    	else
    		_restAuthRequest.clientId(_restConnectOptions.clientId().toString());    		

    	if (_restConnectOptions.location() != null)
    		_location = _restConnectOptions.location();    	
    	
    	if (options != null)
    	{
    		_restConnectOptions.dataFormat(options.dataFormat());
    		_restConnectOptions.transport(options.transport());
    	}
        
    	// submit blocking auth request
    	try {
    		    		
			_restReactor.submitAuthRequestBlocking(_restAuthRequest, _restConnectOptions, errorInfo);

	    	if (errorInfo.code() == ReactorReturnCodes.SUCCESS && requestServices)
	    	{
		    	// request list of services from EDP    	
		    	RestRequest restRequest = createRestRequest();    	
				
				_restReactor.submitRequestBlocking(restRequest, _restConnectOptions, errorInfo);
	    	}    	
    	} 
    	catch (IOException e) 
    	{
    		RestReactor.populateErrorInfo(errorInfo,   				
    				ReactorReturnCodes.FAILURE,
    				"RestClient.connectBlocking", 
    				"Failed to send REST request. exception: " 
    						+  RestReactor.getExceptionCause(e));
    	}
    }
    
    private void requestNewAuthTokenWithUserNameAndPassword(ReactorChannel reactorChannel)
    {
    	_restAuthRequest.grantType(RestReactor.AUTH_PASSWORD);
    	
    	_restConnectOptions.userSpecObject(reactorChannel);
    	
    	_restReactor.submitAuthRequest(reactorChannel, _restAuthRequest, _restConnectOptions, _errorInfo);
    }
    
    void requestRefreshAuthToken (ReactorChannel reactorChannel, ReactorErrorInfo errorInfo) 
    {	
    	_restAuthRequest.refreshToken(_authTokenInfo.refreshToken());

    	_restConnectOptions.userSpecObject(reactorChannel);    	
    	
    	_restReactor.submitAuthRequest(reactorChannel, _restAuthRequest, _restConnectOptions, errorInfo);      	
    }
    
    public abstract void onNewAuthToken(ReactorChannel reactorChannel, ReactorAuthTokenInfo authTokenInfo, ReactorErrorInfo errorInfo);
    public abstract void onError(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo);
    
	@Override
	public int RestResponseCallback(RestResponse response, RestEvent event)
	{
		ReactorChannel reactorChannel = (ReactorChannel)event.userSpecObj();
		switch (event.eventType())
		{

		case RestEventTypes.COMPLETED:
		{
			if (response.jsonObject() != null && response.jsonObject().has(RestReactor.AUTH_ACCESS_TOKEN))
			{
				_failedAuthReRequest = false;

				event._reactorAuthTokenInfo.copy(_restConnectOptions.tokenInformation());
				event._reactorAuthTokenInfo.copy(_authTokenInfo);

				onNewAuthToken(reactorChannel, event._reactorAuthTokenInfo, _errorInfo);
				
				if (reactorChannel != null)
				{
					if (reactorChannel._reactorAuthTokenInfo == null)
						reactorChannel._reactorAuthTokenInfo = new ReactorAuthTokenInfo();
					event._reactorAuthTokenInfo.copy(reactorChannel._reactorAuthTokenInfo);
				
					// if reactor channel not null it means non blocking and if state set it means connection recovery
					if (reactorChannel.state() == State.EDP_RT)
					{
						RestRequest restRequest = createRestRequest();
						_restConnectOptions.userSpecObject(reactorChannel);

						_restReactor.submitRequest(restRequest, reactorChannel, _errorInfo);
					}
				}
				
				return ReactorReturnCodes.SUCCESS;
			}

			if (response.jsonObject() != null)
			{
				JSONArray arr = response.jsonObject().getJSONArray(EDP_RT_SD_SERVICES);
				_reactorServiceEndpointInfoList.clear();
				_endpointConnections = new JSONArray();

				// Create a list of endpoints and find the endpoint with 2 matching locations
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

					for (int l = 0; l < arr.getJSONObject(i).getJSONArray(EDP_RT_SD_DATAFORMAT).length(); l++)
					{
						serviceInfo._dataFormatList.add (arr.getJSONObject(i).getJSONArray(EDP_RT_SD_DATAFORMAT).get(l).toString());
					}				

					if (arr.getJSONObject(i).getJSONArray(EDP_RT_SD_LOCATION).length() > 1 && 
							arr.getJSONObject(i).getJSONArray(EDP_RT_SD_LOCATION).get(0).toString().startsWith(_location))
					{
						_endpointConnections.put(arr.getJSONObject(i));
					}

					_reactorServiceEndpointInfoList.add(serviceInfo);
				}
				
				if (reactorChannel != null && reactorChannel.state() == State.EDP_RT)
				{
					reactorChannel.state(State.EDP_RT_DONE);
				}
			}			
		}
		break;	
		case RestEventTypes.FAILED:	
			if (!_failedAuthReRequest)
			{
				if (reactorChannel != null)
				{
		            // send warning event to reactor channel
					_errorInfo.error().text("Failed to request authentication token with refresh token for user: " + 
							((ConsumerRole)reactorChannel.role()).rdmLoginRequest().userName().toString() + 
							". Will try again with user name and password.");
		            reactorChannel.reactor().sendAndHandleChannelEventCallback("RestClient.RestResponseCallback",
		                                              ReactorChannelEventTypes.WARNING,
		                                              reactorChannel, _errorInfo);
				}
				
				requestNewAuthTokenWithUserNameAndPassword(reactorChannel);
				_failedAuthReRequest = true;
			}
			else
			{
				// re requesting second time, this time fail the request
				if (reactorChannel != null && reactorChannel.state() == State.EDP_RT)
				{			
					reactorChannel.setEDPErrorInfo(event.errorInfo());
					reactorChannel.state(State.EDP_RT_FAILED);
				}
				
				_failedAuthReRequest = false;
				return ReactorReturnCodes.FAILURE;
			}
			break;
		default:
			break;
		}
		return ReactorReturnCodes.SUCCESS;
	}
	
	@Override
	public int RestErrorCallback(RestEvent event)
	{
		ReactorChannel reactorChannel = (ReactorChannel)event.userSpecObj();

		onError( reactorChannel, event.errorInfo() );
		if (reactorChannel != null && reactorChannel.state() == State.EDP_RT)
			reactorChannel.state(State.EDP_RT_FAILED);
		
		return ReactorReturnCodes.SUCCESS;
	}	
	
	private RestRequest createRestRequest()
	{
    	RestRequest restRequest = new RestRequest();    	
    	
    	HashMap<String,String> map = new HashMap<>();    	
    	switch (_restConnectOptions.transport())
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

    	switch(_restConnectOptions.dataFormat())
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
	
	public String endpoint()
	{
		if (_endpointConnections != null && _endpointConnections.length() > 0)
		{
			return _endpointConnections.getJSONObject(0).opt("endpoint").toString();
		}
		return null;
	}
	
	public String port()
	{
		if (_endpointConnections != null && _endpointConnections.length() > 0)
		{
			return _endpointConnections.getJSONObject(0).opt("port").toString();
		}
		return null;
	}
	
	public List<ReactorServiceEndpointInfo> reactorServiceEndpointInfo()
	{
		return _reactorServiceEndpointInfoList;
	}
	
	public ReactorAuthTokenInfo reactorAuthTokenInfo()
	{
		return _authTokenInfo;
	}
	
}
