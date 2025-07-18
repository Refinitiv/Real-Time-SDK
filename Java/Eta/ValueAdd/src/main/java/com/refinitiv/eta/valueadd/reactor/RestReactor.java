/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import javax.net.ssl.SSLContext;

import org.apache.hc.client5.http.async.methods.*;
import org.apache.hc.client5.http.config.ConnectionConfig;
import org.apache.hc.client5.http.config.TlsConfig;
import org.apache.hc.client5.http.impl.async.CloseableHttpAsyncClient;
import org.apache.hc.client5.http.impl.async.HttpAsyncClients;
import org.apache.hc.client5.http.impl.io.PoolingHttpClientConnectionManager;
import org.apache.hc.client5.http.impl.io.PoolingHttpClientConnectionManagerBuilder;
import org.apache.hc.client5.http.impl.nio.PoolingAsyncClientConnectionManager;
import org.apache.hc.client5.http.impl.nio.PoolingAsyncClientConnectionManagerBuilder;
import org.apache.hc.client5.http.ssl.ClientTlsStrategyBuilder;
import org.apache.hc.client5.http.ssl.TlsSocketStrategy;
import org.apache.hc.core5.http.*;
import org.apache.hc.core5.http.io.HttpClientResponseHandler;
import org.apache.hc.core5.http.io.SocketConfig;
import org.apache.hc.core5.http.io.entity.ByteArrayEntity;
import org.apache.hc.core5.http.io.entity.StringEntity;
import org.apache.hc.core5.http.message.StatusLine;
import org.apache.hc.client5.http.ClientProtocolException;
import org.apache.hc.client5.http.classic.HttpClient;
import org.apache.hc.client5.http.config.RequestConfig;
import org.apache.hc.client5.http.entity.UrlEncodedFormEntity;
import org.apache.hc.client5.http.classic.methods.HttpGet;
import org.apache.hc.client5.http.classic.methods.HttpPost;
import org.apache.hc.client5.http.classic.methods.HttpUriRequestBase;
import org.apache.hc.client5.http.protocol.HttpClientContext;
import org.apache.hc.core5.http.protocol.*;
import org.apache.hc.core5.http.ssl.TLS;
import org.apache.hc.core5.http2.HttpVersionPolicy;
import org.apache.hc.core5.io.CloseMode;
import org.apache.hc.core5.net.URIBuilder;
import org.apache.hc.client5.http.impl.classic.CloseableHttpClient;
import org.apache.hc.client5.http.impl.classic.HttpClientBuilder;
import org.apache.hc.core5.http.message.BasicNameValuePair;
import org.apache.hc.core5.net.WWWFormCodec;
import org.apache.hc.core5.pool.PoolConcurrencyPolicy;
import org.apache.hc.core5.pool.PoolReusePolicy;
import org.apache.hc.core5.ssl.SSLContexts;
import org.apache.hc.core5.util.TimeValue;
import org.apache.hc.core5.util.Timeout;
import org.apache.hc.core5.ssl.SSLContextBuilder;
import org.apache.hc.core5.http.io.entity.EntityUtils;
import org.json.JSONException;
import org.slf4j.Logger;

import org.slf4j.LoggerFactory;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorAuthTokenInfo.TokenVersion;

import org.jose4j.lang.*;
import org.jose4j.jwt.*;
import org.jose4j.jwk.*;
import org.jose4j.jws.*;


class RestReactor
{
	static final String AUTH_GRANT_TYPE = "grant_type";
	static final String AUTH_USER_NAME = "username";
	static final String AUTH_PASSWORD = "password";
	static final String AUTH_NEWPASSWORD = "newPassword";
	static final String AUTH_CLIENT_ID = "client_id";
	static final String AUTH_CLIENT_SECRET = "client_secret";
	static final String AUTH_CLIENT_CREDENTIALS_GRANT = "client_credentials";
	static final String AUTH_TAKE_EXCLUSIVE_SIGN_ON_CONTROL = "takeExclusiveSignOnControl";
	static final String AUTH_SCOPE = "scope";
	static final String AUTH_BEARER = "Bearer ";
	static final String AUTH_REFRESH_TOKEN = "refresh_token";
	static final String AUTH_ACCESS_TOKEN = "access_token";
	static final String AUTH_EXPIRES_IN = "expires_in";
	static final String AUTH_TOKEN_TYPE = "token_type";
	static final String AUTH_POST = "POST";
	static final String AUTH_REQUEST_USER_AGENT = "HTTP/1.1";
	static final String AUTH_CLIENT_ASSERTION = "client_assertion";
	static final String AUTH_CLIENT_ASSERTION_TYPE = "client_assertion_type";
	static final String AUTH_CLIENT_ASSERTION_VALUE = "urn:ietf:params:oauth:client-assertion-type:jwt-bearer";

	static final String AUTH_DEFAULT_AUDIENCE = "https://login.ciam.refinitiv.com/as/token.oauth2";

	static final int AUTH_HANDLER = 1;
	static final int DISCOVERY_HANDLER = 2;

	private boolean _reactorActive;
	private RestReactorOptions _restReactorOptions = new RestReactorOptions();
	private final SSLContextBuilder	_sslContextBuilder = new SSLContextBuilder();
    private RestProxyAuthHandler _restProxyAuthHandler;
	private PoolingAsyncClientConnectionManager _connectionManager;
	private CloseableHttpAsyncClient _asyncClient;

    private Logger loggerClient = null;
    private List<StringBuilder> bufferPool = null;
    Lock lock = new ReentrantLock();

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

    		 _restProxyAuthHandler = new RestProxyAuthHandler(this);

			 _connectionManager = PoolingAsyncClientConnectionManagerBuilder.create()
					 .setMaxConnTotal(options.maxConnectTotal())
					 .setMaxConnPerRoute(options.defaultMaxPerRoute())
					 .setTlsStrategy(ClientTlsStrategyBuilder.create()
							 .setSslContext(sslContext)
							 .setTlsVersions(TLS.V_1_2, TLS.V_1_3)
							 .buildAsync())
					 .setPoolConcurrencyPolicy(PoolConcurrencyPolicy.STRICT)
					 .setConnPoolPolicy(PoolReusePolicy.LIFO)
					 .setDefaultConnectionConfig(ConnectionConfig.custom()
							 .setSocketTimeout(Timeout.ofMilliseconds(options.soTimeout()))
							 .setConnectTimeout(Timeout.ofMilliseconds(options.connectTimeout()))
							 .setTimeToLive(TimeValue.ofMinutes(10))
							 .build())
					 .setDefaultTlsConfig(TlsConfig.custom()
							 .setVersionPolicy(HttpVersionPolicy.NEGOTIATE)
							 .setHandshakeTimeout(Timeout.ofMinutes(1))
							 .build())
					 .build();

