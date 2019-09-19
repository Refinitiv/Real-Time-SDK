package com.thomsonreuters.upa.valueadd.reactor;

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

    private int _flags;
    private String _username;
    private String _password;
    private boolean _takeExclusiveSignOnControl = false;
    private String _clientId;
    private String _tokenScope = "trapi";
    private String _grantType = "password";
    private String _refreshToken;
    private HashMap<String,String> _headerAttribute;
	
	public RestAuthOptions()
	{
		clear();
	}
	
	public RestAuthOptions clear() {
		
		_flags = 0;
		_username = "";
		_password = "";
		_takeExclusiveSignOnControl = false;
		_clientId = "";
		_tokenScope = "trapi";
		_grantType = "password";
		_headerAttribute = null;
		return this;
	}

	public RestAuthOptions username(String username)
	{
		_username = username;
		_flags |= RestAuthOptions.HAS_USERNAME;
		return this;
	}
	
	public String username()
	{
		return _username;
	}
	
	public RestAuthOptions password(String password) {

		
		_password = password;
		_flags |= RestAuthOptions.HAS_PASSWORD;
		
		if (password.equals(""))
		{
			_password = null;
			_flags &= ~RestAuthOptions.HAS_PASSWORD;
		}
		
		return this;
	}
	
	public String password()
	{
		return _password;
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
	
	public RestAuthOptions refreshToken(String refreshToken) {
		_refreshToken = refreshToken;
		_flags |= RestAuthOptions.HAS_REFRESH_TOKEN;
		_grantType = "refresh_token";
		return this;
	}
	
	public String refreshToken()
	{
		return _refreshToken;
	}

	public boolean takeExclusiveSignOnControl()
	{
		return _takeExclusiveSignOnControl;
	}

	public RestAuthOptions clientId(String clientId) {
		_clientId = clientId;
		_flags |= RestAuthOptions.HAS_CLIENT_ID;
		return this;
	}

	public String clientId()
	{
		return _clientId;
	}
	
	public RestAuthOptions tokenScope(String tokenScope) {
		_tokenScope = tokenScope;
		_flags |= RestAuthOptions.HAS_TOKEN_SCOPE;
		return this;
	}
	
	public String tokenScope()
	{
		return _tokenScope;
	}
	
	public RestAuthOptions headerAttribute(Map<String, String> attributes) {
		_headerAttribute = (HashMap<String, String>) attributes;
		_flags |= RestAuthOptions.HAS_HEADER_ATTRIB;
		return this;
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
	
	public boolean hasTokenScope()
	{
		return (_flags & RestAuthOptions.HAS_TOKEN_SCOPE) != 0;
	}
	
	public boolean hasHeaderAttribute() 
	{
		return (_flags & RestAuthOptions.HAS_HEADER_ATTRIB) != 0;
	}
    
    public String toString()
	{
		 return "RestAuthOptions" + "\n" + 
				 	"\tgrantType: " + _grantType + "\n" +
	               "\tusername: " + _username + "\n" +
	               "\tpassword: " + _password + "\n" + 
	               "\ttakeExclusiveSignOnControl: " + _takeExclusiveSignOnControl + "\n" + 
	               "\tclientId: " + _clientId + "\n" + 
	               "\ttokenScope: " + _tokenScope + "\n" +
	               "\theaderAttribute: " + _headerAttribute == null ? "" : _headerAttribute.toString() + "\n";
	}

}



