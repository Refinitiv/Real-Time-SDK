using LSEG.Eta.Transports;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Example.VACommon
{
    internal class ConnectionTypeParser
    {
        const int ERROR_RETURN_CODE = -1;
        public ConnectionTypeItem ConnType = new();

        // returns offset past item arguments or -1 if error
        internal bool IsStart(string[] args, int argOffset)
        {
            if (argOffset < args.Length && args[argOffset].StartsWith("connectionType:"))
            {
                return true;
            }
            else if (argOffset < args.Length && args[argOffset].StartsWith("encryptedProtocolType:"))
            {
                return true;
            }

            return false;
        }


        internal int Parse(string[] args, int argOffset)
        {
            int retCode = ERROR_RETURN_CODE;

            String[] commaTokens = args[argOffset].Split(",");
            for (int i = 0; i < commaTokens.Length; i++)
            {
                if (ParseItem(commaTokens[i]) < 0)
                {
                    return ERROR_RETURN_CODE;
                }
            }
            retCode = argOffset + 1;

            return retCode;
        }

        private int ParseItem(string itemStr)
        {
            int retCode = ERROR_RETURN_CODE;

            string[] tokens = itemStr.Split(":");
            if (tokens.Length == 2)
            {
                if ("connectionType".Equals(tokens[0]))
                {
                    // will overwrite connectionArgsParser's connectionList's connectionType based on the flag
                    string connectionType = tokens[1];
                    if (connectionType.Equals("encrypted"))
                    {
                        ConnType.EnableEncrypted = true;
                    }
                }
                else if ("encryptedProtocolType".Equals(tokens[0]))
                {
                    // will overwrite connectionArgsParser's connectionList's connectionType based on the flag
                    String connectionType = tokens[1];
                    if (connectionType.Equals("socket"))
                    {
                        ConnType.EncryptedProtocolType = ConnectionType.SOCKET;
                    }
                }
                retCode = 1;
            }

            return retCode;
        }
    }
}
