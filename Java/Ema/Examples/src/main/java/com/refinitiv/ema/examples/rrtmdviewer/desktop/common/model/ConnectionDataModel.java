/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.common.model;

import com.refinitiv.ema.examples.rrtmdviewer.desktop.specified_endpoint.SpecifiedEndpointConnectionTypes;

public class ConnectionDataModel {

    private SpecifiedEndpointConnectionTypes connectionType;
    private String protocolList;

    public SpecifiedEndpointConnectionTypes getConnectionType() {
        return connectionType;
    }

    public String getProtocolList() {
        return protocolList;
    }

    public boolean isEncrypted() {
        return connectionType.equals(SpecifiedEndpointConnectionTypes.ENCRYPTED_SOCKET)
                || connectionType.equals(SpecifiedEndpointConnectionTypes.ENCRYPTED_WEBSOCKET);
    }

    private ConnectionDataModel(SpecifiedEndpointConnectionTypes connType,
                                String protocolList) {
        this.connectionType = connType;
        this.protocolList = protocolList;
    }

    public static ConnectionDataModelBuilder builder() {
        return new ConnectionDataModelBuilder();
    }

    public static class ConnectionDataModelBuilder {
        private SpecifiedEndpointConnectionTypes connectionType;
        private String protocolList;

        public ConnectionDataModelBuilder connectionType(SpecifiedEndpointConnectionTypes connectionType) {
            this.connectionType = connectionType;
            return this;
        }

        public ConnectionDataModelBuilder protocolList(String protocolList) {
            this.protocolList = protocolList;
            return this;
        }

        public ConnectionDataModel build() {
            return new ConnectionDataModel(connectionType, protocolList);
        }
    }
}
