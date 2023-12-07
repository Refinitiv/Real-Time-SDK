/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

/**
 * ReactorRestProxyOptions set in the {@link ReactorOptions} and
 * used for setting rest proxy options.
 */
public class ReactorRestProxyOptions
{
	private Buffer _proxyHostName = CodecFactory.createBuffer();
	private Buffer _proxyPort = CodecFactory.createBuffer();
	private Buffer _proxyUserName = CodecFactory.createBuffer();
	private Buffer _proxyPassword = CodecFactory.createBuffer();
	private Buffer _proxyDomain = CodecFactory.createBuffer(); // Needed for NTLM and Kerberos authentication protocols
	private Buffer _proxyLocalHostName = CodecFactory.createBuffer();
	private Buffer _proxyKrb5ConfigFile = CodecFactory.createBuffer();
	
	/**
     * Constructor for ReactorRestProxyOptions.
     */	
	ReactorRestProxyOptions()
	{
		clear();
	}
	
	/**
     * Clears all members of ReactorRestProxyOptions.
     */	
	public void clear()
	{
		_proxyHostName.clear();
		_proxyPort.clear();
		_proxyUserName.clear();
		_proxyPassword.clear();
		_proxyDomain.clear();
		_proxyLocalHostName.clear();
		_proxyKrb5ConfigFile.clear();
	}
	
