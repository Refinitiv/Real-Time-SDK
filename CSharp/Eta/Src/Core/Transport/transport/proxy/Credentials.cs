/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

namespace LSEG.Eta.Transports.proxy
{
    internal class Credentials
    {
        private readonly Dictionary<string, string> m_Credentials = new Dictionary<string, string>();

        public Credentials()
        {
        }

        public void SetKeyValue(string name, string value)
        {
            if (name is not null)
            {
                m_Credentials.Add(name, value);
            }
        }

        public string GetKeyValue(string name)
        {
            if (name is not null)
            {
                if (m_Credentials.TryGetValue(name, out string value))
                    return value;
            }

            return null;
        }

        public bool HasKey(string name)
        {
            return name is not null && m_Credentials.ContainsKey(name);
        }
    }
}
