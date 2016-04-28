package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.transport.EncryptDecryptHelpers;

class EncryptionDecryptionSL164Impl implements EncryptDecryptHelpers
{

	@Override
	public long calculateEncryptedSize(Buffer unencryptedInput)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public long calculateEncryptedSize(TransportBuffer unencryptedInput)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int encryptBuffer(Channel channel, Buffer bufferToEncrypt, Buffer encryptedOutput, Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int decryptBuffer(Channel channel, Buffer bufferToDecrypt, Buffer decryptedOutput, Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int encryptBuffer(Channel channel, TransportBuffer bufferToEncrypt, TransportBuffer encryptedOutput, Error error)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int decryptBuffer(Channel channel, TransportBuffer bufferToDecrypt, TransportBuffer decryptedOutput, Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int encryptBuffer(Channel channel, TransportBuffer bufferToEncrypt, Buffer encryptedOutput, Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int decryptBuffer(Channel channel, TransportBuffer bufferToDecrypt, Buffer decryptedOutput, Error error)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int encryptBuffer(Channel channel, Buffer bufferToEncrypt, TransportBuffer encryptedOutput, Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int decryptBuffer(Channel channel, Buffer bufferToDecrypt, TransportBuffer decryptedOutput, Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}
	
}







