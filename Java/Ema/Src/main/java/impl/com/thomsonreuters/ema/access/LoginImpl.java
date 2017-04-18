///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.net.InetAddress;
import java.nio.ByteBuffer;
import java.util.Iterator;

import com.thomsonreuters.ema.access.Data.DataCode;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.domain.login.Login;
import com.thomsonreuters.ema.domain.login.Login.LoginRefresh;
import com.thomsonreuters.ema.domain.login.Login.LoginReq;
import com.thomsonreuters.ema.domain.login.Login.LoginStatus;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.State;

abstract class LoginImpl implements Login
{
    protected int _domainType;
    protected int _nameType;
    protected String _name;
    
    protected boolean _changed;
    
    protected boolean  _nameSet;
    protected boolean  _nameTypeSet;
    
    protected OmmInvalidUsageExceptionImpl _ommInvalidUsageExceptionImpl;
    protected ElementList _elementList; 
}

class LoginReqImpl extends LoginImpl implements LoginReq
{
	private boolean _allowSuspectData;
	private boolean _downloadConnectionConfig;
	private boolean _providePermissionProfile;
	private boolean _providePermissionExpressions;
	private boolean _supportProviderDictionaryDownload;
	private int _role;
	private boolean _singleOpen;
	private String	_applicationId;
	private String	_applicationName;
	private String	_applicationAuthToken;
	private String	_instanceId;
	private String	_password;
	private String	_position;
	private String    _authenticationToken;
	private ByteBuffer _authenticationExtended;
	private boolean _pause;
	
	private boolean	_allowSuspectDataSet;
	private boolean	_downloadConnectionConfigSet;
	private boolean	_providePermissionProfileSet;
	private boolean	_providePermissionExpressionsSet;
	private boolean	_singleOpenSet;
	private boolean	_supportProviderDictionaryDownloadSet;
	private boolean	_roleSet;
	private boolean	_applicationIdSet;
	private boolean	_applicationNameSet;
	private boolean	_applicationAuthTokenSet;
	private boolean	_instanceIdSet;
	private boolean	_passwordSet;
	private boolean	_positionSet;
	private boolean   _pauseSet;
	private boolean   _authenticationExtendedSet;
	
	private static final String defaultApplicationId = "256";
    private static final String defaultApplicationName = "ema"; 
    private static String defaultPosition;
    private static String defaultUserName;

	LoginReqImpl()
	{
		clear();
	}
	
	@Override
	public LoginReq clear()
	{
		_changed = true;
		_allowSuspectDataSet = true;
		_allowSuspectData = true;
		_downloadConnectionConfigSet = false;
		_downloadConnectionConfig = false;
		_providePermissionProfileSet = true;
		_providePermissionProfile = true;
		_providePermissionExpressionsSet = true;
		_providePermissionExpressions = true;
		_singleOpenSet = true;
		_singleOpen = true;
		_supportProviderDictionaryDownloadSet  = false;
		_supportProviderDictionaryDownload = false;
		_roleSet  = true;
		_role = EmaRdm.LOGIN_ROLE_CONS;
		_applicationIdSet  = false;
		_applicationNameSet = false;
		_applicationAuthTokenSet = false;
		_instanceIdSet = false;
		_passwordSet = false;
		_positionSet = false;
        _authenticationExtendedSet = false;
        _nameSet = false;
        _nameTypeSet = true;
        _pauseSet = false;
        
        _pause = false;

        if (_authenticationExtended != null)
            _authenticationExtended.clear();
		
		if (_elementList != null )
		{
			_elementList.clear();
		}
		
		_nameType = EmaRdm.USER_NAME;
		_domainType = EmaRdm.MMT_LOGIN;
		
		try
        {
            defaultPosition = InetAddress.getLocalHost().getHostAddress() + "/"
                    + InetAddress.getLocalHost().getHostName();
        }
        catch (Exception e)
        {
            defaultPosition = "1.1.1.1/net";
        }
		
		try
        {
			defaultUserName = System.getProperty("user.name");
        }
        catch (Exception e)
        {
        	defaultUserName = "upa";
        }

		position(defaultPosition);
		applicationName(defaultApplicationName);
		applicationId(defaultApplicationId);
		name(defaultUserName);
		
		return this;
	}
	
	public LoginReqImpl( ReqMsg reqMsg )
	{
		clear();
		
		decode(reqMsg);
	}
	
	@Override
	public LoginReqImpl message( ReqMsg reqMsg )
	{
		decode(reqMsg);
		
		return this;
	}

	@Override
	public LoginReq allowSuspectData( boolean value )
	{
		_changed = true;
		_allowSuspectDataSet = true;
		_allowSuspectData = value;
		
		return this;
	}

	@Override
	public LoginReq downloadConnectionConfig( boolean value )
	{
		_changed = true;
		_downloadConnectionConfigSet = true;
		_downloadConnectionConfig = value;
		
		return this;
	}

	@Override
	public LoginReq applicationId( String value )
	{
	    _changed = true;
		_applicationIdSet = true;
		_applicationId = value;
		
		return this;
	}

	@Override
	public LoginReq applicationName( String value )
	{
        _changed = true;
		_applicationNameSet = true;
		_applicationName = value;
		
		return this;
	}

	@Override
	public LoginReq applicationAuthorizationToken( String value )
	{
        _changed = true;
		_applicationAuthTokenSet = true;
		_applicationAuthToken = value;
		
		return this;
	}

	@Override
    public LoginReq instanceId( String value )
	{
        _changed = true;
		_instanceIdSet = true;
		_instanceId = value;
		
		return this;
	}

	@Override
    public LoginReq password( String value )
	{
        _changed = true;
		_passwordSet = true;
		_password = value;
		
		return this;
	}

	@Override
    public LoginReq position( String value )
	{
        _changed = true;
		_positionSet = true;
		_position = value;
		
		return this;
	}

	@Override
    public LoginReq providePermissionExpressions( boolean value )
	{
        _changed = true;
		_providePermissionExpressionsSet = true;
		_providePermissionExpressions = value;
		
		return this;
	}

	@Override
    public LoginReq providePermissionProfile( boolean value )
	{
        _changed = true;
		_providePermissionProfileSet = true;
		_providePermissionProfile = value;
		
		return this;
	}

	@Override
    public LoginReq role( int value )
	{
        _changed = true;
		_roleSet = true;
		_role = value;
		
		return this;
	}

	@Override
    public LoginReq singleOpen( boolean value )
	{
        _changed = true;
		_singleOpenSet = true;
		_singleOpen = value;
		
		return this;
	}

	@Override
    public LoginReq supportProviderDictionaryDownload( boolean value )
	{
        _changed = true;
		_supportProviderDictionaryDownloadSet = true;
		_supportProviderDictionaryDownload = value;
		
		return this;
	}
	
	@Override
	public LoginReq pause( boolean value )
	{
	    _changed = true;
	    _pauseSet = true;
	    _pause = value;
        
	    return this;
	}

    @Override
    public LoginReq authenticationExtended(ByteBuffer extended)
    {
        _changed = true;
        _authenticationExtendedSet = true;
        _authenticationExtended = extended;
        
        return this;
    }
    
    @Override
    public LoginReq name(String name)
    {
        _changed = true;
        _nameSet = true;
        _name = name;
        
        return this;
    }
    
