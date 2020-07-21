package com.thomsonreuters.upa.valueadd.reactor;

import java.io.IOException;
import java.security.PrivilegedAction;
import java.util.HashMap;

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
	
	RestProxyAuthHandler(RestReactor restReactor, SSLConnectionSocketFactory sslconSocketFactory)
	{
		clear();
		
		_sslconSocketFactory = sslconSocketFactory;
		_restReactor = restReactor;
		
		_defaultRequestConfig = RequestConfig.custom()
			    .setAuthenticationEnabled(true)
			    .build();
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
		final CloseableHttpClient httpClient = HttpClientBuilder.create().setSSLSocketFactory(_sslconSocketFactory).build();

		try
		{
			final HttpResponse response = httpClient.execute(httpRequest);
			if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
				// Checking for supported proxy authentication types and re-send the request again
				if(response.getStatusLine().getStatusCode() == 407)
				{
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
				}
				else
				{
					if(restHandler == null)
					{
						return RestReactor.populateErrorInfo(errorInfo,   				
			                    ReactorReturnCodes.FAILURE,
			                    "RestProxyAuthHandler.execute", 
			                    "Failed to send HTTP request. Text: " 
			                    + EntityUtils.toString(response.getEntity()));
					}
					else
					{
						restHandler.completed(response);
					}
				}
			}
			else
			{
				if (restHandler == null)
				{
					RestReactor.convertResponse(_restReactor, response, restResponse, errorInfo);
				}
				else
				{
					restHandler.completed(response);
				}
			}
		}
		finally
		{
			httpClient.close();
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
		    HttpResponse response = httpClient.execute(httpRequest);
		    
		    if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
		    	if ( restHandler == null)
		    	{
					return RestReactor.populateErrorInfo(errorInfo,   				
		                    ReactorReturnCodes.FAILURE,
		                    "RestProxyAuthHandler.sendBasicAuthRequest", 
		                    "Failed to send HTTP Request. Text: " 
		                    + EntityUtils.toString(response.getEntity()));
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
					RestReactor.convertResponse(_restReactor, response, restResponse, errorInfo);
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
		    HttpResponse response = httpClient.execute(httpRequest);
		    
		    if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
		    	if(restHandler == null)
		    	{
					return RestReactor.populateErrorInfo(errorInfo,   				
			                ReactorReturnCodes.FAILURE,
			                "RestProxyAuthHandler.sendNTLMRequest", 
			                "Failed to send HTTP Request. Text: " 
			                + EntityUtils.toString(response.getEntity()));
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
					RestReactor.convertResponse(_restReactor, response, restResponse, errorInfo);
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
						response = httpClient.execute(httpRequest);
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
	        if (response.getStatusLine().getStatusCode() != HttpStatus.SC_OK)
			{
	        	if(restHandler == null)
	        	{
					return RestReactor.populateErrorInfo(errorInfo,   				
			                ReactorReturnCodes.FAILURE,
			                "RestProxyAuthHandler.sendKerborosRequest", 
			                "Failed to send HTTP Request. Text: " 
			                + EntityUtils.toString(response.getEntity()));
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
					
					RestReactor.convertResponse(_restReactor, response, resp, errorInfo);
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
