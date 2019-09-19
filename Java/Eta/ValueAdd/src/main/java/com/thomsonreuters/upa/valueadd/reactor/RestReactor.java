package com.thomsonreuters.upa.valueadd.reactor;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.URISyntaxException;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.net.ssl.SSLContext;

import org.apache.http.Consts;
import org.apache.http.Header;
import org.apache.http.HeaderIterator;
import org.apache.http.HttpEntity;
import org.apache.http.HttpHeaders;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.NameValuePair;
import org.apache.http.ParseException;
import org.apache.http.StatusLine;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.protocol.HttpClientContext;
import org.apache.http.client.utils.URIBuilder;
import org.apache.http.config.ConnectionConfig;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.impl.nio.DefaultHttpClientIODispatch;
import org.apache.http.impl.nio.pool.BasicNIOConnFactory;
import org.apache.http.impl.nio.pool.BasicNIOConnPool;
import org.apache.http.impl.nio.reactor.DefaultConnectingIOReactor;
import org.apache.http.impl.nio.reactor.IOReactorConfig;
import org.apache.http.message.BasicHttpEntityEnclosingRequest;
import org.apache.http.message.BasicNameValuePair;
import org.apache.http.nio.protocol.BasicAsyncRequestProducer;
import org.apache.http.nio.protocol.BasicAsyncResponseConsumer;
import org.apache.http.nio.protocol.HttpAsyncRequestExecutor;
import org.apache.http.nio.protocol.HttpAsyncRequester;
import org.apache.http.nio.reactor.ConnectingIOReactor;
import org.apache.http.nio.reactor.IOEventDispatch;
import org.apache.http.nio.reactor.IOReactorException;
import org.apache.http.protocol.HTTP;
import org.apache.http.protocol.HttpCoreContext;
import org.apache.http.protocol.HttpProcessorBuilder;
import org.apache.http.protocol.RequestConnControl;
import org.apache.http.protocol.RequestContent;
import org.apache.http.protocol.RequestExpectContinue;
import org.apache.http.protocol.RequestTargetHost;
import org.apache.http.protocol.RequestUserAgent;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.util.EntityUtils;
import org.json.JSONException;
import org.json.JSONObject;

class RestReactor
{
	static final String AUTH_GRANT_TYPE = "grant_type";
	static final String AUTH_USER_NAME = "username";
	static final String AUTH_PASSWORD = "password";
	static final String AUTH_CLIENT_ID = "client_id";
	static final String AUTH_TAKE_EXCLUSIVE_SIGN_ON_CONTROL = "takeExclusiveSignOnControl";
	static final String AUTH_SCOPE = "scope";
	static final String AUTH_BEARER = "Bearer ";
	static final String AUTH_REFRESH_TOKEN = "refresh_token";
	static final String AUTH_ACCESS_TOKEN = "access_token";
	static final String AUTH_EXPIRES_IN = "expires_in";
	static final String AUTH_TOKEN_TYPE = "token_type";
	static final String AUTH_POST = "POST";
	static final String AUTH_REQUEST_USER_AGENT = "HTTP/1.1";
	
	private boolean _reactorActive;
	private RestReactorOptions _restReactorOptions = new RestReactorOptions();
	private final SSLContextBuilder	_sslContextBuilder = new SSLContextBuilder();
	private SSLConnectionSocketFactory _sslconSocketFactory;
    private final HttpAsyncRequestExecutor _protocolHandler = new HttpAsyncRequestExecutor();
	private IOEventDispatch _ioEventDispatch;
    private ConnectingIOReactor _ioReactor;
    private BasicNIOConnPool _pool;
    
    // Separate instance of handling blocking and non-blocking for proxy authentication
    private RestProxyAuthHandler _restProxyAuthHandler;
    private RestProxyAuthHandler _restProxyAuthHandlerForNonBlocking;
        
