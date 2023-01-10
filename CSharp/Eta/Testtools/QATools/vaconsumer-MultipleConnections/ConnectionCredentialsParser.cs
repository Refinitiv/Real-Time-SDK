uusing System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Eta.Example.VACommon
{
    internal class ConnectionCredentialsParser
    {
        const int ERROR_RETURN_CODE = -1;
        public ConnectionCredentials Creds = new();
        // returns offset past item arguments or -1 if error

        internal bool IsStart(string[] args, int argOffset)
        {
            if (argOffset < args.Length && args[argOffset].StartsWith("clientId:"))
            {
                return true;
            }
            else if (argOffset < args.Length && args[argOffset].StartsWith("clientJwk:"))
            {
                return true;
            }
            else if (argOffset < args.Length && args[argOffset].StartsWith("clientSecret:"))
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
                if ("clientId".Equals(tokens[0]))
                {
                    Creds.ClientId = tokens[1];
                }
                else if ("clientSecret".Equals(tokens[0]))
                {
                    Creds.ClientSecret = tokens[1];
                }
                else if ("clientJwk".Equals(tokens[0]))
                {
                    Creds.ClientJwk = tokens[1];
                }
                retCode = 1;
            }

            return retCode;
        }
    }
}