    @Override
    public LoginReq nameType(int nameType)
    {
        _changed = true;
        _nameTypeSet = true;
        _nameType = nameType;
        
        return this;
    }

    @Override
    public boolean hasAuthenticationExtended()
    {
        return _authenticationExtendedSet;
    }

	@Override
    public boolean hasAllowSuspectData()
	{
		return _allowSuspectDataSet;
	}

	@Override
    public boolean hasDownloadConnectionConfig()
	{
		return _downloadConnectionConfigSet;
	}

	@Override
    public boolean hasApplicationId()
	{
		return _applicationIdSet;
	}

	@Override
    public boolean hasApplicationName()
	{
		return _applicationNameSet;
	}

	@Override
    public boolean hasApplicationAuthorizationToken()
	{
		return _applicationAuthTokenSet;
	}

	@Override
    public boolean hasInstanceId()
	{
		return _instanceIdSet;
	}

	@Override
    public boolean hasPassword()
	{
		return _passwordSet;
	}

	@Override
    public boolean hasPosition()
	{
		return _positionSet;
	}

	@Override
    public boolean hasProvidePermissionExpressions()
	{
		return _providePermissionExpressionsSet;
	}

	@Override
    public boolean hasProvidePermissionProfile()
	{
		return _providePermissionProfileSet;
	}

	@Override
    public boolean hasRole()
	{
		return _roleSet;
	}

	@Override
    public boolean hasSingleOpen()
	{
		return _singleOpenSet;
	}

	@Override
    public boolean hasSupportProviderDictionaryDownload()
	{
		return _supportProviderDictionaryDownloadSet;
	}
	
	@Override
	public boolean hasName()
	{
	    return _nameSet;
	}
	
	@Override
	public boolean hasNameType()
	{
	    return _nameTypeSet;
	}
	
	@Override
	public boolean hasPause()
	{
	    return _pauseSet;
	}

	@Override
    public ReqMsg message()
	{
        ReqMsg reqMsg = EmaFactory.createReqMsg();
        
        reqMsg.domainType(_domainType);
        if (_nameTypeSet)
        {
            if (_nameType == EmaRdm.USER_AUTH_TOKEN)
            {
                if (!_nameSet)
                    _authenticationToken = "\0";
                else
                    _authenticationToken = _name;
                reqMsg.name("\0");
            }

            reqMsg.nameType(_nameType);
        }
            
        if (_nameTypeSet && _nameType != EmaRdm.USER_AUTH_TOKEN)
            reqMsg.name(_name);

        if (_pauseSet)
            reqMsg.pause(_pause);
        
		if(!_changed)
		{
		    reqMsg.attrib(_elementList);
			return reqMsg;
		}
		
		encode(reqMsg);
		
		_changed = false;
		
		return reqMsg;
	}

	@Override
    public boolean allowSuspectData()
	{			
		return _allowSuspectData;
	}

	@Override
    public boolean downloadConnectionConfig()
	{			
		return _downloadConnectionConfig;
	}

	@Override
    public String applicationId()
	{
		if(!_applicationIdSet)
		{
			throw ommIUExcept().message( EmaRdm.ENAME_APP_ID + " element is not set");
		}
		
		return _applicationId;
	}

	@Override
    public String applicationName()
	{
		if(!_applicationNameSet)
		{
			throw ommIUExcept().message( EmaRdm.ENAME_APP_NAME + " element is not set");
		}
		
		return _applicationName;
	}

	@Override
    public String applicationAuthorizationToken()
	{
		if(!_applicationAuthTokenSet)
		{
			throw ommIUExcept().message(EmaRdm.ENAME_APPAUTH_TOKEN + " element is not set");
		}
		
		return _applicationAuthToken;
	}

	@Override
    public String instanceId()
	{
		if(!_instanceIdSet)
		{
			throw ommIUExcept().message(EmaRdm.ENAME_INST_ID + " element is not set");
		}
		
		return _instanceId;
	}

	@Override
    public String password()
	{
		if(!_passwordSet)
		{
			throw ommIUExcept().message(EmaRdm.ENAME_PASSWORD + " element is not set");
		}
		
		return _password;
	}

	@Override
    public String position()
	{
		if(!_positionSet)
		{
			throw ommIUExcept().message(EmaRdm.ENAME_POSITION + " element is not set");
		}
		
		return _position;
	}

   @Override
   public ByteBuffer authenticationExtended()
   {
       if(!_authenticationExtendedSet)
       {
           throw ommIUExcept().message(EmaRdm.ENAME_AUTHN_EXTENDED + " element is not set");
       }
       
       return _authenticationExtended;
   }

	@Override
    public boolean providePermissionExpressions()
	{			
		return _providePermissionExpressions;
	}

	@Override
    public boolean providePermissionProfile()
	{
		return _providePermissionProfile;
	}

	@Override
    public int role()
	{
		return _role;
	}

	@Override
    public boolean singleOpen()
	{
		return _singleOpen;

	}

	@Override
    public boolean supportProviderDictionaryDownload()
	{
		return _supportProviderDictionaryDownload;
	}
	
	@Override
	public boolean pause()
	{
	    return _pause;
	}
	
	@Override
	public String name()
	{
       if(!_nameSet)
       {
           throw ommIUExcept().message(EmaRdm.ENAME_USERNAME + " element is not set");
       }

	    return _name;
	}
	
  @Override
    public int nameType()
    {
       if(!_nameTypeSet)
       {
           throw ommIUExcept().message(EmaRdm.ENAME_USERNAME_TYPE + " element is not set");
       }

        return _nameType;
    }

	@Override
    public String toString()
	{
		StringBuilder text = new StringBuilder();
		
		if ( _allowSuspectDataSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_ALLOW_SUSPECT_DATA + " : " + _allowSuspectData);
		}
		
