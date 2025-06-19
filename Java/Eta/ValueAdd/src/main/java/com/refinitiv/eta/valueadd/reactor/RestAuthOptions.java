/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.HashMap;
import java.util.Map;

class RestAuthOptions {

	 /** (0x00000) No flags set. */
    public static final int NONE = 0x0000;

    public static final int HAS_USERNAME = 0x0001;

    public static final int HAS_PASSWORD = 0x0002;

    public static final int HAS_CLIENT_ID = 0x0004;

    public static final int HAS_TOKEN_SCOPE = 0x008;

    public static final int HAS_GRANT_TYPE = 0x0010;
    
    public static final int HAS_HEADER_ATTRIB = 0x0020;
    
    public static final int HAS_REFRESH_TOKEN = 0x0040;
    
    public static final int HAS_NEW_PASSWORD = 0x0080;
    
    public static final int HAS_CLIENT_SECRET = 0x0100;
    
    public static final int HAS_CLIENT_JWK = 0x0200;
    
    public static final int HAS_AUDIENCE = 0x0400;


    private int _flags;
    private String _username;
    private String _password;
    private String _newPassword;
    private boolean _takeExclusiveSignOnControl;
    private String _clientId;
    private String _tokenScope = "trapi.streaming.pricing.read";
    private String _clientSecret;
    private String _clientJwk;
    private String _audience;
    private String _grantType = RestReactor.AUTH_PASSWORD;
    private HashMap<String,String> _headerAttribute;
    private ReactorTokenSession _tokenSession;
	
	public RestAuthOptions(boolean takeExclusiveSignOnControl)
	{
		clear();
		
		_takeExclusiveSignOnControl = takeExclusiveSignOnControl;
	}
	
	public RestAuthOptions(ReactorTokenSession tokenSession)
	{
		clear();
		
		_tokenSession = tokenSession;
		_takeExclusiveSignOnControl = tokenSession.oAuthCredential().takeExclusiveSignOnControl();
	}
	
	public RestAuthOptions clear() {
		
		_flags = 0;
		_username = "";
		_password = "";
		_newPassword = "";
		_clientJwk = "";
		_takeExclusiveSignOnControl = true;
		_clientId = "";
		_tokenScope = "trapi.streaming.pricing.read";
		_clientSecret = "";
		_clientJwk = "";
		_audience = "";
		_grantType = RestReactor.AUTH_PASSWORD;
		_headerAttribute = null;
		_tokenSession = null;
		return this;
	}
	
	public RestAuthOptions clearSensitiveInfo()
	{
		_password = "";
		_flags &= ~HAS_PASSWORD;
		
		_newPassword = "";
		_flags &= ~HAS_NEW_PASSWORD;
		
		_clientSecret = "";
		_flags &= ~HAS_CLIENT_SECRET;
		
		_clientJwk = "";
		_flags &= ~HAS_CLIENT_JWK;
		
		return this;
	}

	public RestAuthOptions username(String username)
	{
		if ( (username != null) && (!username.isEmpty()) )
		{
			_username = username;
			_flags |= RestAuthOptions.HAS_USERNAME;
		}
		
		return this;
	}
	
	public String username()
	{
		return _username;
	}
	
	public RestAuthOptions password(String password)
	{
		if ( (password != null) && (!password.isEmpty()))
		{
			_password = password;
			_flags |= RestAuthOptions.HAS_PASSWORD;
		}
		
		return this;
	}
	
	public String password()
	{
		return _password;
	}
	
	public RestAuthOptions newPassword(String newPassword) 
	{
		if( (newPassword != null) && (!newPassword.isEmpty()))
		{
			_newPassword = newPassword;		
			_flags |= RestAuthOptions.HAS_PASSWORD;
		}
		
		return this;
	}
	
	public String newPassword()
	{
		return _newPassword;
	}
	
	public RestAuthOptions clientJwk(String clientJwk) 
	{
		if( (clientJwk != null) && (!clientJwk.isEmpty()))
		{
			_clientJwk = clientJwk;		
			_flags |= RestAuthOptions.HAS_CLIENT_JWK;
		}
		
		return this;
	}
	
	public String clientJwk()
	{
		return _clientJwk;
	}
	
	public RestAuthOptions audience(String audience) 
	{
		if( (audience != null) && (!audience.isEmpty()))
		{
			_audience = audience;		
			_flags |= RestAuthOptions.HAS_AUDIENCE;
		}
		
		return this;
	}
	
