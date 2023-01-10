/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using System.Numerics;
using System.Security.Cryptography;
using LSEG.Eta.Transports;
using System.Diagnostics;
using LSEG.Eta.Internal.Interfaces;

namespace LSEG.Eta.Transports
{
    /// <summary>
    /// With some <see cref="ConnectionType"/>, a key is exchanged upon successful connection
    /// establishment.  This key allows for weak encryption and decryption of <see cref="Buffer"/>
    /// content, where the user determines which buffers or portions of buffers to encrypt/decrypt.
    /// <para>
    /// If no key was exchanged or the connection type does not support key exchange, the encryption
    /// and decryption methods will return failure.
    /// </para>
    /// <para>
    /// A single EncryptDecryptHelpers object can be used to encrypt or decrypt buffers across multiple
    /// <see cref="IChannel"/> objects, as each <see cref="IChannel"/> tracks it's own encryption key.
    /// </para>
    /// </summary>
    public class EncryptDecryptHelpers
    {
        /* Encryption type constant for this encryption type */
        internal static readonly byte TR_SL1_64 = 1;

        private ByteBuffer m_Footer;
        private static readonly int TR_SL1_64_KEY_BIT_LEN = 64;
        private static readonly int TR_SL1_64_KEY_LEN = (TR_SL1_64_KEY_BIT_LEN / 8);
        private readonly RandomNumberGenerator m_randGen = RandomNumberGenerator.Create();
        private short[] m_SecretKey;
	
        /// <summary>
        /// The default constructor
        /// </summary>
        public EncryptDecryptHelpers()
        {
            m_SecretKey = new short[TR_SL1_64_KEY_LEN];
            m_Footer = new ByteBuffer(4);
            m_Footer.Write((byte)'T');
            m_Footer.Write((byte)'r');
            m_Footer.Write((byte)'E');
            m_Footer.Write((byte)'p');
            m_Footer.Rewind();
         }

        internal long RandLong()
        {
            byte[] byteArray = new byte[8];

            m_randGen.GetBytes(byteArray);

            long ranValue = System.BitConverter.ToInt32(byteArray, 0);

            return (ranValue & 0x7FFFFFFFFFFFFFFFL);
        }

        /// <returns></returns>
        internal int RandInt()
        {
            byte[] byteArray = new byte[4];

            m_randGen.GetBytes(byteArray);

            return System.BitConverter.ToInt32(byteArray, 0);
        }

        internal long MulMod(long a, long b, long m)
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

        internal long ModPowFast(long baseValue, long exp, long mod)
        {
            long a = baseValue;
            long b = exp;
            long c = 1;

            while (b != 0)
            {
                while (((b & 1) == 0))
                {
                    b >>= 1;
                    a = MulMod(a, a, mod);
                }
                b--;
                c = MulMod(a, c, mod);
            }
            return c;
        }

        private bool TestBit(BigInteger bigInt, int n)
        {
            long value = (long)bigInt;

            return (value | (long)System.Math.Pow(2, n)) == value;
        }

        // Variant of above that uses BigInteger
        internal long FastModPow(long ibase, long iexp, long imod)
        {
            BigInteger result = BigInteger.One;
            BigInteger baseValue = new BigInteger(ibase);
            BigInteger exponent = new BigInteger(iexp);
            BigInteger modulo = new BigInteger(imod);

            while(exponent.CompareTo(BigInteger.Zero) > 0)
            {
                if (TestBit(exponent,0)) // then exponent is odd
                {
                    result = BigInteger.Multiply(result, baseValue) % modulo;
                }

                exponent = exponent >> 1;
                baseValue = BigInteger.Multiply(baseValue, baseValue) % modulo;
            }

            return (long)(result % modulo);
        }

        private static void LongToByteArray(ulong value, short[] output)
        {
            output[7] = (short)((value >> 56) & 0xFF);
            output[6] = (short)((value >> 48) & 0xFF);
            output[5] = (short)((value >> 40) & 0xFF);
            output[4] = (short)((value >> 32) & 0xFF);
            output[3] = (short)((value >> 24) & 0xFF);
            output[2] = (short)((value >> 16) & 0xFF);
            output[1] = (short)((value >> 8) & 0xFF);
            output[0] = (short)(value & 0xFF);
        }

