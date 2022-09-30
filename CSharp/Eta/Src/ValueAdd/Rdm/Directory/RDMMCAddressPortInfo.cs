/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Rdm
{
    public class RDMMCAddressPortInfo
    {
        public List<Buffer> Address { get; set; }
        public List<long> Port { get; set; }
        public List<long> Domain { get; set; }
        public int AddressCount { get; set; }

        public RDMMCAddressPortInfo()
        {
            Address = new List<Buffer>();
            Port = new List<long>();
            Domain = new List<long>();
            AddressCount = 0;
        }

        
        public void AddAddress(Buffer addr, int location)
        {
            Buffer newAddr = new Buffer();
            BufferHelper.CopyBuffer(addr, newAddr);
            Address.Insert(location, newAddr);
        }

        public void SetPort(long port, int location)
        {
            Port.Insert(location, port);
        }

        public void SetDomain(long domain, int location)
        {
            Domain.Insert(location, domain);
        }

        public void Clear()
        {
            Address.Clear();
            Port.Clear();
            Domain.Clear();
            AddressCount = 0;
        }
    }
}