		if ( _applicationIdSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_APP_ID + " : " + _applicationId);
		}
		
		if ( _applicationNameSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_APP_NAME + " : " + _applicationName);
		}
		
		if ( _applicationAuthTokenSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_APPAUTH_TOKEN + " : " + _applicationAuthToken);
		}
		
		if ( _downloadConnectionConfigSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_DOWNLOAD_CON_CONFIG + " : " + _downloadConnectionConfig);
		}
		
		if ( _instanceIdSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_INST_ID + " : " + _instanceId);
		}
		
		if ( _passwordSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_PASSWORD + " : " + _password);
		}
		
		if ( _positionSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_POSITION + " : " + _position);
		}
		
		if ( _providePermissionExpressionsSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_PROV_PERM_EXP + " : " + _providePermissionExpressions);
		}
		
		if ( _providePermissionProfileSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_PROV_PERM_PROF + " : " + _providePermissionProfile);
		}
		
		if ( _roleSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_ROLE + " : " + _role);
		}
		
		if ( _singleOpenSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SINGLE_OPEN + " : " + _singleOpen);
		}
		
		if ( _supportProviderDictionaryDownloadSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD + " : " + _supportProviderDictionaryDownload);
		}
		
         if ( _pauseSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_PAUSE + " : " + _pause);
        }
         
        if ( _authenticationExtendedSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_EXTENDED + " : " + _authenticationExtended);
        }
        
        if (_nameSet)
        {
            if (_nameTypeSet && _nameType == EmaRdm.USER_AUTH_TOKEN)
            {
                text.append("\r\n" + EmaRdm.ENAME_AUTHN_TOKEN + " : " + _authenticationToken);
            }
            else
                text.append("\r\n" + EmaRdm.ENAME_USERNAME + " : " + _name);
        }
        
        if (_nameTypeSet)
        {
            text.append("\r\n" + EmaRdm.ENAME_USERNAME_TYPE + " : " + _nameType);
        }
		
		return text.toString();
	}
	
	private void decode(ReqMsg reqMsg)
	{
	    if (reqMsg.domainType() != EmaRdm.MMT_LOGIN)
	    {
	        throw ommIUExcept().message("Domain type must be Login.");
	    }
	    
		_allowSuspectDataSet = false;
		_downloadConnectionConfigSet = false;
		_providePermissionProfileSet = false;
		_providePermissionExpressionsSet = false;
		_singleOpenSet = false;
		_supportProviderDictionaryDownloadSet = false;
		_roleSet = false;
		_applicationIdSet = false;
		_applicationNameSet = false;
		_applicationAuthTokenSet = false;
		_instanceIdSet = false;
		_passwordSet = false;
		_positionSet = false;
		_pauseSet = false;
		_authenticationExtendedSet = false;
	    
	    if (reqMsg.hasNameType())
	        nameType(reqMsg.nameType());
	    if (reqMsg.hasName())
	        name(reqMsg.name());
	    if (reqMsg.pause())
	        pause(reqMsg.pause());
	    
	    if (reqMsg.attrib().dataType() == DataTypes.ELEMENT_LIST)
	    {
    		Iterator<ElementEntry> iterator = reqMsg.attrib().elementList().iterator();
    		
    		ElementEntry elementEntry;
    		String elementName = null;
    		
    		try
    		{
    			while(iterator.hasNext())
    			{
    				elementEntry = iterator.next();
    				elementName = elementEntry.name();
    				
    				switch(elementName)
    				{
    				case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
    				{
    					long allowSuspectData = elementEntry.uintValue();
    					
    					if(allowSuspectData > 0)
    					{
    						allowSuspectData(true);
    					}
    					else
    					{
    						allowSuspectData(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_APP_ID:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						applicationId(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_APP_NAME:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						applicationName(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_APPAUTH_TOKEN:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						applicationAuthorizationToken(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_DOWNLOAD_CON_CONFIG:
    				{
    					long downloadConnectionConfig = elementEntry.uintValue();
    					
    					if(downloadConnectionConfig > 0 )
    					{
    						downloadConnectionConfig(true);
    					}
    					else
    					{
    						downloadConnectionConfig(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_INST_ID:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						instanceId(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_PASSWORD:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						password(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_POSITION:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						position(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_AUTHN_EXTENDED:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						authenticationExtended(elementEntry.buffer().buffer());
    					}
    				}
    				    break;
    				case EmaRdm.ENAME_PROV_PERM_EXP:
    				{
    					long providePermissionExpressions = elementEntry.uintValue();
    					
    					if ( providePermissionExpressions > 0 )
    					{
    						providePermissionExpressions(true);
    					}
    					else
    					{
    						providePermissionExpressions(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_PROV_PERM_PROF:
    				{
    					long providePermissionProfile = elementEntry.uintValue();
    					
    					if ( providePermissionProfile > 0 )
    					{
    						providePermissionProfile(true);
    					}
    					else
    					{
    						providePermissionProfile(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_ROLE:
    				{
    					long role = elementEntry.uintValue();
    					
    					if ( role == 1 )
    					{
    						role(EmaRdm.LOGIN_ROLE_PROV);
    					}
    					else if ( role == 0 )
    					{
    						role(EmaRdm.LOGIN_ROLE_CONS);
    					}
    					else
    					{
    						throw ommIUExcept().message("Invalid element value of " + role);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SINGLE_OPEN:
    				{
    					long singleOpen = elementEntry.uintValue();
    					
    					if ( singleOpen > 0 )
    					{
    						singleOpen(true);
    					}
    					else
    					{
    						singleOpen(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
    				{
    					long supportProviderDictionaryDownload = elementEntry.uintValue();
    					
    					if ( supportProviderDictionaryDownload > 0 )
    					{
    						supportProviderDictionaryDownload(true);
    					}
    					else
    					{
    						supportProviderDictionaryDownload(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_AUTHN_TOKEN:
    				{
    				    if (!_nameTypeSet || _nameType != EmaRdm.USER_AUTH_TOKEN)
    				    {
    				    	if(elementEntry.code() != DataCode.BLANK)
        					{
    				    		throw ommIUExcept().message("NameType must be USER_AUTH_TOKEN when element list contains AuthenticationToken");
        					}
    				    }
    				    name(elementEntry.ascii().ascii());
    				}
    				    break;
    				}
    			}
    		}
    		catch(OmmInvalidUsageException ommInvlaidUsageException)
    		{
    			throw ommIUExcept().message("Decoding error for " + elementName + " element. " + ommInvlaidUsageException.getMessage());
    		}
	    }
	}
	
	private void encode(ReqMsg reqMsg)
	{
		if ( _elementList == null )
		{
			_elementList = EmaFactory.createElementList();
		}
		else
		{
			_elementList.clear();
		}
		
		if ( _allowSuspectDataSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, _allowSuspectData == true ? 1 : 0));
		}
		
		if ( _applicationIdSet )
		{
			_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, _applicationId));
		}
		
		if ( _applicationNameSet )
		{
			_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, _applicationName));
		}
		
		if ( _applicationAuthTokenSet )
		{
			_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APPAUTH_TOKEN, _applicationAuthToken));
		}
		
		if ( _downloadConnectionConfigSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_DOWNLOAD_CON_CONFIG, _downloadConnectionConfig == true ? 1 : 0));
		}
		
		if ( _instanceIdSet )
		{
			_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_INST_ID, _instanceId));
		}
		
		if ( _passwordSet )
		{
			_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_PASSWORD, _password));
		}
		
		if ( _positionSet )
		{
			_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, _position));
		}
		
		if ( _providePermissionExpressionsSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_EXP, _providePermissionExpressions == true ? 1 : 0));
		}
		
		if ( _providePermissionProfileSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_PROF, _providePermissionProfile == true ? 1 : 0));
		}
		
		if ( _roleSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ROLE, _role));
		}
		
		if ( _singleOpenSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SINGLE_OPEN, _singleOpen == true ? 1 : 0));
		}
		
		if ( _supportProviderDictionaryDownloadSet )
		{
			_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, _supportProviderDictionaryDownload == true ? 1 : 0));
		}
		
		if (_nameTypeSet && _nameType == EmaRdm.USER_AUTH_TOKEN)
		{
		    _elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_TOKEN, _authenticationToken));

	        if ( _authenticationExtendedSet )
	        {
	            _elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_AUTHN_EXTENDED, _authenticationExtended));
	        }
		}
        
        reqMsg.attrib(_elementList);

	}
	
	private OmmInvalidUsageExceptionImpl ommIUExcept()
	{    
		if (_ommInvalidUsageExceptionImpl == null)
			_ommInvalidUsageExceptionImpl = new OmmInvalidUsageExceptionImpl();
		
		return _ommInvalidUsageExceptionImpl;
	}

}