	public String audience()
	{
		return _audience;
	}

	public RestAuthOptions grantType(String grantType) {
		_grantType = grantType;
		_flags |= RestAuthOptions.HAS_GRANT_TYPE;
		return this;
	}
	
	public String grantType()
	{
		return _grantType;
	}
	
	public boolean takeExclusiveSignOnControl()
	{
	    return _takeExclusiveSignOnControl;
	}
	
	public void takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl)
	{
	    _takeExclusiveSignOnControl = takeExclusiveSignOnControl;
	}

	public String takeExclusiveSignOnControlAsString()
	{
		return _takeExclusiveSignOnControl == true ? "true" : "false";
	}

	public RestAuthOptions clientId(String clientId)
	{
		if ( (clientId != null) && (!clientId.isEmpty()))
		{
			_clientId = clientId;
			_flags |= RestAuthOptions.HAS_CLIENT_ID;
		}
		
		return this;
	}

	public String clientId()
	{
		return _clientId;
	}
	
	public RestAuthOptions tokenScope(String tokenScope)
	{
		if ( (tokenScope != null) && (!tokenScope.isEmpty()))
		{
			_tokenScope = tokenScope;
			_flags |= RestAuthOptions.HAS_TOKEN_SCOPE;
		}
		
		return this;
	}
	
	public String tokenScope()
	{
		return _tokenScope;
	}
	
	public RestAuthOptions clientSecret(String clientSecret)
	{
		if ( (clientSecret != null) && (!clientSecret.isEmpty()))
		{
			_clientSecret = clientSecret;
			_flags |= RestAuthOptions.HAS_CLIENT_SECRET;
		}
		
		return this;
	}
	
	public String clientSecret()
	{
		return _clientSecret;
	}
	
	public RestAuthOptions headerAttribute(Map<String, String> attributes) {
		_headerAttribute = (HashMap<String, String>) attributes;
		_flags |= RestAuthOptions.HAS_HEADER_ATTRIB;
		return this;
	}
	
	public ReactorTokenSession tokenSession()
	{
		return _tokenSession;
	}
	
	public Map<String, String> headerAttribute() {
		return _headerAttribute;
	}
	
	public boolean hasUsername()
	{
		return (_flags & RestAuthOptions.HAS_USERNAME) != 0;
	}
	
	public boolean hasPassword()
	{
		return (_flags & RestAuthOptions.HAS_PASSWORD) != 0;
	}
	
	public boolean hasNewPassword()
	{
		return (_flags & RestAuthOptions.HAS_NEW_PASSWORD) != 0;
	}
	
	public boolean hasGrantType()
	{
		return (_flags & RestAuthOptions.HAS_GRANT_TYPE) != 0;
	}
	
	public boolean hasRefreshToken()
	{
		return (_flags & RestAuthOptions.HAS_REFRESH_TOKEN) != 0;
	}

	public boolean hasClientId()
	{
		return (_flags & RestAuthOptions.HAS_CLIENT_ID) != 0;
	}
	
	public boolean hasClientSecret()
	{
		return (_flags & RestAuthOptions.HAS_CLIENT_SECRET) != 0;
	}
	
	public boolean hasTokenScope()
	{
		return (_flags & RestAuthOptions.HAS_TOKEN_SCOPE) != 0;
	}
	
	public boolean hasHeaderAttribute() 
	{
		return (_flags & RestAuthOptions.HAS_HEADER_ATTRIB) != 0;
	}
	
	public boolean hasClientJwk() 
	{
		return (_flags & RestAuthOptions.HAS_CLIENT_JWK) != 0;
	}
	
	public boolean hasAudience() 
	{
		return (_flags & RestAuthOptions.HAS_AUDIENCE) != 0;
	}
    
    public String toString()
	{
		 return "RestAuthOptions" + "\n" + 
				 	"\tgrantType: " + _grantType + "\n" +
	               "\tusername: " + _username + "\n" +
	               "\tpassword: " + _password + "\n" +
	               "\tnewPassword: " + _newPassword + "\n" +
	               "\tclientSecret: " + _clientSecret + "\n" + 
	               "\ttakeExclusiveSignOnControl: " + _takeExclusiveSignOnControl + "\n" + 
	               "\tclientId: " + _clientId + "\n" + 
	               "\ttokenScope: " + _tokenScope + "\n" +
	               "\theaderAttribute: " + _headerAttribute == null ? "" : _headerAttribute.toString() + "\n";
	}
}



