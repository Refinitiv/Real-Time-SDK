/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.common;

import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.json.converter.JsonConverterError;
import com.refinitiv.eta.json.converter.ServiceNameIdConverter;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;

/**
 * This is the source directory handler for the ETA consumer application. It
 * provides methods for sending the source directory request to a provider and
 * processing the response. Methods for setting the service name, getting the
 * service information, and closing a source directory stream are also provided.
 */
public class DirectoryHandler implements ServiceNameIdConverter
{
    private static final int SRCDIR_STREAM_ID = 2;

    private static final long FILTER_TO_REQUEST = Directory.ServiceFilterFlags.INFO |
            Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP;

    public static int TRANSPORT_BUFFER_SIZE_REQUEST = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    private Buffer serviceName ;    // requested service
    private Service service;        // service cache for the requested service
    private State state;            // directory stream state

    private DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
    private DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
    private DirectoryUpdate directoryUpdate = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
    private DirectoryClose directoryClose = (DirectoryClose)DirectoryMsgFactory.createMsg();
    private DirectoryStatus directoryStatus = (DirectoryStatus)DirectoryMsgFactory.createMsg();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    /**
     * Instantiates a new directory handler.
     */
    public DirectoryHandler()
    {
        serviceName = CodecFactory.createBuffer();
        service = DirectoryMsgFactory.createService();
        state = CodecFactory.createState();
        directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
        directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
    }

    /**
     * Sets the service name requested by the application.
     *
     * @param servicename - The service name requested by the application
     */
    public void serviceName(String servicename)
    {
        serviceName.data(servicename);
    }

    /**
     * Checks if is requested service up.
     *
     * @return true if service requested by application is up, false if not.
     */
    public boolean isRequestedServiceUp()
    {
        return  service.checkHasState() && (!service.state().checkHasAcceptingRequests() || service.state().acceptingRequests() == 1) && service.state().serviceState() == 1;
    }

    /**
     * Service info.
     *
     * @return service info associated with the service name requested by application.
     */
    public Service serviceInfo()
    {
        return service;
    }

    /**
     * Sends a source directory request to a channel. This consists of getting a
     * message buffer, encoding the source directory request, and sending the
     * source directory request to the server.
     *
     * @param chnl - The channel to send a source directory request to
     * @param error the error
     * @return the int
     */
    public int sendRequest(ChannelSession chnl, Error error)
    {
        /* get a buffer for the source directory request */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        //initialize directory state
        //this will be updated as refresh and status messages are received
        state.dataState(DataStates.NO_CHANGE);
        state.streamState(StreamStates.UNSPECIFIED);

        //encode source directory request
        directoryRequest.clear();
        directoryRequest.streamId(SRCDIR_STREAM_ID);
        directoryRequest.filter(FILTER_TO_REQUEST);
        directoryRequest.applyStreaming();
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
        int ret = directoryRequest.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeDirectoryRequest(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }
        System.out.println(directoryRequest.toString());

        //send source directory request
        ret = chnl.write(msgBuf, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
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
     * @param dIter - The decode iterator
     * @param error the error
     * @return the int
     */
    public int processResponse(Channel chnl, Msg msg, DecodeIterator dIter, Error error)
    {
        int ret = CodecReturnCodes.SUCCESS;
        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
                System.out.println("Received Source Directory Refresh");
                ret = directoryRefresh.decode(dIter, msg);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("Error decoding directory refresh: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }

                System.out.println(directoryRefresh.toString());

                state.dataState(directoryRefresh.state().dataState());
                state.streamState(directoryRefresh.state().streamState());

                processServiceRefresh(directoryRefresh.serviceList(), error);
                if (service.action() == MapEntryActions.DELETE)
                {
                    error.text("processResponse(): Failed: directory service is deleted");
                    return CodecReturnCodes.FAILURE;
                }
                break;

            case MsgClasses.UPDATE:
                System.out.println("Received Source Directory Update");
                ret = directoryUpdate.decode(dIter, msg);

                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("Error decoding directory update: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }

                System.out.println(directoryUpdate.toString());
                processServiceUpdate(directoryUpdate.serviceList(), error);
                if (service.action() == MapEntryActions.DELETE)
                {
                    error.text("processResponse(): Failed: directory service is deleted");
                    return CodecReturnCodes.FAILURE;
                }
                break;

            case MsgClasses.STATUS:
                ret = directoryStatus.decode(dIter, msg);

                if (ret != CodecReturnCodes.SUCCESS)
                {
                    error.text("Error decoding directory status: <" + CodecReturnCodes.toString(ret) + ">");
                    return ret;
                }
                System.out.println("Received Source Directory Status:");
                System.out.println(directoryStatus.toString());

                if (directoryStatus.checkHasState())
                {
                    this.state.dataState(directoryStatus.state().dataState());
                    this.state.streamState(directoryStatus.state().streamState());
                }
                break;

            default:
                error.text("Received Unhandled Source Directory Msg Class: " + msg.msgClass());
                return CodecReturnCodes.FAILURE;
        }

        return ret;
    }

    private void processServiceUpdate(List<Service> serviceList, Error error)
    {
        for (Service rdmService : serviceList)
        {
            if (rdmService.action() == MapEntryActions.DELETE && rdmService.serviceId() == service.serviceId() )
            {
                service.action(MapEntryActions.DELETE);
            }

            if(rdmService.info().serviceName().toString() != null)
            {
                System.out.println("Received serviceName: " + rdmService.info().serviceName());
            }

            // update service cache - assume cache is built with previous refresh message
            if (rdmService.serviceId() == service.serviceId())
            {
                rdmService.copy(service);
            }
        }
    }


    private void processServiceRefresh(List<Service> serviceList, Error error)
    {
        for (Service rdmService : serviceList)
        {
            if (rdmService.action() == MapEntryActions.DELETE && rdmService.serviceId() == service.serviceId() )
            {
                service.action(MapEntryActions.DELETE);
            }

            if(rdmService.info().serviceName().toString() != null)
            {
                System.out.println("Received serviceName: " + rdmService.info().serviceName());
            }
            // cache service requested by the application
            if (rdmService.info().serviceName().equals(serviceName))
            {
                rdmService.copy(service);
            }
        }
    }

    /**
     * Close the source directory stream.
     *
     * @param chnl - The channel to send a source directory close to
     * @param error the error
     * @return the int
     */
    public int closeStream(ChannelSession chnl, Error error)
    {
        /*
         * we only want to close a stream if it was not already closed (e.g.
         * rejected by provider, closed via refresh or status)
         */
        if (state.isFinal())
            return CodecReturnCodes.SUCCESS;

        //get a buffer for the source directory close
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false,
                error);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        // encode source directory close
        directoryClose.clear();
        directoryClose.streamId(SRCDIR_STREAM_ID);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
        int ret = directoryClose.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeSourceDirectoryClose(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        ret = chnl.write(msgBuf, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            return ret;
        }

        service.clear(); //stream is closed. cached service information is invalid.

        return TransportReturnCodes.SUCCESS;
    }
    @Override
    public int serviceNameToId(String serviceName, JsonConverterError error) {

        if (this.serviceName.toString().equals(serviceName))
            return service.serviceId();

        return -1;
    }

    @Override
    public String serviceIdToName(int id, JsonConverterError error) {

        if (service.serviceId() == id)
            return this.serviceName.toString();

        return null;
    }
}
