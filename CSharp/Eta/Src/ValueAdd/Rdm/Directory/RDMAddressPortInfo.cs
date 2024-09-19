/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using System.Diagnostics;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.ValueAdd.Rdm
{
    /// <summary>
    /// RDM Address and Port information structure used for TCP-based connections.
    /// </summary>
    sealed public class RDMAddressPortInfo
    {
        private Buffer _address;

        /// <summary>
        /// <see cref="Buffer"/> containing the address information.
        /// </summary>
        public Buffer Address { get => _address; set { Debug.Assert(value != null);  BufferHelper.CopyBuffer(value, _address); } }

        /// <summary>
        /// Port number
        /// </summary>
        public long Port { get; set; }

        /// <summary>
        /// RDM Address Port Info constructor.
        /// </summary>
        public RDMAddressPortInfo()
        {
            _address = new Buffer();
            Clear();
        }

        /// <summary>
        /// Clears the current contents of the RDM Address Port Info object and prepares it for re-use.
        /// </summary>
        public void Clear()
        {            
            _address.Clear();
            Port = 0;
        }
    }
}
