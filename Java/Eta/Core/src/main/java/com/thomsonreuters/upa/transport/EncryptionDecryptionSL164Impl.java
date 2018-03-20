package com.thomsonreuters.upa.transport;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.security.SecureRandom;

import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.transport.EncryptDecryptHelpers;

class EncryptionDecryptionSL164Impl implements EncryptDecryptHelpers
{
    /* Encryption type constant for this encryption type */
    protected static final byte TR_SL1_64 = 1;

    private ByteBuffer _footer;
    private static final int TR_SL1_64_KEY_BIT_LEN = 64;
    private static final int TR_SL1_64_KEY_LEN = (TR_SL1_64_KEY_BIT_LEN / 8);
    private SecureRandom _randGen;
    private short[] _secretKey;
	
    {
        _randGen = new SecureRandom();
        _randGen.setSeed(System.currentTimeMillis());
        _secretKey = new short[TR_SL1_64_KEY_LEN];
        _footer = ByteBuffer.allocate(4);
        _footer.put((byte)'T');
        _footer.put((byte)'r');
        _footer.put((byte)'E');
        _footer.put((byte)'p');
        _footer.rewind();
    }

    protected long randLong()
    {
        return (_randGen.nextLong() & (long)0x7FFFFFFFFFFFFFFFL);
    }

    protected int randInt()
    {
        return _randGen.nextInt();
    }

    private long mulMod(long a, long b, long m)
    {
        long x = 0;
        long y = a % m;

        while (b > 0)
        {
            if (b % 2 == 1)
                x = (x + y) % m;
            y = (y * 2) % m;
            b /= 2;
        }
        return x % m;
    }

    protected long modPowFast(long base, long exp, long mod)
    {
        long a = base;
        long b = exp;
        long c = 1;

        while (b != 0)
        {
            while (((b & 1) == 0))
            {
                b >>= 1;
                a = mulMod(a, a, mod);
            }
            b--;
            c = mulMod(a, c, mod);
        }
        return c;
    }

    // Variant of above that uses BigInteger
    protected long fastModPow(long ibase, long iexp, long imod)
    {
        BigInteger result = BigInteger.ONE;
        BigInteger base = BigInteger.valueOf(ibase);
        BigInteger exponent = BigInteger.valueOf(iexp);
        BigInteger modulo = BigInteger.valueOf(imod);

        while (exponent.compareTo(BigInteger.ZERO) > 0)
        {
            if (exponent.testBit(0)) // then exponent is odd
                result = (result.multiply(base)).mod(modulo);
            exponent = exponent.shiftRight(1);
            base = (base.multiply(base)).mod(modulo);
        }
        return (result.mod(modulo).longValue());
    }	

    protected static final void longToByteArray(long value, short[] output)
    {
        output[7] = (short)((value >>> 56) & 0xFF);
        output[6] = (short)((value >>> 48) & 0xFF);
        output[5] = (short)((value >>> 40) & 0xFF);
        output[4] = (short)((value >>> 32) & 0xFF);
        output[3] = (short)((value >>> 24) & 0xFF);
        output[2] = (short)((value >>> 16) & 0xFF);
        output[1] = (short)((value >>> 8) & 0xFF);
        output[0] = (short)(value & 0xFF);
    }

    @Override
    public long calculateEncryptedSize(Buffer unencryptedInput)
    {
        // content ((length + sizeof(u32) + sizeof(u32) + 4 (for footer) + 7)*8)/8
        // where +7 and the 8's are for getting to double word alignment
        return (((unencryptedInput.length() + 19) / 8) * 8);
    }

    @Override
    public long calculateEncryptedSize(TransportBuffer unencryptedInput)
    {
        // content ((length + sizeof(u32) + sizeof(u32) + 4 (for footer) + 7)*8)/8
        // where +7 and the 8's are for getting to double word alignment
        return (((unencryptedInput.length() + 19) / 8) * 8);
    }
	
