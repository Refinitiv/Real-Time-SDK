package com.refinitiv.eta.transport;

class EncryptionOptsImpl implements EncryptionOpts {
	
	private int _encryptedProtocol = ConnectionTypes.SOCKET;

	@Override
	public void encryptedProtocol(int encryptedProtocol) {
		_encryptedProtocol = encryptedProtocol;
	}

	@Override
	public int encryptedProtocol() {
		return _encryptedProtocol;
	}
	
	void copy(EncryptionOptsImpl destOpts)
	{
		destOpts.encryptedProtocol(_encryptedProtocol);
	}
	
	@Override
    public String toString()
    {
        return "EncryptionOpts" + "\n" + 
               "\t\tencryptedProtocol: " + _encryptedProtocol;
    }
}
