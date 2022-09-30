/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Internal;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.Transports.Internal;

namespace Refinitiv.Eta.Transports
{
    class IpcProtocolManager
    {
        private ConnectionsVersions m_StartingVersion = ConnectionsVersions.VERSION14;
        private readonly IpcProtocol m_Ripc11Protocol = new Ripc11Protocol();
        private readonly IpcProtocol m_Ripc12Protocol = new Ripc12Protocol();
        private readonly IpcProtocol m_Ripc13Protocol = new Ripc13Protocol();
        private readonly IpcProtocol m_Ripc14Protocol = new Ripc14Protocol();

        public IpcProtocol NextProtocol(IChannel channel, IpcProtocol currentProtocol, ConnectOptions connectOptions)
        {
            IpcProtocol retProtocol = null;

            if (m_StartingVersion == ConnectionsVersions.VERSION14)
            {
                if (currentProtocol == null)
                    retProtocol = m_Ripc14Protocol;
                else if (currentProtocol.ConnectionVersion() == ConnectionsVersions.VERSION14)
                {
                    /* If we are rolling back beyond version 14, we need to unset key exchange */
                    currentProtocol.ProtocolOptions.KeyExchange = false;
                    retProtocol = m_Ripc13Protocol;
                }
                else if (currentProtocol.ConnectionVersion() == ConnectionsVersions.VERSION13)
                    retProtocol = m_Ripc12Protocol;
                else if (currentProtocol.ConnectionVersion() == ConnectionsVersions.VERSION12
                        && (int)connectOptions.ProtocolType == Codec.Codec.ProtocolType())
                    // apps using version 11 shouldn't be using anything others than RWF
                    retProtocol = m_Ripc11Protocol;
            }
            else if (m_StartingVersion == ConnectionsVersions.VERSION13)
            {
                if (currentProtocol == null)
                    retProtocol = m_Ripc13Protocol;
                else if (currentProtocol.ConnectionVersion() == ConnectionsVersions.VERSION13)
                    retProtocol = m_Ripc12Protocol;
                else if (currentProtocol.ConnectionVersion() == ConnectionsVersions.VERSION12
                        && (int)connectOptions.ProtocolType == Codec.Codec.ProtocolType())
                    // apps using version 11 shouldn't be using anything others than RWF
                    retProtocol = m_Ripc11Protocol;
            }
            // for JUnit testing so method returns something lower than Ripc13Protocol()
            else if (m_StartingVersion == ConnectionsVersions.VERSION12)
            {
                if (currentProtocol == null)
                    retProtocol = m_Ripc12Protocol;
                else if (currentProtocol.ConnectionVersion() == ConnectionsVersions.VERSION12
                        && (int)connectOptions.ProtocolType == Codec.Codec.ProtocolType())
                    // apps using version 11 shouldn't be using anything others than RWF
                    retProtocol = m_Ripc11Protocol;
            }
            else if (m_StartingVersion == ConnectionsVersions.VERSION11)
            {
                if (currentProtocol == null)
                    retProtocol = m_Ripc11Protocol;
            }

            if (retProtocol != null)
                retProtocol.Channel = channel;

            return retProtocol;
        }

        public IpcProtocol DetermineProtocol(IChannel channel, ConnectionsVersions connectionVersion)
        {
            IpcProtocol retProtocol = null;

            if (connectionVersion == ConnectionsVersions.VERSION14)
                retProtocol = m_Ripc14Protocol;
            else if (connectionVersion == ConnectionsVersions.VERSION13)
                retProtocol = m_Ripc13Protocol;
            else if (connectionVersion == ConnectionsVersions.VERSION12)
                retProtocol = m_Ripc12Protocol;
            else if (connectionVersion == ConnectionsVersions.VERSION11)
                retProtocol = m_Ripc11Protocol;

            if (retProtocol != null)
                retProtocol.Channel = channel;

            return retProtocol;
        }

        public void SetStartingRipcVersion(ConnectionsVersions startingVersion)
        {
            m_StartingVersion = startingVersion;
        }
    }
}
