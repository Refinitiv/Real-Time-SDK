package com.rtsdk.eta.valueadd.domainrep.rdm.login;

import java.nio.ByteBuffer;
import java.util.List;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.rdm.Login.ServerTypes;

/**
 * Connection config is representation of login response payload and contains
 * standby configuration information.
 */
public interface LoginConnectionConfig
{
    /**
     * Performs a deep copy of {@link LoginConnectionConfig} object.
     * 
     * @param destConnectionConfig ConnectionConfig object to copy this object
     *            into. It cannot be null.
     * 
     * @return UPA return value indicating success or failure of copy operation.
     */
    public int copy(LoginConnectionConfig destConnectionConfig);

    /**
     * Clears the current contents of the server info object and prepares it
     * for re-use.
     */
    public void clear();
    
    /**
     * Returns the number of standby servers.
     * 
     * @return numStandbyServers
     */
    public long numStandbyServers();

    /**
     * Sets the number of standby servers.
     *
     * @param numStandbyServers the num standby servers
     */
    public void numStandbyServers(long numStandbyServers);

    /**
     * Sets list of {@link ServerInfo} for standby connection configuration.
     *
     * @param serverInfo the server info
     */
    public void serverList(List<ServerInfo> serverInfo);

    /**
     * Returns list of {@link ServerInfo} for standby connection configuration.
     * 
     * @return serverList
     */
    public List<ServerInfo> serverList();

    /**
     * Information about available servers. A list of RDMServerInfo is
     * used by an RDMLoginRefresh to list servers available for connecting
     * to, and whether to use them as Standby servers.
     * 
     * @see ServerInfoFlags
     * @see LoginRefresh
     */
    public static class ServerInfo
    {
        private int flags;

        private long serverIndex;
        private Buffer hostName;
        private long port;
        private long loadFactor;
        private int serverType;

        private final static String eol = System.getProperty("line.separator");
        private final static String tab = "\t";
        private StringBuilder stringBuf = new StringBuilder();

        /**
         * Instantiates a new server info.
         */
        public ServerInfo()
        {
            flags = 0;
            hostName = CodecFactory.createBuffer();
            loadFactor = 65535;
            serverType = ServerTypes.STANDBY;
            serverIndex = 0;
        }

        /**
         * Sets the server info flags. Populated by {@link ServerInfoFlags}.
         *
         * @param flags the flags
         */
        public void flags(int flags)
        {
            this.flags = flags;
        }

        /**
         * Returns the server info flags. Populated by {@link ServerInfoFlags}.
         * 
         * @return flags
         */
        public int flags()
        {
            return flags;
        }

        /**
         * Performs a deep copy of {@link ServerInfo} object.
         * 
         * @param destServerInfo ServerInfo object to copy this object into. It
         *            cannot be null.
         * 
         * @return UPA return value indicating success or failure of copy
         *         operation.
         */
        public int copy(ServerInfo destServerInfo)
        {
            assert (destServerInfo != null) : "destServerInfo can not be null";

            destServerInfo.flags(flags);

            ByteBuffer byteBuffer = ByteBuffer.allocate(hostName.length());
            hostName.copy(byteBuffer);
            destServerInfo.hostName().data(byteBuffer);
            destServerInfo.port(port);
            destServerInfo.serverIndex(serverIndex);

            if (checkHasLoadFactor())
                destServerInfo.loadFactor(loadFactor);

            if (checkHasType())
                destServerInfo.serverType(serverType);

            return CodecReturnCodes.SUCCESS;
        }

        /* (non-Javadoc)
         * @see java.lang.Object#toString()
         */
        public String toString()
        {
            stringBuf.setLength(0);
            stringBuf.append(tab);
            stringBuf.append("Server:");
            stringBuf.append(eol);
            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("hostName: ");
            stringBuf.append(hostName().toString());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("port: ");
            stringBuf.append(port());
            stringBuf.append(eol);

            stringBuf.append(tab);
            stringBuf.append(tab);
            stringBuf.append("serverIndex: ");
            stringBuf.append(serverIndex());
            stringBuf.append(eol);

            if (checkHasLoadFactor())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("loadFactor: ");
                stringBuf.append(loadFactor());
                stringBuf.append(eol);
            }
            if (checkHasType())
            {
                stringBuf.append(tab);
                stringBuf.append(tab);
                stringBuf.append("serverType: ");
                stringBuf.append(serverType());
                stringBuf.append(eol);
            }

            return stringBuf.toString();
        }

