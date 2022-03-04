/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.niprovider;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FilterEntryActions;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.QosRates;
import com.refinitiv.eta.codec.QosTimeliness;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.examples.common.ChannelSession;
import com.refinitiv.eta.examples.common.NIProviderDictionaryHandler;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;

/**
 * This is the source directory handler for the ETA NIProvider application. It
 * provides methods for sending the source directory refresh to a consumer
 * (e.g. ADH). Methods for setting the service name, getting the service
 * information, and closing a source directory stream are also provided.
 */
public class DirectoryHandler
{
    private static final int SRCDIR_STREAM_ID = -1;
    private static final int OPEN_LIMIT = 5;
    private static final String VENDOR = "Refinitiv";
    private static final String LINK_NAME = "NI_PUB";

    private static final long FILTER_TO_REFRESH = Directory.ServiceFilterFlags.INFO
            | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD
            | Directory.ServiceFilterFlags.LINK;

    public static int TRANSPORT_BUFFER_SIZE_REFRESH = ChannelSession.MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = ChannelSession.MAX_MSG_SIZE;

    //service name requested by application 
    private Buffer serviceName = CodecFactory.createBuffer();

    //service Id requested by application
    private int serviceId = 1;

    private Service service = DirectoryMsgFactory.createService();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();

    private DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
    {
        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
    }

    private DirectoryClose directoryClose = (DirectoryClose)DirectoryMsgFactory.createMsg();
    {
        directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
    }

    /**
     * Sets the service name requested by the application.
     * 
     * @param servicename The service name requested by the application.
     */
    public void serviceName(String servicename)
    {
        serviceName.data(servicename);
    }

    /**
     * Sets the service Id requested by the application.
     * 
     * @param serviceId The service Id requested by the application.
     */
    public void serviceId(int serviceId)
    {
        this.serviceId = serviceId;
    }

    /**
     * Checks if the service is up. The service will be up once the refresh is
     * sent.
     * 
     * @return true if the service is up.
     */
    public boolean isServiceUp()
    {
        if (service.checkHasState() && service.state().checkHasAcceptingRequests()
                && service.state().serviceState() == 1 && service.state().acceptingRequests() == 1)
            return true;

        return false;
    }

    /**
     * Get source directory service information.
     * 
     * @return Service.
     */
    public Service serviceInfo()
    {
        return service;
    }

    /**
     * Sends a source directory refresh to a channel. This consists of getting a
     * message buffer, encoding the source directory refresh, sending the source
     * directory refresh to the server.
     *
     * @param chnl The channel to send a source directory refresh to.
     * @param error Populated if an error occurs.
     * @return the int
     */
    public int sendRefresh(ChannelSession chnl, com.refinitiv.eta.transport.Error error)
    {
        TransportBuffer msgBuf = null;

        //get a buffer for the source directory request 
        msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_REFRESH, false, error);
        if (msgBuf == null)
            return CodecReturnCodes.FAILURE;

        //encode source directory request
        directoryRefresh.clear();
        directoryRefresh.streamId(SRCDIR_STREAM_ID);

        //clear cache
        directoryRefresh.applyClearCache();

        //state information for response message
        Buffer text = CodecFactory.createBuffer();
        text.data("Source Directory Refresh Completed");
        directoryRefresh.state().clear();
        directoryRefresh.state().streamState(StreamStates.OPEN);
        directoryRefresh.state().dataState(DataStates.OK);
        directoryRefresh.state().code(StateCodes.NONE);
        directoryRefresh.state().text(text);

        //attribInfo information for response message
        directoryRefresh.filter(FILTER_TO_REFRESH);

        //service
        service.clear();
        service.action(MapEntryActions.ADD);

