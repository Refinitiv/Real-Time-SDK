/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
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

import org.apache.http.Header;
import org.apache.http.HttpEntity;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.auth.AuthSchemeProvider;
import org.apache.http.auth.AuthScope;
import org.apache.http.auth.NTCredentials;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.CredentialsProvider;
import org.apache.http.client.config.AuthSchemes;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.config.Registry;
import org.apache.http.config.RegistryBuilder;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.impl.auth.BasicSchemeFactory;
import org.apache.http.impl.auth.KerberosSchemeFactory;
import org.apache.http.impl.auth.NTLMSchemeFactory;
import org.apache.http.impl.auth.SPNegoSchemeFactory;
import org.apache.http.impl.client.BasicCredentialsProvider;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClientBuilder;
import org.apache.http.impl.client.ProxyAuthenticationStrategy;
import org.apache.http.util.EntityUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

class RestProxyAuthHandler
{
	final static int BASIC = 0x10;
	final static int NTLM  = 0x20;
	final static int KERBEROS = 0x40;
	final static int NEGOTIATE = 0x80;
	
	private int authSchemeFlag;
	private SSLConnectionSocketFactory _sslconSocketFactory;
	private RestReactor _restReactor;
	private RequestConfig _defaultRequestConfig;
	
	private Logger loggerClient = null;
	
