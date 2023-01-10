/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System;
using System.Collections.Generic;

using LSEG.Eta.Rdm;

namespace LSEG.Eta.Example.VACommon
{
    internal class ItemArgsParser
    {
        const int ERROR_RETURN_CODE = -1;
        public List<ItemArg> ItemList { get; private set; } = new List<ItemArg>();

        internal bool IsStart(string[] args, int argOffset)
        {
            if (args[argOffset].StartsWith("mp:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("mpps:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("mbo:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("mbops:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("mbp:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("mbpps:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("yc:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("ycps:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("sl:"))
            {
                return true;
            }
            else if (args[argOffset].StartsWith("sl"))
            {
                return true;
            }

            return false;
        }

        // returns offset past item arguments or -1 if error
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
                ItemArg itemArg = new ItemArg();
                itemArg.Domain = StringToDomain(tokens[0]);
                if (tokens[0].Contains("ps"))
                {
                    itemArg.EnablePrivateStream = true;
                }
                if (tokens[0].Contains("sld"))
                {
                    itemArg.SymbolListData = true;
                }
                itemArg.ItemName = tokens[1];
                ItemList.Add(itemArg);
                retCode = 1;
            }
            else if (tokens.Length == 1 && itemStr.Equals("sl"))
            {
                ItemList.Add(new ItemArg()
                {
                    Domain = DomainType.SYMBOL_LIST
                });
                retCode = 1;
            }
            else if (tokens.Length == 1 && itemStr.Equals("sld"))
            {
                ItemList.Add(new ItemArg()
                {
                    Domain = DomainType.SYMBOL_LIST,
                    SymbolListData = true
                });
                retCode = 1;
            }

            return retCode;
        }

        private DomainType StringToDomain(string domainStr)
        {
            DomainType retCode = 0;

            if (domainStr.StartsWith("mp"))
            {
                retCode = DomainType.MARKET_PRICE;
            }
            else if (domainStr.StartsWith("mbo"))
            {
                retCode = DomainType.MARKET_BY_ORDER;
            }
            else if (domainStr.StartsWith("mbp"))
            {
                retCode = DomainType.MARKET_BY_PRICE;
            }
            else if (domainStr.StartsWith("yc"))
            {
                retCode = DomainType.YIELD_CURVE;
            }
            else if (domainStr.StartsWith("sl"))
            {
                retCode = DomainType.SYMBOL_LIST;
            }

            return retCode;
        }

    }

}