	/**
     * Copies data of this ReactorRestProxyOptions to the passed in destination parameter.
     * 
     * @param destination ReactorRestProxyOptions to copy data into.
     * @return {@link ReactorReturnCodes#SUCCESS} if the copy succeeded, 
     * otherwise {@link ReactorReturnCodes#FAILURE}.
     */	
	public int copy(ReactorRestProxyOptions dest)
	{
		if (dest == null)
			return ReactorReturnCodes.FAILURE;
		
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyHostName.length());
        	_proxyHostName.copy(byteBuffer);
        	dest.proxyHostName().data(byteBuffer);
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyPort.length());
        	_proxyPort.copy(byteBuffer);
        	dest.proxyPort().data(byteBuffer);        
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyUserName.length());
        	_proxyUserName.copy(byteBuffer);
        	dest.proxyUserName().data(byteBuffer);  
        }
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyPassword.length());
        	_proxyPassword.copy(byteBuffer);
        	dest.proxyPassword().data(byteBuffer);  
        } 
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyDomain.length());
        	_proxyDomain.copy(byteBuffer);
        	dest.proxyDomain().data(byteBuffer);  
        } 
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyLocalHostName.length());
        	_proxyLocalHostName.copy(byteBuffer);
        	dest.proxyLocalHostName().data(byteBuffer);  
        } 
        
        {
        	ByteBuffer byteBuffer = ByteBuffer.allocate(_proxyKrb5ConfigFile.length());
        	_proxyKrb5ConfigFile.copy(byteBuffer);
        	dest.proxyKrb5ConfigFile().data(byteBuffer);  
        } 
        
		return ReactorReturnCodes.SUCCESS;
	}
	
	/**
     * Specifies the address or hostname of the HTTP proxy server.
     * 
     * @param proxyHostName the proxy hostname.
     */	
	public void proxyHostName(Buffer proxyHostName)
	{
		assert(proxyHostName != null) : "proxyHostName can not be null";
		proxyHostName().data(proxyHostName.data(), proxyHostName.position(), proxyHostName.length());
	}
	
    /**
     * Returns the address or hostname of the HTTP proxy server.
     * 
     * @return the proxy hostname.
     */		
	public Buffer proxyHostName()
	{
		return _proxyHostName;
	}
	
	/**
     * Specifies the Port Number of the HTTP proxy server.
     * 
     * @param proxyPort the proxy port.
     */	
	public void proxyPort(Buffer proxyPort)
	{
		_proxyPort = proxyPort;
	}
	
    /**
     * Gets the Port Number of the HTTP proxy server.
     * 
     * @return the proxy port
     */		
	public Buffer proxyPort()
	{
		return _proxyPort;
	}
	
	/**
     * Sets the proxy user name to authenticate.
     * 
     * @param proxyUserName the proxy user name.
     */	
	public void proxyUserName(Buffer proxyUserName)
	{
		assert(proxyUserName != null) : "proxyUserName can not be null";
		proxyUserName().data(proxyUserName.data(), proxyUserName.position(), proxyUserName.length());
	}
	
    /**
     * Gets the proxy user name to authenticate.
     * 
     * @return the proxy user name.
     */		
	public Buffer proxyUserName()
	{
		return _proxyUserName;
	}
	
	/**
     * Sets the proxy password to authenticate.
     * 
     * @param proxyPassword the proxy password.
     */	
	public void proxyPassword(Buffer proxyPassword)
	{
		assert(proxyPassword != null) : "proxyPassword can not be null";
		proxyPassword().data(proxyPassword.data(), proxyPassword.position(), proxyPassword.length());
	}
	
    /**
     * Returns the proxy password to authenticate.
     * 
     * @return the proxy password.
     */		
	public Buffer proxyPassword()
	{
		return _proxyPassword;
	}

	/**
     * Specifies the proxy domain of the user to authenticate. 
     * 
     * @param proxyDomain the proxy domain.
     */	
	public void proxyDomain(Buffer proxyDomain)
	{
		assert(proxyDomain != null) : "proxyDomain can not be null";
		proxyDomain().data(proxyDomain.data(), proxyDomain.position(), proxyDomain.length());
	}
	
    /**
     * Gets the proxy domain of the user to authenticate.
     * 
     * @return the proxy domain.
     */		
	public Buffer proxyDomain()
	{
		return _proxyDomain;
	}
	
	/**
     * Specifies the proxy local hostname. Needed for NTLM authentication protocol only.
     * 
     * @param proxyLocalHostName the proxy local hostname.
     */	
	public void proxyLocalHostName(Buffer proxyLocalHostName)
	{
		assert(proxyLocalHostName != null) : "proxyLocalHostName can not be null";
		proxyLocalHostName().data(proxyLocalHostName.data(), proxyLocalHostName.position(), proxyLocalHostName.length());
	}
	
    /**
     * Gets the proxy local hostname. Used for NTLM authentication protocol only.
     * 
     * @return the proxy local hostname.
     */		
	public Buffer proxyLocalHostName()
	{
		return _proxyLocalHostName;
	}
	
	/**
     * Specifies the complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or custom file).
     * Needed to Negotiate/Kerberos and Kerberos authentications for the proxy connection.
     * 
     * The default locations could be the following:
     * Windows: c:\winnt\krb5.ini or c:\windows\krb5.ini
     * Linux: /etc/krb5.conf
     * Other Unix: /etc/krb5/krb5.conf
     * 
     * @param proxyKrb5ConfigFile the proxy Kerberos5 Config File for the proxy connection.
     */	
	public void proxyKrb5ConfigFile(Buffer proxyKrb5ConfigFile)
	{
		assert(proxyKrb5ConfigFile != null) : "proxyKrb5ConfigFile can not be null";
		proxyKrb5ConfigFile().data(proxyKrb5ConfigFile.data(), proxyKrb5ConfigFile.position(), proxyKrb5ConfigFile.length());
	}
	
    /**
     * The complete path of the Kerberos5 configuration file (krb5.ini or krb5.conf, or custom file).
     * Needed to Negotiate/Kerberos and Kerberos authentications for the proxy connection.
     * 
     * The default locations could be the following:
     * Windows: c:\winnt\krb5.ini or c:\windows\krb5.ini
     * Linux: /etc/krb5.conf
     * Other Unix: /etc/krb5/krb5.conf
     * 
     * @return the proxy Kerberos5 Config File for the proxy connection.
     */		
	public Buffer proxyKrb5ConfigFile()
	{
		return _proxyKrb5ConfigFile;
	}
}