	RestProxyAuthHandler(RestReactor restReactor, SSLConnectionSocketFactory sslconSocketFactory)
	{
		clear();
		
		_sslconSocketFactory = sslconSocketFactory;
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
	
	int executeSync(HttpRequestBase httpRequest, RestConnectOptions connOptions, RestResponse restResponse, ReactorErrorInfo errorInfo) 
			throws ClientProtocolException, IOException
	{
		return execute(httpRequest, connOptions, errorInfo, null, restResponse);
	}
	
	int executeAsync(HttpRequestBase httpRequest, RestConnectOptions connOptions, RestHandler restHandler, ReactorErrorInfo errorInfo) 
			throws ClientProtocolException, IOException
	{
		return execute(httpRequest, connOptions, errorInfo, restHandler, null);
	}
	
	private int execute(HttpRequestBase httpRequest, RestConnectOptions connOptions, ReactorErrorInfo errorInfo, 
			RestHandler restHandler, RestResponse restResponse) 
			throws ClientProtocolException, IOException
	{	
		boolean done = false;
		int attemptCount = 0;
		while ((attemptCount <= 1) && (!done)) {
			final CloseableHttpClient httpClient = HttpClientBuilder.create().setSSLSocketFactory(_sslconSocketFactory).build();

			try
			{
				final HttpResponse response = _restReactor.executeRequest(httpRequest, connOptions, httpClient, loggerClient);
				
		  		// Extracting content string for further logging and processing
		  		HttpEntity entityFromResponse = response.getEntity();
		  		String contentString = null;
		  		Exception extractingContentException = null;
		  		try {
		  			contentString =  EntityUtils.toString(entityFromResponse);
		  			
		  			if(Objects.nonNull(restHandler))
		  			{
		  				restHandler.contentString(contentString);
		  			}
		  			
		  		} catch (Exception e) {
		  			extractingContentException = e;
		  		}
				
				if (loggerClient.isTraceEnabled()) {
					loggerClient.trace(_restReactor.prepareResponseString(response, contentString,
							extractingContentException));
				}
				
				switch(response.getStatusLine().getStatusCode()) {
					case HttpStatus.SC_OK:                  // 200
						if (restHandler == null)
						{
							RestReactor.convertResponse(_restReactor, response, restResponse, errorInfo,
									contentString, extractingContentException);
						}
						else
						{
							restHandler.completed(response);
						}
						done = true;
						break;
					case HttpStatus.SC_MOVED_PERMANENTLY:   // 301
					case HttpStatus.SC_MOVED_TEMPORARILY:	// 302
					case HttpStatus.SC_TEMPORARY_REDIRECT:  // 307
					case 308:                               // HttpStatus.SC_PERMANENT_REDIRECT:
						if(restHandler == null)
						{
	   	   					Header header = response.getFirstHeader("Location");
	   	   					try {
		   	   	                if( header != null && header.getValue() != null)
		   	   	                {
		   	   	                    httpRequest.setURI(new URI(header.getValue()));
		   	   	                    done = false;
		   	   	                    break;
		   	   	                }
		   	   	                else
		   	   	                {
		   	   	                	RestReactor.populateErrorInfo(errorInfo,   				
		   	   	                		ReactorReturnCodes.FAILURE,
		   	   	                		"RestProxyAuthHandler.execute", 
		   	   	                		"Failed to send request. " 
		   	   	                		+ "Malformed redirection response.");
	   	   	                    
		   	   	                	return ReactorReturnCodes.FAILURE;
		   	   	                }
	   	   					} catch ( URISyntaxException e)  {
	   	   	   	                RestReactor.populateErrorInfo(errorInfo,
	   	   	                            ReactorReturnCodes.FAILURE,
	   	   	                            "RestProxyAuthHandler.execute",
	   	   	                            "Failed to request authentication token information. "
	   	   	                            + "Incorrect redirecting.");
	
	   	   	   	                return ReactorReturnCodes.FAILURE;
	   	   					}
						}
						else
						{
							restHandler.completed(response);
							done = true;
							break;
						}
					case HttpStatus.SC_PROXY_AUTHENTICATION_REQUIRED:  // 407
						processProxyAuthResponse(response);

						if( (authSchemeFlag & NEGOTIATE) != 0 )
						{
							return sendKerborosRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
						}
						else if ( (authSchemeFlag & KERBEROS) != 0 )
						{
							return sendKerborosRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
						}
						else if ( (authSchemeFlag & NTLM) != 0 )
						{
							return sendNTLMRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
						}
						else if ( (authSchemeFlag & BASIC) != 0 )
						{
							return sendBasicAuthRequest(httpRequest, connOptions, errorInfo, restHandler, restResponse);
						}
						done = true;
						break;
					case HttpStatus.SC_FORBIDDEN:
					case HttpStatus.SC_NOT_FOUND:
					case HttpStatus.SC_GONE:
					case 451: //  Unavailable For Legal Reasons
					default:
						if(restHandler == null)
						{
							RestReactor.populateErrorInfo(errorInfo,
			                    ReactorReturnCodes.FAILURE,
			                    "RestProxyAuthHandler.execute", 
			                    "Failed to send HTTP request. Text: " 
			                    +  (Objects.nonNull(contentString) ? contentString : ""));
							return ReactorReturnCodes.FAILURE;
						}
						else
						{
							restHandler.completed(response);
							done = true;
							break;	
						}				
				}
			}
			finally
			{
				httpClient.close();
			}
			attemptCount++;
		}
		if ((attemptCount > 1) && (!done)) {
			if(restHandler == null)
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
		Header headers[] = httpResponse.getAllHeaders();
		
		authSchemeFlag = 0;
		
		// Check all available proxy authentication types
		for(int index = 0; index < headers.length; index++)
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
	
	private int sendBasicAuthRequest(HttpRequestBase httpRequest, RestConnectOptions connOptions, ReactorErrorInfo errorInfo,
			RestHandler restHandler, RestResponse restResponse) throws ClientProtocolException, IOException
	{
		Registry<AuthSchemeProvider> authSchemeRegistry = RegistryBuilder.<AuthSchemeProvider>create()
		            .register(AuthSchemes.BASIC, new BasicSchemeFactory()).build();
		
		CredentialsProvider credsProvider = new BasicCredentialsProvider();
		credsProvider.setCredentials(
        		AuthScope.ANY,
                new UsernamePasswordCredentials(connOptions.proxyUserName(), connOptions.proxyPassword()));
	        
	    HttpClientBuilder httpBuilder  = HttpClientBuilder.create()
			            .useSystemProperties()
			            .setDefaultAuthSchemeRegistry(authSchemeRegistry)
			            .setDefaultCredentialsProvider(credsProvider)
			            .setSSLSocketFactory(_sslconSocketFactory);
	    
	    final CloseableHttpClient httpClient = httpBuilder
		            .setProxy(new HttpHost(connOptions.proxyHost(), connOptions.proxyPort()))
		            .setProxyAuthenticationStrategy(ProxyAuthenticationStrategy.INSTANCE)
		            .build();
	    
	    httpRequest.setConfig(_defaultRequestConfig);
	    
	    try
	    {
			HttpResponse response = _restReactor.executeRequest(httpRequest, connOptions, httpClient, loggerClient);
		    
	  		// Extracting content string for further logging and processing
	  		HttpEntity entityFromResponse = response.getEntity();
	  		String contentString = null;
	  		Exception extractingContentException = null;
	  		try {
	  			contentString =  EntityUtils.toString(entityFromResponse);
	  			
	  			if(Objects.nonNull(restHandler))
	  			{
	  				restHandler.contentString(contentString);
	  			}
	  			
	  		} catch (Exception e) {
	  			extractingContentException = e;
	  		}
	  		
			if (loggerClient.isTraceEnabled()) {
				loggerClient.trace(_restReactor.prepareResponseString(response, 
						contentString, extractingContentException));
			}
			
			if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
		    	if ( restHandler == null)
		    	{
					return RestReactor.populateErrorInfo(errorInfo,   				
		                    ReactorReturnCodes.FAILURE,
		                    "RestProxyAuthHandler.sendBasicAuthRequest", 
		                    "Failed to send HTTP Request. Text: " 
		                    +  (Objects.nonNull(contentString) ? contentString : ""));
		    	}
		    	else
		    	{
		    		restHandler.completed(response);
		    	}
				
				return ReactorReturnCodes.SUCCESS;
			}
			else
			{
				if( restHandler == null )
				{
					RestReactor.convertResponse(_restReactor, response, restResponse, errorInfo,
							contentString, extractingContentException);
				}
				else
				{
					restHandler.completed(response);
				}
				
				return ReactorReturnCodes.SUCCESS;
			}
	    }
	    finally
	    {
	    	httpClient.close();
	    }
	}
	
	private int sendNTLMRequest(HttpRequestBase httpRequest, RestConnectOptions connOptions, ReactorErrorInfo errorInfo, 
			RestHandler restHandler, RestResponse restResponse) throws ClientProtocolException, IOException
	{
		Registry<AuthSchemeProvider> authSchemeRegistry = RegistryBuilder.<AuthSchemeProvider>create()
	            .register(AuthSchemes.NTLM, new NTLMSchemeFactory())
	            .register(AuthSchemes.BASIC, new BasicSchemeFactory())
	            .build();
	
		CredentialsProvider credsProvider = new BasicCredentialsProvider();
		credsProvider.setCredentials(
        		AuthScope.ANY,
                new UsernamePasswordCredentials(connOptions.proxyUserName(), connOptions.proxyPassword()));
	        credsProvider.setCredentials(
	        		AuthScope.ANY, new NTCredentials(connOptions.proxyUserName(), connOptions.proxyPassword()
				        ,connOptions.proxyLocalHostName(), connOptions.proxyDomain()));
	        
	    HttpClientBuilder httpBuilder  = HttpClientBuilder.create()
			            .useSystemProperties()
			            .setDefaultAuthSchemeRegistry(authSchemeRegistry)
			            .setDefaultCredentialsProvider(credsProvider)
			            .setSSLSocketFactory(_sslconSocketFactory);
	    
	    final CloseableHttpClient httpClient = httpBuilder
		            .setProxy(new HttpHost(connOptions.proxyHost(), connOptions.proxyPort()))
		            .setProxyAuthenticationStrategy(ProxyAuthenticationStrategy.INSTANCE)
		            .build();
	    
	    httpRequest.setConfig(_defaultRequestConfig);
	    
	    try
	    {
	    	HttpResponse response = _restReactor.executeRequest(httpRequest, connOptions, httpClient, loggerClient);
		    
	  		// Extracting content string for further logging and processing
	  		HttpEntity entityFromResponse = response.getEntity();
	  		String contentString = null;
	  		Exception extractingContentException = null;
	  		try {
	  			contentString =  EntityUtils.toString(entityFromResponse);
	  			
	  			if(Objects.nonNull(restHandler))
	  			{
	  				restHandler.contentString(contentString);
	  			}
	  			
	  		} catch (Exception e) {
	  			extractingContentException = e;
	  		}
		    
			if (loggerClient.isTraceEnabled()) {
				loggerClient.trace(_restReactor.prepareResponseString(response, 
						contentString, extractingContentException));
			}
			
			if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
		    	if(restHandler == null)
		    	{
					return RestReactor.populateErrorInfo(errorInfo,   				
			                ReactorReturnCodes.FAILURE,
			                "RestProxyAuthHandler.sendNTLMRequest", 
			                "Failed to send HTTP Request. Text: " 
			                +  (Objects.nonNull(contentString) ? contentString : ""));
		    	}
		    	else
		    	{
		    		restHandler.completed(response);
		    	}
				
				return ReactorReturnCodes.SUCCESS;
			}
			else
			{
				if(restHandler == null)
				{
					RestReactor.convertResponse(_restReactor, response, restResponse, errorInfo,
							contentString, extractingContentException);
				}
				else
				{
					restHandler.completed(response);
				}
				
				return ReactorReturnCodes.SUCCESS;
			}
	    }
	    finally
	    {
	    	httpClient.close();
	    }
	}
	
	private int sendKerborosRequest(final HttpRequestBase httpRequest, RestConnectOptions connOptions, final ReactorErrorInfo errorInfo, 
			final RestHandler restHandler, RestResponse restResponse) throws ClientProtocolException, IOException
	{
		System.setProperty("java.security.krb5.conf", connOptions.proxyKRB5ConfigFile());
		
		loadLoginConfig();
			
		LoginContext loginCOntext;
		try {
			loginCOntext = new LoginContext("etaj-restclient-kerberos", new KerberosCallBackHandler(connOptions.proxyUserName(), 
					connOptions.proxyPassword()));
			
			 loginCOntext.login();
		} catch (LoginException e) {
			
			if(restHandler == null)
			{
				return RestReactor.populateErrorInfo(errorInfo,   				
		                ReactorReturnCodes.FAILURE,
		                "RestProxyAuthHandler.sendKerborosRequest", 
		                "Failed to send HTTP Request. Text: " 
		                + e.getMessage());
			}
			else
			{
				restHandler.failed(e);
			}
			
			return ReactorReturnCodes.SUCCESS;
		}
         
        Subject serviceSubject = loginCOntext.getSubject();
        
        Registry<AuthSchemeProvider> authSchemeRegistry;
        
    	authSchemeRegistry = RegistryBuilder.<AuthSchemeProvider>create()
    			.register(AuthSchemes.NTLM, new NTLMSchemeFactory())
    			.register(AuthSchemes.BASIC, new BasicSchemeFactory())
		        .register(AuthSchemes.SPNEGO, new SPNegoSchemeFactory())
		        .register(AuthSchemes.KERBEROS, new KerberosSchemeFactory())
    			.build();
  
        CredentialsProvider credsProvider = new BasicCredentialsProvider();
        
        credsProvider.setCredentials(AuthScope.ANY, new NTCredentials(connOptions.proxyUserName(), connOptions.proxyPassword()
        		,connOptions.proxyLocalHostName(), connOptions.proxyDomain()));

        HttpClientBuilder httpBuilder  = HttpClientBuilder.create()
	            .useSystemProperties()
	            .setDefaultAuthSchemeRegistry(authSchemeRegistry)
	            .setDefaultCredentialsProvider(credsProvider)
	            .setSSLSocketFactory(_sslconSocketFactory);
        
        final CloseableHttpClient httpClient = httpBuilder
		            .setProxy(new HttpHost(connOptions.proxyHost(), connOptions.proxyPort()))
		            .setProxyAuthenticationStrategy(ProxyAuthenticationStrategy.INSTANCE)
		            .build();
        
        httpRequest.setConfig(_defaultRequestConfig);
			
        HttpResponse  response = Subject.doAs(serviceSubject,  new PrivilegedAction<HttpResponse>() {
			
        	@Override
			public HttpResponse run() {
				
				HttpResponse response = null;
				
					try {
						response = _restReactor.executeRequest(httpRequest, connOptions, httpClient, loggerClient);
					} catch (IOException e) {
						
						if(restHandler == null)
						{
							RestReactor.populateErrorInfo(errorInfo,   				
					                ReactorReturnCodes.FAILURE,
					                "RestProxyAuthHandler.sendKerborosRequest", 
					                "Failed to send HTTP Request. Text: " 
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
        
        if(response != null)
        {
	  		// Extracting content string for further logging and processing
	  		HttpEntity entityFromResponse = response.getEntity();
	  		String contentString = null;
	  		Exception extractingContentException = null;
	  		try {
	  			contentString =  EntityUtils.toString(entityFromResponse);
	  			
	  			if(Objects.nonNull(restHandler))
	  			{
	  				restHandler.contentString(contentString);
	  			}
	  			
	  		} catch (Exception e) {
	  			extractingContentException = e;
	  		}
	  		
			if (loggerClient.isTraceEnabled()) {
				loggerClient.trace(_restReactor.prepareResponseString(response, 
						contentString, extractingContentException));
			}
			
			if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
	        	if(restHandler == null)
	        	{
					return RestReactor.populateErrorInfo(errorInfo,   				
			                ReactorReturnCodes.FAILURE,
			                "RestProxyAuthHandler.sendKerborosRequest", 
			                "Failed to send HTTP Request. Text: " 
			               +  (Objects.nonNull(contentString) ? contentString : ""));
	        	}
	        	else
	        	{
	        		restHandler.completed(response);
	        	}
			}
			else
			{
				if(restHandler == null)
				{
					RestResponse resp = new RestResponse();
					RestReactor.convertResponse(_restReactor, response, resp, errorInfo,
							contentString, extractingContentException);
				}
				else
				{
					restHandler.completed(response);
				}
			}
        }
        else
        {
        	 return ReactorReturnCodes.FAILURE;
        }
        
        try {
			loginCOntext.logout();
		} catch (LoginException e) {
			// Do nothing
		}
        
        return ReactorReturnCodes.SUCCESS;
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