    @Override
    public int encryptBuffer(Channel channel, Buffer bufferToEncrypt, Buffer encryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToEncrypt != null) : "bufferToEncrypt cannot be null";
        assert (encryptedOutput != null) : "encryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        long actualLength = calculateEncryptedSize(bufferToEncrypt);
        if ((encryptedOutput.data().limit() - encryptedOutput.data().position()) < actualLength)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0020 Cannot encrypt into output buffer of size ("
                    + (encryptedOutput.data().limit() - encryptedOutput.data().position()) + "). Expected length: (" + actualLength + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if (((bufferToEncrypt.data().limit() - bufferToEncrypt.data().position()) == 0))
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted.");
            return TransportReturnCodes.FAILURE;
        }

        int startPos = encryptedOutput.data().position();
        long randomHead = (randInt() & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)randomHead);

        long mangledLength = ((bufferToEncrypt.length() ^ randomHead) & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)mangledLength);

        int curPos = bufferToEncrypt.data().position() - bufferToEncrypt.length();
        int outputPos = encryptedOutput.data().position();
        for (int k = 0; k < bufferToEncrypt.length(); k++)
        {
            encryptedOutput.data().put((outputPos + k), bufferToEncrypt.data().get(curPos + k));
        }

        encryptedOutput.data().position(encryptedOutput.data().position() + bufferToEncrypt.length());

        encryptedOutput.data().put(_footer);
        _footer.rewind();

        // go back to start to go through and encrypt it now
        encryptedOutput.data().position(startPos);
        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                byte tempVal = encryptedOutput.data().get(startPos + i + j);
                encryptedOutput.data().put((startPos + i + j), (byte)(tempVal ^ _secretKey[j]));
                tempVal = encryptedOutput.data().get(startPos + i + j);
                _secretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                _secretKey[j]++;
            }
        }

        // now put it at the end again.
        encryptedOutput.data().position((int)(startPos + actualLength));
        return ret;
    }
		
    @Override
    public int decryptBuffer(Channel channel, Buffer bufferToDecrypt, Buffer decryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToDecrypt != null) : "bufferToDecrypt cannot be null";
        assert (decryptedOutput != null) : "decryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        int length = bufferToDecrypt.data().limit() - bufferToDecrypt.data().position();
        if (length < (TR_SL1_64_KEY_LEN * 2))
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if ((length % TR_SL1_64_KEY_LEN) != 0)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        ByteBuffer messageTemp = ByteBuffer.allocate(length);
        messageTemp.put(bufferToDecrypt.data());

        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                short temp = (short)((~(messageTemp.get(i + j) - 1)) & 0xFF);
                short tempVal = (short)(messageTemp.get(i + j) & 0xFF);
                messageTemp.put((i + j), (byte)((tempVal ^ _secretKey[j]) & 0xFF));
                _secretKey[j] = temp;
            }
        }

        messageTemp.rewind();

        long randomHead = (messageTemp.getInt() & 0xFFFFFFFF);

        long encryptedLen = (messageTemp.getInt() & 0xFFFFFFFF);

        long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

        if ((tempLen < 0) || (tempLen > (length - 12)))
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1010 Content appears invalid after decryption.");
            return TransportReturnCodes.FAILURE;
        }

        if (decryptedOutput.capacity() < tempLen)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size (" + decryptedOutput.capacity() + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        int curPos = messageTemp.position();
        int outputPos = decryptedOutput.data().position();
        for (int k = 0; k < tempLen; k++)
            decryptedOutput.data().put((outputPos + k), messageTemp.get(curPos + k));

        decryptedOutput.data().limit((int)tempLen);

        return ret;
    }	

    @Override
    public int encryptBuffer(Channel channel, TransportBuffer bufferToEncrypt, TransportBuffer encryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToEncrypt != null) : "bufferToEncrypt cannot be null";
        assert (encryptedOutput != null) : "encryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        long actualLength = calculateEncryptedSize(bufferToEncrypt);
        if ((encryptedOutput.data().limit() - encryptedOutput.data().position()) < actualLength)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0020 Cannot encrypt into output buffer of size ("
                    + (encryptedOutput.data().limit() - encryptedOutput.data().position()) + "). Expected length: (" + actualLength + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        TransportBufferImpl buffer = (TransportBufferImpl)bufferToEncrypt;
        if (buffer.encodedLength() == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted.");
            return TransportReturnCodes.FAILURE;
        }

        int startPos = encryptedOutput.data().position();
        long randomHead = (randInt() & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)randomHead);

        long mangledLength = ((bufferToEncrypt.length() ^ randomHead) & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)mangledLength);

        int curPos = bufferToEncrypt.data().position() - bufferToEncrypt.length();
        int outputPos = encryptedOutput.data().position();
        for (int k = 0; k < bufferToEncrypt.length(); k++)
        {
            encryptedOutput.data().put((outputPos + k), bufferToEncrypt.data().get(curPos + k));
        }

        encryptedOutput.data().position(encryptedOutput.data().position() + bufferToEncrypt.length());

        encryptedOutput.data().put(_footer);
        _footer.rewind();

        // go back to start to go through and encrypt it now
        encryptedOutput.data().position(startPos);
        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                byte tempVal = encryptedOutput.data().get(startPos + i + j);
                encryptedOutput.data().put((startPos + i + j), (byte)(tempVal ^ _secretKey[j]));
                tempVal = encryptedOutput.data().get(startPos + i + j);
                _secretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                _secretKey[j]++;
            }
        }

        // now put it at the end again.
        encryptedOutput.data().position((int)(startPos + actualLength));
        return ret;
    }	
	
    @Override
    public int decryptBuffer(Channel channel, TransportBuffer bufferToDecrypt, TransportBuffer decryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToDecrypt != null) : "bufferToDecrypt cannot be null";
        assert (decryptedOutput != null) : "decryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        int length = bufferToDecrypt.data().limit() - bufferToDecrypt.data().position();
        if (length < (TR_SL1_64_KEY_LEN * 2))
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if ((length % TR_SL1_64_KEY_LEN) != 0)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        ByteBuffer messageTemp = ByteBuffer.allocate(length);
        messageTemp.put(bufferToDecrypt.data());

        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                short temp = (short)((~(messageTemp.get(i + j) - 1)) & 0xFF);
                short tempVal = (short)(messageTemp.get(i + j) & 0xFF);
                messageTemp.put((i + j), (byte)((tempVal ^ _secretKey[j]) & 0xFF));
                _secretKey[j] = temp;
            }
        }

        messageTemp.rewind();

        long randomHead = (messageTemp.getInt() & 0xFFFFFFFF);

        long encryptedLen = (messageTemp.getInt() & 0xFFFFFFFF);

        long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

        if ((tempLen < 0) || (tempLen > (length - 12)))
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1010 Content appears invalid after decryption.");
            return TransportReturnCodes.FAILURE;
        }

        if (decryptedOutput.capacity() < tempLen)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size (" + decryptedOutput.capacity() + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        int curPos = messageTemp.position();
        int outputPos = decryptedOutput.data().position();
        for (int k = 0; k < tempLen; k++)
            decryptedOutput.data().put((outputPos + k), messageTemp.get(curPos + k));

        decryptedOutput.data().limit((int)tempLen);

        return ret;
    }

    @Override
    public int encryptBuffer(Channel channel, TransportBuffer bufferToEncrypt, Buffer encryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToEncrypt != null) : "bufferToEncrypt cannot be null";
        assert (encryptedOutput != null) : "encryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        long actualLength = calculateEncryptedSize(bufferToEncrypt);
        if ((encryptedOutput.data().limit() - encryptedOutput.data().position()) < actualLength)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0020 Cannot encrypt into output buffer of size ("
                    + (encryptedOutput.data().limit() - encryptedOutput.data().position()) + "). Expected length: (" + actualLength + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        TransportBufferImpl buffer = (TransportBufferImpl)bufferToEncrypt;
        if (buffer.encodedLength() == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted.");
            return TransportReturnCodes.FAILURE;
        }

        int startPos = encryptedOutput.data().position();
        long randomHead = (randInt() & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)randomHead);

        long mangledLength = ((bufferToEncrypt.length() ^ randomHead) & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)mangledLength);

        int curPos = bufferToEncrypt.data().position() - bufferToEncrypt.length();
        int outputPos = encryptedOutput.data().position();
        for (int k = 0; k < bufferToEncrypt.length(); k++)
        {
            encryptedOutput.data().put((outputPos + k), bufferToEncrypt.data().get(curPos + k));
        }

        encryptedOutput.data().position(encryptedOutput.data().position() + bufferToEncrypt.length());

        encryptedOutput.data().put(_footer);
        _footer.rewind();

        // go back to start to go through and encrypt it now
        encryptedOutput.data().position(startPos);
        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                byte tempVal = encryptedOutput.data().get(startPos + i + j);
                encryptedOutput.data().put((startPos + i + j), (byte)(tempVal ^ _secretKey[j]));
                tempVal = encryptedOutput.data().get(startPos + i + j);
                _secretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                _secretKey[j]++;
            }
        }

        // now put it at the end again.
        encryptedOutput.data().position((int)(startPos + actualLength));
        return ret;
    }	

    @Override
    public int decryptBuffer(Channel channel, TransportBuffer bufferToDecrypt, Buffer decryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToDecrypt != null) : "bufferToDecrypt cannot be null";
        assert (decryptedOutput != null) : "decryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        int length = bufferToDecrypt.data().limit() - bufferToDecrypt.data().position();
        if (length < (TR_SL1_64_KEY_LEN * 2))
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if ((length % TR_SL1_64_KEY_LEN) != 0)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        ByteBuffer messageTemp = ByteBuffer.allocate(length);
        messageTemp.put(bufferToDecrypt.data());

        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                short temp = (short)((~(messageTemp.get(i + j) - 1)) & 0xFF);
                short tempVal = (short)(messageTemp.get(i + j) & 0xFF);
                messageTemp.put((i + j), (byte)((tempVal ^ _secretKey[j]) & 0xFF));
                _secretKey[j] = temp;
            }
        }

        messageTemp.rewind();

        long randomHead = (messageTemp.getInt() & 0xFFFFFFFF);

        long encryptedLen = (messageTemp.getInt() & 0xFFFFFFFF);

        long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

        if ((tempLen < 0) || (tempLen > (length - 12)))
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1010 Content appears invalid after decryption.");
            return TransportReturnCodes.FAILURE;
        }

        if (decryptedOutput.capacity() < tempLen)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size (" + decryptedOutput.capacity() + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        int curPos = messageTemp.position();
        int outputPos = decryptedOutput.data().position();
        for (int k = 0; k < tempLen; k++)
            decryptedOutput.data().put((outputPos + k), messageTemp.get(curPos + k));

        decryptedOutput.data().limit((int)tempLen);

        return ret;
    }	

    @Override
    public int encryptBuffer(Channel channel, Buffer bufferToEncrypt, TransportBuffer encryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToEncrypt != null) : "bufferToEncrypt cannot be null";
        assert (encryptedOutput != null) : "encryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        long actualLength = calculateEncryptedSize(bufferToEncrypt);
        if ((encryptedOutput.data().limit() - encryptedOutput.data().position()) < actualLength)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0020 Cannot encrypt into output buffer of size ("
                    + (encryptedOutput.data().limit() - encryptedOutput.data().position()) + "). Expected length: (" + actualLength + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if (((bufferToEncrypt.data().limit() - bufferToEncrypt.data().position()) == 0))
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("encryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted.");
            return TransportReturnCodes.FAILURE;
        }

        int startPos = encryptedOutput.data().position();
        long randomHead = (randInt() & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)randomHead);

        long mangledLength = ((bufferToEncrypt.length() ^ randomHead) & 0xFFFFFFFF);
        encryptedOutput.data().putInt((int)mangledLength);

        int curPos = bufferToEncrypt.data().position() - bufferToEncrypt.length();
        int outputPos = encryptedOutput.data().position();
        for (int k = 0; k < bufferToEncrypt.length(); k++)
        {
            encryptedOutput.data().put((outputPos + k), bufferToEncrypt.data().get(curPos + k));
        }

        encryptedOutput.data().position(encryptedOutput.data().position() + bufferToEncrypt.length());

        encryptedOutput.data().put(_footer);
        _footer.rewind();

        // go back to start to go through and encrypt it now
        encryptedOutput.data().position(startPos);
        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                byte tempVal = encryptedOutput.data().get(startPos + i + j);
                encryptedOutput.data().put((startPos + i + j), (byte)(tempVal ^ _secretKey[j]));
                tempVal = encryptedOutput.data().get(startPos + i + j);
                _secretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                _secretKey[j]++;
            }
        }

        // now put it at the end again.
        encryptedOutput.data().position((int)(startPos + actualLength));
        return ret;
    }	

    @Override
    public int decryptBuffer(Channel channel, Buffer bufferToDecrypt, TransportBuffer decryptedOutput, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        long shared_key = 0;

        // null checks, ensure key is present
        assert (channel != null) : "channel cannot be null";
        assert (bufferToDecrypt != null) : "bufferToDecrypt cannot be null";
        assert (decryptedOutput != null) : "decryptedOutput cannot be null";
        assert (error != null) : "error cannot be null";

        switch (channel.connectionType())
        {
            case ConnectionTypes.SEQUENCED_MCAST:
                // if we ever support this, we will need to add this here
            case ConnectionTypes.RELIABLE_MCAST:
                // for reliable mcast, we can't get to the shared_key since its in the channel impl and there is no interface.
                shared_key = 0;
                break;
            default: // all other transport types
                shared_key = ((RsslSocketChannel)channel)._shared_key;
        }

        if (shared_key == 0)
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange.");
            return TransportReturnCodes.FAILURE;
        }

        int length = bufferToDecrypt.data().limit() - bufferToDecrypt.data().position();
        if (length < (TR_SL1_64_KEY_LEN * 2))
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        if ((length % TR_SL1_64_KEY_LEN) != 0)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0019 encryptedInput length of (" + length + ") is not long enough for an encrypted buffer.");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        ByteBuffer messageTemp = ByteBuffer.allocate(length);
        messageTemp.put(bufferToDecrypt.data());

        longToByteArray(shared_key, _secretKey);

        for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
        {
            for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
            {
                short temp = (short)((~(messageTemp.get(i + j) - 1)) & 0xFF);
                short tempVal = (short)(messageTemp.get(i + j) & 0xFF);
                messageTemp.put((i + j), (byte)((tempVal ^ _secretKey[j]) & 0xFF));
                _secretKey[j] = temp;
            }
        }

        messageTemp.rewind();

        long randomHead = (messageTemp.getInt() & 0xFFFFFFFF);

        long encryptedLen = (messageTemp.getInt() & 0xFFFFFFFF);

        long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

        if ((tempLen < 0) || (tempLen > (length - 12)))
        {
            error.channel(channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("decryptBuffer() Error: 1010 Content appears invalid after decryption.");
            return TransportReturnCodes.FAILURE;
        }

        if (decryptedOutput.capacity() < tempLen)
        {
            error.channel(channel);
            error.errorId(CodecReturnCodes.BUFFER_TOO_SMALL);
            error.sysError(0);
            error.text("decryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size (" + decryptedOutput.capacity() + ").");
            return CodecReturnCodes.BUFFER_TOO_SMALL;
        }

        int curPos = messageTemp.position();
        int outputPos = decryptedOutput.data().position();
        for (int k = 0; k < tempLen; k++)
            decryptedOutput.data().put((outputPos + k), messageTemp.get(curPos + k));

        decryptedOutput.data().limit((int)tempLen);

        return ret;
    }
	
}







