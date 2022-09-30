/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Transports;
using System.Text;
using Xunit;

namespace Refinitiv.Eta.ValuedAdd.Tests
{
    public class CopiedTransportBuffer : ITransportBuffer
    {
        ByteBuffer _data;
        int _length;


        public CopiedTransportBuffer(ITransportBuffer transportBuffer)
        {
            _data = new ByteBuffer(transportBuffer.Length);
            _length = transportBuffer.Length;
            Assert.True(transportBuffer.Copy(_data) == (int)TransportReturnCode.SUCCESS);
            _data.Flip();
        }

    
        public ByteBuffer Data
        {
            get => _data;
            set { _data = value; }
        }

        public int Length => _length;

        public int Capacity => _length;

        public int DataStartPosition => 0;

        public int Copy(ByteBuffer destBuffer)
        {
            return 0;
        }

        public override string ToString()
        {
            return Encoding.ASCII.GetString(_data.Contents);
        }
    }
}
