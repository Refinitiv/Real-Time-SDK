/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using LSEG.Eta.Transports;

namespace LSEG.Eta.Example.VACommon
{
    public class ConnectionArgsParser
    {
        const int ERROR_RETURN_CODE = -1;
        public List<ConnectionArg> ConnectionList { get; private set; } = new List<ConnectionArg>();


        /// <summary>
        /// Returns true if argOffset is start of connection arguments.
        /// </summary>
        ///
        /// <param name="args">array of command line arguments</param>
        /// <param name="argOffset">offset into array of command line arguments</param>
        ///
        /// <returns>true if argOffset is start of connection arguments and false otherwise</returns>
        public bool IsStart(string[] args, int argOffset)
        {
            if ("-c".Equals(args[argOffset])
                || "-tcp".Equals(args[argOffset]))
            {
                return true;
            }

            return false;
        }

        /// <summary>
        /// Parses connection arguments.
        /// </summary>
        ///
        /// <param name="args">array of command line arguments</param>
        /// <param name="argOffset">offset into array of command line arguments</param>
        ///
        /// <returns>argument offset after connection arguments or -1 if error</returns>
        public int Parse(string[] args, int argOffset)
        {
            int offset = 0;

            if ("-c".Equals(args[argOffset]) ||
                "-tcp".Equals(args[argOffset]))
            {
                offset = ParseSocketConnectionArgs(args, argOffset);
            }
            else
            {
                offset = ERROR_RETURN_CODE;
            }

            return offset;
        }

        /// <summary>
        /// parses TCP socket connection arguments
        /// </summary>
        /// <param name="args"></param>
        /// <param name="argOffset"></param>
        /// <returns></returns>
        private int ParseSocketConnectionArgs(string[] args, int argOffset)
        {
            int retCode = ERROR_RETURN_CODE;
            ConnectionArg? connectionArg;

            if ((args.Length - 1) >= argOffset + 3)
            {
                if (args[argOffset + 1].Contains(':')
                    && !args[argOffset + 2].Contains(':'))
                {
                    string[] tokens = args[argOffset + 1].Split(":");
                    if (tokens.Length == 2)
                    {
                        connectionArg = new ConnectionArg();
                        connectionArg.ConnectionType = ConnectionType.SOCKET;
                        if (!args[argOffset + 2].StartsWith("-"))
                        {
                            connectionArg.Service = args[argOffset + 2];
                            retCode = argOffset + 3;
                        }
                        else
                        {
                            retCode = argOffset + 2;
                        }
                        connectionArg.Hostname = tokens[0];
                        connectionArg.Port = tokens[1];
                        ConnectionList.Add(connectionArg);
                    }
                    else
                    {
                        return retCode;
                    }
                }
                else
                {
                    return retCode;
                }
            }
            else
            {
                return retCode;
            }

            // parse item arguments for this connection
            ItemArgsParser itemArgsParser = new ItemArgsParser();
            if (itemArgsParser.IsStart(args, retCode))
            {
                retCode = itemArgsParser.Parse(args, retCode);
                connectionArg.ItemList = itemArgsParser.ItemList;
            }
            else if (!args[retCode].Equals("-tsServiceName")
                     && !args[retCode].Equals("-tunnel")
                     && !args[retCode].Equals("-tsAuth")
                     && !args[retCode].Equals("-tsDomain"))
            {
                retCode = ERROR_RETURN_CODE;
            }

            return retCode;
        }
    }
}
