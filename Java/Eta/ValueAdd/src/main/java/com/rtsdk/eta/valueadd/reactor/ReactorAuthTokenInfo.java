package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.transport.TransportReturnCodes;

public class ReactorAuthTokenInfo 
{
	private String _accessToken;
	private String _refreshToken;
	private int _expiresIn = -1;
	private String _tokenType;
	private String _scope;

	ReactorAuthTokenInfo()
	{
	}
	
	ReactorAuthTokenInfo(String accessToken, String refreshToken, int expiresIn, String scope, String tokenType)
	{
		_accessToken = accessToken;
		_refreshToken = refreshToken;
		_expiresIn = expiresIn;
		_scope = scope;
		_tokenType = tokenType;
	}

	public void clear()
	{
		_accessToken = null;
		_refreshToken = null;
		_expiresIn = 0;
		_scope = null;
		_tokenType = null;
	}

    /**
     * Represents the access token used to invoke REST data API calls.
     * 
     * @return String the access token
     */
	public String accessToken() {
		return _accessToken;
	}

    /**
     * Represents the refresh token used for getting next access token.
     * 
     * @return String the refresh token
     */	
	public String refreshToken() {
		return _refreshToken;
	}

    /**
     * Represents access token validity time in seconds.
     * 
     * @return time in seconds for how long the access token is valid
     */ 	
	public int expiresIn() {
		return _expiresIn;
	}

    /**
     * Represents a list of all the scopes this token can be used with.
     * 
     * @return scope of the token
     */	
	public String scope() {
		return _scope;
	}

    /**
     * Represents a token type for specifying in the Authorization header.
     * 
     * @return token type
     */		
	public String tokenType() {
		return _tokenType;
	}

	void accessToken(String accessToken) {
		_accessToken = accessToken;
	}

	void refreshToken(String refreshToken) {
		_refreshToken = refreshToken;
	}

	void expiresIn(int expiresIn) {
		_expiresIn = expiresIn;
	}

	void scope(String scope) {
		_scope = scope;
	}

	void tokenType(String tokenType) {
		_tokenType = tokenType;
	}
	
	public int copy(ReactorAuthTokenInfo destOpts)
    {
        if (destOpts == null)
            return TransportReturnCodes.FAILURE;

        destOpts._accessToken = _accessToken;
        destOpts._refreshToken = _refreshToken;
        destOpts._expiresIn = _expiresIn;
        destOpts._scope = _scope;
        destOpts._tokenType = _tokenType;
        
        return TransportReturnCodes.SUCCESS;
    }
	
	public String toString()
	{
		 return "ReactorAuthTokenInfo" + "\n" + 
	               "\taccessToken: " + _accessToken + "\n" + 
	               "\trefreshToken: " + _refreshToken + "\n" + 
	               "\texpiresIn: " + _expiresIn + "\n" + 
	               "\tscope: " + _scope + "\n" + 
	               "\ttokenType: " + _tokenType;
	}
}

