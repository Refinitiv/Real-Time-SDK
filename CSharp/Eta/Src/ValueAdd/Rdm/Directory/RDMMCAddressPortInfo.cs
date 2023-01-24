/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// RDM Address and Port information structure used for Multicast-based connections.
    /// </summary>
    sealed public class RDMMCAddressPortInfo
    {
        /// <summary>
        /// List of <see cref="Buffer"/> containing the address information.
        /// </summary>
        public List<Buffer> Address { get; set; }

        /// <summary>
        /// List of ports.
        /// </summary>
        public List<long> Port { get; set; }

        /// <summary>
        /// List of <see cref="Eta.Rdm.DomainType"/>associated with this stucture.
        /// </summary>
        public List<long> Domain { get; set; }

        /// <summary>
        /// Total number of addresses in this structure.
        /// </summary>
        public int AddressCount { get; set; }

        /// <summary>
        /// Adds a <see cref="Buffer"/> containing the address information to <c>location</c>
        /// </summary>
        /// <param name="addr">The address information</param>
        /// <param name="location">The location</param>
        public void AddAddress(Buffer addr, int location)
        {
            Buffer newAddr = new Buffer();
            BufferHelper.CopyBuffer(addr, newAddr);
            Address.Insert(location, newAddr);
        }

        /// <summary>
        /// Adds a port <c>long</c> to <c>location</c>
        /// </summary>
        /// <param name="port">The set port</param>
        /// <param name="location">The location</param>
        public void SetPort(long port, int location)
        {
            Port.Insert(location, port);
        }

        /// <summary>
        /// Adds a domain type to <c>location</c>
        /// </summary>
        /// <param name="domain">The set domain</param>
        /// <param name="location">The location</param>
        public void SetDomain(long domain, int location)
        {
            Domain.Insert(location, domain);
        }

        /// <summary>
        /// RDM Address Port Info constructor.
        /// </summary>
        public RDMMCAddressPortInfo()
        {
            Address = new List<Buffer>();
            Port = new List<long>();
            Domain = new List<long>();
            AddressCount = 0;
        }

        /// <summary>
        /// Clears the current contents of the RDM Address Port Info object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {
            Address.Clear();
            Port.Clear();
            Domain.Clear();
            AddressCount = 0;
        }
    }
}
