package com.thomsonreuters.upa.valueadd.reactor;

import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.http.HttpStatus;
import org.json.JSONObject;

import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.SessionMgntState;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.State;

class ReactorTokenSession implements RestCallback
{
	private Reactor _reactor;
	private ReactorAuthTokenInfo _authTokenInfo = new ReactorAuthTokenInfo();
	private SessionState _sessionState = SessionState.UNKNOWN;
	private ReactorOAuthCredential _reactorOAuthCredential = new ReactorOAuthCredential(); /* OAuth credential for this token session */
	private ReactorOAuthCredentialRenewal _reactorOAuthCredentialRenewal = new ReactorOAuthCredentialRenewal();
	private boolean _initialized = false;
	private List<ReactorChannel> _reactorChannelList = new LinkedList<ReactorChannel>();
	private Lock _accessTokenLock = new ReentrantLock();
	private Lock _reactorChannelListLock = new ReentrantLock();
	private RestAuthOptions _restAuthRequest;
	private RestConnectOptions _restConnectOptions;
	private RestResultClosure _resultClosure;
	private long _nextAuthTokenRequestTime;
	private long _authTokenExpiresIn;
	private int _tokenReissueAttempts;
	private int _originalExpiresIn; /* Keeps the original expires in seconds from the password grant type. */
	private RestReactor _restReactor;
	private ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
	private boolean _setProxyInfo = false;
	
	
	  /** The ReactorTokenSession's state. */
    enum SessionState
    {
    	UNKNOWN,
    	STOP_REQUESTING,
    	PARSE_RESP_FAILURE,
    	REQUEST_TOKEN_FAILURE,
    	REQ_INIT_AUTH_TOKEN,
    	REQ_AUTH_TOKEN_USING_PASSWORD,
    	REQ_AUTH_TOKEN_USING_REFRESH_TOKEN,
    	RECEIVED_AUTH_TOKEN,
    	AUTHENTICATE_USING_PASSWD_GRANT,
    	AUTHENTICATE_USING_PASSWD_REFRESH_TOKEN
    }
    
    
    ReactorTokenSession(Reactor reactor, ReactorOAuthCredential reactorOAuthCredential)
    {
    	_reactor = reactor;
    	_restReactor = reactor._restClient._restReactor;
    	reactorOAuthCredential.copy(_reactorOAuthCredential);
    	
    	_restConnectOptions = new RestConnectOptions(_reactor.reactorOptions());
    	
    	_restAuthRequest = new RestAuthOptions(this);
    	_restAuthRequest.username(_reactorOAuthCredential.userName().toString());
    	_restAuthRequest.password(_reactorOAuthCredential.password().toString());
    	_restAuthRequest.clientId(_reactorOAuthCredential.clientId().toString());
    	
    	_resultClosure = new RestResultClosure(this, this);
    	_restConnectOptions.restResultClosure(_resultClosure);
    	
		_nextAuthTokenRequestTime = 0;
		_tokenReissueAttempts = 0;
		_originalExpiresIn = 0;
    }
    
    void setProxyInfo(ReactorConnectInfo connectInfo)
    {
    	if(!_setProxyInfo)
    	{
    		_restConnectOptions.applyProxyInfo(connectInfo.connectOptions());
    		_setProxyInfo = true;
    	}
    }
    
	void calculateNextAuthTokenRequestTime(int expiresIn)
	{
		if (expiresIn > 0)
			_authTokenExpiresIn = (long)(((double)expiresIn * _reactor._reactorOptions.tokenReissueRatio()) * 1000000000);
		_nextAuthTokenRequestTime = System.nanoTime() + _authTokenExpiresIn;
	}
	
	long nextAuthTokenRequestTime()
    {
        return _nextAuthTokenRequestTime;
    }
	
	boolean checkMiniumTimeForReissue(ReactorErrorInfo errorInfo)
	{
		long remainingTimeForReissueNs = _nextAuthTokenRequestTime - System.nanoTime();
		
		if ( remainingTimeForReissueNs < (long)(((double) _reactor._reactorOptions.tokenReissueRatio()) * 1000000000))
		{
			_reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, 
					"ReactorTokenSession.checkMiniumTimeForReissue()", 
					"Couldn't add the channel to the token session as the token reissue interval is too small for this channel.");
			
			return false;
		}
		