        /// <summary>
        /// Calculates the number of byte needed to encrypt the passed in content.
        /// </summary>
        /// <param name="unencryptedInput">the <see cref="Buffer"/> to determine encryption length for.</param>
        /// <returns>the number of bytes needed to encrypt the passed in <see cref="Buffer"/></returns>
        public long CalculateEncryptedSize(Buffer unencryptedInput)
        {
            return (((unencryptedInput.Length + 19) / 8) * 8);
        }

        /// <summary>
        /// Calculates the number of byte needed to encrypt the passed in content.
        /// </summary>
        /// <param name="unencryptedInput">the <see cref="ITransportBuffer"/>} to determine encryption length for.</param>
        /// <returns>the number of bytes needed to encrypt the passed in <see cref="ITransportBuffer"/></returns>
        public long CalculateEncryptedSize(ITransportBuffer unencryptedInput)
        {
            return (((unencryptedInput.Length() + 19) / 8) * 8);
        }

        /// <summary>
        /// Performs weak encryption on the buffer contents
        /// </summary>
        /// <param name="channel">the <see cref="IChannel"/> the content should be encrypted for</param>
        /// <param name="bufferToEncrypt"><see cref="Buffer"/> housing the contents to encrypt.</param>
        /// <param name="encryptedOutput">the <see cref="Buffer"/> to encrypt into.
        /// This must have enough capacity for encryption. See <see cref="CalculateEncryptedSize(Buffer)"/>.
        /// </param>
        /// <param name="error">the <see cref="CodecError"/>, populated in cases where failure occurs</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncryptBuffer(IChannel channel, Buffer bufferToEncrypt, Buffer encryptedOutput, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            ulong shared_key = 0;

            // null checks, ensure key is present
            Debug.Assert(channel != null,"channel cannot be null");
            Debug.Assert(bufferToEncrypt != null,"bufferToEncrypt cannot be null");
            Debug.Assert(encryptedOutput != null,"encryptedOutput cannot be null");

            switch (channel.ConnectionType)
            {
                default: // all other transport types
                    shared_key = (ulong)((ChannelBase)channel).Shared_Key;
                    break;
            }

            if (shared_key == 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "EncryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange."
                };

                return CodecReturnCode.FAILURE;
            }

