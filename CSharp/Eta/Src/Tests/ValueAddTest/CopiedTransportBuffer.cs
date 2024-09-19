/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using System.Text;
using Xunit;

namespace LSEG.Eta.ValuedAdd.Tests
{
    public class CopiedTransportBuffer : ITransportBuffer
    {
        ByteBuffer _data;
        int _length;


        public CopiedTransportBuffer(ITransportBuffer transportBuffer)
        {
            _data = new ByteBuffer(transportBuffer.Length());
            _length = transportBuffer.Length();
            Assert.True(transportBuffer.Copy(_data) == (int)TransportReturnCode.SUCCESS);
            _data.Flip();
        }

    
        public ByteBuffer Data
        {
            get => _data;
            set { _data = value; }
        }

        public bool IsOwnedByApp { get; set; }

        public int Length() => _length;

        public int Capacity() => _length;

        public int GetDataStartPosition()
        {
            return 0;
        }

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
