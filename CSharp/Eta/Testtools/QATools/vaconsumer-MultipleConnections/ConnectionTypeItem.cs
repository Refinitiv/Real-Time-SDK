using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Example.VACommon
{
    public class ConnectionTypeItem
    {
        public ConnectionType EncryptedProtocolType { get; set; }

        public bool EnableEncrypted { get; set; }
    }
}