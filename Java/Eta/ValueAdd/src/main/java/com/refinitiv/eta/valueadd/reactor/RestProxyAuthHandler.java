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
import java.security.PrivilegedAction;
import java.util.HashMap;
import java.util.Objects;

import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.login.AppConfigurationEntry;
import javax.security.auth.login.Configuration;
import javax.security.auth.login.LoginContext;
import javax.security.auth.login.LoginException;

import org.apache.hc.client5.http.auth.*;
import org.apache.hc.client5.http.impl.io.PoolingHttpClientConnectionManager;
import org.apache.hc.core5.http.*;
import org.apache.hc.client5.http.config.RequestConfig;
import org.apache.hc.client5.http.classic.methods.HttpUriRequestBase;
import org.apache.hc.core5.http.config.Registry;
import org.apache.hc.core5.http.config.RegistryBuilder;
import org.apache.hc.client5.http.impl.auth.*;
import org.apache.hc.client5.http.impl.classic.CloseableHttpClient;
import org.apache.hc.client5.http.impl.classic.HttpClientBuilder;
import org.apache.hc.client5.http.impl.DefaultAuthenticationStrategy;
import org.apache.hc.core5.http.io.entity.EntityUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@SuppressWarnings("deprecation")
class RestProxyAuthHandler
{
	final static int BASIC = 0x10;
	final static int NTLM  = 0x20;
	final static int KERBEROS = 0x40;
	final static int NEGOTIATE = 0x80;
	
	private int authSchemeFlag;
	private RestReactor _restReactor;
	private RequestConfig _defaultRequestConfig;
	
	private Logger loggerClient = null;
	
	RestProxyAuthHandler(RestReactor restReactor)
	{
		clear();

		_restReactor = restReactor;
		
		_defaultRequestConfig = RequestConfig.custom()
			    .setAuthenticationEnabled(true)
			    .build();

		loggerClient = LoggerFactory.getLogger(RestReactor.class);
	}
	
	void clear()
	{
		authSchemeFlag = 0;
	}
	
	int executeSync(HttpUriRequestBase httpRequest, RestConnectOptions connOptions, RestResponse restResponse, ReactorErrorInfo errorInfo)
			throws IOException
	{
		return execute(httpRequest, connOptions, errorInfo, null, restResponse);
	}
	
	int executeAsync(HttpUriRequestBase httpRequest, RestConnectOptions connOptions, RestHandler restHandler, ReactorErrorInfo errorInfo)
			throws IOException
	{
		return execute(httpRequest, connOptions, errorInfo, restHandler, null);
	}
	
