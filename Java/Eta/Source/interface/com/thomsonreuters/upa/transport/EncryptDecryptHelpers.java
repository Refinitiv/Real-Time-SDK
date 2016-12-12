package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.codec.Buffer;

/**
 * With some {@link ConnectionTypes}, a key is exchanged upon successful connection 
 * establishment.  This key allows for weak encryption and decryption of
 * {@link Buffer} content, where the user determines which buffers or portions of 
 * buffers to encrypt/decrypt.
 * <p>
 * If no key was exchanged or the connection type does not support key exchange,
 * the encryption and decryption methods will return failure.
 * <p>
 * A single EncryptDecryptHelpers object can be used to encrypt or decrypt
 * buffers across multiple {@link Channel} objects,
 * as each {@link Channel} tracks it's own encryption key.
 */
public interface EncryptDecryptHelpers 
{
   
    /**
     * Calculates the number of byte needed to encrypt the passed in content.
     * 
     * @param unencryptedInput the {@link Buffer} to determine encryption length for.
     * 
     * @return the number of bytes needed to encrypt the passed in {@link Buffer}
     */
    long calculateEncryptedSize(Buffer unencryptedInput);
	
    /**
     * Calculates the number of byte needed to encrypt the passed in content.
     * 
     * @param unencryptedInput the {@link TransportBuffer} to determine encryption length for.
     * 
     * @return the number of bytes needed to encrypt the passed in {@link TransportBuffer}
     */
    long calculateEncryptedSize(TransportBuffer unencryptedInput);

    /**
     * Performs weak encryption on the buffer contents
     * 
     * @param channel the {@link Channel} the content should be encrypted for
     * @param bufferToEncrypt the {@link Buffer} housing the contents to encrypt.
     * @param encryptedOutput the {@link Buffer} to encrypt into.
     *        This must have enough capacity for encryption. See calculateEncryptedSize()
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int encryptBuffer(Channel channel, Buffer bufferToEncrypt, Buffer encryptedOutput, Error error);

    /**
     * Decrypts by reversing weak encryption on buffer contents
     * 
     * @param channel the {@link Channel} the content should be decrypted from
     * @param bufferToDecrypt the {@link Buffer} housing the contents to decrypt.
     * @param decryptedOutput the {@link Buffer} to decrypt into.
     *        This must have enough capacity for encryption.
     *        Should contain at least bufferToDecrypt.length() bytes.
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int decryptBuffer(Channel channel, Buffer bufferToDecrypt, Buffer decryptedOutput, Error error);
	
    /**
     * Performs weak encryption on the buffer contents
     * 
     * @param channel the {@link Channel} the content should be encrypted for
     * @param bufferToEncrypt the {@link TransportBuffer} housing the contents to encrypt.
     * @param encryptedOutput the {@link TransportBuffer} to encrypt into.
     *        This must have enough capacity for encryption. See calculateEncryptedSize()
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int encryptBuffer(Channel channel, TransportBuffer bufferToEncrypt, TransportBuffer encryptedOutput, Error error);

    /**
     * Decrypts by reversing weak encryption on buffer contents
     * 
     * @param channel the {@link Channel} the content should be decrypted from
     * @param bufferToDecrypt the {@link TransportBuffer} housing the contents to decrypt.
     * @param decryptedOutput the {@link TransportBuffer} to decrypt into.
     *        This must have enough capacity for encryption.  Should contain at least bufferToDecrypt.length() bytes.
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int decryptBuffer(Channel channel, TransportBuffer bufferToDecrypt, TransportBuffer decryptedOutput, Error error);
	
    /**
     * Performs weak encryption on the buffer contents
     * 
     * @param channel the {@link Channel} the content should be encrypted for
     * @param bufferToEncrypt the {@link TransportBuffer} housing the contents to encrypt.
     * @param encryptedOutput the {@link Buffer} to encrypt into.
     *        This must have enough capacity for encryption. See See calculateEncryptedSize()
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int encryptBuffer(Channel channel, TransportBuffer bufferToEncrypt, Buffer encryptedOutput, Error error);

    /**
     * Decrypts by reversing weak encryption on buffer contents
     * 
     * @param channel the {@link Channel} the content should be decrypted from
     * @param bufferToDecrypt the {@link TransportBuffer} housing the contents to decrypt.
     * @param decryptedOutput the {@link Buffer} to decrypt into.
     *        This must have enough capacity for encryption.  Should contain at least bufferToDecrypt.length() bytes.
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int decryptBuffer(Channel channel, TransportBuffer bufferToDecrypt, Buffer decryptedOutput, Error error);
	
    /**
     * Performs weak encryption on the buffer contents
     * 
     * @param channel the {@link Channel} the content should be encrypted for
     * @param bufferToEncrypt the {@link Buffer} housing the contents to encrypt.
     * @param encryptedOutput the {@link TransportBuffer} to encrypt into.
     *        This must have enough capacity for encryption. See See calculateEncryptedSize()
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int encryptBuffer(Channel channel, Buffer bufferToEncrypt, TransportBuffer encryptedOutput, Error error);

    /**
     * Decrypts by reversing weak encryption on buffer contents
     * 
     * @param channel the {@link Channel} the content should be decrypted from
     * @param bufferToDecrypt the {@link Buffer} housing the contents to decrypt.
     * @param decryptedOutput the {@link TransportBuffer} to decrypt into.
     *        This must have enough capacity for encryption.  Should contain at least bufferToDecrypt.length() bytes.
     * @param error the {@link Error}, populated in cases where failure occurs
     * 
     * @return {@link TransportReturnCodes} and {@link com.thomsonreuters.upa.codec.CodecReturnCodes}
     */
    int decryptBuffer(Channel channel, Buffer bufferToDecrypt, TransportBuffer decryptedOutput, Error error);	
    
}