/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rtviewer.desktop.common.model;

import com.refinitiv.ema.examples.rtviewer.desktop.common.AsyncResponseStatuses;
import javafx.beans.property.ObjectProperty;
import javafx.beans.property.SimpleObjectProperty;

public class AsyncResponseModel {

    private final ObjectProperty<AsyncResponseStatuses> responseStatus = new SimpleObjectProperty<>(AsyncResponseStatuses.NONE);

    private Object response;

    public AsyncResponseStatuses getResponseStatus() {
        return responseStatus.get();
    }

    public ObjectProperty<AsyncResponseStatuses> responseStatusProperty() {
        return responseStatus;
    }

    public void setResponseStatus(AsyncResponseStatuses responseStatus) {
        this.responseStatus.setValue(responseStatus);
    }

    public Object getResponse() {
        return response;
    }

    public void setResponse(Object response) {
        this.response = response;
    }

    public void clear() {
        responseStatus.setValue(AsyncResponseStatuses.NONE);
    }
}
