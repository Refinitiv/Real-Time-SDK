/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using System.Diagnostics;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    public class RDMAddressPortInfo
    {
        private Buffer _address;
        public Buffer Address { get => _address; set { Debug.Assert(value != null);  BufferHelper.CopyBuffer(value, _address); } }
        public long Port { get; set; }
        public RDMAddressPortInfo()
        {
            _address = new Buffer();
            Clear();
        }

        public void Clear()
        {            
            _address.Clear();
            Port = 0;
        }
    }
}
