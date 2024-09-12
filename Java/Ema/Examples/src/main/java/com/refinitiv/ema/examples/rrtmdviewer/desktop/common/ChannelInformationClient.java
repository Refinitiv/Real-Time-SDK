/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022,2024 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.common;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview.TabViewModel;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.ConnectionTypes;

public class ChannelInformationClient implements OmmConsumerClient {

    private int configScenario;

    private int connectionType = -1;

    private int encryptedType = -1;

    private int protocolType;

    private String hostName = "";

    private int channelState = ChannelInformation.ChannelState.INACTIVE;

    private int port;

    private boolean hasChanged;

    private StringBuilder stringBuilder = new StringBuilder(150);

    private TabViewModel tabViewModel;

    private long lastRttLatency;

    public void setTabViewModel(TabViewModel tabViewModel)
    {
        this.tabViewModel = tabViewModel;
    }

    public static class ConfigScenario
    {
        public static final int DISCOVERY_ENDPOINT = 1;

        public static final int SPECIFY_ENDPOINT = 2;
    }

    public void setConfigScenario(int configScenario) {
        this.configScenario = configScenario;
    }

    public void clear()
    {
        configScenario = 0;
        connectionType = -1;
        encryptedType = -1;
        protocolType = 0;
        hostName = "";
        channelState = ChannelInformation.ChannelState.INACTIVE;
        port = 0;
    }

    private void applyChannelInformation(ChannelInformation channelInformation) {

        hasChanged = false;

        if(connectionType != channelInformation.connectionType())
        {
            connectionType = channelInformation.connectionType();
            hasChanged = true;
        }

        if(connectionType == ChannelInformation.ConnectionType.ENCRYPTED && encryptedType != channelInformation.encryptedConnectionType())
        {
            encryptedType = channelInformation.encryptedConnectionType();
            hasChanged = true;
        }

        if(protocolType != channelInformation.protocolType())
        {
       	    protocolType = channelInformation.protocolType();
            hasChanged = true;
        }

        if(channelInformation.hostname() != null && hostName.equals(channelInformation.hostname()) == false)
        {
            hostName = channelInformation.hostname();
            hasChanged = true;
        }

        if(port != channelInformation.port())
        {
            port = channelInformation.port();
            hasChanged = true;
        }

        if(channelState != channelInformation.channelState())
        {
            channelState = channelInformation.channelState();
            hasChanged = true;
        }

        /* Notify the listener */
        notifyListener();
    }

    private void applyRtt(GenericMsg genericMsg) {
        hasChanged = false;
        if (genericMsg.domainType() == DomainTypes.LOGIN && genericMsg.payload().dataType() == DataTypes.ELEMENT_LIST) {
            ElementList data = genericMsg.payload().elementList();
            for ( ElementEntry elem : data) {
                if (elem.name().equals(EmaRdm.ENAME_LATENCY)) {
                    hasChanged = true;
                    this.lastRttLatency = elem.uintValue();
                }
            }
        }
        notifyListener();
    }

    private void notifyListener() {
        if(hasChanged)
        {
            if(tabViewModel != null)
            {
                tabViewModel.getConnectionProperty().set(!tabViewModel.getConnectionProperty().get());
            }
        }
    }

    public int getChannelState()
    {
        return channelState;
    }

    @Override
    public String toString()
    {
        stringBuilder.setLength(0);

        if(configScenario == ConfigScenario.SPECIFY_ENDPOINT)
        {
            stringBuilder.append("Specify Endpoint, ");
        }
        else if (configScenario == ConfigScenario.DISCOVERY_ENDPOINT)
        {
            stringBuilder.append("Discovery Endpoint, ");
        }

        if(connectionType == ChannelInformation.ConnectionType.ENCRYPTED )
        {
            stringBuilder.append("ConnectionType=Encrypted ");
            stringBuilder.append(ConnectionTypes.toString(encryptedType) + ", ");
        }
        else {
            stringBuilder.append("ConnectionType=" + ConnectionTypes.toString(connectionType) + ", ");
        }

        if(protocolType == ChannelInformation.ProtocolType.RWF)
        {
            stringBuilder.append("ProtocolType=Refinitiv wire format, ");
        }
        else if (protocolType == ChannelInformation.ProtocolType.JSON)
        {
            stringBuilder.append("ProtocolType=Refinitiv JSON format, ");
        }

        stringBuilder.append("Host=" + hostName + ", ");

        stringBuilder.append("Port=" + port + ", ");

        stringBuilder.append("State=");

        switch (channelState)
        {
            case ChannelInformation.ChannelState.CLOSED:
                stringBuilder.append("Closed");
                break;
            case ChannelInformation.ChannelState.INACTIVE:
                stringBuilder.append("Inactive");
                break;
            case ChannelInformation.ChannelState.INITIALIZING:
                stringBuilder.append("Initializing");
                break;
            case ChannelInformation.ChannelState.ACTIVE:
                stringBuilder.append("Active");
                break;
            default:
                stringBuilder.append(stringBuilder);
                break;
        }

        stringBuilder.append(", Last RTT Latency(usec)=");
        if (lastRttLatency <= 0) {
            stringBuilder.append("Not calculated");
        } else {
            stringBuilder.append(lastRttLatency / 1000);
        }
        return stringBuilder.toString();
    }

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent) {
    }

    @Override
    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent) {

    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent) {
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {
        applyRtt(genericMsg);
    }

    @Override
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {

    }

    @Override
    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {
        applyChannelInformation(consumerEvent.channelInformation());
    }
}