class LoginRefreshImpl extends LoginImpl implements LoginRefresh
{
	private boolean _allowSuspectData;
	private boolean _providePermissionExpressions;
	private boolean _providePermissionProfile;
	private boolean _singleOpen;
	private boolean _solicited;

	private int _supportBatchRequests;
	private int _supportEnhancedSymbolList;
	private boolean _supportOmmPost;
	private boolean _supportOptimizedPauseResume;
	private boolean _supportViewRequests;
	private boolean _supportStandby;

	private boolean _supportProviderDictionaryDownload;
	private String	_applicationId;
	private String	_applicationName;
	private String	_position;
	
	private ByteBuffer _authenticationExtendedResp;
	private long      _authenticationTTReissue;
	private long      _authenticationErrorCode;
	private String    _authenticationErrorText;

	private boolean	_allowSuspectDataSet;
	private boolean	_providePermissionProfileSet;
	private boolean	_providePermissionExpressionsSet;
	private boolean	_singleOpenSet;
	private boolean _supportBatchRequestsSet;
	private boolean _supportOptimizedPauseResumeSet;
	private boolean	_supportProviderDictionaryDownloadSet;
	private boolean	_applicationIdSet;
	private boolean	_applicationNameSet;
	private boolean	_positionSet;
	private boolean _supportViewRequestsSet;
	private boolean _supportStandbySet;
	private boolean _supportOmmPostSet;
	private boolean _supportEnhancedSymbolListSet;
	private boolean _authenticationExtendedRespSet;
	private boolean _authenticationTTReissueSet;
	private boolean _authenticationErrorCodeSet;
	private boolean _authenticationErrorTextSet;
	private boolean _solicitedSet;
	private boolean _seqNumSet;
	private boolean _stateSet;

    private State _rsslState;
    private OmmStateImpl _state;
    private Buffer _stateText;
    
    private int _seqNum;

	LoginRefreshImpl()
	{
		clear();
	}
	
	LoginRefreshImpl( RefreshMsg refreshMsg )
	{
		clear();
		
		decode(refreshMsg);
	}
	
	@Override
    public LoginRefresh clear()
	{
		_changed = true;
		_allowSuspectDataSet = true;
		_allowSuspectData = true;
		_providePermissionExpressionsSet = true;
		_providePermissionExpressions = true;
		_providePermissionProfileSet = true;
		_providePermissionProfile = true;
		_singleOpenSet = true;
		_singleOpen = true;
		_solicitedSet = true;
		_solicited = true;
		_supportProviderDictionaryDownloadSet = false;
		_supportProviderDictionaryDownload = false;
		_supportBatchRequestsSet = false;
		_supportBatchRequests = 0;
		_supportOptimizedPauseResumeSet = false;
		_supportOptimizedPauseResume = false;
		_supportOmmPostSet = false;
		_supportOmmPost = false;
		_applicationIdSet = false;
		_applicationNameSet = false;
		_positionSet = false;
		_supportViewRequestsSet = false;
		_supportViewRequests = false;
		_supportStandbySet = false;
		_supportStandby = false;
		_supportEnhancedSymbolListSet = false;
		_supportEnhancedSymbolList = EmaRdm.SUPPORT_SYMBOL_LIST_NAMES_ONLY;
        _authenticationExtendedRespSet = false;
        _authenticationTTReissueSet = false;
        _authenticationErrorCodeSet = false;
        _authenticationErrorTextSet = false;
        _nameSet = false;
        _nameTypeSet = true;
        _stateSet = false;
        _seqNumSet = false;
        
        _nameType = EmaRdm.USER_NAME;
        _domainType = EmaRdm.MMT_LOGIN;

        if (_stateText != null)
            _stateText.clear();
        else
            _stateText = CodecFactory.createBuffer();
        
        if (_rsslState != null)
            _rsslState.clear();
        else
            _rsslState = CodecFactory.createState();
        
        if (_state == null)
            _state = new OmmStateImpl();
        
        if (_authenticationExtendedResp != null)
            _authenticationExtendedResp.clear();

		return this;
	}

	/** Sets loginRefresh with ElementList containing login request elements
		allows easy decoding of login request elements
		@return reference to this object
	*/
	@Override
    public LoginRefresh message( RefreshMsg refreshMsg )
	{
		decode(refreshMsg);
		
		return this;
	}

	@Override
    public LoginRefresh allowSuspectData( boolean value )
	{
		_changed = true;
		_allowSuspectDataSet = true;
		_allowSuspectData = value;
		
		return this;
	}

	@Override
    public LoginRefresh applicationId( String value )
	{
		_changed = true;
		_applicationIdSet = true;
		_applicationId = value;
		
		return this;
	}

	@Override
    public LoginRefresh applicationName( String value )
	{
		_changed = true;
		_applicationNameSet = true;
		_applicationName = value;
		
		return this;
	}

	@Override
    public LoginRefresh position( String value )
	{
		_changed = true;
		_positionSet = true;
		_position = value;
		
		return this;
	}

	@Override
    public LoginRefresh providePermissionExpressions( boolean value )
	{
		_changed = true;
		_providePermissionExpressionsSet = true;
		_providePermissionExpressions = value;
		
		return this;
	}

	@Override
    public LoginRefresh providePermissionProfile( boolean value )
	{
		_changed = true;
		_providePermissionProfileSet = true;
		_providePermissionProfile = value;
		
		return this;
	}

	@Override
    public LoginRefresh singleOpen( boolean value )
	{
		_changed = true;
		_singleOpenSet = true;
		_singleOpen = value;
		
		return this;
	}
	
	@Override
	public LoginRefresh solicited( boolean value )
	{
	    _changed = true;
	    _solicitedSet = true;
	    _solicited = value;
	    
	    return this;
	}

	@Override
    public LoginRefresh supportBatchRequests( int value )
	{
		_changed = true;
		_supportBatchRequestsSet = true;
		_supportBatchRequests = value;
		
		return this;
	}

	@Override
    public LoginRefresh supportEnhancedSymbolList( int value )
	{
		_changed = true;
		_supportEnhancedSymbolListSet = true;
		_supportEnhancedSymbolList = value;
		
		return this;
	}

	@Override
    public LoginRefresh supportOMMPost( boolean value )
	{
		_changed = true;
		_supportOmmPostSet = true;
		_supportOmmPost = value;
		
		return this;
	}

	@Override
    public LoginRefresh supportOptimizedPauseResume( boolean value )
	{
		_changed = true;
		_supportOptimizedPauseResumeSet = true;
		_supportOptimizedPauseResume = value;
		
		return this;
	}

	@Override
    public LoginRefresh supportProviderDictionaryDownload( boolean value )
	{
		_changed = true;
		_supportProviderDictionaryDownloadSet = true;
		_supportProviderDictionaryDownload = value;
		
		return this;
	}

	@Override
    public LoginRefresh supportViewRequests( boolean value )
	{
		_changed = true;
		_supportViewRequestsSet = true;
		_supportViewRequests = value;
		
		return this;
	}

	@Override
    public LoginRefresh supportStandby( boolean value )
	{
		_changed = true;
		_supportStandbySet = true;
		_supportStandby = value;
		
		return this;
	}

    @Override
    public LoginRefresh authenticationExtendedResp(ByteBuffer extended)
    {
        _changed = true;
        _authenticationExtendedRespSet = true;
        _authenticationExtendedResp = extended;
        
        return this;
    }

