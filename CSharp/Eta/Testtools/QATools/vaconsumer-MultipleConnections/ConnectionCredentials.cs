using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Example.VACommon
{
    public class ConnectionCredentials
    {
        public string? UserName { get; set; }

        public string? Passwd { get; set; }

        public string? ClientId { get; set; }

        public string? ClientSecret { get; set; }

        public string? ClientJwk { get; set; }
    }
}