    public RestReactor(RestReactorOptions options, ReactorErrorInfo errorInfo)
    {
    	 if (errorInfo == null)
         {
             throw new UnsupportedOperationException("ReactorErrorInfo cannot be null");
         }
         else if (options != null)
         {
        	 options.copy(_restReactorOptions);
         }
         else
         {
             populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "RestReactor.constructor",
                               "options was null and cannot continue.");
         }
    	 
    	 try
         {
    		 final SSLContext sslContext =  _sslContextBuilder.build();
    		 
    		 // used by blocking interfaces
    		 _sslconSocketFactory = new SSLConnectionSocketFactory(sslContext);
    		 
    		 _restProxyAuthHandler = new RestProxyAuthHandler(this, _sslconSocketFactory);
    		 _restProxyAuthHandlerForNonBlocking = new RestProxyAuthHandler(this, _sslconSocketFactory);
    		
    		 final ConnectionConfig connectionConfig = ConnectionConfig.custom()
    				 .setBufferSize(options.bufferSize())
    				 .setFragmentSizeHint(options.fragmentSizeHint())
    				 .build();
    		 
    		 _ioEventDispatch = new DefaultHttpClientIODispatch<HttpAsyncRequestExecutor>(_protocolHandler, sslContext, connectionConfig);
    		 
			_ioReactor = new DefaultConnectingIOReactor(IOReactorConfig.custom()
				      .setConnectTimeout(options.connectTimeout())
				      .setSoTimeout(options.soTimeout())
				      .setTcpNoDelay(options.tcpNoDelay())
				      .setIoThreadCount(options.ioThreadCount())
				      .setSoKeepAlive(options.soKeepAlive())
				      .setSelectInterval(options.selectInterval())
				      .setShutdownGracePeriod(options.shutdownGracePeriod())
				      .build());
			
	   		final BasicNIOConnFactory nioConnFactory = new BasicNIOConnFactory(sslContext, null, connectionConfig);
			_pool = new BasicNIOConnPool(_ioReactor, nioConnFactory, options.connectTimeout());
		    _pool.setDefaultMaxPerRoute(options.defaultMaxPerRoute());
		    _pool.setMaxTotal(options.maxConnectTotal());
		   
    			
         }
    	 catch (KeyManagementException | NoSuchAlgorithmException e)
         {
             populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "RestReactor.initialize",
                                      "failed to initialize the SSLConnectionSocketFactory, exception="
                                              + getExceptionCause(e));
             return;
         }
    	 catch (IOReactorException e)
         {
             populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "RestReactor.initialize",
                                      "failed to initialize the DefaultConnectingIOReactor, exception="
                                              + getExceptionCause(e));
             return;
         }
    	 
         _reactorActive = true;
    }

    RestReactorOptions reactorOptions()
    {
    	return _restReactorOptions;
    }

    /* Clears then populates the specified errorInfo object. */
    static int populateErrorInfo(ReactorErrorInfo errorInfo, int reactorReturnCode, String location, String text)
    {
        errorInfo.clear();
        errorInfo.code(reactorReturnCode).location(location);
        errorInfo.error().errorId(reactorReturnCode);
        if (text != null)
        	errorInfo.error().text(text);
        return reactorReturnCode;
    }
    
    public int submitAuthRequest(ReactorChannel reactorChannel, final RestAuthOptions options, final RestConnectOptions restConnectOptions, final ReactorErrorInfo errorInfo)
   	{
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.submitAuthRequest", "RestReactor is not active, aborting");
    	}

		final List<NameValuePair> params = new ArrayList<>(6);
		params.add(new BasicNameValuePair(AUTH_GRANT_TYPE, options.grantType()));
		params.add(new BasicNameValuePair(AUTH_USER_NAME, ((ConsumerRole)reactorChannel.role()).rdmLoginRequest().userName().toString()));		
		
		if  (options.clientId() == null || options.clientId().length() == 0)
		{	  
			return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.PARAMETER_INVALID,
                    "RestReactor.submitAuthRequest", "Required parameter clientId is not set");
		}
		else
			params.add(new BasicNameValuePair(AUTH_CLIENT_ID,  options.clientId().toString()));
		
		if (options.hasRefreshToken() && options.grantType().equals(AUTH_REFRESH_TOKEN)) //for new refresh token
		{
			params.add(new BasicNameValuePair(AUTH_REFRESH_TOKEN, reactorChannel._reactorAuthTokenInfo.refreshToken()));			
			//must set for the first access_token, otherwise receive status code: 403 forbidden.
			//must not include scope if the scope for reissue is same, client will issue new token in the same scope.
		}
		else 
		{
			params.add(new BasicNameValuePair(AUTH_TAKE_EXCLUSIVE_SIGN_ON_CONTROL, "true"));
			params.add(new BasicNameValuePair(AUTH_SCOPE, options.tokenScope())); 
			params.add(new BasicNameValuePair(AUTH_PASSWORD, reactorChannel._loginRequestForEDP.password().toString()));
		}

		final String url = restConnectOptions.tokenServiceURL();

		final RestHandler restHandler = new RestHandler(this, reactorChannel);
		
		if( (restConnectOptions.proxyHost() == null ||restConnectOptions.proxyHost().isEmpty() ) || (restConnectOptions.proxyPort() == -1) )
		{
		    final BasicHttpEntityEnclosingRequest httpRequest = new BasicHttpEntityEnclosingRequest(AUTH_POST, url);
		    
		    httpRequest.setEntity(new UrlEncodedFormEntity(params, Consts.UTF_8));
		    if ( options.hasHeaderAttribute() )	
		    {
		    	Map<String,String> headerAttribs = options.headerAttribute();
		    	for (Map.Entry<String,String> entry : headerAttribs.entrySet())
		    		httpRequest.addHeader(entry.getKey(), entry.getValue());
		    }
			final HttpAsyncRequester requester = new HttpAsyncRequester(HttpProcessorBuilder.create()
	                .add(new RequestContent())
	                .add(new RequestTargetHost())
	                .add(new RequestConnControl())
	                .add(new RequestUserAgent(AUTH_REQUEST_USER_AGENT))
	                .add(new RequestExpectContinue(true)).build());
			
			  requester.execute(
		                 new BasicAsyncRequestProducer(restConnectOptions.tokenServiceHost(), httpRequest),
		                 new BasicAsyncResponseConsumer(),
		                 _pool,
		                 HttpClientContext.create(),
		                 restHandler);
		}
		else
		{
			new Thread() {
				public void run() {
					
					final HttpPost httppost = new HttpPost(url);
					
				    if ( options.hasHeaderAttribute() )
				    {
				    	Map<String,String> headerAttribs = options.headerAttribute();
				    	for (Map.Entry<String,String> entry : headerAttribs.entrySet())
				    		httppost.addHeader(entry.getKey(), entry.getValue());
				    }
				    
				    httppost.setEntity(new UrlEncodedFormEntity(params, Consts.UTF_8));
				    
				    HttpHost proxy = new HttpHost(restConnectOptions.proxyHost(), restConnectOptions.proxyPort(), "http");
		   			RequestConfig config = RequestConfig.custom()
		                    .setProxy(proxy)
		                    .build();
		   			httppost.setConfig(config);
		   			
		   			try {
						_restProxyAuthHandlerForNonBlocking.execute(httppost, restConnectOptions, errorInfo, restHandler);
					} catch (IOException e) {
						restHandler.failed(e);
					}
		   	
				    }
			}.start();
			
		}

        return ReactorReturnCodes.SUCCESS;
   	}
    
    public int submitRequestForServiceDiscovery(RestRequest request, ReactorChannel reactorChannel, final ReactorErrorInfo errorInfo) 
    {
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.submitRequest", "RestReactor is not active, aborting");
    	}
    	
    	final RestConnectOptions restConnectOptions = reactorChannel.restConnectOptions();
    	 		
    	URIBuilder uriBuilder = null;
    	
    	try {
    		uriBuilder = new URIBuilder(restConnectOptions.serviceDiscoveryURL());    		
    	}
    	catch (Exception e)
    	{
	    	return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.submitRequestBlocking", "failed to submit a request, exception = " + getExceptionCause(e));
    	}
		
		Map<String,String> queryParameter = request.queryParameter();
		if ( queryParameter != null )
		{
			for (Map.Entry<String,String> entry : queryParameter.entrySet())
				uriBuilder.setParameter(entry.getKey(), entry.getValue());
		}
		
		String url = null;
		try
		{
			url = uriBuilder.build().toString();
		}
		catch (URISyntaxException e)
		{
			 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.submitRequest", "failed to submit a request, exception = " + getExceptionCause(e));
		}
		
		final HttpGet httpRequest = new HttpGet(url);
		if ( request.hasHeaderAttribute() )
		{
			Map<String,String> headerAttribs = request.headerAttribute();
			for (Map.Entry<String,String> entry : headerAttribs.entrySet())
				httpRequest.addHeader(entry.getKey(), entry.getValue());
		}
		
		String token = reactorChannel._reactorAuthTokenInfo.accessToken();
		
		httpRequest.setHeader(HttpHeaders.AUTHORIZATION, AUTH_BEARER + token);
		
		final RestHandler restHandler = new RestHandler(this, reactorChannel);
		
		if( (restConnectOptions.proxyHost() == null ||restConnectOptions.proxyHost().isEmpty() ) || (restConnectOptions.proxyPort() == -1) )
		{
			final HttpAsyncRequester requester = new HttpAsyncRequester(HttpProcessorBuilder.create()
	                .add(new RequestContent())
	                .add(new RequestTargetHost())
	                .add(new RequestConnControl())
	                .add(new RequestUserAgent(AUTH_REQUEST_USER_AGENT))
	                .add(new RequestExpectContinue(true)).build());
			requester.execute(
	             new BasicAsyncRequestProducer(restConnectOptions.serviceDiscoveryHost(), httpRequest),
	             new BasicAsyncResponseConsumer(),
	             _pool,
	             HttpCoreContext.create(),
	             restHandler);
		}
		else
		{
			new Thread() {
				public void run() {
				    
				    HttpHost proxy = new HttpHost(restConnectOptions.proxyHost(), restConnectOptions.proxyPort(), "http");
		   			RequestConfig config = RequestConfig.custom()
		                    .setProxy(proxy)
		                    .build();
		   			httpRequest.setConfig(config);
		   			
		   			try {
						_restProxyAuthHandlerForNonBlocking.execute(httpRequest, restConnectOptions, errorInfo, restHandler);
					} catch (IOException e) {
					
						restHandler.failed(e);
					}				    
				    }
			}.start();
		}

        return ReactorReturnCodes.SUCCESS;
    }
    
    
    public int dispatch(ReactorErrorInfo errorInfo)
    {
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.dispatch", "RestReactor is not active, aborting");
    	}
    	
    	try {
            // Ready to go!
            _ioReactor.execute(_ioEventDispatch);
            
        } catch (final InterruptedIOException e) {
        	 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.dispatch",
                     "received Interrupted IOException, exception = "
                             + getExceptionCause(e));
        } catch (final IOException e) {
        	return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.dispatch",
                    "received IOException, exception = "
                            + getExceptionCause(e));
        }
    	
    	shutdown(errorInfo);
        
        return ReactorReturnCodes.SUCCESS;
    }
    
	public boolean isShutdown()
    {
        return !_reactorActive;
    }

    public int shutdown(ReactorErrorInfo errorInfo)
    {
    	if (!_reactorActive)
    		 return ReactorReturnCodes.SUCCESS;
    	
    	_reactorActive = false;
    	
        try
		{
			_ioReactor.shutdown(_restReactorOptions.shutdownGracePeriod());
			_ioReactor = null;
		} catch (IOException e)
		{
			return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.shutdown",
                    "received IOException, exception = "
                            + getExceptionCause(e));
		}
        
        return ReactorReturnCodes.SUCCESS;
    }
    
    public int submitAuthRequestBlocking(RestAuthOptions options, 
    		RestConnectOptions restConnectOptions, ReactorErrorInfo errorInfo) throws IOException
   	{
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.submitAuthRequest", "RestReactor is not active, aborting");
    	}
    	
    	if (_sslconSocketFactory == null)
    		return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.submitAuthRequestBlocking", "failed to initialize the SSLConnectionSocketFactory");
    	
    	final List<NameValuePair> params = new ArrayList<>(6);
		params.add(new BasicNameValuePair(AUTH_GRANT_TYPE,options.grantType()));
		params.add(new BasicNameValuePair(AUTH_USER_NAME, options.username()));
		params.add(new BasicNameValuePair(AUTH_PASSWORD, options.password()));
		params.add(new BasicNameValuePair(AUTH_CLIENT_ID, options.clientId()));
		params.add(new BasicNameValuePair(AUTH_TAKE_EXCLUSIVE_SIGN_ON_CONTROL, "true")); //must set true here
		if (options.hasRefreshToken()) //for new refresh token
			params.add(new BasicNameValuePair(AUTH_REFRESH_TOKEN, options.refreshToken()));
		//must set for the first access_token, otherwise receive status code: 403 forbidden.
		//must not include scope if the scope for reissue is same, client will issue new token in the same scope.
		else 
			params.add(new BasicNameValuePair(AUTH_SCOPE, options.tokenScope())); 
	
   		try
   		{
   			final UrlEncodedFormEntity entity = new UrlEncodedFormEntity(params, Consts.UTF_8);
   			if (options.hasHeaderAttribute())
   			{
   				Map<String,String>  headers = options.headerAttribute();
   				if (headers.containsKey(HttpHeaders.TRANSFER_ENCODING) && headers.get(HttpHeaders.TRANSFER_ENCODING).contentEquals(HTTP.CHUNK_CODING))
   					entity.setChunked(true);
   				if (headers.containsKey(HttpHeaders.CONTENT_ENCODING))
   					entity.setContentEncoding(headers.get(HttpHeaders.CONTENT_ENCODING));
   				if (headers.containsKey(HttpHeaders.CONTENT_TYPE))
   					entity.setContentType(headers.get(HttpHeaders.CONTENT_TYPE));
   			}
   			
   			String url = restConnectOptions.tokenServiceURL();

   			final HttpPost httppost = new HttpPost(url);

   			httppost.setEntity(entity);
   			
   			if( (restConnectOptions.proxyHost() != null && !restConnectOptions.proxyHost().isEmpty()) && (restConnectOptions.proxyPort() != -1))
   			{
   				HttpHost proxy = new HttpHost(restConnectOptions.proxyHost(), restConnectOptions.proxyPort(), "http");
	   			RequestConfig config = RequestConfig.custom()
	                    	.setProxy(proxy).setSocketTimeout(_restReactorOptions.soTimeout())
	                    	.build();
	   			httppost.setConfig(config);
	   			
   				return _restProxyAuthHandler.execute(httppost, restConnectOptions, errorInfo, null);
   			}
   			else
   			{
   				final CloseableHttpClient httpClient = HttpClientBuilder.create().setSSLSocketFactory(_sslconSocketFactory).build();
   				RequestConfig config = RequestConfig.custom().setSocketTimeout(_restReactorOptions.soTimeout()).build();
   				httppost.setConfig(config);
   				try
   				{
		   			final HttpResponse response = httpClient.execute(httppost);
		   			if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
		   			{
		   				populateErrorInfo(errorInfo,   				
		                            ReactorReturnCodes.FAILURE,
		                            "RestReactor.submitAuthRequestBlocking", 
		                            "Failed to request authentication token information. Text: " 
		                            + EntityUtils.toString(response.getEntity()));
		   				
		   				return ReactorReturnCodes.SUCCESS;
		   			}
		   			else
		   			{
		   				RestEvent event = new RestEvent(RestEventTypes.COMPLETED, restConnectOptions.userSpecObject());
		   				RestResponse resp = new RestResponse();
		   				
		   				convertResponse(this, response, resp, event);
		   				processResponse(this, resp, event);
		   			}
   				}
   				finally
   				{
   					httpClient.close();
   				}
   			}
   		}
   		catch(JSONException | ParseException | IOException e)
   		{
   			 return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "RestReactor.submitAuthRequestBlocking", "failed to submit authorization request, exception = " + getExceptionCause(e));
   		}

   		return ReactorReturnCodes.SUCCESS;
   	}
    
    public int submitServiceDiscoveryRequestBlocking(RestRequest request, 
    		RestConnectOptions restConnectOptions, ReactorErrorInfo errorInfo) throws IOException
   	{
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.FAILURE,
                     "RestReactor.submitServiceDiscoveryRequestBlocking", "RestReactor is not active, aborting");
    	}
    	
    	if (_sslconSocketFactory == null)
    		return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.submitServiceDiscoveryRequestBlocking", "failed to initialize the SSLConnectionSocketFactory");
    	
    	URIBuilder uriBuilder = null;
    	
    	try {
    		uriBuilder = new URIBuilder(restConnectOptions.serviceDiscoveryURL());    		
    	}
    	catch (Exception e)
    	{
	    	return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.submitServiceDiscoveryRequestBlocking", "failed to submit a request, exception = " + getExceptionCause(e));
    	}
    			
    	if (request.hasQueryParameter())
    	{
			Map<String,String> queryParameter = request.queryParameter();
			if (queryParameter != null)
			{
				for (Map.Entry<String,String> entry : queryParameter.entrySet())
				{
					uriBuilder.setParameter(entry.getKey(), entry.getValue());
				}
			}
    	}
		    	
		try
		{
			HttpGet httpget = new HttpGet(uriBuilder.build());
			
			if (request.hasHeaderAttribute())
   			{
   				for (Map.Entry<String, String> entry : request.headerAttribute().entrySet())
   					httpget.addHeader(entry.getKey(), entry.getValue());
   			}

			String token = restConnectOptions.tokenInformation().accessToken();
			
			httpget.setHeader(HttpHeaders.AUTHORIZATION, AUTH_BEARER + token);
		
			if((restConnectOptions.proxyHost() != null && !restConnectOptions.proxyHost().isEmpty()) && (restConnectOptions.proxyPort() != -1))
   			{
   				HttpHost proxy = new HttpHost(restConnectOptions.proxyHost(), restConnectOptions.proxyPort(), "http");
	   			RequestConfig config = RequestConfig.custom()
	                    .setProxy(proxy).setSocketTimeout(_restReactorOptions.soTimeout())
	                    .build();
	   			httpget.setConfig(config);
	   			
   				return _restProxyAuthHandler.execute(httpget, restConnectOptions, errorInfo, null);
   			}
   			else
   			{
   				final CloseableHttpClient httpClient = HttpClientBuilder.create().setSSLSocketFactory(_sslconSocketFactory).build();
   				RequestConfig config = RequestConfig.custom().setSocketTimeout(_restReactorOptions.soTimeout()).build();
	   			httpget.setConfig(config);
   				try
   				{
	   				HttpResponse response = httpClient.execute(httpget);

	   				if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
					{
		   				populateErrorInfo(errorInfo,   				
		                        ReactorReturnCodes.FAILURE,
		                        "RestReactor.submitServiceDiscoveryRequestBlocking", 
		                        "Failed to request service discovery information. Text: " 
		                        + EntityUtils.toString(response.getEntity()));
		   				
		   				return ReactorReturnCodes.FAILURE;
					}
					else
					{
						final RestEvent event = new RestEvent(RestEventTypes.COMPLETED, restConnectOptions.userSpecObject());
		   				final RestResponse resp = new RestResponse();
		   				
		   				convertResponse(this, response, resp, event);
		   				processResponse(this, resp, event);
					}
   				}
   				finally
   				{
   					httpClient.close();
   				}
   			}
   		}
	    catch (URISyntaxException e)
		{
	    	return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.submitServiceDiscoveryRequestBlocking", "failed to submit a request, exception = " + getExceptionCause(e));
		}
   		catch(ClientProtocolException e)
   		{
   			return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "RestReactor.submitServiceDiscoveryRequestBlocking", "failed to submit a request, exception = " + getExceptionCause(e));
   			
   		}
   		catch (IOException e)
   		{
   			return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.submitServiceDiscoveryRequestBlocking", "failed to submit a request, exception = " + getExceptionCause(e));
   		}
   		
   		return ReactorReturnCodes.SUCCESS;
   	}
    
    static int convertResponse(RestReactor RestReactor, HttpResponse response, RestResponse clientResponse, RestEvent event)
    {
    	HttpEntity entity = response.getEntity();
    	String entityString = null;
    	try
		{
			entityString =  EntityUtils.toString(entity);
		} catch (ParseException | IOException e)
    	{
			return populateErrorInfo(event.errorInfo(),
                    ReactorReturnCodes.FAILURE,
                    "RestHandler.handleResponse", "failed to convert entity to json object, exception = " + getExceptionCause(e));
    	}
    	
		StatusLine statusLine = response.getStatusLine();
		
		clientResponse.statusCode(statusLine.getStatusCode());
		clientResponse.statusText(statusLine.getReasonPhrase());
		clientResponse.protocolVersion(statusLine.getProtocolVersion());
		
		HeaderIterator headerIt = response.headerIterator();
		while(headerIt.hasNext())
		{
			Header header = headerIt.nextHeader();
			clientResponse.headerAttribute().put(header.getName(), header.getValue());
		}

		if (entity != null)
		{
			Header contentType = entity.getContentType();
			
			if(contentType != null)
			{
				clientResponse.contentType(entity.getContentType().getValue());
			}
			
			clientResponse.body(entityString, event.errorInfo());
		}
		
		return ReactorReturnCodes.SUCCESS;
	}
    
    static int processResponse(RestReactor restReactor, RestResponse response, RestEvent event)
    {
		if (response.statusCode() == HttpStatus.SC_OK && response.jsonObject() != null && response.jsonObject().has(AUTH_ACCESS_TOKEN))
		{
			JSONObject body =  response.jsonObject();
			ReactorAuthTokenInfo tokenInfo = new ReactorAuthTokenInfo();
			tokenInfo.clear();
			tokenInfo.accessToken(body.getString(AUTH_ACCESS_TOKEN));
			tokenInfo.refreshToken(body.getString(AUTH_REFRESH_TOKEN));
			tokenInfo.expiresIn(body.getInt(AUTH_EXPIRES_IN));
			tokenInfo.scope(body.getString(AUTH_SCOPE));
			tokenInfo.tokenType(body.getString(AUTH_TOKEN_TYPE));
			
			event._reactorAuthTokenInfo = tokenInfo;
			
			if (restReactor._restReactorOptions.authorizationCallback() != null)
			{
				restReactor._restReactorOptions.authorizationCallback().RestResponseCallback(response, event);
			}
			else if (restReactor._restReactorOptions.defaultRespCallback() != null)
			{
				restReactor._restReactorOptions.defaultRespCallback().RestResponseCallback(response, event);
			}
			return ReactorReturnCodes.SUCCESS;
		}
		
		if (restReactor._restReactorOptions.defaultRespCallback() != null)
		{
			restReactor._restReactorOptions.defaultRespCallback().RestResponseCallback(response, event);
		}
		
		return ReactorReturnCodes.SUCCESS;
    }
    
    static String getExceptionCause(Exception e)
    {
    	String  cause = e.getLocalizedMessage();
		if (cause == null)
		{
			cause = " ";
			Throwable test = e.getCause();
			while (test != null)
			{
				cause += test.getLocalizedMessage();
				test = test.getCause();
			}
		}
		return cause;
    }
    
}