			 _asyncClient = HttpAsyncClients.custom()
					 .setIOReactorConfig(org.apache.hc.core5.reactor.IOReactorConfig.custom()
					 	.setSoTimeout(options.soTimeout(), TimeUnit.MILLISECONDS)
					 	.setTcpNoDelay(options.tcpNoDelay())
					 	.setSoKeepAlive(options.soKeepAlive())
					 	.setIoThreadCount(options.ioThreadCount())
					 	.setSelectInterval(TimeValue.of(options.selectInterval(), TimeUnit.MILLISECONDS))
					 	.build())
					 .setConnectionManager(_connectionManager)
					 .build();

			 _asyncClient.start();
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
    	 catch (Exception e)
         {
             populateErrorInfo(errorInfo,
                                      ReactorReturnCodes.FAILURE,
                                      "RestReactor.initialize",
                                      "failed to initialize the DefaultConnectingIOReactor, exception="
                                              + getExceptionCause(e));
             return;
         }

         _reactorActive = true;

         loggerClient = LoggerFactory.getLogger(this.getClass());
         bufferPool = new ArrayList<>();
    }

    RestReactorOptions restReactorOptions()
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

    private String getClientJwt(String clientId, String clientJwk, String endpointUrl, String audience, final ReactorErrorInfo errorInfo)
    {
		Date currentDate = new Date( System.currentTimeMillis() );
		// Set expiry time as current time plus 15 minutes.
	    long expTime = (currentDate.getTime() / 1000) + 900L;
	    long iatTime = currentDate.getTime() / 1000;
	    PublicJsonWebKey parsedJwkKeyPair = null;
	    JsonWebKey webkey = null;

	    String tmpAudience = null;

	    /* Get the Json Web Key out of the JWK JSON string */
	    try {
	    	webkey = JsonWebKey.Factory.newJwk(clientJwk);
	    	if(webkey == null)
	    	{
	    		populateErrorInfo(errorInfo,
	                     ReactorReturnCodes.FAILURE,
	                     "RestReactor.getClientAssertion", "Unable to parse the clientJwk string.");
	    		return null;
	    	}
	    	/* Get the JWK as a string from the webkey object */
	    	String jwkAsString = webkey.toJson(JsonWebKey.OutputControlLevel.INCLUDE_PRIVATE);
	    	parsedJwkKeyPair = PublicJsonWebKey.Factory.newPublicJwk(jwkAsString);

	    } catch (JoseException e) {
	    	populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.getClientAssertion", "Jose4j exception: " + e.getMessage());
		    return null;
		} catch (Exception e) {
			populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.getClientAssertion", "Exception: " + e.getMessage());
		    return null;
		}

	    if(audience == null || audience.isEmpty())
	    {
	    	tmpAudience = AUTH_DEFAULT_AUDIENCE;
	    }
	    else
	    {
	    	tmpAudience = audience;
	    }

	    JwtClaims claims = new JwtClaims();
		claims.setClaim("iss", clientId);
		claims.setClaim("sub", clientId);
		claims.setClaim("aud", tmpAudience);
		claims.setClaim("exp", expTime);
		claims.setClaim("iat", iatTime);

		// Create the JWT based on the claims above and the webkey.
		try {
			JsonWebSignature jws = new JsonWebSignature();
			jws.setPayload(claims.toJson());
			jws.setKey(parsedJwkKeyPair.getPrivateKey());
			jws.setHeader("alg", webkey.getAlgorithm());
			jws.setHeader("typ", "JWT");
			jws.setHeader("kid", webkey.getKeyId());

			// Sign the JWT with our private key
			jws.sign();

			// Convert to text
			String sResult = jws.getCompactSerialization();
			return sResult;
		} catch (JoseException e) {
			populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.getClientAssertion", "Exception from JWT signing: " + e.getMessage());
		    return null;
		}
    }

    public int submitAuthRequest(RestAuthOptions authOptions, final RestConnectOptions restConnectOptions,
   		 ReactorAuthTokenInfo authTokenInfo, final ReactorErrorInfo errorInfo,
   		 boolean redirect, String location)
  	{
    	restConnectOptions.authRedirect(redirect);
    	restConnectOptions.authRedirectLocation(location);

    	return submitAuthRequest(authOptions, restConnectOptions, authTokenInfo, errorInfo);
  	}

    public int submitAuthRequest(RestAuthOptions authOptions, final RestConnectOptions restConnectOptions,
    		 ReactorAuthTokenInfo authTokenInfo, final ReactorErrorInfo errorInfo) {
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.SHUTDOWN,
                     "RestReactor.submitAuthRequest", "RestReactor is not active, aborting");
    	}

    	final List<NameValuePair> params = new ArrayList<>(7);

		String url = null;

    	if (authTokenInfo.tokenVersion() == TokenVersion.V1)
    	{

			params.add(new BasicNameValuePair(AUTH_GRANT_TYPE, authOptions.grantType()));
			params.add(new BasicNameValuePair(AUTH_USER_NAME, authOptions.username()));
			params.add(new BasicNameValuePair(AUTH_CLIENT_ID,  authOptions.clientId()));

			if (authOptions.grantType().equals(AUTH_REFRESH_TOKEN)) //for new refresh token
			{
				params.add(new BasicNameValuePair(AUTH_REFRESH_TOKEN, authTokenInfo.refreshToken()));
				//must set for the first access_token, otherwise receive status code: 403 forbidden.
				//must not include scope if the scope for reissue is same, client will issue new token in the same scope.
			}
			else
			{
				params.add(new BasicNameValuePair(AUTH_TAKE_EXCLUSIVE_SIGN_ON_CONTROL, authOptions.takeExclusiveSignOnControlAsString()));
				params.add(new BasicNameValuePair(AUTH_SCOPE, authOptions.tokenScope()));
				params.add(new BasicNameValuePair(AUTH_PASSWORD, authOptions.password()));

				if (authOptions.hasNewPassword())
				{
					params.add(new BasicNameValuePair(AUTH_NEWPASSWORD, authOptions.newPassword()));
				}

				if (authOptions.hasClientSecret())
				{
					params.add(new BasicNameValuePair(AUTH_CLIENT_SECRET, authOptions.clientSecret()));
				}

				if (authOptions.tokenSession() != null)
				{
					authOptions.tokenSession().originalExpiresIn(0); /* Unset to indicate that the password grant will be sent. */
				}
			}

			if (url == null)
	    	{
				if ((restConnectOptions.authRedirect()) && (restConnectOptions.authRedirectLocation() != null)) {
					url = restConnectOptions.authRedirectLocation();
				} else {
					url = restConnectOptions.tokenServiceURLV1();
				}
	    	}
    	}
    	else
    	{
    		if (url == null)
	    	{
	    		if ((restConnectOptions.authRedirect()) && (restConnectOptions.authRedirectLocation() != null)) {
					url = restConnectOptions.authRedirectLocation();
				} else {
					url = restConnectOptions.tokenServiceURLV2();
				}
	    	}

			params.add(new BasicNameValuePair(AUTH_GRANT_TYPE, AUTH_CLIENT_CREDENTIALS_GRANT));
			params.add(new BasicNameValuePair(AUTH_CLIENT_ID,  authOptions.clientId()));
			params.add(new BasicNameValuePair(AUTH_SCOPE, authOptions.tokenScope()));

			if (authOptions.clientJwk().isEmpty())
			{
				params.add(new BasicNameValuePair(AUTH_CLIENT_SECRET,  authOptions.clientSecret()));
			}
			else
			{

				String jwt = getClientJwt(authOptions.clientId(), authOptions.clientJwk(), url, authOptions.audience(), errorInfo);

				if(jwt == null)
				{
					return ReactorReturnCodes.FAILURE;
				}

				params.add(new BasicNameValuePair(AUTH_CLIENT_ASSERTION_TYPE, AUTH_CLIENT_ASSERTION_VALUE));
				params.add(new BasicNameValuePair(AUTH_CLIENT_ASSERTION, jwt));
			}
    	}

		final RestHandler restHandler = new RestHandler(this, authOptions, restConnectOptions, authTokenInfo, errorInfo);

		if (restConnectOptions.proxyHost() == null || restConnectOptions.proxyHost().isEmpty() || restConnectOptions.proxyPort() == -1)
		{
			String body = WWWFormCodec.format(params, StandardCharsets.UTF_8);
			final SimpleHttpRequest httpRequest = SimpleRequestBuilder
					.post(url)
					.setHttpHost(authTokenInfo.tokenVersion() == TokenVersion.V1 ? restConnectOptions.tokenServiceHost() : restConnectOptions.tokenServiceHostV2())
					.setBody(body, ContentType.APPLICATION_FORM_URLENCODED)
					.setHeader("Content-Type", "application/x-www-form-urlencoded")
					.build();

		    if (authOptions.hasHeaderAttribute())
		    {
		    	Map<String,String> headerAttribs = authOptions.headerAttribute();
		    	for (Map.Entry<String,String> entry : headerAttribs.entrySet()) {
					httpRequest.addHeader(entry.getKey(), entry.getValue());
				}
		    }

			executeAsync(restHandler, httpRequest);
		}
		else
		{
			final RestProxyAuthHandler proxyAuthHandler = new RestProxyAuthHandler(this);
			final String threadUrl = url;

			new Thread() {
				public void run() {

					final HttpPost httppost = new HttpPost(threadUrl);

				    if (authOptions.hasHeaderAttribute())
				    {
				    	Map<String, String> headerAttribs = authOptions.headerAttribute();
				    	for (Map.Entry<String,String> entry : headerAttribs.entrySet())
				    		httppost.addHeader(entry.getKey(), entry.getValue());
				    }

				    httppost.setEntity(new UrlEncodedFormEntity(params, StandardCharsets.UTF_8));
		   			RequestConfig config = RequestConfig.custom()
		                    .setRedirectsEnabled(false)
		                    .build();
		   			httppost.setConfig(config);

		   			try {
		   				proxyAuthHandler.executeAsync(httppost, restConnectOptions, restHandler, errorInfo);
					} catch (IOException e) {
						restHandler.failed(e);
					}

				    }
			}.start();
		}

        return ReactorReturnCodes.SUCCESS;
   	}

	void executeAsync(RestHandler restHandler, SimpleHttpRequest httpRequest) {

		HttpProcessor httpProcessor = HttpProcessorBuilder.create()
				.add(new RequestContent())
				.add(new RequestTargetHost())
				.add(new RequestConnControl())
				.add(new RequestUserAgent(AUTH_REQUEST_USER_AGENT))
				.add(new RequestExpectContinue())
				.build();

		HttpContext localContext = new HttpClientContext();
		localContext.setAttribute(HttpProcessor.class.getName(), httpProcessor);

		restHandler.setCurrentRequest(httpRequest);
		_asyncClient.execute(SimpleRequestProducer.create(httpRequest), ClassicAsyncResponseConsumer.create(), localContext, restHandler);
	}

    public int submitRequestForServiceDiscovery(RestRequest request,
    		RestConnectOptions restConnectOptions, ReactorAuthTokenInfo authTokenInfo,
    		List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList,
    		final ReactorErrorInfo errorInfo,
    		boolean redirect, String location)
    {
    	restConnectOptions.discoveryRedirect(redirect);
    	restConnectOptions.discoveryRedirectLocation(location);

    	return submitRequestForServiceDiscovery(request, restConnectOptions, authTokenInfo,
        		reactorServiceEndpointInfoList, errorInfo);
    }

	public int submitRequestForServiceDiscovery(RestRequest request, RestConnectOptions restConnectOptions, ReactorAuthTokenInfo authTokenInfo,
												List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList, final ReactorErrorInfo errorInfo)
	{
		if (!_reactorActive)
		{
			return populateErrorInfo(errorInfo,
					ReactorReturnCodes.SHUTDOWN,
					"RestReactor.submitRequestForServiceDiscovery", "RestReactor is not active, aborting");
		}

		URIBuilder uriBuilder = null;

		try {
			uriBuilder = new URIBuilder(restConnectOptions.serviceDiscoveryURL());
		}
		catch (Exception e)
		{
			return populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE,
					"RestReactor.submitRequestForServiceDiscovery", "failed to submit a request, exception = " + getExceptionCause(e));
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
			if ((restConnectOptions.discoveryRedirect())
					&& (restConnectOptions.discoveryRedirectLocation() != null)) {
				url = restConnectOptions.discoveryRedirectLocation();
			} else {
				url = uriBuilder.build().toString();
			}
		}
		catch (URISyntaxException e)
		{
			return populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE,
					"RestReactor.submitRequestForServiceDiscovery", "failed to submit a request, exception = " + getExceptionCause(e));
		}

		final SimpleHttpRequest httpRequest = SimpleRequestBuilder
				.get(url)
				.setHttpHost(restConnectOptions.serviceDiscoveryHost())
				.setHeader("Content-Type", "application/x-www-form-urlencoded")
				.build();

		if (request.hasHeaderAttribute())
		{
			Map<String,String> headerAttribs = request.headerAttribute();
			for (Map.Entry<String,String> entry : headerAttribs.entrySet())
				httpRequest.addHeader(entry.getKey(), entry.getValue());
		}

		String token = authTokenInfo.accessToken();
		httpRequest.setHeader(HttpHeaders.AUTHORIZATION, AUTH_BEARER + token);

		final RestHandler restHandler = new RestHandler(this, request, restConnectOptions, authTokenInfo,
				reactorServiceEndpointInfoList, errorInfo);

		if ((restConnectOptions.proxyHost() == null || restConnectOptions.proxyHost().isEmpty()) || (restConnectOptions.proxyPort() == -1))
		{
			executeAsync(restHandler, httpRequest);
		}
		else
		{
			final RestProxyAuthHandler proxyAuthHandler = new RestProxyAuthHandler(this);
			final HttpGet httpget = new HttpGet(url);
			if (request.hasHeaderAttribute())
			{
				Map<String,String> headerAttribs = request.headerAttribute();
				for (Map.Entry<String,String> entry : headerAttribs.entrySet())
					httpget.addHeader(entry.getKey(), entry.getValue());
			}
			httpget.setHeader(HttpHeaders.AUTHORIZATION, AUTH_BEARER + token);

			new Thread() {
				public void run() {
					RequestConfig config = RequestConfig.custom()
							.setRedirectsEnabled(false)
							.build();
					httpget.setConfig(config);

					try {
						proxyAuthHandler.executeAsync(httpget, restConnectOptions, restHandler, errorInfo);
					} catch (IOException e) {

						restHandler.failed(e);
					}
				}
			}.start();
		}

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
			_asyncClient.close(CloseMode.GRACEFUL);

		} catch (Exception e)
		{
			return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestReactor.shutdown",
                    "received Exception, exception = "
                            + getExceptionCause(e));
		}

        return ReactorReturnCodes.SUCCESS;
    }

	@SuppressWarnings("fallthrough")
	public int submitAuthRequestBlocking(RestAuthOptions authOptions,
										 RestConnectOptions restConnectOptions, ReactorAuthTokenInfo authTokenInfo, ReactorErrorInfo errorInfo) throws IOException
	{
		if (!_reactorActive)
		{
			return populateErrorInfo(errorInfo,
					ReactorReturnCodes.SHUTDOWN,
					"RestReactor.submitAuthRequestBlocking", "RestReactor is not active, aborting");
		}

		final List<NameValuePair> params = new ArrayList<>(7);

		String url = null;

		if (authTokenInfo.tokenVersion() == TokenVersion.V1)
		{

			params.add(new BasicNameValuePair(AUTH_GRANT_TYPE, authOptions.grantType()));
			params.add(new BasicNameValuePair(AUTH_USER_NAME, authOptions.username()));
			params.add(new BasicNameValuePair(AUTH_CLIENT_ID,  authOptions.clientId()));

			if (authOptions.grantType().equals(AUTH_REFRESH_TOKEN)) //for new refresh token
			{
				params.add(new BasicNameValuePair(AUTH_REFRESH_TOKEN, authTokenInfo.refreshToken()));
				//must set for the first access_token, otherwise receive status code: 403 forbidden.
				//must not include scope if the scope for reissue is same, client will issue new token in the same scope.
			}
			else
			{
				params.add(new BasicNameValuePair(AUTH_TAKE_EXCLUSIVE_SIGN_ON_CONTROL, authOptions.takeExclusiveSignOnControlAsString()));
				params.add(new BasicNameValuePair(AUTH_SCOPE, authOptions.tokenScope()));
				params.add(new BasicNameValuePair(AUTH_PASSWORD, authOptions.password()));

				if(authOptions.hasNewPassword())
				{
					params.add(new BasicNameValuePair(AUTH_NEWPASSWORD, authOptions.newPassword()));
				}

				if(authOptions.hasClientSecret())
				{
					params.add(new BasicNameValuePair(AUTH_CLIENT_SECRET, authOptions.clientSecret()));
				}

				if (authOptions.tokenSession() != null)
				{
					authOptions.tokenSession().originalExpiresIn(0); /* Unset to indicate that the password grant will be sent. */
				}
			}

			if (url == null)
			{
				if ((restConnectOptions.authRedirect()) && (restConnectOptions.authRedirectLocation() != null)) {
					url = restConnectOptions.authRedirectLocation();
				} else {
					url = restConnectOptions.tokenServiceURLV1();
				}
			}
		}
		else
		{
			params.add(new BasicNameValuePair(AUTH_GRANT_TYPE, AUTH_CLIENT_CREDENTIALS_GRANT));
			params.add(new BasicNameValuePair(AUTH_CLIENT_ID,  authOptions.clientId()));
			params.add(new BasicNameValuePair(AUTH_SCOPE, authOptions.tokenScope()));

			if (authOptions.clientJwk().isEmpty())
			{
				params.add(new BasicNameValuePair(AUTH_CLIENT_SECRET,  authOptions.clientSecret()));
			}
			else
			{

				String jwt = getClientJwt(authOptions.clientId(), authOptions.clientJwk(), url, authOptions.audience(), errorInfo);

				if(jwt == null)
				{
					return ReactorReturnCodes.FAILURE;
				}

				params.add(new BasicNameValuePair(AUTH_CLIENT_ASSERTION_TYPE, AUTH_CLIENT_ASSERTION_VALUE));
				params.add(new BasicNameValuePair(AUTH_CLIENT_ASSERTION, jwt));
			}

			if (url == null)
			{
				if ((restConnectOptions.authRedirect()) && (restConnectOptions.authRedirectLocation() != null)) {
					url = restConnectOptions.authRedirectLocation();
				} else {
					url = restConnectOptions.tokenServiceURLV2();
				}
			}
		}

		try
		{
			final StringEntity entity;
			if (authOptions.hasHeaderAttribute())
			{
				Map<String,String>  headers = authOptions.headerAttribute();
				boolean isChunked = headers.containsKey(HttpHeaders.TRANSFER_ENCODING) && headers.get(HttpHeaders.TRANSFER_ENCODING).contentEquals("chunked"); // TODO: find appropriate constant!
				entity = new StringEntity(WWWFormCodec.format(params, StandardCharsets.UTF_8),
						ContentType.APPLICATION_FORM_URLENCODED.withCharset(StandardCharsets.UTF_8),
						headers.containsKey(HttpHeaders.CONTENT_ENCODING) ? headers.get(HttpHeaders.CONTENT_ENCODING) : null,
						isChunked
				);
			}
			else
			{
				entity = new UrlEncodedFormEntity(params, StandardCharsets.UTF_8);
			}

			final RestResponse restResponse = new RestResponse();

			int attemptCount = 0;
			final HttpPost httppost = new HttpPost(url);
			httppost.setEntity(entity);
			RequestConfig requestConfig = RequestConfig.custom()
					.setRedirectsEnabled(false)
					.build();
			httppost.setConfig(requestConfig);

			try (CloseableHttpClient httpClient = HttpClientBuilder.create()
					.setConnectionManager(getHttpClientConnectionManager())
					.build())
			{
				if ((restConnectOptions.proxyHost() != null && !restConnectOptions.proxyHost().isEmpty()) && (restConnectOptions.proxyPort() != -1))
				{
					RequestConfig config = RequestConfig.custom()
							.setRedirectsEnabled(false)
							.build();
					httppost.setConfig(config);

					int ret = _restProxyAuthHandler.executeSync(httppost, restConnectOptions, restResponse, errorInfo);

					if (ret == ReactorReturnCodes.SUCCESS)
					{
						ReactorTokenSession.parseTokenInfomation(restResponse, authTokenInfo);
					}

					return ret;
				}
				else
				{
					while (attemptCount <= 1)
					{
						RestHttpHandlerResponse resp = executeRequest(
								httppost,
								restConnectOptions,
								httpClient,
								loggerClient,
								classicResponse -> {

									boolean finish = false;

									// Extracting content string for further logging and processing
									HttpEntity entityFromResponse = classicResponse.getEntity();
									String contentString = null;
									Exception extractingContentException = null;
									try
									{
										contentString =  EntityUtils.toString(entityFromResponse);
									} catch (Exception e)
									{
										extractingContentException = e;
									}

									if (loggerClient.isTraceEnabled()) {
										loggerClient.trace(prepareResponseString(classicResponse, contentString, extractingContentException));
									}

									int statusCode = classicResponse.getCode();
									switch (statusCode) {
										case HttpStatus.SC_OK:
											convertResponse(classicResponse, restResponse, errorInfo, contentString, extractingContentException);
											ReactorTokenSession.parseTokenInfomation(restResponse, authTokenInfo);
											finish = true;
											break;
										case HttpStatus.SC_MOVED_PERMANENTLY:
										case HttpStatus.SC_MOVED_TEMPORARILY:
										case HttpStatus.SC_TEMPORARY_REDIRECT:
										case 308:                // HttpStatus.SC_PERMANENT_REDIRECT:
											Header header = classicResponse.getFirstHeader("Location");
											if( header != null )
											{
												String newHost = header.getValue();
												if (newHost != null)
												{
													try {
														httppost.setUri(new URI(newHost));
													} catch (URISyntaxException e) {
														populateErrorInfo(errorInfo,
																ReactorReturnCodes.FAILURE,
																"RestReactor.submitAuthRequestBlocking",
																"Failed to parse new URI: "
																		+ classicResponse.getCode()
																		+ ". Malformed redirection response.");
														break;
													}

													if ((statusCode == HttpStatus.SC_MOVED_PERMANENTLY) || (statusCode == 308)) {
														Buffer newUrl = CodecFactory.createBuffer();
														newUrl.data(newHost);
														if (authTokenInfo.tokenVersion() == TokenVersion.V1)
														{
															restConnectOptions.reactorOptions().tokenServiceURL_V1(newUrl);
														}
														else
														{
															restConnectOptions.reactorOptions().tokenServiceURL_V2(newUrl);
														}
													}
												}
												finish = false;
												break;
											}
											else
											{
												populateErrorInfo(errorInfo,
														ReactorReturnCodes.FAILURE,
														"RestReactor.submitAuthRequestBlocking",
														"Failed to request authentication token information with HTTP error "
																+ classicResponse.getCode()
																+ ". Malformed redirection response.");
											}

										case HttpStatus.SC_FORBIDDEN:
										case HttpStatus.SC_NOT_FOUND:
										case HttpStatus.SC_GONE:
										case 451: //  Unavailable For Legal Reasons
										default:
											convertResponse(classicResponse, restResponse, errorInfo, contentString, extractingContentException);
											finish = true;
											populateErrorInfo(errorInfo,
													ReactorReturnCodes.FAILURE,
													"RestReactor.submitAuthRequestBlocking",
													"Failed to request authentication token information with HTTP error "
															+ classicResponse.getCode() + ". Text: "
															+  (Objects.nonNull(contentString) ? contentString : ""));
											break;
									}
									return new RestHttpHandlerResponse(errorInfo.code(), finish, RestHttpHandlerResponse.Action.NO_ACTION);
								});

						if (resp.finished)
							return resp.returnCode;

						attemptCount++;
					}
				}

				if (attemptCount > 1) {
					populateErrorInfo(errorInfo,
							ReactorReturnCodes.FAILURE,
							"RestReactor.submitAuthRequestBlocking",
							"Failed to request authentication token information. "
									+ "Too many redirect attempts.");

					return ReactorReturnCodes.FAILURE;
				}
			}
		}
		catch (JSONException | IOException e)
		{
			return populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE,
					"RestReactor.submitAuthRequestBlocking", "failed to submit authorization request, exception = " + getExceptionCause(e));
		}

		return ReactorReturnCodes.SUCCESS;
	}

	@SuppressWarnings("fallthrough")
    public int submitServiceDiscoveryRequestBlocking(RestRequest request,
													 RestConnectOptions restConnectOptions,
													 ReactorAuthTokenInfo authTokenInfo,
													 List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList,
													 ReactorErrorInfo errorInfo) throws IOException
   	{
    	if (!_reactorActive)
    	{
    		 return populateErrorInfo(errorInfo,
                     ReactorReturnCodes.SHUTDOWN,
                     "RestReactor.submitServiceDiscoveryRequestBlocking", "RestReactor is not active, aborting");
    	}

		String url = null;
    	if (restConnectOptions.discoveryRedirect() == true)
    	{
    		url = restConnectOptions.discoveryRedirectLocation();
    	}
    	else
    	{
    		url = restConnectOptions.serviceDiscoveryURL();
    	}

		try
		{
			URIBuilder uriBuilder = null;
			String token = authTokenInfo.accessToken();
			RestResponse restResponse = new RestResponse();
   			int attemptCount = 0;

			try {
				uriBuilder = new URIBuilder(url);
			} catch (Exception e)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "RestReactor.submitServiceDiscoveryRequestBlocking", "failed to submit a request, exception = " + getExceptionCause(e));
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
			HttpGet httpget = new HttpGet(uriBuilder.build());
			if (request.hasHeaderAttribute())
			{
				for (Map.Entry<String, String> entry : request.headerAttribute().entrySet())
					httpget.addHeader(entry.getKey(), entry.getValue());
			}
			httpget.setHeader(HttpHeaders.AUTHORIZATION, AUTH_BEARER + token);

			if (restConnectOptions.proxyHost() != null && !restConnectOptions.proxyHost().isEmpty() && restConnectOptions.proxyPort() != -1)
			{
				RequestConfig config = RequestConfig.custom()
						.setRedirectsEnabled(false)
						.build();
				httpget.setConfig(config);

				int ret = _restProxyAuthHandler.executeSync(httpget, restConnectOptions, restResponse, errorInfo);

				if (ret == ReactorReturnCodes.SUCCESS)
				{
					RestClient.parseServiceDiscovery(restResponse, reactorServiceEndpointInfoList);
				}

				return ret;
			}
			else {
				final CloseableHttpClient httpClient = HttpClientBuilder.create()
						.setConnectionManager(getHttpClientConnectionManager())
						.build();
				try {
					while (attemptCount <= 1) {
						RestHttpHandlerResponse response = executeRequest(httpget,
								restConnectOptions,
								httpClient,
								loggerClient,
								classicResponse -> {

									RestHttpHandlerResponse resp = new RestHttpHandlerResponse();
									// Extracting content string for further logging and processing
									HttpEntity entityFromResponse = classicResponse.getEntity();
									String contentString = null;
									Exception extractingContentException = null;
									try
									{
										contentString =  EntityUtils.toString(entityFromResponse);
									} catch (Exception e)
									{
										extractingContentException = e;
									}

									if (loggerClient.isTraceEnabled()) {
										loggerClient.trace(prepareResponseString(classicResponse, contentString, extractingContentException));
									}

									int statusCode = classicResponse.getCode();
									switch (statusCode) {
										case HttpStatus.SC_OK:
											convertResponse(classicResponse, restResponse, errorInfo, contentString, extractingContentException);
											RestClient.parseServiceDiscovery(restResponse, reactorServiceEndpointInfoList);
											resp.finished = true;
											break;
										case HttpStatus.SC_MOVED_PERMANENTLY:
										case HttpStatus.SC_MOVED_TEMPORARILY:
										case HttpStatus.SC_TEMPORARY_REDIRECT:
										case 308:                // HttpStatus.SC_PERMANENT_REDIRECT:
											Header header = classicResponse.getFirstHeader("Location");
											if ( header != null )
											{
												String newHost = header.getValue();
												if (newHost != null)
												{
													try {
														httpget.setUri(new URI(newHost));
													} catch (URISyntaxException e) {
														populateErrorInfo(errorInfo,
																ReactorReturnCodes.FAILURE,
																"RestReactor.submitServiceDiscoveryRequestBlocking",
																"Failed to request service discovery information. "
																		+ "Malformed redirection response.");
														resp.returnCode = ReactorReturnCodes.FAILURE;
														break;
													}

													if ((statusCode == HttpStatus.SC_MOVED_PERMANENTLY) || (statusCode == 308)) {
														Buffer newUrl = CodecFactory.createBuffer();
														newUrl.data(newHost);
														restConnectOptions.reactorOptions().serviceDiscoveryURL(newUrl);
													}
												}

												resp.finished = false;
												break;
											}
											else
											{
												populateErrorInfo(errorInfo,
														ReactorReturnCodes.FAILURE,
														"RestReactor.submitServiceDiscoveryRequestBlocking",
														"Failed to request service discovery information. "
																+ "Malformed redirection response.");
												resp.returnCode = ReactorReturnCodes.FAILURE;
											}

										case HttpStatus.SC_FORBIDDEN:
										case HttpStatus.SC_NOT_FOUND:
										case HttpStatus.SC_GONE:
										case 451: //  Unavailable For Legal Reasons
										default:
											convertResponse(classicResponse, restResponse, errorInfo, contentString, extractingContentException);
											resp.finished = true;
											populateErrorInfo(errorInfo,
													ReactorReturnCodes.FAILURE,
													"RestReactor.submitServiceDiscoveryRequestBlocking",
													"Failed to request service discovery information. Text: "
															+ (Objects.nonNull(contentString) ? contentString : ""));

											resp.returnCode = ReactorReturnCodes.FAILURE;
											break;
									}
									return resp;
								});

						if (response.finished) {
							return response.returnCode;
						}

						attemptCount++;
					}
					if (attemptCount > 1) {
						populateErrorInfo(errorInfo,
								ReactorReturnCodes.FAILURE,
								"RestReactor.submitServiceDiscoveryRequestBlocking",
								"Failed to request service discovery information. "
										+ "Too many redirect attempts.");

						return ReactorReturnCodes.FAILURE;
					}
				}
				finally {
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

    static int convertResponse(ClassicHttpResponse response, RestResponse clientResponse,
							   ReactorErrorInfo errorInfo, String entityString, Exception extractingContentException)
    {
    	if (extractingContentException != null) {
			return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestHandler.handleResponse", "failed to convert entity to json object, exception = "
                    + getExceptionCause(extractingContentException));
    	}

    	HttpEntity entity = response.getEntity();

		clientResponse.statusCode(response.getCode());
		clientResponse.statusText(response.getReasonPhrase());
		clientResponse.protocolVersion(response.getVersion());

		Iterator<Header> headerIt = response.headerIterator();
		while (headerIt.hasNext())
		{
			Header header = headerIt.next();
			clientResponse.headerAttribute().put(header.getName(), header.getValue());
		}

		if (entity != null)
		{
			String contentType = entity.getContentType();

			if (contentType != null)
			{
				clientResponse.contentType(entity.getContentType());
			}

			clientResponse.body(entityString, errorInfo);
		}

		return ReactorReturnCodes.SUCCESS;
	}

	static int convertResponse(RestReactor RestReactor, SimpleHttpResponse response, RestResponse clientResponse,
							   ReactorErrorInfo errorInfo, String entityString, Exception extractingContentException)
	{
		if (extractingContentException != null) {
			return populateErrorInfo(errorInfo,
					ReactorReturnCodes.FAILURE,
					"RestHandler.handleResponse", "failed to convert entity to json object, exception = "
							+ getExceptionCause(extractingContentException));
		}

		clientResponse.statusCode(response.getCode());
		clientResponse.statusText(response.getReasonPhrase());
		clientResponse.protocolVersion(response.getVersion());

		Iterator<Header> headerIt = response.headerIterator();
		while (headerIt.hasNext())
		{
			Header header = headerIt.next();
			clientResponse.headerAttribute().put(header.getName(), header.getValue());
		}

		if (response.getContentType() != null)
		{
			String contentType = response.getContentType().toString();

			if (contentType != null)
			{
				clientResponse.contentType(response.getContentType().toString());
			}

			clientResponse.body(entityString, errorInfo);
		}

		return ReactorReturnCodes.SUCCESS;
	}

    static int processResponse(RestReactor restReactor, RestResponse response, RestEvent event)
    {
    	RestResultClosure resultClosure = event.resultClosure();

    	resultClosure.restCallback().RestResponseCallback(response, event);

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


    public String prepareRequestString(HttpUriRequestBase request,
									   RestConnectOptions restConnectOptions) {
    	StringBuilder requestLogBuffer = null;
    	try {
    		lock.lock();
    		requestLogBuffer = extractFromPool();
    	} finally {
    		lock.unlock();
    	}

    	requestLogBuffer.setLength(0);

    	requestLogBuffer.append("The next HTTP request was sent\n");
    	requestLogBuffer.append(request.toString() + "\n");
    	Header[] headers = request.getHeaders();
    	for(Header header : headers) {
    		requestLogBuffer.append("    " + header.getName());
    		requestLogBuffer.append(": ");
    		requestLogBuffer.append(header.getValue());
    		requestLogBuffer.append("\n");
    	}

    	if (restConnectOptions.proxyHost() != null) {
    		requestLogBuffer.append("    Proxy:\n");
    		requestLogBuffer.append("        Proxy host: " + restConnectOptions.proxyHost());
    		requestLogBuffer.append("\n");
    		requestLogBuffer.append("        Proxy port: " + restConnectOptions.proxyPort());
    		requestLogBuffer.append("\n");
    		String proxyUser = restConnectOptions.proxyUserName();
    		if (proxyUser != null ) {
    			requestLogBuffer.append("        Proxy user: " + proxyUser);
    			requestLogBuffer.append("\n");
    		}
    		String proxyPassword = restConnectOptions.proxyPassword();
    		if (proxyPassword != null) {
    			requestLogBuffer.append("        *** proxy password ***" + proxyUser);
    			requestLogBuffer.append("\n");
    		}
    	} else {
    		requestLogBuffer.append("    No proxy is used\n");
    	}

    	HttpEntity entity = request.getEntity();
	    if (entity != null) {
			String contentEncoding = entity.getContentEncoding();
			if (contentEncoding != null) {
				requestLogBuffer.append("    " + contentEncoding);
				requestLogBuffer.append("\n");
			}
			requestLogBuffer.append("    ContentLength: " + entity.getContentLength());
			requestLogBuffer.append("\n");
			String contentType = entity.getContentType();
			if (contentType != null) {
				requestLogBuffer.append("    " + contentType);
				requestLogBuffer.append("\n");
			}
			String contentString = null;
			boolean parsed = false;
			try {
				contentString = EntityUtils.toString(entity);
				parsed = true;
			} catch (ParseException e) {
				requestLogBuffer.append("    Invalid content of request:\n");
				parsed = false;
			} catch (IOException e) {
				requestLogBuffer.append("    Invalid content of request:\n");
				parsed = false;

			}
			if ((contentString != null) && !contentString.isEmpty() && parsed) {
				requestLogBuffer.append("    Content of request:\n");

				String[] contentComponents = contentString.split("&");
				for (String component : contentComponents) {
					String[] nameValue = component.split("=");

					if(nameValue.length == 2)
					{
						if ("password".equals(nameValue[0])) {
							nameValue[1] = "<*** password ***>";
						}
						if ("newPassword".equals(nameValue[0])) {
							nameValue[1] = "<*** newPassword ***>";
						}
						if ("client_secret".equals(nameValue[0])) {
							nameValue[1] = "<*** client_secret ***>";
						}

						requestLogBuffer.append("        " + nameValue[0] + ": " + nameValue[1]);
					}
					else if (nameValue.length == 1)
					{
						requestLogBuffer.append("        " + nameValue[0] + ": N/A");
					}

					requestLogBuffer.append("\n");
				}
			}
		}
    	requestLogBuffer.append("\n");

    	String returnString = requestLogBuffer.toString();
    	try {
    		lock.lock();
    		returnToPool(requestLogBuffer);
    	} finally {
    		lock.unlock();
    	}

    	return returnString;
    }

	public String prepareRequestString(SimpleHttpRequest request,
									   RestConnectOptions restConnectOptions) {
		StringBuilder requestLogBuffer = null;
		try {
			lock.lock();
			requestLogBuffer = extractFromPool();
		} finally {
			lock.unlock();
		}

		requestLogBuffer.setLength(0);

		requestLogBuffer.append("The next HTTP request was sent\n");
		requestLogBuffer.append(request.toString() + "\n");
		Header[] headers = request.getHeaders();
		for(Header header : headers) {
			requestLogBuffer.append("    " + header.getName());
			requestLogBuffer.append(": ");
			requestLogBuffer.append(header.getValue());
			requestLogBuffer.append("\n");
		}

		if (restConnectOptions.proxyHost() != null) {
			requestLogBuffer.append("    Proxy:\n");
			requestLogBuffer.append("        Proxy host: " + restConnectOptions.proxyHost());
			requestLogBuffer.append("\n");
			requestLogBuffer.append("        Proxy port: " + restConnectOptions.proxyPort());
			requestLogBuffer.append("\n");
			String proxyUser = restConnectOptions.proxyUserName();
			if (proxyUser != null ) {
				requestLogBuffer.append("        Proxy user: " + proxyUser);
				requestLogBuffer.append("\n");
			}
			String proxyPassword = restConnectOptions.proxyPassword();
			if (proxyPassword != null) {
				requestLogBuffer.append("        *** proxy password ***" + proxyUser);
				requestLogBuffer.append("\n");
			}
		} else {
			requestLogBuffer.append("    No proxy is used\n");
		}

		try {
			Header contentEncodingHeader = request.getHeader("Content-Encoding");
			if (contentEncodingHeader != null) {
				requestLogBuffer.append("    " + contentEncodingHeader.getValue());
				requestLogBuffer.append("\n");
			}
		}
		catch (ProtocolException e) {
			requestLogBuffer.append("    Invalid content of request.\n");
		}

		ContentType contentType = request.getContentType();
		if (contentType != null) {
			requestLogBuffer.append("    " + contentType.toString());
			requestLogBuffer.append("\n");
		}

		ByteArrayEntity entity = new ByteArrayEntity(request.getBodyBytes(), contentType);

		if (entity != null) {

			String contentString = null;
			boolean parsed = false;
			try {
				contentString = EntityUtils.toString(entity);
				parsed = true;
			} catch (ParseException e) {
				requestLogBuffer.append("    Invalid content of request:\n");
				parsed = false;
			} catch (IOException e) {
				requestLogBuffer.append("    Invalid content of request:\n");
				parsed = false;

			}
			if ((contentString != null) && !contentString.isEmpty() && parsed) {
				requestLogBuffer.append("    Content of request:\n");

				String[] contentComponents = contentString.split("&");
				for (String component : contentComponents) {
					String[] nameValue = component.split("=");

					if (nameValue.length == 2)
					{
						if ("password".equals(nameValue[0])) {
							nameValue[1] = "<*** password ***>";
						}
						if ("newPassword".equals(nameValue[0])) {
							nameValue[1] = "<*** newPassword ***>";
						}
						if ("client_secret".equals(nameValue[0])) {
							nameValue[1] = "<*** client_secret ***>";
						}

						requestLogBuffer.append("        " + nameValue[0] + ": " + nameValue[1]);
					}
					else if (nameValue.length == 1)
					{
						requestLogBuffer.append("        " + nameValue[0] + ": N/A");
					}

					requestLogBuffer.append("\n");
				}
			}
		}
		requestLogBuffer.append("\n");

		String returnString = requestLogBuffer.toString();
		try {
			lock.lock();
			returnToPool(requestLogBuffer);
		} finally {
			lock.unlock();
		}

		return returnString;
	}

    public String prepareResponseString(ClassicHttpResponse response, String contentString,
    		Exception extractingContentException) {
    	StringBuilder responseLogBuffer = null;
    	try {
    		lock.lock();
    		responseLogBuffer = extractFromPool();
    	} finally {
    		lock.unlock();
    	}

    	responseLogBuffer.setLength(0);

    	responseLogBuffer.append("The next HTTP response was received\n");
    	responseLogBuffer.append(new StatusLine(response) + "\n");
    	Header[] headers = response.getHeaders();
    	for(Header header : headers) {
    		responseLogBuffer.append("    " + header.getName());
    		responseLogBuffer.append(": ");
    		responseLogBuffer.append(header.getValue());
    		responseLogBuffer.append("\n");
    	}
    	HttpEntity entity = response.getEntity();
    	if (entity != null) {
    		if ((contentString != null) && (!contentString.isEmpty())) {
    			responseLogBuffer.append("    Content string:\n");
    			responseLogBuffer.append("        " + contentString + "\n");
    		} else {
    			if (extractingContentException == null) {
    				responseLogBuffer.append("    Content string is empty\n");
    			} else {
    				responseLogBuffer.append("    The next exception is thrown while reading content string:\n");
    				responseLogBuffer.append("    " + getExceptionCause(extractingContentException));
    				responseLogBuffer.append("\n");
    			}
    		}

    	}
    	responseLogBuffer.append("\n");

    	String returnString = responseLogBuffer.toString();
    	try {
    		lock.lock();
    		returnToPool(responseLogBuffer);
    	} finally {
    		lock.unlock();
    	}

    	return returnString;
    }

    private StringBuilder extractFromPool() {
    	return bufferPool.isEmpty() ? new StringBuilder() : bufferPool.remove(bufferPool.size() - 1);
    }

    private void returnToPool(StringBuilder buffer) {
    	if (!bufferPool.contains(buffer)) {
    		bufferPool.add(buffer);
    	}
    }

	RestHttpHandlerResponse executeRequest(HttpUriRequestBase httpRequest,
									   RestConnectOptions connOptions,
									   HttpClient httpClient,
									   Logger loggerClient, HttpClientResponseHandler<RestHttpHandlerResponse> handler) throws IOException
	{
		RestHttpHandlerResponse response;

		try
		{
			response = httpClient.execute(httpRequest, handler);
		} catch (Exception e) {
			if (loggerClient.isTraceEnabled())
			{
				loggerClient.trace(prepareRequestString(httpRequest, connOptions));
			}
			throw e;
		}

		if (loggerClient.isTraceEnabled())
		{
			loggerClient.trace(prepareRequestString(httpRequest, connOptions));
		}

		return response;
	}

	PoolingHttpClientConnectionManager getHttpClientConnectionManager() {
		return PoolingHttpClientConnectionManagerBuilder.create()
				.setTlsSocketStrategy(ClientTlsStrategyBuilder.create()
						.setSslContext(SSLContexts.createSystemDefault())
						.setTlsVersions(TLS.V_1_2, TLS.V_1_3)
						.buildClassic())
				.setDefaultSocketConfig(SocketConfig.custom()
						.setSoTimeout(Timeout.ofMilliseconds(_restReactorOptions.soTimeout()))
						.build())
				.setPoolConcurrencyPolicy(PoolConcurrencyPolicy.STRICT)
				.setConnPoolPolicy(PoolReusePolicy.LIFO)
				.setDefaultConnectionConfig(ConnectionConfig.custom()
						.setSocketTimeout(Timeout.ofMilliseconds(_restReactorOptions.soTimeout()))
						.setTimeToLive(TimeValue.ofMinutes(10))
						.build())
				.build();
	}

}