    @Override
    public LoginRefresh authenticationTTReissue(long authenticationTTReissue)
    {
        _changed = true;
        _authenticationTTReissueSet = true;
        _authenticationTTReissue = authenticationTTReissue;
        
        return this;
    }

    @Override
    public LoginRefresh authenticationErrorCode(long authenticationErrorCode)
    {
        _changed = true;
        _authenticationErrorCodeSet = true;
        _authenticationErrorCode = authenticationErrorCode;
        
        return this;
    }

    @Override
    public LoginRefresh authenticationErrorText(String authenticationErrorText)
    {
        _changed = true;
        _authenticationErrorTextSet = true;
        _authenticationErrorText = authenticationErrorText;
        
        return this;
    }

    @Override
    public LoginRefresh name(String name)
    {
        _changed = true;
        _nameSet = true;
        _name = name;
        
        return this;
    }

    @Override
    public LoginRefresh nameType(int nameType)
    {
        _changed = true;
        _nameTypeSet = true;
        _nameType = nameType;
        
        return this;
    }
    
    @Override
    public LoginRefresh seqNum(int seqNum)
    {
        _changed = true;
        _seqNumSet = true;
        _seqNum = seqNum;
        
        return this;
    }
    
    @Override
    public LoginRefresh state(int streamState, int dataState, int statusCode, String statusText)
    {
        _changed = true;
        _stateSet = true;
        
        _rsslState.streamState(streamState);
        _rsslState.dataState(dataState);
        _rsslState.code(statusCode);
        _stateText.data(statusText);
        _rsslState.text(_stateText);
        
        return this;
        
    }
    
    public LoginRefresh state(OmmState value)
    {
        _changed = true;
        _stateSet = true;
        
        _rsslState.streamState(value.streamState());
        _rsslState.dataState(value.dataState());
        _rsslState.code(value.statusCode());
        _stateText.data(value.statusText());
        _rsslState.text(_stateText);
        
        return this;
        
    }

	@Override
    public boolean hasAllowSuspectData()
	{
		return _allowSuspectDataSet;
	}

	@Override
    public boolean hasApplicationId()
	{
		return _applicationIdSet;
	}

	@Override
    public boolean hasApplicationName()
	{
		return _applicationNameSet;
	}

	@Override
    public boolean hasPosition()
	{
		return _positionSet;
	}

	@Override
    public boolean hasProvidePermissionExpressions()
	{
		return _providePermissionExpressionsSet;
	}

	@Override
    public boolean hasProvidePermissionProfile()
	{
		return _providePermissionProfileSet;
	}

	@Override
    public boolean hasSingleOpen()
	{
		return _singleOpenSet;
	}
	
   @Override
    public boolean hasSolicited()
    {
        return _solicitedSet;
    }

	@Override
    public boolean hasSupportBatchRequests()
	{
		return _supportBatchRequestsSet;
	}

	@Override
    public boolean hasSupportEnhancedSymbolList()
	{
		return _supportEnhancedSymbolListSet;
	}

	@Override
    public boolean hasSupportOMMPost()
	{
		return _supportOmmPostSet;
	}

	@Override
    public boolean hasSupportOptimizedPauseResume()
	{
		return _supportOptimizedPauseResumeSet;
	}

	@Override
    public boolean hasSupportProviderDictionaryDownload()
	{
		return _supportProviderDictionaryDownloadSet;
	}

	@Override
    public boolean hasSupportViewRequests()
	{
		return _supportViewRequestsSet;
	}

	@Override
    public boolean hasSupportStandby()
	{
		return _supportStandbySet;
	}
	
    @Override
    public boolean hasAuthenticationExtended()
    {
        return _authenticationExtendedRespSet;
    }

    @Override
    public boolean hasAuthenticationTTReissue()
    {
        return _authenticationTTReissueSet;
    }

    @Override
    public boolean hasAuthenticationErrorCode()
    {
        return _authenticationErrorCodeSet;
    }

    @Override
    public boolean hasAuthenticationErrorText()
    {
        return _authenticationErrorTextSet;
    }
    
    @Override
    public boolean hasName()
    {
        return _nameSet;
    }
    
    @Override
    public boolean hasNameType()
    {
        return _nameTypeSet;
    }
    
    @Override
    public boolean hasSeqNum()
    {
        return _seqNumSet;
    }
    
    @Override
    public boolean hasState()
    {
        return _stateSet;
    }

    @Override
    public RefreshMsg message()
	{
        RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
        
        refreshMsg.domainType(_domainType);
        
        if (_stateSet)
            refreshMsg.state(_rsslState.streamState(), _rsslState.dataState(), _rsslState.code(), new String(_rsslState.text().data().array()));

        if (_seqNumSet)
            refreshMsg.seqNum(_seqNum);
        if (_nameTypeSet)
            refreshMsg.nameType(_nameType);
        if (_nameSet)
            refreshMsg.name(_name);
        if (_solicitedSet)
            refreshMsg.solicited(_solicited);
        refreshMsg.complete(true);

		if(!_changed)
		{
			return refreshMsg;
		}
		
		encode(refreshMsg);
		
		_changed = false;
		
		return refreshMsg;
	}

	@Override
    public boolean allowSuspectData()
	{
		return _allowSuspectData;
	}

	@Override
    public String applicationId()
	{
		if(!_applicationIdSet)
		{
			throw ommIUExcept().message( EmaRdm.ENAME_APP_ID + " element is not set");
		}
		
		return _applicationId;
	}

	@Override
    public String applicationName()
	{
		if(!_applicationNameSet)
		{
			throw ommIUExcept().message( EmaRdm.ENAME_APP_NAME + " element is not set");
		}
		
		return _applicationName;
	}

	@Override
    public String position()
	{
		if(!_positionSet)
		{
			throw ommIUExcept().message( EmaRdm.ENAME_POSITION + " element is not set");
		}
		
		return _position;
	}

    @Override
    public ByteBuffer authenticationExtended()
    {
        if (!_authenticationExtendedRespSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_AUTHN_EXTENDED + " element is not set");
        }
        