		return true;
	}
    
    long nextTokenReissueAttemptReqTime()
    {
    	long nextReqTime = ((long)_reactor._reactorOptions.reissueTokenAttemptInterval() * 1000000);
    	
    	return (System.nanoTime() + nextReqTime);
    }
    
    /* This function is used to check whether to perform token reissue attempts. Returns true to send another token reissue otherwise false.*/
    boolean handlesTokenReissueFailed()
    {	/* Don't retry to send token reissue */
    	if (_reactor._reactorOptions.reissueTokenAttemptLimit() == 0)
    	{
    		return false;
    	} /* There is no retry limit */
    	else if (_reactor._reactorOptions.reissueTokenAttemptLimit() == -1)
    	{
    		return true;
    	}
    	else
    	{
    		++_tokenReissueAttempts;
    		
    		/* Retry until reach the number of attempt limit */
    		if(_tokenReissueAttempts > _reactor._reactorOptions.reissueTokenAttemptLimit())
    		{
    			return false;
    		}
    		else
    		{
    			return true;
    		}
    	}
    }
    
    void addReactorChannel(ReactorChannel reactorChannel)
    {
    	_reactorChannelListLock.lock();
    	
    	if(!_reactorChannelList.contains(reactorChannel))
    		_reactorChannelList.add(reactorChannel);
    	
    	_reactorChannelListLock.unlock();
    }
    
    int removeReactorChannel(ReactorChannel reactorChannel)
    {
    	int numberOfChannels = 0;
    	
    	_reactorChannelListLock.lock();
    	
    	if(_reactorChannelList.contains(reactorChannel))
    	{
    		reactorChannel.tokenSession(null);
    		_reactorChannelList.remove(reactorChannel);
    	}
    	
    	numberOfChannels = _reactorChannelList.size();
    	
    	if(numberOfChannels == 0)
    	{
    		_initialized = false;
    		_setProxyInfo = false;
    	}
    	
    	_reactorChannelListLock.unlock();
    	
    	return numberOfChannels;
    }
    
    void removeAllReactorChannel()
    {
    	_reactorChannelListLock.lock();
    	
    	_reactorChannelList.clear();
    	
    	_reactorChannelListLock.unlock();
    }
    
    void resetTokenReissueAttempts()
    {
    	_tokenReissueAttempts = 0;
    }
    
    void originalExpiresIn(int expiresIn)
    {
    	_originalExpiresIn = expiresIn;
    }
    
    int originalExpiresIn()
    {
    	return _originalExpiresIn;
    }
    
    ReactorOAuthCredential oAuthCredential()
    {
    	return _reactorOAuthCredential;
    }
    
    ReactorOAuthCredentialRenewal oAuthCredentialRenewal()
    {
    	return _reactorOAuthCredentialRenewal;
    }
    
    ReactorAuthTokenInfo authTokenInfo()
    {
    	return _authTokenInfo;
    }
    
    RestAuthOptions authOptoins()
    {
    	return _restAuthRequest;
    }
    
    RestConnectOptions restConnectOptions()
    {
    	return _restConnectOptions;
    }
    
    RestResultClosure resultClosure()
    {
    	return _resultClosure;
    }
    
    void lock()
    {
    	_accessTokenLock.lock();
    }
    
    void unlock()
    {
    	_accessTokenLock.unlock();
    }

	SessionState sessionMgntState() {

		return _sessionState;
	}
	
	boolean isInitialized()
	{
		return _initialized;
	}
	
	void isInitialized(boolean initialized)
	{
		_initialized = initialized;
	}
	
	final static void parseTokenInfomation(RestResponse response, ReactorAuthTokenInfo authTokenInfo)
	{
		if (response.statusCode() == HttpStatus.SC_OK && response.jsonObject() != null)
		{
			JSONObject body =  response.jsonObject();

			authTokenInfo.clear();
			
			if (body.has(RestReactor.AUTH_ACCESS_TOKEN))
			{
				authTokenInfo.accessToken(body.getString(RestReactor.AUTH_ACCESS_TOKEN));
			}
			
			if (body.has(RestReactor.AUTH_REFRESH_TOKEN))
			{
				authTokenInfo.refreshToken(body.getString(RestReactor.AUTH_REFRESH_TOKEN));
			}
			
			if (body.has(RestReactor.AUTH_EXPIRES_IN))
			{
				authTokenInfo.expiresIn(body.getInt(RestReactor.AUTH_EXPIRES_IN));
			}
			
			if (body.has(RestReactor.AUTH_SCOPE))
			{
				authTokenInfo.scope(body.getString(RestReactor.AUTH_SCOPE));
			}
			
			if (body.has(RestReactor.AUTH_TOKEN_TYPE))
			{
				authTokenInfo.tokenType(body.getString(RestReactor.AUTH_TOKEN_TYPE));
			}
		}
	}
	
	int sendAuthRequestWithSensitiveInfo(String password, String newPassword, String clientSecret)
	{
		int ret = 0;
		_sessionState = SessionState.REQ_AUTH_TOKEN_USING_PASSWORD;
		_restAuthRequest.grantType(RestReactor.AUTH_PASSWORD);
		
		_restAuthRequest.password(password);
		_restAuthRequest.newPassword(newPassword);
		_restAuthRequest.clientSecret(clientSecret);
		
		ret = _restReactor.submitAuthRequest(_restAuthRequest, _restConnectOptions, _authTokenInfo, _errorInfo);
		
		_restAuthRequest.clearSensitiveInfo();
		
		return ret;
	}
	
	void handleTokenReissue()
	{
		
		_reactorChannelListLock.lock();
    	
		try
		{
			/* Don't send token reissue request when there is no channel for this session. */
			if(_reactorChannelList.size() == 0)
				return;
		}
		finally
		{
			_reactorChannelListLock.unlock();
		}
		
		if (_sessionState == SessionState.REQUEST_TOKEN_FAILURE || _sessionState == SessionState.AUTHENTICATE_USING_PASSWD_GRANT)
		{
			if(_reactorOAuthCredential.reactorOAuthCredentialEventCallback() == null)
			{
				_sessionState = SessionState.REQ_AUTH_TOKEN_USING_PASSWORD;
				_restAuthRequest.grantType(RestReactor.AUTH_PASSWORD);
				_restReactor.submitAuthRequest(_restAuthRequest, _restConnectOptions, _authTokenInfo, _errorInfo);
			}
			else
			{
				_reactorOAuthCredentialRenewal.clear();
				_reactorOAuthCredentialRenewal.userName().data(_reactorOAuthCredential.userName().toString());
				_reactorOAuthCredentialRenewal.clientId().data(_reactorOAuthCredential.clientId().toString());
				_reactorOAuthCredentialRenewal.tokenScope().data(_reactorOAuthCredential.tokenScope().toString());
				
				/* Creates the TOKEN_CREDENTIAL_RENEWAL event to the reactor for the application to submit sensitive information */
				_reactor.sendCredentialRenewalEvent(this, _errorInfo);
			}
		}
		else
		{
			_sessionState = SessionState.REQ_AUTH_TOKEN_USING_REFRESH_TOKEN;
			_restAuthRequest.grantType(RestReactor.AUTH_REFRESH_TOKEN);
			_restReactor.submitAuthRequest(_restAuthRequest, _restConnectOptions, _authTokenInfo, _errorInfo);
		}
	}
	
	boolean hasAccessToken()
	{
		boolean hasAccessToken = false;
		_accessTokenLock.lock();
		
		try
		{
			
			if(_authTokenInfo.accessToken() != null && (!_authTokenInfo.accessToken().isEmpty()))
			{
				hasAccessToken = true;
			}
		}
		finally
		{
			_accessTokenLock.unlock();
		}
		
		return hasAccessToken;
	}
	
	void onNewAuthToken(ReactorChannel reactorChannel, ReactorAuthTokenInfo authTokenInfo, ReactorErrorInfo errorInfo)
	{
       	if (reactorChannel.state() == State.UP || reactorChannel.state() == State.READY || reactorChannel.state() == State.EDP_RT)
    	{
       		// only send reissue if watchlist enabled
       		if (reactorChannel.watchlist() != null)
       			reactorChannel.reactor().loginReissue(reactorChannel, authTokenInfo.accessToken(), errorInfo);
       		
       		_reactor.sendAuthTokenEvent(reactorChannel, this, errorInfo);
       	}
	}

	@Override
	public int RestResponseCallback(RestResponse response, RestEvent event) 
	{
		lock();
		
		parseTokenInfomation(response, _authTokenInfo);
		
		unlock();
		
		switch (event.eventType())
		{
			case RestEventTypes.COMPLETED:
			{
				/* Creates and sends worker event to get an access token */
				_sessionState = SessionState.RECEIVED_AUTH_TOKEN;
				
				if(originalExpiresIn() != 0)
				{
					if(originalExpiresIn() != _authTokenInfo.expiresIn())
					{
						/* Perform another authorization using the password grant type as the refresh token is about to expire. */
						_sessionState = SessionState.AUTHENTICATE_USING_PASSWD_GRANT;
						originalExpiresIn(0); /* Unset to indicate that the password grant will be sent. */
						_reactor.sendAuthTokenWorkerEvent(this);
						return ReactorReturnCodes.SUCCESS;
					}
				}
				else
				{
					/* Set the original expires in seconds for the password grant. */
					originalExpiresIn(_authTokenInfo.expiresIn());
				}
				
				/* Iterates through the ReactorChannel list */
				_reactorChannelListLock.lock();
				
				try
				{
					for(int i = 0; i < _reactorChannelList.size(); i++)
					{
						ReactorChannel reactorChannel = _reactorChannelList.get(i);
						onNewAuthToken(reactorChannel, _authTokenInfo, event.errorInfo());
						
						if (reactorChannel.state() == State.EDP_RT)
						{
							ReactorConnectInfo reactorConnectInfo = reactorChannel.getReactorConnectInfo();
							
							/* Checks whether to get a host and port from the EDP-RT service discovery */
							if(Reactor.requestServiceDiscovery(reactorConnectInfo))
							{
								RestRequest restRequest = RestClient.createRestRequestForServiceDiscovery(reactorChannel.restConnectOptions().transport(),reactorChannel.restConnectOptions().dataFormat());
					
								reactorChannel.sessionMgntState(SessionMgntState.QUERYING_SERVICE_DISCOVERY);
								_restReactor.submitRequestForServiceDiscovery(restRequest, reactorChannel.restConnectOptions(),
										_authTokenInfo, reactorChannel.reactorServiceEndpointInfoList(), event.errorInfo());
							}
							else
							{
								reactorChannel.state(State.EDP_RT_DONE);
							}
						}
					}
				}
				finally
				{
					_reactorChannelListLock.unlock();
				}
				
				/* Send a worker event to renew the token */
				_reactor.sendAuthTokenWorkerEvent(this);
				
				break;
			}
			case RestEventTypes.FAILED:
			{
				String errorText = "failed to get an access token from the token service"; // Default error status text if data body is not defined
				
				if (response.jsonObject() != null)
				{
					errorText = response.jsonObject().toString();
				}
				
				ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
				
				_reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "ReactorTokenSession.RestResponseCallback", 
						"Failed REST request for the token service from HTTP status code " + response.statusCode() + " for user: " + _reactorOAuthCredential.userName().toString() + 
						". Text: " + errorText);
				
				/* Iterates through the ReactorChannel list */
				_reactorChannelListLock.lock();
				
				try
				{
					for(int i = 0; i < _reactorChannelList.size(); i++)
					{
						ReactorChannel reactorChannel = _reactorChannelList.get(i);
						
						/* Send the warning only when the ReactorChannel is active */
						if(reactorChannel.state() == State.READY || reactorChannel.state() == ReactorChannel.State.UP)
						{
							reactorChannel.reactor().sendChannelWarningEvent(reactorChannel, errorInfo);
						}
						
						_reactor.sendAuthTokenEvent(reactorChannel, this, errorInfo);
		
						reactorChannel.sessionMgntState(SessionMgntState.REQ_FAILURE_FOR_TOKEN_SERVICE);
						
						/* This is used to indicate that the token is no longer valid */
						reactorChannel._loginRequestForEDP.userName().data("");
						
						if(reactorChannel.state() == State.EDP_RT)
						{
							reactorChannel.state(State.EDP_RT_FAILED);
						}
					}
				}
				finally
				{
					_reactorChannelListLock.unlock();
				}
				
				_sessionState = SessionState.REQUEST_TOKEN_FAILURE;
				
				if (handlesTokenReissueFailed() )
				{
					_reactor.sendAuthTokenWorkerEvent(this);
				}
				
				break;
			}
		}

		return ReactorReturnCodes.SUCCESS;
	}

	@Override
	public int RestErrorCallback(RestEvent event, String errorText)
	{		
		/* Iterates through the ReactorChannel list */
		_reactorChannelListLock.lock();
		
		RestReactor.populateErrorInfo(event.errorInfo(),
                ReactorReturnCodes.FAILURE,
                "RestHandler.failed", "Failed REST request for the token service. Text: " + errorText);
		
		try
		{
			for(int i = 0; i < _reactorChannelList.size(); i++)
			{
				ReactorChannel reactorChannel = _reactorChannelList.get(i);
				
				/* Send the warning only when the ReactorChannel is active */
				if(reactorChannel.state() == State.READY || reactorChannel.state() == State.UP)
				{
					reactorChannel.reactor().sendChannelWarningEvent(reactorChannel, event.errorInfo());
				}
				
				_reactor.sendAuthTokenEvent(reactorChannel, this, event.errorInfo());
				
				reactorChannel.sessionMgntState(SessionMgntState.REQ_FAILURE_FOR_TOKEN_SERVICE);
				
				/* This is used to indicate that the token is no longer valid */
				reactorChannel._loginRequestForEDP.userName().data("");
				
				if(reactorChannel.state() == State.EDP_RT)
				{
					reactorChannel.state(State.EDP_RT_FAILED);
				}
			}	
		}
		finally
		{
			_reactorChannelListLock.unlock();
		}
		
		_sessionState = SessionState.REQUEST_TOKEN_FAILURE;
		
		if (handlesTokenReissueFailed() )
		{
			_reactor.sendAuthTokenWorkerEvent(this);
		}
		
		return ReactorReturnCodes.SUCCESS;
	}
}