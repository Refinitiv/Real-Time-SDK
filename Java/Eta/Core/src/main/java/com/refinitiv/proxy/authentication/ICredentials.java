package com.refinitiv.proxy.authentication;

public interface ICredentials
{
    /* Sets the specified credential
     * 
     * name is the name of the credential to set
     * value is the value of the credential (may be null)
     */
    void set(String name, String value);

    /* Returns the specified credential, which may be null (naturally, or if it was not set)
     * 
     * name is the name of the credential to get
     */
    String get(String name);

    /* Returns true if the specified credential is set
     * 
     * name is the name of the credential
     */
    boolean isSet(String name);
	
    Object clone() throws CloneNotSupportedException;
}