        /**
         * Copy references.
         *
         * @param srcServerInfo the src server info
         * @return the int
         */
        int copyReferences(ServerInfo srcServerInfo)
        {
            assert (srcServerInfo != null) : "srcServerInfo can not be null";

            hostName().data(srcServerInfo.hostName().data(), srcServerInfo.hostName().position(), srcServerInfo.hostName().length());
            flags(srcServerInfo.flags());
            if (srcServerInfo.checkHasLoadFactor())
                loadFactor(srcServerInfo.loadFactor());
            if (srcServerInfo.checkHasType())
                serverType(srcServerInfo.serverType());
            port(srcServerInfo.port());
            serverIndex(srcServerInfo.serverIndex());

            return CodecReturnCodes.SUCCESS;
        }

        /**
         * Clears the current contents of the server info object and prepares it
         * for re-use.
         */
        public void clear()
        {
            hostName.clear();
            port = 0;
            loadFactor = 65535;
            serverType = ServerTypes.STANDBY;
            serverIndex = 0;
        }

        /**
         * Returns hostName for the server.
         * 
         * @return Server's hostName to connect to.
         */
        public Buffer hostName()
        {
            return hostName;
        }

        /**
         * Sets hostName for the server. Note that this creates garbage if
         * buffer is backed by String object.
         * 
         * @param hostName to connect to.
         */
        public void hostName(Buffer hostName)
        {
            assert (hostName != null) : "hostName can not be null";

            hostName().data(hostName.data(), hostName.position(), hostName.length());
        }

        /**
         * Returns port for the server.
         * 
         * @return Server's port to connect to.
         */
        public long port()
        {
            return port;
        }

        /**
         * Sets port for the server.
         * 
         * @param port to connect to.
         */
        public void port(long port)
        {
            this.port = port;
        }

        /**
         * Returns Server's loadFactor.
         * 
         * @return Server's loadFactor.
         */
        public long loadFactor()
        {
            return loadFactor;
        }

        /**
         * Sets Server's loadFactor.
         *
         * @param loadFactor the load factor
         */
        public void loadFactor(long loadFactor)
        {
            assert (checkHasLoadFactor());
            this.loadFactor = loadFactor;
        }

        /**
         * Applies loadFactor presence flag.
         * 
         * This flag can also be bulk-set by {@link #flags(int)}
         * 
         */
        public void applyHasLoadFactor()
        {
            flags |= ServerInfoFlags.HAS_LOAD_FACTOR;
        }

        /**
         * Checks the presence of loadFactor field.
         * 
         * This flag can also be bulk-get by {@link #flags()}
         * 
         * @return true - if loadFactor is present, false - if not.
         */
        public boolean checkHasLoadFactor()
        {
            return (flags & ServerInfoFlags.HAS_LOAD_FACTOR) != 0;
        }

        /**
         * Returns Server's serverType.
         * 
         * @return Server's serverType.
         */
        public int serverType()
        {
            return serverType;
        }

        /**
         * Sets Server's serverType.
         *
         * @param serverType the server type
         */
        public void serverType(int serverType)
        {
            assert (checkHasType());
            this.serverType = serverType;
        }

        /**
         * Applies serverType presence flag.
         * 
         * This flag can also be bulk-set by {@link #flags(int)}
         * 
         */
        public void applyHasType()
        {
            flags |= ServerInfoFlags.HAS_TYPE;
        }

        /**
         * Checks the presence of serverType field.
         * 
         * This flag can also be bulk-get by {@link #flags()}
         * 
         * @return true - if serverType is present, false - if not.
         */
        public boolean checkHasType()
        {
            return (flags & ServerInfoFlags.HAS_TYPE) != 0;
        }

        /**
         * Sets an index for the server.
         *
         * @param serverIndex the server index
         */
        public void serverIndex(long serverIndex)
        {
            this.serverIndex = serverIndex;
        }

        /**
         * Returns an index for the server.
         * 
         * @return an index for the server.
         */
        public long serverIndex()
        {
            return serverIndex;
        }
    }

    /**
     * The RDM Login Server Info Flags.
     * 
     * @see ServerInfo
     * @see LoginRefresh
     */
    public static class ServerInfoFlags
    {
        /**
         * (0x00) No flags set
         */
        public static final int NONE = 0x00;

        /**
         * (0x01) Indicates presence of the loadFactor member.
         */
        public static final int HAS_LOAD_FACTOR = 0x01;

        /**
         * (0x02) Indicates presence of the serverType member.
         */
        public static final int HAS_TYPE = 0x02;
    }
}
