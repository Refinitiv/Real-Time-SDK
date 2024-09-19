/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.domainrep.rdm.login;

import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.ElementEntry;
import com.refinitiv.eta.codec.ElementList;
import com.refinitiv.eta.codec.ElementListFlags;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.LocalElementSetDefDb;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.Vector;
import com.refinitiv.eta.codec.VectorEntry;
import com.refinitiv.eta.codec.VectorEntryActions;
import com.refinitiv.eta.codec.VectorFlags;
import com.refinitiv.eta.rdm.ElementNames;

class LoginConnectionConfigImpl implements LoginConnectionConfig
{
    private long numStandbyServers;
    private List<ServerInfo> serverList;

    
    private final static String eol = System.getProperty("line.separator");
    private final static String tab = "\t";
    private StringBuilder stringBuf = new StringBuilder();
    
    private Vector vector = CodecFactory.createVector();
    private VectorEntry vectorEntry = CodecFactory.createVectorEntry();
    private ElementList elementList = CodecFactory.createElementList();
    private ElementEntry elementEntry = CodecFactory.createElementEntry();
    private ElementList serverElementList = CodecFactory.createElementList();
    private ElementEntry serverElementEntry = CodecFactory.createElementEntry();
    private com.refinitiv.eta.codec.Enum tmpEnum = CodecFactory.createEnum();
    private UInt tmpUInt = CodecFactory.createUInt();
    private LocalElementSetDefDb setDb = CodecFactory.createLocalElementSetDefDb();

    LoginConnectionConfigImpl()
    {
        serverList = new ArrayList<LoginConnectionConfig.ServerInfo>();
    }

    public int copy(LoginConnectionConfig destConnectionConfig)
    {
        assert (destConnectionConfig != null) : "destConnectionConfig can not be null";

        destConnectionConfig.numStandbyServers(numStandbyServers());

        int ret = CodecReturnCodes.SUCCESS;

        for (ServerInfo serverInfo : serverList())
        {
            ServerInfo destServerInfo = new ServerInfo();
            ret = serverInfo.copy(destServerInfo);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
            destConnectionConfig.serverList().add(destServerInfo);
        }

        return CodecReturnCodes.SUCCESS;
    }

    public void clear()
    {
        numStandbyServers = 0;
        serverList.clear();
    }

    public int encode(EncodeIterator encodeIter)
    {
        // Encode Element "ConnectionConfig
        elementList.clear();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);

        int ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        elementEntry.clear();

        elementEntry.dataType(DataTypes.VECTOR);
        elementEntry.name(ElementNames.CONNECTION_CONFIG);

