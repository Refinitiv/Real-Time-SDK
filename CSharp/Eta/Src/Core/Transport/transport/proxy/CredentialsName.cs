/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.Transports.proxy
{
    internal class CredentialName
    {
        /// <summary>
        /// The domain of the user to authenticate
        /// </summary>
        public const string DOMAIN = "domain";

        /// <summary>
        /// The username to authenticate
        /// </summary>
        public const string USERNAME = "username";

        /// <summary>
        /// The password to authenticate
        /// </summary>
        public const string PASSWORD = "password";

        /// <summary>
        /// The local hostname (server name)
        /// </summary>
        public const string LOCAL_HOSTNAME = "localhostname";

        /// <summary>
        /// Complete path of the Kerberos5 configuration file.
        /// Needed for Negotiate/Kerberos and Kerberos authentication.
        /// </summary>
        public const string KRB5_CONFIG_FILE = "krb5.ini";
    }
}

