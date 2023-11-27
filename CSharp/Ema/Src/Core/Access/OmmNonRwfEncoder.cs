/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class OmmNonRwfEncoder : Encoder
    {
        public void EncodeBuffer(EmaBuffer data)
        {
            Encode(data.m_Buffer, data.Length);
        }

        public void EncodeBuffer(string data)
        {
            byte[] stringBytes = Encoding.GetEncoding("UTF-8").GetBytes(data);
            Encode(stringBytes, stringBytes.Length);
        }

        private void Encode(byte[] data, int length)
        {
            if (m_containerComplete)
            {
                return;
            }

            if (m_encodeIterator == null)
            {
                AcquireEncodeIterator();
            }

            Buffer buffer = new Buffer();
            buffer.Clear();

            var ret = m_encodeIterator!.EncodeNonRWFInit(buffer);
            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Internal failure in OmmNonRwfEncoder.EncodeBuffer(): {ret.GetAsString()}");
            }

            while (buffer.Length < length)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_encodeIterator!.EncodeNonRWFInit(buffer);
            }

            buffer.Data().Put(data, 0, length);

            ret = m_encodeIterator.EncodeNonRWFComplete(buffer, true);
            while (ret == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                ReallocateEncodeIteratorBuffer();
                ret = m_encodeIterator!.EncodeNonRWFComplete(buffer, true);
            }

            if (ret != CodecReturnCode.SUCCESS)
            {
                throw new OmmInvalidUsageException($"Failed to complete encoding Non-RWF data: {ret.GetAsString()}");
            }

            if (!OwnsIterator() && m_iteratorOwner != null)
            {
                m_iteratorOwner.EndEncodingEntry!();
            }

            m_containerComplete = true;
        }
    }
}