        ret = elementEntry.encodeInit(encodeIter, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        // Encode Server Entries
        vector.clear();
        vector.containerType(DataTypes.ELEMENT_LIST);
        vector.flags(VectorFlags.HAS_SUMMARY_DATA);

        ret = vector.encodeInit(encodeIter, 0, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        // encode numStandbyServers in summary data.
        elementList.clear();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);

        ret = elementList.encodeInit(encodeIter, null, 0);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        elementEntry.clear();

        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(ElementNames.NUM_STANDBY_SERVERS);
        tmpUInt.value(numStandbyServers());
        ret = elementEntry.encode(encodeIter, tmpUInt);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }
        ret = elementList.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = vector.encodeSummaryDataComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        for (ServerInfo serverInfo : serverList())
        {
            vectorEntry.clear();
            vectorEntry.index(serverInfo.serverIndex());
            vectorEntry.action(VectorEntryActions.SET);
            ret = vectorEntry.encodeInit(encodeIter, 0);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            // Encode Element List describing server
            elementList.clear();
            elementList.flags(ElementListFlags.HAS_STANDARD_DATA);

            ret = elementList.encodeInit(encodeIter, null, 0);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            elementEntry.clear();

            elementEntry.dataType(DataTypes.ASCII_STRING);
            elementEntry.name(ElementNames.HOSTNAME);
            ret = elementEntry.encode(encodeIter, serverInfo.hostName());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            
            elementEntry.dataType(DataTypes.UINT);
            elementEntry.name(ElementNames.PORT);
            tmpUInt.value(serverInfo.port());
            ret = elementEntry.encode(encodeIter, tmpUInt);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            
            if (serverInfo.checkHasLoadFactor())
            {
                elementEntry.dataType(DataTypes.UINT);
                elementEntry.name(ElementNames.LOAD_FACT);
                tmpUInt.value(serverInfo.loadFactor());
                ret = elementEntry.encode(encodeIter, tmpUInt);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            if (serverInfo.checkHasType())
            {
                elementEntry.dataType(DataTypes.ENUM);
                elementEntry.name(ElementNames.SERVER_TYPE);
                tmpEnum.value(serverInfo.serverType());
                ret = elementEntry.encode(encodeIter, tmpEnum);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }

            ret = elementList.encodeComplete(encodeIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            ret = vectorEntry.encodeComplete(encodeIter, true);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        // Complete
        ret = vector.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        ret = elementEntry.encodeComplete(encodeIter, true);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        return elementList.encodeComplete(encodeIter, true);
    }

    public int decode(DecodeIterator dIter)
    {
        clear();
        int ret = vector.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        if (vector.containerType() != DataTypes.ELEMENT_LIST)
        {
            return CodecReturnCodes.FAILURE;
        }

        if (vector.checkHasSetDefs())
        {
            setDb.clear();
            ret = setDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
        }

        // Decode payload
        if (vector.checkHasSummaryData())
        {
            ret = serverElementList.decode(dIter, null);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            boolean foundNumStandbyServers = false;
            // decode each element entry in list
            while ((ret = serverElementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }

                if (ElementNames.NUM_STANDBY_SERVERS.equals(serverElementEntry.name()))
                {
                    if (serverElementEntry.dataType() != DataTypes.UINT)
                        return CodecReturnCodes.FAILURE;
                    ret = tmpUInt.decode(dIter);
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                    numStandbyServers(tmpUInt.toLong());
                    foundNumStandbyServers = true;
                }
            }

            if (!foundNumStandbyServers)
                return CodecReturnCodes.FAILURE;
        } // end summary data decode

        vectorEntry.clear();
        boolean foundHostName = false;
        boolean foundServerPort = false;
        if ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }
            do
            {
                ret = serverElementList.decode(dIter, setDb);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    return ret;
                }
                
                ServerInfo serverInfo = new ServerInfo();
                serverList().add(serverInfo);
                serverInfo.serverIndex(vectorEntry.index());
                
                // decode each element entry in list
                while ((ret = serverElementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
                {
                    if (ret != CodecReturnCodes.SUCCESS)
                    {
                        return ret;
                    }

                    if (serverElementEntry.name().equals(ElementNames.HOSTNAME))
                    {
                        foundHostName = true;
                        serverInfo.hostName(serverElementEntry.encodedData());
                    }

                    if (serverElementEntry.name().equals(ElementNames.PORT))
                    {
                        ret = tmpUInt.decode(dIter);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        foundServerPort = true;
                        serverInfo.port(tmpUInt.toLong());
                    }

                    if (serverElementEntry.name().equals(ElementNames.LOAD_FACT))
                    {
                        ret = tmpUInt.decode(dIter);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        serverInfo.applyHasLoadFactor();
                        serverInfo.loadFactor(tmpUInt.toLong());
                    }

                    if (serverElementEntry.name().equals(ElementNames.SERVER_TYPE))
                    {
                        ret = tmpEnum.decode(dIter);
                        if (ret != CodecReturnCodes.SUCCESS)
                        {
                            return ret;
                        }
                        serverInfo.applyHasType();
                        serverInfo.serverType(tmpEnum.toInt());
                    }

                }

                if (!foundServerPort)
                    return CodecReturnCodes.FAILURE;

                if (!foundHostName)
                    return CodecReturnCodes.FAILURE;
            }
            while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER);
        }
        return CodecReturnCodes.SUCCESS;
    }

    public String toString()
    {
        stringBuf.setLength(0);

        stringBuf.append(tab);
        stringBuf.append("numStandbyServers: ");
        stringBuf.append(numStandbyServers());
        stringBuf.append(eol);

        for(ServerInfo serverInfo : serverList)
        {
            stringBuf.append(serverInfo.toString());
        }
        stringBuf.append(eol);
        
        return stringBuf.toString();
    }
    
    public long numStandbyServers()
    {
        return numStandbyServers;
    }

    public void numStandbyServers(long numStandbyServers)
    {
        this.numStandbyServers = numStandbyServers;
    }

    public void serverList(List<ServerInfo> serverList)
    {
        serverList().clear();

        for (ServerInfo server : serverList)
        {
            serverList().add(server);
        }
    }

    public List<ServerInfo> serverList()
    {
        return serverList;
    }

    public void copyReferences(LoginConnectionConfig srcConnectionConfig)
    {
        assert (srcConnectionConfig != null) : "srcConnectionConfig can not be null";

        numStandbyServers(srcConnectionConfig.numStandbyServers());
        serverList(srcConnectionConfig.serverList());
    }
}
