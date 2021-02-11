package com.refinitiv.eta.transport;

/**
 * Options used for configuring encryption options.
 * 
 * @see ConnectOptions
 */

public interface EncryptionOpts 
{
	 /**
     * Defines the encrypted protocol used for this connection.
     * <p>The default value of this option is {@link ConnectionTypes#SOCKET} </p>
     * 
     * @param encryptedProtocol the encrypted protocol defined in {@link ConnectionTypes}
     * 
     * @see ConnectionTypes
     */
    public void encryptedProtocol(int encryptedProtocol);

    /**
     * Type of encrypted protocol.
     * 
     * @return the encrypted protocol
     * 
     * @see ConnectionTypes
     */
    public int encryptedProtocol();
}