        return _authenticationExtendedResp;
    }

    @Override
    public long authenticationTTReissue()
    {
        if (!_authenticationTTReissueSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_AUTHN_TT_REISSUE + " element is not set");
        }
        
        return _authenticationTTReissue;
    }

    @Override
    public long authenticationErrorCode()
    {
        if (!_authenticationErrorCodeSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_AUTHN_ERRORCODE + " element is not set");
        }
        
        return _authenticationErrorCode;
    }

    @Override
    public String authenticationErrorText()
    {
        if (!_authenticationErrorTextSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_AUTHN_ERRORTEXT + " element is not set");
        }
        
        return _authenticationErrorText;
    }

	@Override
    public boolean providePermissionExpressions()
	{
		return _providePermissionExpressions;
	}

	@Override
    public boolean providePermissionProfile()
	{
		return _providePermissionProfile;
	}

	@Override
    public boolean singleOpen()
	{
		return _singleOpen;
	}
	
   @Override
    public boolean solicited()
    {
        return _solicited;
    }

	@Override
    public int supportBatchRequests()
	{
		return _supportBatchRequests;
	}

	@Override
    public int supportEnhancedSymbolList()
	{
		return _supportEnhancedSymbolList;
	}

	@Override
    public boolean supportOMMPost()
	{
		return _supportOmmPost;
	}

	@Override
    public boolean supportOptimizedPauseResume()
	{
		return _supportOptimizedPauseResume;
	}

	@Override
    public boolean supportProviderDictionaryDownload()
	{
		return _supportProviderDictionaryDownload;
	}

	@Override
    public boolean supportViewRequests()
	{
		return _supportViewRequests;
	}

	@Override
    public boolean supportStandby()
	{
		return _supportStandby;
	}
	
    @Override
    public String name()
    {
        if (!_nameSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_USERNAME + " element is not set");
        }
        
        return _name;
    }
    
    @Override
    public int nameType()
    {
        if (!_nameTypeSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_USERNAME_TYPE + " element is not set");
        }
        
        return _nameType;
    }
    
    @Override
    public int seqNum()
    {
        if (!_seqNumSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_SEQ_NUM + " element is not set");
        }
        
        return _seqNum;
    }
    
    @Override
    public OmmState state()
    {
        if (!_stateSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_STATE + " element is not set");
        }

        _state.decode(_rsslState);

        return _state;

    }


	@Override
    public String toString()
	{
		StringBuilder text = new StringBuilder();
		
		if ( _allowSuspectDataSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_ALLOW_SUSPECT_DATA + " : " + _allowSuspectData);
		}
		
		if ( _applicationIdSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_APP_ID + " : " + _applicationId);
		}
		
		if ( _applicationNameSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_APP_NAME + " : " + _applicationName);
		}
		
		if ( _positionSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_POSITION + " : " + _position);
		}
		
		if ( _providePermissionExpressionsSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_PROV_PERM_EXP + " : " + _providePermissionExpressions);
		}
		
		if ( _providePermissionProfileSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_PROV_PERM_PROF + " : " + _providePermissionProfile);
		}
		
		if ( _singleOpenSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SINGLE_OPEN + " : " + _singleOpen);
		}
		
		if ( _supportBatchRequestsSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_BATCH + " : " + _supportBatchRequests);
		}
		
		if ( _supportOmmPostSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_POST + " : " + _supportOmmPost);
		}
		
		if ( _supportProviderDictionaryDownloadSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD + " : " + _supportProviderDictionaryDownload);
		}
		
		if ( _supportOptimizedPauseResumeSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_OPR + " : " + _supportOptimizedPauseResume);
		}
		
		if ( _supportViewRequestsSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_VIEW + " : " + _supportViewRequests);
		}
		
		if ( _supportStandbySet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_STANDBY + " : " + _supportStandby);
		}
		
		if ( _supportEnhancedSymbolListSet )
		{
			text.append("\r\n" + EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST + " : " + _supportEnhancedSymbolList);
		}
        
        if ( _authenticationExtendedRespSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_EXTENDED + " : " + _authenticationExtendedResp);
        }
        
        if ( _authenticationTTReissueSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_TT_REISSUE + " : " + _authenticationTTReissue);
        }
        
        if ( _authenticationErrorCodeSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_ERRORCODE + " : " + _authenticationErrorCode);
        }
        
        if ( _authenticationErrorTextSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_ERRORTEXT + " : " + _authenticationErrorText);
        }

        if ( _solicitedSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_SOLICITED + " : " + _solicited);
        }
        
        if ( _nameSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_USERNAME + " : " + _name);
        }
        
        if ( _nameTypeSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_USERNAME_TYPE + " : " + _nameType);
        }
        
        if ( _seqNumSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_SEQ_NUM + " : " + _seqNum);
        }
        
        if ( _stateSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_STATE + " : StreamState: " + _rsslState.streamState() + " DataState: " + _rsslState.dataState() + " StatusCode: " + _rsslState.code() + "StatusText: " + new String(_rsslState.text().data().array()));
        }
		
		return text.toString();
	}
	
	private void decode(RefreshMsg refreshMsg)
	{
        if (refreshMsg.domainType() != EmaRdm.MMT_LOGIN)
        {
            throw ommIUExcept().message("Domain type must be Login.");
        }
        
    	_allowSuspectDataSet = false;
    	_providePermissionProfileSet = false;
    	_providePermissionExpressionsSet = false;
    	_singleOpenSet = false;
    	_supportBatchRequestsSet = false;
    	_supportOptimizedPauseResumeSet = false;
    	_supportProviderDictionaryDownloadSet = false;
    	_applicationIdSet = false;
    	_applicationNameSet = false;
    	_positionSet = false;
    	_supportViewRequestsSet = false;
    	_supportStandbySet = false;
    	_supportOmmPostSet = false;
    	_supportEnhancedSymbolListSet = false;
    	_authenticationExtendedRespSet = false;
    	_authenticationTTReissueSet = false;
    	_authenticationErrorCodeSet = false;
    	_authenticationErrorTextSet = false;
    	_solicitedSet = false;
    	_seqNumSet = false;
    	_stateSet = false;
        
	    if (refreshMsg.hasSeqNum())
	        seqNum((int)refreshMsg.seqNum());
	    if (refreshMsg.hasNameType())
	        nameType(refreshMsg.nameType());
	    if (refreshMsg.hasName())
	        name(refreshMsg.name());
        state(refreshMsg.state());
        solicited(refreshMsg.solicited());
	    
        if (refreshMsg.attrib().dataType() == DataTypes.ELEMENT_LIST)
        {
    		Iterator<ElementEntry> iterator = refreshMsg.attrib().elementList().iterator();
    		
    		ElementEntry elementEntry;
    		String elementName = null;
    
    		try
    		{
    			while(iterator.hasNext())
    			{
    				elementEntry = iterator.next();
    				elementName = elementEntry.name();
    				
    				switch(elementEntry.name())
    				{
    				case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
    				{
    					long allowSuspectData = elementEntry.uintValue();
    					
    					if(allowSuspectData > 0)
    					{
    						allowSuspectData(true);
    					}
    					else
    					{
    						allowSuspectData(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_APP_ID:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						applicationId(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_APP_NAME:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						applicationName(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_POSITION:
    				{
    					if(elementEntry.code() != DataCode.BLANK)
    					{
    						position(elementEntry.ascii().ascii());
    					}
    				}
    					break;
    				case EmaRdm.ENAME_PROV_PERM_EXP:
    				{
    					long providePermissionExpressions = elementEntry.uintValue();
    					
    					if ( providePermissionExpressions > 0 )
    					{
    						providePermissionExpressions(true);
    					}
    					else
    					{
    						providePermissionExpressions(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_PROV_PERM_PROF:
    				{
    					long providePermissionProfile = elementEntry.uintValue();
    					
    					if ( providePermissionProfile > 0 )
    					{
    						providePermissionProfile(true);
    					}
    					else
    					{
    						providePermissionProfile(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SINGLE_OPEN:
    				{
    					long singleOpen = elementEntry.uintValue();
    					
    					if ( singleOpen > 0 )
    					{
    						singleOpen(true);
    					}
    					else
    					{
    						singleOpen(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_BATCH:
    				{
    					long supportBatchRequests = elementEntry.uintValue();
    					
    					_changed = true;
    					_supportBatchRequestsSet = true;
    					
    					if ( supportBatchRequests == 0 )
    					{
    						_supportBatchRequests = 0;
    					}
    					else
    					{
    						if ( ( supportBatchRequests & 0x1 ) == 0x1 )
    						{
    						    _supportBatchRequests |= EmaRdm.SUPPORT_BATCH_REQUEST;
    						}
    						
    						if ( ( supportBatchRequests & 0x2 ) == 0x2 )
    						{
    						    _supportBatchRequests |= EmaRdm.SUPPORT_BATCH_REISSUE;
    						}
    						
    						if ( ( supportBatchRequests & 0x4 ) == 0x4 )
    						{
    						    _supportBatchRequests |= EmaRdm.SUPPORT_BATCH_CLOSE;
    						}
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_POST:
    				{
    					long supportOmmPost = elementEntry.uintValue();
    					
    					if ( supportOmmPost > 0 )
    					{
    						supportOMMPost(true);
    					}
    					else
    					{
    						supportOMMPost(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_VIEW:
    				{
    					long supportView = elementEntry.uintValue();
    					
    					if ( supportView > 0 )
    					{
    						supportViewRequests(true);
    					}
    					else
    					{
    						supportViewRequests(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD:
    				{
    					long supportProviderDictionaryDownload = elementEntry.uintValue();
    					
    					if ( supportProviderDictionaryDownload > 0 )
    					{
    						supportProviderDictionaryDownload(true);
    					}
    					else
    					{
    						supportProviderDictionaryDownload(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_OPR:
    				{
    					long supportOPR = elementEntry.uintValue();
    					
    					if ( supportOPR > 0 )
    					{
    						supportOptimizedPauseResume(true);
    					}
    					else
    					{
    						supportOptimizedPauseResume(false);
    					}
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_STANDBY:
    				{
    					long supportStandby = elementEntry.uintValue();
    					
    					if ( supportStandby > 0 )
    					{
    						supportStandby(true);
    					}
    					else
    					{
    						supportStandby(false);
    					}
    					
    				}
    					break;
    				case EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST:
    				{
    					long supportEnhancedSymbolList = elementEntry.uintValue();
    					
    					if ( supportEnhancedSymbolList > 0 )
    					{
    						supportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_DATA_STREAMS);
    					}
    					else
    					{
    						supportEnhancedSymbolList(EmaRdm.SUPPORT_SYMBOL_LIST_NAMES_ONLY);
    					}
    				}
    					break;
                    case EmaRdm.ENAME_AUTHN_EXTENDED_RESP:
                    {
                    	if(elementEntry.code() != DataCode.BLANK)
    					{
                    		authenticationExtendedResp(elementEntry.buffer().buffer());
    					}
                    }
                        break;
                    case EmaRdm.ENAME_AUTHN_TT_REISSUE:
                    {
                        authenticationTTReissue(elementEntry.uintValue());
                    }
                        break;
                    case EmaRdm.ENAME_AUTHN_ERRORCODE:
                    {
                        authenticationErrorCode(elementEntry.uintValue());
                    }
                        break;
                    case EmaRdm.ENAME_AUTHN_ERRORTEXT:
                    {
                    	if(elementEntry.code() != DataCode.BLANK)
    					{
                    		authenticationErrorText(elementEntry.ascii().ascii());
    					}
                    }
                        break;
    				}
    			}
    		}
    		catch(OmmInvalidUsageException ommInvlaidUsageException)
    		{
    			throw ommIUExcept().message("Decoding error for " + elementName + " element. " + ommInvlaidUsageException.getMessage());
    			}
	        }
    	}
		
		private void encode(RefreshMsg refreshMsg)
		{
			if ( _elementList == null )
			{
				_elementList = EmaFactory.createElementList();
			}
			else
			{
				_elementList.clear();
			}
			
			if ( _allowSuspectDataSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, _allowSuspectData == true ? 1 : 0));
			}
			
			if ( _applicationIdSet )
			{
				_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, _applicationId));
			}
			
			if ( _applicationNameSet )
			{
				_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, _applicationName));
			}
			
			if ( _positionSet )
			{
				_elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, _position));
			}
			
			if ( _providePermissionExpressionsSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_EXP, _providePermissionExpressions == true ? 1 : 0));
			}
			
			if ( _providePermissionProfileSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_PROV_PERM_PROF, _providePermissionProfile == true ? 1 : 0));
			}
			
			if ( _singleOpenSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SINGLE_OPEN, _singleOpen == true ? 1 : 0));
			}
			
			if ( _supportBatchRequestsSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_BATCH, _supportBatchRequests));
			}
			
			if ( _supportOmmPostSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_POST, _supportOmmPost == true ? 1 : 0));
			}
			
			if ( _supportProviderDictionaryDownloadSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD, _supportProviderDictionaryDownload == true ? 1 : 0));
			}
			
			if ( _supportOptimizedPauseResumeSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_OPR, _supportOptimizedPauseResume == true ? 1 : 0));
			}
			
			if ( _supportViewRequestsSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_VIEW, _supportViewRequests == true ? 1 : 0));
			}
			
			if ( _supportStandbySet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_STANDBY, _supportStandby == true ? 1 : 0));
			}
			
			if ( _supportEnhancedSymbolListSet )
			{
				_elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_ENH_SYMBOL_LIST, _supportEnhancedSymbolList));
			}

            if ( _authenticationExtendedRespSet )
            {
                _elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_AUTHN_EXTENDED, _authenticationExtendedResp));
            }
            
            if ( _authenticationTTReissueSet )
            {
                _elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_AUTHN_TT_REISSUE, _authenticationTTReissue));
            }
            
            if ( _authenticationErrorCodeSet )
            {
                _elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_AUTHN_ERRORCODE, _authenticationErrorCode));
            }
            
            if ( _authenticationErrorTextSet )
            {
                _elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_ERRORTEXT, _authenticationErrorText));
            }
            
            refreshMsg.attrib(_elementList);

		}
		
		private OmmInvalidUsageExceptionImpl ommIUExcept()
		{    
			if (_ommInvalidUsageExceptionImpl == null)
				_ommInvalidUsageExceptionImpl = new OmmInvalidUsageExceptionImpl();
			
			return _ommInvalidUsageExceptionImpl;
		}

	}
	
   class LoginStatusImpl extends LoginImpl implements LoginStatus
    {
        private long      _authenticationErrorCode;
        private String    _authenticationErrorText;

        private boolean _authenticationErrorCodeSet;
        private boolean _authenticationErrorTextSet;
        private boolean _stateSet;
        
        private State _rsslState;
        private OmmStateImpl _state;
        private Buffer _stateText;

        LoginStatusImpl()
        {
            clear();
        }
        
        LoginStatusImpl( StatusMsg statusMsg )
        {
            clear();
            
            decode(statusMsg);
        }
        
        @Override
        public LoginStatus clear()
        {
            _changed = true;
            _authenticationErrorCodeSet = false;
            _authenticationErrorTextSet = false;
            _nameSet = false;
            _nameTypeSet = true;
            _stateSet = false;
            
            if (_stateText != null)
                _stateText.clear();
            else
                _stateText = CodecFactory.createBuffer();
            
            if (_rsslState != null)
                _rsslState.clear();
            else
                _rsslState = CodecFactory.createState();

            if (_state == null)
                _state = new OmmStateImpl();
            
            if (_elementList != null )
            {
                _elementList.clear();
            }
            
            _nameType = EmaRdm.USER_NAME;
            _domainType = EmaRdm.MMT_LOGIN;
            
            return this;
        }

        /** Sets loginRefresh with ElementList containing login request elements
        allows easy decoding of login request elements
        @return reference to this object
    */
    @Override
    public LoginStatus message( StatusMsg statusMsg )
    {
        decode(statusMsg);
        
        return this;
    }

    @Override
    public LoginStatus authenticationErrorCode(long authenticationErrorCode)
    {
        _changed = true;
        _authenticationErrorCodeSet = true;
        _authenticationErrorCode = authenticationErrorCode;
        
        return this;
    }

    @Override
    public LoginStatus authenticationErrorText(String authenticationErrorText)
    {
        _changed = true;
        _authenticationErrorTextSet = true;
        _authenticationErrorText = authenticationErrorText;
        
        return this;
    }
    
    @Override
    public LoginStatus name(String name)
    {
        _changed = true;
        _nameSet = true;
        _name = name;
        
        return this;
    }
    
   @Override
    public LoginStatus nameType(int nameType)
    {
        _changed = true;
        _nameTypeSet = true;
        _nameType = nameType;
        
        return this;
    }
   
   @Override
   public LoginStatus state(int streamState, int dataState, int statusCode, String statusText)
   {
       _changed = true;
       _stateSet = true;
       
       _rsslState.streamState(streamState);
       _rsslState.dataState(dataState);
       _rsslState.code(statusCode);
       _stateText.data(statusText);
       _rsslState.text(_stateText);
       
       return this;
       
   }
   
   public LoginStatus state(OmmState value)
   {
       _changed = true;
       _stateSet = true;
       
       _rsslState.streamState(value.streamState());
       _rsslState.dataState(value.dataState());
       _rsslState.code(value.statusCode());
       _stateText.data(value.statusText());
       _rsslState.text(_stateText);
       
       return this;
       
   }

   @Override
   public boolean hasAuthenticationErrorCode()
   {
       return _authenticationErrorCodeSet;
   }
   
    @Override
    public boolean hasAuthenticationErrorText()
    {
        return _authenticationErrorTextSet;
    }
    
    @Override
    public boolean hasName()
    {
        return _nameSet;
    }
    
    @Override
    public boolean hasNameType()
    {
        return _nameTypeSet;
    }
    
    @Override
    public boolean hasState()
    {
        return _stateSet;
    }

    @Override
    public StatusMsg message()
    {
        StatusMsg statusMsg = EmaFactory.createStatusMsg();
        
        statusMsg.domainType(_domainType);
        
        if (_stateSet)
            statusMsg.state(_rsslState.streamState(), _rsslState.dataState(), _rsslState.code(), new String(_rsslState.text().data().array()));

        if (_nameTypeSet)
            statusMsg.nameType(_nameType);
        if (_nameSet)
            statusMsg.name(_name);
        
        if (!_changed)
        {
            return statusMsg;
        }
        
        encode(statusMsg);
        
        _changed = false;
        
        return statusMsg;
    }

    @Override
    public long authenticationErrorCode()
    {
        if (!_authenticationErrorCodeSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_AUTHN_ERRORCODE + " element is not set");
        }
        
        return _authenticationErrorCode;
    }

    @Override
    public String authenticationErrorText()
    {
        if (!_authenticationErrorTextSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_AUTHN_ERRORTEXT + " element is not set");
        }
        
        return _authenticationErrorText;
    }
    
    @Override
    public String name()
    {
        if (!_nameSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_USERNAME + " element is not set");
        }
        
        return _name;
    }
    
    @Override
    public int nameType()
    {
        if (!_nameTypeSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_USERNAME_TYPE + " element is not set");
        }
        
        return _nameType;
    }
    
    @Override
    public OmmState state()
    {
        if (!_stateSet)
        {
            throw ommIUExcept().message( EmaRdm.ENAME_STATE + " element is not set");
        }

        _state.decode(_rsslState);

        return _state;

    }

    @Override
    public String toString()
    {
        StringBuilder text = new StringBuilder();
        
        if ( _authenticationErrorCodeSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_ERRORCODE + " : " + _authenticationErrorCode);
        }
        
        if ( _authenticationErrorTextSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_AUTHN_ERRORTEXT + " : " + _authenticationErrorText);
        }
        
        if ( _nameSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_USERNAME + " : " + _name);
        }
        
        if ( _nameTypeSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_USERNAME_TYPE + " : " + _nameType);
        }
        
        if ( _stateSet )
        {
            text.append("\r\n" + EmaRdm.ENAME_STATE + " : StreamState: " + _rsslState.streamState() + " DataState: " + _rsslState.dataState() + " StatusCode: " + _rsslState.code() + "StatusText: " + new String(_rsslState.text().data().array()));
        }
        
        return text.toString();
    }
    
    private void decode(StatusMsg statusMsg)
    {
        if (statusMsg.domainType() != EmaRdm.MMT_LOGIN)
        {
            throw ommIUExcept().message("Domain type must be Login.");
        }
        
        _authenticationErrorCodeSet = false;
        _authenticationErrorTextSet = false;
        _stateSet = false;
        
        if (statusMsg.hasName())
            name(statusMsg.name());
        if (statusMsg.hasNameType())
            nameType(statusMsg.nameType());
        if (statusMsg.hasState())
            state(statusMsg.state());
        
        if (statusMsg.attrib().dataType() == DataTypes.ELEMENT_LIST)
        {
            Iterator<ElementEntry> iterator = statusMsg.attrib().elementList().iterator();
            
            ElementEntry elementEntry;
            String elementName = null;
    
            try
            {
                while(iterator.hasNext())
                {
                    elementEntry = iterator.next();
                    elementName = elementEntry.name();
                    
                    switch(elementEntry.name())
                    {
                    case EmaRdm.ENAME_AUTHN_ERRORCODE:
                    {
                        authenticationErrorCode(elementEntry.uintValue());
                    }
                        break;
                    case EmaRdm.ENAME_AUTHN_ERRORTEXT:
                    {
                    	if(elementEntry.code() != DataCode.BLANK)
    					{
                    		authenticationErrorText(elementEntry.ascii().ascii());
    					}
                    }
                        break;
                    }
                }
            }
            catch(OmmInvalidUsageException ommInvlaidUsageException)
            {
                throw ommIUExcept().message("Decoding error for " + elementName + " element. " + ommInvlaidUsageException.getMessage());
            }
        }
    }
    
    private void encode(StatusMsg statusMsg)
    {
        if ( _elementList == null )
        {
            _elementList = EmaFactory.createElementList();
        }
        else
        {
            _elementList.clear();
        }

        if ( _authenticationErrorCodeSet )
        {
            _elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_AUTHN_ERRORCODE, _authenticationErrorCode));
        }
        
        if ( _authenticationErrorTextSet )
        {
            _elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_AUTHN_ERRORTEXT, _authenticationErrorText));
        }
        
        statusMsg.attrib(_elementList);

    }
    
    private OmmInvalidUsageExceptionImpl ommIUExcept()
    {    
        if (_ommInvalidUsageExceptionImpl == null)
            _ommInvalidUsageExceptionImpl = new OmmInvalidUsageExceptionImpl();
        
        return _ommInvalidUsageExceptionImpl;
    }

}