            long actualLength = CalculateEncryptedSize(bufferToEncrypt);
            if ((encryptedOutput.Data().Limit - encryptedOutput.Data().Position) < actualLength)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"EncryptBuffer() Error: 0020 Cannot encrypt into output buffer of size " +
                    $"({encryptedOutput.Data().Limit - encryptedOutput.Data().Position}). Expected length: ({actualLength})"
                };

                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if (((bufferToEncrypt.Data().Limit - bufferToEncrypt.Data().ReadPosition) == 0))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = $"EncryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted."
                };

                return CodecReturnCode.FAILURE;
            }

            int startPos = encryptedOutput.Data().Position;
            long randomHead = RandInt() & 0xFFFFFFFF;
            encryptedOutput.Data().Write((int)randomHead);

            long mangledLength = (bufferToEncrypt.Length ^ randomHead) & 0xFFFFFFFF;
            encryptedOutput.Data().Write((int)mangledLength);

            int curPos = bufferToEncrypt.Data().Position - bufferToEncrypt.Length;
            int outputPos = encryptedOutput.Data().Position;
            for (int k = 0; k < bufferToEncrypt.GetLength(); k++)
            {
                encryptedOutput.Data().WriteAt(outputPos + k, (byte)bufferToEncrypt.Data().ReadByteAt(curPos + k));
            }

            encryptedOutput.Data().WritePosition = encryptedOutput.Data().Position + bufferToEncrypt.GetLength();

            encryptedOutput.Data().Put(m_Footer);
            m_Footer.Rewind();

            // go back to start to go through and encrypt it now
            encryptedOutput.Data().WritePosition = startPos;
            LongToByteArray(shared_key, m_SecretKey);

            for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
            {
                for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
                {
                    byte tempVal = (byte)encryptedOutput.Data().ReadByteAt(startPos + i + j);
                    encryptedOutput.Data().WriteAt((startPos + i + j), (byte)(tempVal ^ m_SecretKey[j]));
                    tempVal = (byte)encryptedOutput.Data().ReadByteAt(startPos + i + j);
                    m_SecretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                    m_SecretKey[j]++;
                }
            }

            // now put it at the end again.
            encryptedOutput.Data().WritePosition = ((int)(startPos + actualLength));

            return ret;
        }

        /// <summary>
        /// Decrypts by reversing weak encryption on buffer contents
        /// </summary>
        /// <param name="channel">the <see cref="IChannel"/> the content should be decrypted from</param>
        /// <param name="bufferToDecrypt">the <see cref="Buffer"/> housing the contents to decrypt.</param>
        /// <param name="decryptedOutput">the <see cref="Buffer"/> to decrypt into.
        /// This must have enough capacity for encryption. Should contain at least BufferToDecrypt.Length bytes.
        /// </param>
        /// <param name="error">the <see cref="CodecError"/>, populated in cases where failure occurs</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode DecryptBuffer(IChannel channel, Buffer bufferToDecrypt, Buffer decryptedOutput, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            ulong shared_key = 0;

            // null checks, ensure key is present
            Debug.Assert(channel != null, "channel cannot be null");
            Debug.Assert(bufferToDecrypt != null, "bufferToDecrypt cannot be null");
            Debug.Assert(decryptedOutput != null, "decryptedOutput cannot be null");

            switch (channel.ConnectionType)
            {
                default: // all other transport types
                    shared_key = (ulong)((ChannelBase)channel).Shared_Key;
                    break;
            }

            if (shared_key == 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "DecryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange."
                };

                return CodecReturnCode.FAILURE;
            }

            int length = bufferToDecrypt.Data().Limit - bufferToDecrypt.Data().ReadPosition;
            if (length < (TR_SL1_64_KEY_LEN * 2))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0019 encryptedInput length of ({length}) is not long enough for an encrypted buffer."
                };

                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if ((length % TR_SL1_64_KEY_LEN) != 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0019 encryptedInput length of ({length}) is not long enough for an encrypted buffer."
                };
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            ByteBuffer messageTemp = new ByteBuffer(length);
            messageTemp.Put(bufferToDecrypt.Data());

            LongToByteArray(shared_key, m_SecretKey);

            for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
            {
                for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
                {
                    short temp = (short)((~(messageTemp.ReadByteAt(i + j) - 1)) & 0xFF);
                    short tempVal = (short)(messageTemp.ReadByteAt(i + j) & 0xFF);
                    messageTemp.WriteAt((i + j), (byte)((tempVal ^ m_SecretKey[j]) & 0xFF));
                    m_SecretKey[j] = temp;
                }
            }

            messageTemp.Rewind();

            long randomHead = (messageTemp.ReadInt() & 0xFFFFFFFF);

            long encryptedLen = (messageTemp.ReadInt() & 0xFFFFFFFF);

            long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

            if ((tempLen < 0) || (tempLen > (length - 12)))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "DecryptBuffer() Error: 1010 Content appears invalid after decryption."
                };
                return CodecReturnCode.FAILURE;
            }

            if (decryptedOutput.Capacity < tempLen)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size ({ decryptedOutput.Capacity})."
                };
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            int curPos = messageTemp.ReadPosition;
            int outputPos = decryptedOutput.Data().Position;
            for (int k = 0; k < tempLen; k++)
                decryptedOutput.Data().WriteAt((outputPos + k), (byte)messageTemp.ReadByteAt(curPos + k));

            decryptedOutput.Data().WritePosition = (int)tempLen;

            return ret;
        }

        /// <summary>
        /// Performs weak encryption on the buffer contents
        /// </summary>
        /// <param name="channel">the <see cref="IChannel"/> the content should be encrypted for</param>
        /// <param name="bufferToEncrypt"><see cref="ITransportBuffer"/> housing the contents to encrypt.</param>
        /// <param name="encryptedOutput">the <see cref="ITransportBuffer"/> to encrypt into.
        /// This must have enough capacity for encryption. See <see cref="CalculateEncryptedSize(Buffer)"/>.
        /// </param>
        /// <param name="error">the <see cref="CodecError"/>, populated in cases where failure occurs</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncryptBuffer(IChannel channel, ITransportBuffer bufferToEncrypt, ITransportBuffer encryptedOutput, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            ulong shared_key = 0;

            // null checks, ensure key is present
            Debug.Assert(channel != null, "channel cannot be null");
            Debug.Assert(bufferToEncrypt != null, "bufferToEncrypt cannot be null");
            Debug.Assert(encryptedOutput != null, "encryptedOutput cannot be null");

            switch (channel.ConnectionType)
            {
                default: // all other transport types
                    shared_key = (ulong)((ChannelBase)channel).Shared_Key;
                    break;
            }

            if (shared_key == 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "EncryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange."
                };

                return CodecReturnCode.FAILURE;
            }

            long actualLength = CalculateEncryptedSize(bufferToEncrypt);
            if ((encryptedOutput.Data.Limit - encryptedOutput.Data.Position) < actualLength)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"EncryptBuffer() Error: 0020 Cannot encrypt into output buffer of size " +
                    $"({encryptedOutput.Data.Limit - encryptedOutput.Data.Position}). Expected length: ({actualLength})"
                };

                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if (((bufferToEncrypt.Data.Limit - bufferToEncrypt.Data.ReadPosition) == 0))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = $"EncryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted."
                };

                return CodecReturnCode.FAILURE;
            }

            int startPos = encryptedOutput.Data.Position;
            long randomHead = (RandInt() & 0xFFFFFFFF);
            encryptedOutput.Data.Write((int)randomHead);

            long mangledLength = ((bufferToEncrypt.Length() ^ randomHead) & 0xFFFFFFFF);
            encryptedOutput.Data.Write((int)mangledLength);

            int curPos = bufferToEncrypt.GetDataStartPosition();
            int outputPos = encryptedOutput.Data.Position;
            for (int k = 0; k < bufferToEncrypt.Length(); k++)
            {
                encryptedOutput.Data.WriteAt((outputPos + k), (byte)bufferToEncrypt.Data.ReadByteAt(curPos + k));
            }

            encryptedOutput.Data.WritePosition = encryptedOutput.Data.Position + bufferToEncrypt.Length();

            encryptedOutput.Data.Put(m_Footer);
            m_Footer.Rewind();

            // go back to start to go through and encrypt it now
            encryptedOutput.Data.WritePosition = startPos;
            LongToByteArray(shared_key, m_SecretKey);

            for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
            {
                for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
                {
                    byte tempVal = (byte)encryptedOutput.Data.ReadByteAt(startPos + i + j);
                    encryptedOutput.Data.WriteAt((startPos + i + j), (byte)(tempVal ^ m_SecretKey[j]));
                    tempVal = (byte)encryptedOutput.Data.ReadByteAt(startPos + i + j);
                    m_SecretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                    m_SecretKey[j]++;
                }
            }

            // now put it at the end again.
            encryptedOutput.Data.WritePosition = ((int)(startPos + actualLength));

            return ret;
        }

        /// <summary>
        /// Decrypts by reversing weak encryption on buffer contents
        /// </summary>
        /// <param name="channel">the <see cref="IChannel"/> the content should be decrypted from</param>
        /// <param name="bufferToDecrypt">the <see cref="ITransportBuffer"/> housing the contents to decrypt.</param>
        /// <param name="decryptedOutput">the <see cref="ITransportBuffer"/> to decrypt into.
        /// This must have enough capacity for encryption. Should contain at least BufferToDecrypt.Length bytes.
        /// </param>
        /// <param name="error">the <see cref="CodecError"/>, populated in cases where failure occurs</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode DecryptBuffer(IChannel channel, ITransportBuffer bufferToDecrypt, ITransportBuffer decryptedOutput, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            ulong shared_key = 0;

            // null checks, ensure key is present
            Debug.Assert(channel != null, "channel cannot be null");
            Debug.Assert(bufferToDecrypt != null, "bufferToDecrypt cannot be null");
            Debug.Assert(decryptedOutput != null, "decryptedOutput cannot be null");

            switch (channel.ConnectionType)
            {
                default: // all other transport types
                    shared_key = (ulong)((ChannelBase)channel).Shared_Key;
                    break;
            }

            if (shared_key == 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "DecryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange."
                };

                return CodecReturnCode.FAILURE;
            }

            int length = bufferToDecrypt.Data.Limit - bufferToDecrypt.Data.ReadPosition;
            if (length < (TR_SL1_64_KEY_LEN * 2))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0019 encryptedInput length of ({length}) is not long enough for an encrypted buffer."
                };

                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if ((length % TR_SL1_64_KEY_LEN) != 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0019 encryptedInput length of ({length}) is not long enough for an encrypted buffer."
                };
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            ByteBuffer messageTemp = new ByteBuffer(length);
            messageTemp.Put(bufferToDecrypt.Data);

            LongToByteArray(shared_key, m_SecretKey);

            for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
            {
                for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
                {
                    short temp = (short)((~(messageTemp.ReadByteAt(i + j) - 1)) & 0xFF);
                    short tempVal = (short)(messageTemp.ReadByteAt(i + j) & 0xFF);
                    messageTemp.WriteAt((i + j), (byte)((tempVal ^ m_SecretKey[j]) & 0xFF));
                    m_SecretKey[j] = temp;
                }
            }

            messageTemp.Rewind();

            long randomHead = (messageTemp.ReadInt() & 0xFFFFFFFF);

            long encryptedLen = (messageTemp.ReadInt() & 0xFFFFFFFF);

            long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

            if ((tempLen < 0) || (tempLen > (length - 12)))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "DecryptBuffer() Error: 1010 Content appears invalid after decryption."
                };
                return CodecReturnCode.FAILURE;
            }

            if (decryptedOutput.Capacity() < tempLen)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size ({ decryptedOutput.Capacity()})."
                };
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            int curPos = messageTemp.ReadPosition;
            int outputPos = decryptedOutput.Data.Position;
            for (int k = 0; k < tempLen; k++)
                decryptedOutput.Data.WriteAt((outputPos + k), (byte)messageTemp.ReadByteAt(curPos + k));

            decryptedOutput.Data.WritePosition = (int)tempLen;

            return ret;
        }

        /// <summary>
        /// Performs weak encryption on the buffer contents
        /// </summary>
        /// <param name="channel">the <see cref="IChannel"/> the content should be encrypted for</param>
        /// <param name="bufferToEncrypt"><see cref="Buffer"/> housing the contents to encrypt.</param>
        /// <param name="encryptedOutput">the <see cref="ITransportBuffer"/> to encrypt into.
        /// This must have enough capacity for encryption. See <see cref="CalculateEncryptedSize(Buffer)"/>.
        /// </param>
        /// <param name="error">the <see cref="CodecError"/>, populated in cases where failure occurs</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode EncryptBuffer(IChannel channel, Buffer bufferToEncrypt, ITransportBuffer encryptedOutput, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            ulong shared_key = 0;

            // null checks, ensure key is present
            Debug.Assert(channel != null, "channel cannot be null");
            Debug.Assert(bufferToEncrypt != null, "bufferToEncrypt cannot be null");
            Debug.Assert(encryptedOutput != null, "encryptedOutput cannot be null");

            switch (channel.ConnectionType)
            {
                default: // all other transport types
                    shared_key = (ulong)((ChannelBase)channel).Shared_Key;
                    break;
            }

            if (shared_key == 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "EncryptBuffer() Error: 1005 No encryption key present, connection does not support key exchange."
                };

                return CodecReturnCode.FAILURE;
            }

            long actualLength = CalculateEncryptedSize(bufferToEncrypt);
            if ((encryptedOutput.Data.Limit - encryptedOutput.Data.Position) < actualLength)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"EncryptBuffer() Error: 0020 Cannot encrypt into output buffer of size " +
                    $"({encryptedOutput.Data.Limit - encryptedOutput.Data.Position}). Expected length: ({actualLength})"
                };

                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if (((bufferToEncrypt.Data().Limit - bufferToEncrypt.Data().ReadPosition) == 0))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = $"EncryptBuffer() Error: 0009 Buffer of length zero cannot be encrypted."
                };

                return CodecReturnCode.FAILURE;
            }

            int startPos = encryptedOutput.Data.Position;
            long randomHead = (RandInt() & 0xFFFFFFFF);
            encryptedOutput.Data.Write((int)randomHead);

            long mangledLength = ((bufferToEncrypt.Length ^ randomHead) & 0xFFFFFFFF);
            encryptedOutput.Data.Write((int)mangledLength);

            int curPos = bufferToEncrypt.Data().Position - bufferToEncrypt.Length;
            int outputPos = encryptedOutput.Data.Position;
            for (int k = 0; k < bufferToEncrypt.GetLength(); k++)
            {
                encryptedOutput.Data.WriteAt(outputPos + k, (byte)bufferToEncrypt.Data().ReadByteAt(curPos + k));
            }

            encryptedOutput.Data.WritePosition = encryptedOutput.Data.Position + bufferToEncrypt.GetLength();

            encryptedOutput.Data.Put(m_Footer);
            m_Footer.Rewind();

            // go back to start to go through and encrypt it now
            encryptedOutput.Data.WritePosition = startPos;
            LongToByteArray(shared_key, m_SecretKey);

            for (int i = 0; i < actualLength; i += TR_SL1_64_KEY_LEN)
            {
                for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
                {
                    byte tempVal = (byte)encryptedOutput.Data.ReadByteAt(startPos + i + j);
                    encryptedOutput.Data.WriteAt((startPos + i + j), (byte)(tempVal ^ m_SecretKey[j]));
                    tempVal = (byte)encryptedOutput.Data.ReadByteAt(startPos + i + j);
                    m_SecretKey[j] = (short)((~tempVal) & 0xFF); // For CBC encryption with twist
                    m_SecretKey[j]++;
                }
            }

            // now put it at the end again.
            encryptedOutput.Data.WritePosition = ((int)(startPos + actualLength));

            return ret;
        }

        /// <summary>
        /// Decrypts by reversing weak encryption on buffer contents
        /// </summary>
        /// <param name="channel">the <see cref="IChannel"/> the content should be decrypted from</param>
        /// <param name="bufferToDecrypt">the <see cref="ITransportBuffer"/> housing the contents to decrypt.</param>
        /// <param name="decryptedOutput">the <see cref="Buffer"/> to decrypt into.
        /// This must have enough capacity for encryption. Should contain at least BufferToDecrypt.Length bytes.
        /// </param>
        /// <param name="error">the <see cref="CodecError"/>, populated in cases where failure occurs</param>
        /// <returns><see cref="CodecReturnCode"/></returns>
        public CodecReturnCode DecryptBuffer(IChannel channel, ITransportBuffer bufferToDecrypt, Buffer decryptedOutput, out CodecError error)
        {
            error = null;
            CodecReturnCode ret = CodecReturnCode.SUCCESS;
            ulong shared_key = 0;

            // null checks, ensure key is present
            Debug.Assert(channel != null, "channel cannot be null");
            Debug.Assert(bufferToDecrypt != null, "bufferToDecrypt cannot be null");
            Debug.Assert(decryptedOutput != null, "decryptedOutput cannot be null");

            switch (channel.ConnectionType)
            {
                default: // all other transport types
                    shared_key = (ulong)((ChannelBase)channel).Shared_Key;
                    break;
            }

            if (shared_key == 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "DecryptBuffer() Error: 1005 No decryption key present, connection does not support key exchange."
                };

                return CodecReturnCode.FAILURE;
            }

            int length = bufferToDecrypt.Data.WritePosition - bufferToDecrypt.Data.ReadPosition;
            if (length < (TR_SL1_64_KEY_LEN * 2))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0019 encryptedInput length of ({length}) is not long enough for an encrypted buffer."
                };

                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            if ((length % TR_SL1_64_KEY_LEN) != 0)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0019 encryptedInput length of ({length}) is not long enough for an encrypted buffer."
                };
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            ByteBuffer messageTemp = new ByteBuffer(length);
            messageTemp.Put(bufferToDecrypt.Data);

            LongToByteArray(shared_key, m_SecretKey);

            for (int i = 0; i < length; i += TR_SL1_64_KEY_LEN)
            {
                for (int j = 0; j < TR_SL1_64_KEY_LEN; j++)
                {
                    short temp = (short)((~(messageTemp.ReadByteAt(i + j) - 1)) & 0xFF);
                    short tempVal = (short)(messageTemp.ReadByteAt(i + j) & 0xFF);
                    messageTemp.WriteAt((i + j), (byte)((tempVal ^ m_SecretKey[j]) & 0xFF));
                    m_SecretKey[j] = temp;
                }
            }

            messageTemp.Rewind();

            long randomHead = (messageTemp.ReadInt() & 0xFFFFFFFF);

            long encryptedLen = (messageTemp.ReadInt() & 0xFFFFFFFF);

            long tempLen = (((int)encryptedLen ^ (int)randomHead) & 0xFFFFFFFF);

            if ((tempLen < 0) || (tempLen > (length - 12)))
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.FAILURE,
                    Text = "DecryptBuffer() Error: 1010 Content appears invalid after decryption."
                };
                return CodecReturnCode.FAILURE;
            }

            if (decryptedOutput.Capacity < tempLen)
            {
                error = new CodecError
                {
                    ErrorId = CodecReturnCode.BUFFER_TOO_SMALL,
                    Text = $"DecryptBuffer() Error: 0020 Cannot fit decrypted output into output buffer of size ({ decryptedOutput.Capacity})."
                };
                return CodecReturnCode.BUFFER_TOO_SMALL;
            }

            int curPos = messageTemp.ReadPosition;
            int outputPos = decryptedOutput.Data().Position;
            for (int k = 0; k < tempLen; k++)
                decryptedOutput.Data().WriteAt((outputPos + k), (byte)messageTemp.ReadByteAt(curPos + k));

            decryptedOutput.Data().WritePosition = (int)tempLen;

            return ret;
        }
    }
}