        //set the service Id (map key)
        service.serviceId(serviceId);

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.INFO) != 0)
        {
            service.applyHasInfo();
            service.info().action(FilterEntryActions.SET);

            //vendor 
            service.info().applyHasVendor();
            service.info().vendor().data(VENDOR);


            //service name - required
            service.info().serviceName().data(serviceName.toString());

            //Qos Range is not supported
            service.info().applyHasSupportsQosRange();
            service.info().supportsQosRange(0);
          

            //capabilities - required
            service.info().capabilitiesList().add((long)DomainTypes.MARKET_PRICE);
            service.info().capabilitiesList().add((long)DomainTypes.MARKET_BY_ORDER);

            //qos
            service.info().applyHasQos();
            Qos qos = CodecFactory.createQos();
            qos.rate(QosRates.TICK_BY_TICK);
            qos.timeliness(QosTimeliness.REALTIME);
            service.info().qosList().add(qos);


            //dictionary used
            service.info().applyHasDictionariesUsed();
            service.info().dictionariesUsedList().add(NIProviderDictionaryHandler.FIELD_DICTIONARY_DOWNLOAD_NAME);
            service.info().dictionariesUsedList().add(NIProviderDictionaryHandler.ENUM_TYPE_DOWNLOAD_NAME);

            //isSource = Service is provided directly from original publisher
            service.info().applyHasIsSource();
            service.info().isSource(1);
            
            /*
             * itemList - Name of SymbolList that includes all of the items that
             * the publisher currently provides. Blank for this example
             */
            service.info().applyHasItemList();
            service.info().itemList().data("");


            service.info().applyHasAcceptingConsumerStatus();
            //accepting customer status = no
            service.info().acceptingConsumerStatus(0);

            service.info().applyHasSupportsOutOfBandSnapshots();
            //supports out of band snapshots = no
            service.info().supportsOutOfBandSnapshots(0);
        }

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.STATE) != 0)
        {
            service.applyHasState();
            service.state().action(FilterEntryActions.SET);

            //service state
            service.state().serviceState(1);

            //accepting requests
            service.state().applyHasAcceptingRequests();
            service.state().acceptingRequests(1);


            //status
            service.state().applyHasStatus();
            service.state().status().dataState(DataStates.OK);
            service.state().status().streamState(StreamStates.OPEN);
            service.state().status().code(StateCodes.NONE);
            service.state().status().text().data("OK");
        }

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.LOAD) != 0)
        {
            service.applyHasLoad();
            service.load().action(FilterEntryActions.SET);

            //open limit
            service.load().applyHasOpenLimit();
            service.load().openLimit(OPEN_LIMIT);

            //load factor
            service.load().applyHasLoadFactor();
            service.load().loadFactor(1);
        }

        if ((FILTER_TO_REFRESH & Directory.ServiceFilterFlags.LINK) != 0)
        {
            service.applyHasLink();
            service.link().action(FilterEntryActions.SET);

            Service.ServiceLink serviceLink = new Service.ServiceLink();

            //link name - Map Entry Key
            serviceLink.name().data(LINK_NAME);

            //link type
            serviceLink.applyHasType();
            serviceLink.type(Directory.LinkTypes.INTERACTIVE);

            //link state
            serviceLink.linkState(Directory.LinkStates.UP);

            //link code
            serviceLink.applyHasCode();
            serviceLink.linkCode(Directory.LinkCodes.OK);

            //link text
            serviceLink.applyHasText();
            serviceLink.text().data("Link state is up");

            service.link().linkList().add(serviceLink);
        }

        directoryRefresh.serviceList().add(service);

        //encode directory request
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
        int ret = directoryRefresh.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeDirectoryRefresh(): Failed <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        //send source directory request
        return chnl.write(msgBuf, error);
    }

    /**
     * Close the source directory stream.
     *
     * @param chnl The channel to send a source directory close to.
     * @param error the error
     * @return the int
     */
    public int closeStream(ChannelSession chnl, com.refinitiv.eta.transport.Error error)
    {
        service.clear(); // mark service as down.

        //get a buffer for the source directory close */
        TransportBuffer msgBuf = chnl.getTransportBuffer(TRANSPORT_BUFFER_SIZE_CLOSE, false, error);
        if (msgBuf == null)
        {
            return CodecReturnCodes.FAILURE;
        }

        //encode source directory close
        directoryClose.clear();
        directoryClose.streamId(SRCDIR_STREAM_ID);
        encIter.clear();
        encIter.setBufferAndRWFVersion(msgBuf, chnl.channel().majorVersion(), chnl.channel().minorVersion());
        int ret = directoryClose.encode(encIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("encodeSourceDirectoryClose(): Failed <" + CodecReturnCodes.toString(ret)
                    + ">");
            return ret;
        }

        //send close
        return chnl.write(msgBuf, error);
    }
}

