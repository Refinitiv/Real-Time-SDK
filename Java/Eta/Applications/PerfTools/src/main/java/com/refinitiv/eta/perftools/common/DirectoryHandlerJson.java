/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import java.util.List;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.core.JsonGenerator;

import java.io.IOException;

/**
 * This is the source directory handler for the ETA consumer application. It
 * provides methods for sending the source directory request to a provider and
 * processing the response. Methods for setting the service name, getting the
 * service information, and closing a source directory stream are also provided.
 */
public class DirectoryHandlerJson
{
    private static final int SRCDIR_STREAM_ID = 2;

    private static final long FILTER_TO_REQUEST = Directory.ServiceFilterFlags.INFO |
            Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;

    public static final int MAX_MSG_SIZE = 1024;
    public static int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

    private String serviceName;                 // requested service name
    private int serviceId;                      // requested service id
    private Qos serviceQos;                     // requested service Qos
    private int serviceAcceptingRequests;       // JSON value "ServiceState"
    private int serviceState;                   // JSON value "AcceptingRequests"

    /**
     * Instantiates a new directory handler.
     */
    public DirectoryHandlerJson()
    {
        serviceQos = null;
        serviceAcceptingRequests = 0;
        serviceState = 0;
    }

    /**
     * Sets the service name requested by the application.
     *
     * @param servicename - The service name requested by the application
     */
    public void serviceName(String servicename)
    {
        serviceName = servicename;
    }

    /**
     * Checks if is requested service up.
     *
     * @return true if service requested by application is up, false if not.
     */
    public boolean isRequestedServiceUp()
    {
        return (serviceAcceptingRequests == 1 && serviceState == 1);
    }

    /**
     * Service info.
     *
     * @return service id associated with the service name requested by application.
     */
    public int serviceId()
    {
        return serviceId;
    }

    /**
     * Service info.
     *
     * @return service Qos associated with the service name requested by application.
     */
    public Qos serviceQos()
    {
        return serviceQos;
    }

    /**
     * Sends a source directory request to a channel. This consists of getting a
     * message buffer, encoding the source directory request, and sending the
     * source directory request to the server.
     *
     * @param channel the channel
     * @param error the error
     * @param mapper the JSON mapper
     * @param generator the JSON generator
     * @param byteStream the byte stream object
     * @return the request
     */
    public TransportBuffer getRequest(Channel channel, Error error, ObjectMapper mapper, JsonGenerator generator, ByteBufferOutputStream byteStream)
    {
        /* get a buffer for the source directory request */
        TransportBuffer msgBuf = channel.getBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
        if (msgBuf == null)
            return null;

        byteStream.setByteBuffer(msgBuf.data());

        //encode source directory request
        // {"ID":2,"Type":"Request","Domain":"Source","KeyInUpdates":false,"Key":{"Filter":7}}

        ObjectNode json = mapper.createObjectNode()
            .put("ID", Integer.valueOf(SRCDIR_STREAM_ID))
            .put("Type", "Request")
            .put("Domain", "Source")
            .put("KeyInUpdates", Boolean.FALSE)
            .set("Key", mapper.createObjectNode()
                .put("Filter", Long.valueOf(FILTER_TO_REQUEST)));

        try
        {
            mapper.writeTree(generator, json);
        }
        catch (IOException e)
        {
            return null;
        }

        //return encoded source directory request
        return msgBuf;
    }

    public int directoryRefreshDecode(JsonNode jsonNode, Msg msg)
    {
        int ret = CodecReturnCodes.SUCCESS;
        return ret;
    }

    /**
     * Processes a source directory response. This consists of looking at the
     * msg class and decoding message into corresponding RDM directory
     * message. After decoding, service status state (up or down) is updated
     * from a refresh or update message for a requested service.
     *
     * @param chnl - The channel of the response msg - The partially decoded
     *            message
     * @param msg the msg
     * @param jsonNode - the JSON node to process
     * @param error the error
     * @return the int
     */
    public int processResponse(Channel chnl, Msg msg, JsonNode jsonNode, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                System.out.println("Received Source Directory Refresh");
                ret = directoryRefreshDecode(jsonNode, msg);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("Error decoding directory refresh: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }

                processServiceRefresh(jsonNode, error);

                break;

            case MsgClasses.UPDATE:
                System.out.println("Received Source Directory Update");
                System.out.println("JSON Source Directory Update is not supported.");

                break;

            case MsgClasses.STATUS:
                System.out.println("Received Source Directory Status");
                System.out.println("JSON Source Directory Status is not supported.");

                break;

            default:
                error.text("Received Unhandled Source Directory Msg Class: " + msg.msgClass());
                return CodecReturnCodes.FAILURE;
        }

        return ret;
    }

    private void processServiceRefresh(JsonNode jsonNode, Error error)
    {
        JsonNode mapEntriesNode = jsonNode.path("Map").path("Entries");
        for (final JsonNode mapEntryNode : mapEntriesNode)
        {
            String serviceName = null;
            int serviceState = 0;
            int acceptingRequests = 0;

            JsonNode filterListEntriesNode = mapEntryNode.path("FilterList").path("Entries");
            for (final JsonNode filterEntryNode : filterListEntriesNode)
            {
                int id = filterEntryNode.path("ID").intValue();
                switch (id)
                {
                    case Directory.ServiceFilterIds.INFO: /* (1) Service Info Filter ID */
                        serviceName = filterEntryNode.path("Elements").path("Name").textValue();
                        break;
                    case Directory.ServiceFilterIds.STATE: /* (2) Source State Filter ID */
                        serviceState = filterEntryNode.path("Elements").path("ServiceState").intValue();
                        acceptingRequests = filterEntryNode.path("Elements").path("AcceptingRequests").intValue();
                        break;
                    case Directory.ServiceFilterIds.GROUP: /* (3) Source Group Filter ID is not supported in this app */
                        break;
                    case Directory.ServiceFilterIds.LOAD: /* (4) Source Load Filter ID is not supported in this app */
                        break;
                    case Directory.ServiceFilterIds.DATA: /* (5) Source Data Filter ID is not supported in this app */
                        break;
                    case Directory.ServiceFilterIds.LINK: /* (6) Communication Link Filter ID is not supported in this app */
                        break;
                    default:
                        break;
                }
            }
            if (serviceName != null)
            {
                System.out.println("Received serviceName: " + serviceName);
                if (this.serviceName.equals(serviceName) && serviceState == 1 && acceptingRequests == 1)
                {
                    this.serviceState = serviceState;
                    this.serviceAcceptingRequests = acceptingRequests;
                    this.serviceId = mapEntryNode.path("Key").intValue();
                }
            }
        }
    }
}