	private int execute(HttpUriRequestBase httpRequest, RestConnectOptions connOptions, ReactorErrorInfo errorInfo,
			RestHandler restHandler, RestResponse restResponse)
			throws IOException
	{
		HttpHost proxy = new HttpHost("http", connOptions.proxyHost(), connOptions.proxyPort());
		CloseableHttpClient httpClient = HttpClientBuilder.create()
				.setConnectionManager(_restReactor.getHttpClientConnectionManager())
				.setProxy(proxy)
				.build();

		RestHttpHandlerResponse handlerResponse = null;
		int attemptCount = 0;
		try {
			while (attemptCount <= 1) {

				handlerResponse = _restReactor.executeRequest(httpRequest,
						connOptions,
						httpClient,
						loggerClient,
						classicResponse -> {

							RestHttpHandlerResponse hr = new RestHttpHandlerResponse();

							// Extracting content string for further logging and processing
							HttpEntity entityFromResponse = classicResponse.getEntity();
							String contentString = null;
							Exception extractingContentException = null;
							try {
								contentString =  EntityUtils.toString(entityFromResponse);

								if (Objects.nonNull(restHandler))
								{
									restHandler.contentString(contentString);
								}

							} catch (Exception e) {
								extractingContentException = e;
							}

							if (loggerClient.isTraceEnabled()) {
								loggerClient.trace(_restReactor.prepareResponseString(classicResponse, contentString, extractingContentException));
							}

							switch (classicResponse.getCode()) {
								case HttpStatus.SC_OK:                  // 200
									if (restHandler == null)
									{
										RestReactor.convertResponse(classicResponse, restResponse, errorInfo, contentString, extractingContentException);
									}
									else
									{
										restHandler.completed(classicResponse);
									}
									hr.finished = true;
									break;
								case HttpStatus.SC_MOVED_PERMANENTLY:   // 301
								case HttpStatus.SC_MOVED_TEMPORARILY:	// 302
								case HttpStatus.SC_TEMPORARY_REDIRECT:  // 307
								case 308:                               // HttpStatus.SC_PERMANENT_REDIRECT:
									if (restHandler == null)
									{
										Header header = classicResponse.getFirstHeader("Location");
										try {
											if (header != null && header.getValue() != null)
											{
												httpRequest.setUri(new URI(header.getValue()));
												hr.finished = false;
												break;
											}
											else
											{
												RestReactor.populateErrorInfo(errorInfo,
														ReactorReturnCodes.FAILURE,
														"RestProxyAuthHandler.execute",
														"Failed to send request. "
																+ "Malformed redirection response.");
												hr.finished = true;
												hr.returnCode = ReactorReturnCodes.FAILURE;
												return hr;
											}
										}
										catch ( URISyntaxException e)  {
											RestReactor.populateErrorInfo(errorInfo,
													ReactorReturnCodes.FAILURE,
													"RestProxyAuthHandler.execute",
													"Failed to request authentication token information with HTTP error "
															+ classicResponse.getCode() + ". "
															+ "Incorrect redirecting.");
											hr.finished = true;
											hr.returnCode = ReactorReturnCodes.FAILURE;
											return hr;
										}
									}
									else
									{
										restHandler.completed(classicResponse);
										hr.finished = true;
										break;
									}
								case HttpStatus.SC_PROXY_AUTHENTICATION_REQUIRED:  // 407
									processProxyAuthResponse(classicResponse);

									if( (authSchemeFlag & NEGOTIATE) != 0 )
									{
										hr.action = RestHttpHandlerResponse.Action.SEND_KERBOROS_AUTH;
									}
									else if ((authSchemeFlag & KERBEROS) != 0)
									{
										hr.action = RestHttpHandlerResponse.Action.SEND_KERBOROS_AUTH;
									}
									else if ((authSchemeFlag & NTLM) != 0)
									{
										hr.action = RestHttpHandlerResponse.Action.SEND_NTLM_AUTH;
									}
									else if ((authSchemeFlag & BASIC) != 0)
									{
										hr.action = RestHttpHandlerResponse.Action.SEND_BASIC_AUTH_REQUEST;
									}
									hr.finished = true;
									return hr;
								case HttpStatus.SC_FORBIDDEN:
								case HttpStatus.SC_NOT_FOUND:
								case HttpStatus.SC_GONE:
								case 451: //  Unavailable For Legal Reasons
								default:
									if (restHandler == null)
									{
										RestReactor.populateErrorInfo(errorInfo,
												ReactorReturnCodes.FAILURE,
												"RestProxyAuthHandler.execute",
												"Failed to request authentication token information with HTTP error "
														+ classicResponse.getCode() + ". Text: "
														+  (Objects.nonNull(contentString) ? contentString : ""));
									}
									else
									{
										restHandler.completed(classicResponse);
									}
									hr.finished = true;
									hr.action = RestHttpHandlerResponse.Action.NO_ACTION;
									hr.returnCode = ReactorReturnCodes.FAILURE;
									break;
							}
							return hr;
						});

				if (handlerResponse.finished) {
					if (handlerResponse.action == RestHttpHandlerResponse.Action.NO_ACTION)
						return handlerResponse.returnCode;
					else
						break;
				}

				attemptCount++;
			}
		}
		finally {
			httpClient.close();
		}

		if (handlerResponse != null) {
			switch (handlerResponse.action) {
				case NO_ACTION:
					break;
				case SEND_BASIC_AUTH_REQUEST:
					return sendBasicAuthRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
				case SEND_KERBOROS_AUTH:
					return sendKerborosRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
				case SEND_NTLM_AUTH:
					return sendNTLMRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
			}
		}

		if ((attemptCount > 1)) {
			if (restHandler == null)
			{
				RestReactor.populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "RestProxyAuthHandler.execute", 
                    "Failed to send request. " 
                    + "Too many redirect attempts.");

				return ReactorReturnCodes.FAILURE;
			}
		}
		return ReactorReturnCodes.SUCCESS;
	}

	private void processProxyAuthResponse(HttpResponse httpResponse)
	{
		Header headers[] = httpResponse.getHeaders();
		
		authSchemeFlag = 0;
		// Check all available proxy authentication types
		for (int index = 0; index < headers.length; index++)
		{
			if(headers[index].getName().indexOf("Proxy-Authenticate") != -1)
			{
				if (headers[index].getValue().indexOf("Negotiate") != -1 )
				{
					authSchemeFlag |= NEGOTIATE;
				}
				else if (headers[index].getValue().indexOf("Kerberos") != -1 )
				{
					authSchemeFlag |= KERBEROS;
				}
				else if (headers[index].getValue().indexOf("NTLM") != -1 )
				{
					authSchemeFlag |= NTLM;
				}
				else if (headers[index].getValue().indexOf("Basic") != -1 )
				{
					authSchemeFlag |= BASIC;
				}
			}
		}
	}
	
	private int sendBasicAuthRequest(HttpUriRequestBase httpRequest, RestConnectOptions connOptions, ReactorErrorInfo errorInfo,
			RestHandler restHandler, RestResponse restResponse) throws IOException
	{
		Registry<AuthSchemeFactory> authSchemeRegistry = RegistryBuilder.<AuthSchemeFactory>create()
		            .register(StandardAuthScheme.BASIC, new BasicSchemeFactory()).build();

		BasicCredentialsProvider credsProvider = new BasicCredentialsProvider();
		credsProvider.setCredentials(
				new AuthScope(null, null, -1, null, null),
                new UsernamePasswordCredentials(connOptions.proxyUserName(), connOptions.proxyPassword().toCharArray()));

		PoolingHttpClientConnectionManager connectionManager = _restReactor.getHttpClientConnectionManager();
	    HttpClientBuilder httpBuilder  = HttpClientBuilder.create()
				.useSystemProperties()
			    .setDefaultAuthSchemeRegistry(authSchemeRegistry)
			    .setDefaultCredentialsProvider(credsProvider)
				.setConnectionManager(connectionManager);
	    
	    final CloseableHttpClient httpClient = httpBuilder
		            .setProxy(new HttpHost(connOptions.proxyHost(), connOptions.proxyPort()))
		            .setProxyAuthenticationStrategy(DefaultAuthenticationStrategy.INSTANCE)
		            .build();
	    
	    httpRequest.setConfig(_defaultRequestConfig);
	    
	    try
	    {
			RestHttpHandlerResponse resp = _restReactor.executeRequest(httpRequest,
					connOptions,
					httpClient,
					loggerClient, classicResponse -> {

						// Extracting content string for further logging and processing
						HttpEntity entityFromResponse = classicResponse.getEntity();
						String contentString = null;
						Exception extractingContentException = null;
						try {
							contentString =  EntityUtils.toString(entityFromResponse);

							if (Objects.nonNull(restHandler))
							{
								restHandler.contentString(contentString);
							}
						}
						catch (Exception e) {
							extractingContentException = e;
						}

						if (loggerClient.isTraceEnabled()) {
							loggerClient.trace(_restReactor.prepareResponseString(classicResponse, contentString, extractingContentException));
						}

						if (classicResponse.getCode() != HttpStatus.SC_OK)
						{
							if (restHandler == null)
							{
								RestReactor.populateErrorInfo(errorInfo,
										ReactorReturnCodes.FAILURE,
										"RestProxyAuthHandler.sendBasicAuthRequest",
										"Failed to request authentication token information with HTTP error "
												+ classicResponse.getCode() + ". Text: "
												+  (Objects.nonNull(contentString) ? contentString : ""));
							}
							else
							{
								restHandler.completed(classicResponse);
							}

							return new RestHttpHandlerResponse(ReactorReturnCodes.FAILURE, true, RestHttpHandlerResponse.Action.NO_ACTION);
						}
						else
						{
							if( restHandler == null )
							{
								RestReactor.convertResponse(classicResponse, restResponse, errorInfo,
										contentString, extractingContentException);
							}
							else
							{
								restHandler.completed(classicResponse);
							}

							return new RestHttpHandlerResponse(ReactorReturnCodes.SUCCESS, true, RestHttpHandlerResponse.Action.NO_ACTION);
						}
					}
			);
		    
	  		return resp.returnCode;
	    }
	    finally
	    {
	    	httpClient.close();
	    }
	}
	
	private int sendNTLMRequest(HttpUriRequestBase httpRequest, RestConnectOptions connOptions, ReactorErrorInfo errorInfo, 
			RestHandler restHandler, RestResponse restResponse) throws IOException
	{
		Registry<AuthSchemeFactory> authSchemeRegistry = RegistryBuilder.<AuthSchemeFactory>create()
	            .register(StandardAuthScheme.NTLM, new NTLMSchemeFactory())
	            .register(StandardAuthScheme.BASIC, new BasicSchemeFactory())
	            .build();

		BasicCredentialsProvider credsProvider = new BasicCredentialsProvider();
		credsProvider.setCredentials(
				new AuthScope(null, null, -1, null, null),
                new UsernamePasswordCredentials(connOptions.proxyUserName(), connOptions.proxyPassword().toCharArray())
		);
	    credsProvider.setCredentials(
				new AuthScope(null, null, -1, null, null),
				new NTCredentials(connOptions.proxyUserName(),
						connOptions.proxyPassword().toCharArray(),
						connOptions.proxyLocalHostName(),
						connOptions.proxyDomain())
		);

		PoolingHttpClientConnectionManager connectionManager = _restReactor.getHttpClientConnectionManager();
	    HttpClientBuilder httpBuilder  = HttpClientBuilder.create()
				.useSystemProperties()
				.setDefaultAuthSchemeRegistry(authSchemeRegistry)
				.setDefaultCredentialsProvider(credsProvider)
				.setConnectionManager(connectionManager);
	    
	    final CloseableHttpClient httpClient = httpBuilder
				.setProxy(new HttpHost(connOptions.proxyHost(), connOptions.proxyPort()))
				.setProxyAuthenticationStrategy(DefaultAuthenticationStrategy.INSTANCE)
				.build();
	    
	    httpRequest.setConfig(_defaultRequestConfig);
	    
	    try
	    {
	    	RestHttpHandlerResponse resp = _restReactor.executeRequest(httpRequest,
					connOptions,
					httpClient,
					loggerClient,
					classicResponse -> {

						// Extracting content string for further logging and processing
						HttpEntity entityFromResponse = classicResponse.getEntity();
						String contentString = null;
						Exception extractingContentException = null;
						try {
							contentString =  EntityUtils.toString(entityFromResponse);

							if (Objects.nonNull(restHandler))
							{
								restHandler.contentString(contentString);
							}

						} catch (Exception e) {
							extractingContentException = e;
						}

						if (loggerClient.isTraceEnabled()) {
							loggerClient.trace(_restReactor.prepareResponseString(classicResponse, contentString, extractingContentException));
						}

						if (classicResponse.getCode() != HttpStatus.SC_OK)
						{
							if (restHandler == null)
							{
								RestReactor.populateErrorInfo(errorInfo,
										ReactorReturnCodes.FAILURE,
										"RestProxyAuthHandler.sendNTLMRequest",
										"Failed to request authentication token information with HTTP error "
												+ classicResponse.getCode() + ". Text: "
												+  (Objects.nonNull(contentString) ? contentString : ""));
								return new RestHttpHandlerResponse(ReactorReturnCodes.FAILURE, true, RestHttpHandlerResponse.Action.NO_ACTION);
							}
							else
							{
								restHandler.completed(classicResponse);
							}
							return new RestHttpHandlerResponse(ReactorReturnCodes.FAILURE, true, RestHttpHandlerResponse.Action.NO_ACTION);
						}
						else
						{
							if (restHandler == null)
							{
								RestReactor.convertResponse(classicResponse, restResponse, errorInfo, contentString, extractingContentException);
							}
							else
							{
								restHandler.completed(classicResponse);
							}
							return new RestHttpHandlerResponse(ReactorReturnCodes.SUCCESS, true, RestHttpHandlerResponse.Action.NO_ACTION);
						}
					});
		    
	  		return resp.returnCode;
	    }
	    finally
	    {
	    	httpClient.close();
	    }
	}

	@SuppressWarnings("removal") //Subject.doAs cannot be replaced for backward compatibility reasons. No equivalent method supporting both java 8 (Subject.doAs) and java >=19 (AccessController.doPrivilaged) without warnings.
	private int sendKerborosRequest(final HttpUriRequestBase httpRequest, RestConnectOptions connOptions, final ReactorErrorInfo errorInfo, 
			final RestHandler restHandler, RestResponse restResponse) throws IOException
	{
		System.setProperty("java.security.krb5.conf", connOptions.proxyKRB5ConfigFile());
		
		loadLoginConfig();
			
		LoginContext loginContext;
		try {
			loginContext = new LoginContext("etaj-restclient-kerberos", new KerberosCallBackHandler(connOptions.proxyUserName(),
					connOptions.proxyPassword()));
			
			 loginContext.login();
		} catch (LoginException e) {
			
			if (restHandler == null)
			{
				return RestReactor.populateErrorInfo(errorInfo,   				
		                ReactorReturnCodes.FAILURE,
		                "RestProxyAuthHandler.sendKerborosRequest", 
		                "Failed to request authentication token information. Text: " 
		                + e.getMessage());
			}
			else
			{
				restHandler.failed(e);
			}
			
			return ReactorReturnCodes.SUCCESS;
		}
         
        Subject serviceSubject = loginContext.getSubject();
        
        Registry<AuthSchemeFactory> authSchemeRegistry;
        
    	authSchemeRegistry = RegistryBuilder.<AuthSchemeFactory>create()
    			.register(StandardAuthScheme.NTLM, new NTLMSchemeFactory())
    			.register(StandardAuthScheme.BASIC, new BasicSchemeFactory())
		        .register(StandardAuthScheme.SPNEGO, SPNegoSchemeFactory.DEFAULT)
		        .register(StandardAuthScheme.KERBEROS, KerberosSchemeFactory.DEFAULT)
    			.build();

		BasicCredentialsProvider credsProvider = new BasicCredentialsProvider();
        
        credsProvider.setCredentials(new AuthScope(null, null, -1, null, null),
				new NTCredentials(connOptions.proxyUserName(),
						connOptions.proxyPassword().toCharArray(),
						connOptions.proxyLocalHostName(),
						connOptions.proxyDomain())
		);

        HttpClientBuilder httpBuilder  = HttpClientBuilder.create()
	            .useSystemProperties()
	            .setDefaultAuthSchemeRegistry(authSchemeRegistry)
	            .setDefaultCredentialsProvider(credsProvider)
	            .setConnectionManager(_restReactor.getHttpClientConnectionManager());
        
        final CloseableHttpClient httpClient = httpBuilder
		            .setProxy(new HttpHost(connOptions.proxyHost(), connOptions.proxyPort()))
		            .setProxyAuthenticationStrategy(DefaultAuthenticationStrategy.INSTANCE)
		            .build();
        
        httpRequest.setConfig(_defaultRequestConfig);
		RestHttpHandlerResponse response;

		try {
			response = Subject.doAs(serviceSubject,  new PrivilegedAction<RestHttpHandlerResponse>() {

				@Override
				public RestHttpHandlerResponse run() {

					RestHttpHandlerResponse response = null;

					try {
						response = _restReactor.executeRequest(httpRequest,
								connOptions,
								httpClient,
								loggerClient,
								classicResponse -> {
									if (classicResponse != null)
									{
										// Extracting content string for further logging and processing
										HttpEntity entityFromResponse = classicResponse.getEntity();
										String contentString = null;
										Exception extractingContentException = null;
										try {
											contentString =  EntityUtils.toString(entityFromResponse);

											if (Objects.nonNull(restHandler))
											{
												restHandler.contentString(contentString);
											}

										} catch (Exception e) {
											extractingContentException = e;
										}

										if (loggerClient.isTraceEnabled()) {
											loggerClient.trace(_restReactor.prepareResponseString(classicResponse, contentString, extractingContentException));
										}

										if (classicResponse.getCode() != HttpStatus.SC_OK)
										{
											if (restHandler == null)
											{
												RestReactor.populateErrorInfo(errorInfo,
														ReactorReturnCodes.FAILURE,
														"RestProxyAuthHandler.sendKerborosRequest",
														"Failed to request authentication token information with HTTP error "
																+ classicResponse.getCode() + ". Text: "
																+  (Objects.nonNull(contentString) ? contentString : ""));
											}
											else
											{
												restHandler.completed(classicResponse);
											}
											return new RestHttpHandlerResponse(ReactorReturnCodes.FAILURE, true, RestHttpHandlerResponse.Action.NO_ACTION);
										}
										else
										{
											if (restHandler == null)
											{
												RestResponse resp = new RestResponse();
												RestReactor.convertResponse(classicResponse, resp, errorInfo, contentString, extractingContentException);
											}
											else
											{
												restHandler.completed(classicResponse);
											}
											return new RestHttpHandlerResponse(ReactorReturnCodes.SUCCESS, true, RestHttpHandlerResponse.Action.NO_ACTION);
										}
									}
									else
									{
										return new RestHttpHandlerResponse(ReactorReturnCodes.FAILURE, true, RestHttpHandlerResponse.Action.NO_ACTION);
									}
								});
					}
					catch (IOException e) {

						if (restHandler == null)
						{
							RestReactor.populateErrorInfo(errorInfo,
									ReactorReturnCodes.FAILURE,
									"RestProxyAuthHandler.sendKerborosRequest",
									"Failed to request authentication token information. Text: "
											+ e.getMessage());
						}
						else
						{
							restHandler.failed(e);
						}
					}
					finally
					{
						try {
							httpClient.close();
						} catch (IOException e) {
							// Do nothing
						}
					}

					return response;
				}
			});
		}
		finally {
			httpClient.close();
		}

        
        try {
			loginContext.logout();
		} catch (LoginException e) {
			// Do nothing
		}
        
        return response.returnCode;
	}
	
	private static class KerberosCallBackHandler implements CallbackHandler {

        private final String user;
        private final String password;

        public KerberosCallBackHandler(String user, String password) {
            this.user = user;
            this.password = password;
        }

        public void handle(Callback[] callbacks) throws IOException{
        	
            for (Callback callback : callbacks) {

                if (callback instanceof NameCallback) {
                    NameCallback nc = (NameCallback) callback;
                    nc.setName(user);
                } else if (callback instanceof PasswordCallback) {
                    PasswordCallback pc = (PasswordCallback) callback;
                    pc.setPassword(password.toCharArray());
                }
            }
        }
    }
	
	private static String db; // debug env variable
    private static HashMap<String, String> loginConfigOptions = new HashMap<String, String>();
    
    /* Load Kerberos login config manually (not using login.conf) */
    private static void loadLoginConfig()
    {
        Configuration.setConfiguration(new Configuration()
        {
            @Override
            public AppConfigurationEntry[] getAppConfigurationEntry(String cname)
            {
            	@SuppressWarnings("restriction")
                String name = com.sun.security.auth.module.Krb5LoginModule.class.getName();

                 loginConfigOptions.put("com.sun.security.auth.module.Krb5LoginModule", "required");
                 String useTC = System.getProperty("krb_login_config_useTicketCache");
                 if ((useTC!=null) && useTC.equals("true"))
                 {
                     loginConfigOptions.put("useTicketCache", "true");

                     String doNotPrompt = System.getProperty("krb_login_config_doNotPrompt");
                     if ((doNotPrompt!=null) && doNotPrompt.equals("true"))
                         loginConfigOptions.put("doNotPrompt", "true");

                     String TC = System.getProperty("krb_login_config_ticketCache");
                     if ((TC!=null) && !TC.isEmpty())
                         loginConfigOptions.put("ticketCache", TC);
                 }
                 if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                     loginConfigOptions.put("debug", "true");
                             
                 AppConfigurationEntry ace = new AppConfigurationEntry(name, 
                                                                       AppConfigurationEntry.LoginModuleControlFlag.REQUIRED,
                                                                       loginConfigOptions);
                 AppConfigurationEntry[] entry = {ace};
                 return entry;
            }
        });     
    }
}
