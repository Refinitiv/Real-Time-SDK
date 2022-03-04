/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.converter;

import com.fasterxml.jackson.databind.JsonNode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

class JsonConverterState {

    private Buffer currentBufferData = CodecFactory.createBuffer();
    private JsonNode currentRoot;
    private JsonNode workingNode;
    private JsonNode failedNode;
    private byte[] failedMessage;
    private int arrayCounter;
    private int entryCounter;

    public Buffer getCurrentBufferData() {
        return currentBufferData;
    }

    public JsonNode getCurrentRoot() {
        return currentRoot;
    }

    public void setCurrentRoot(JsonNode currentRoot) {
        this.currentRoot = currentRoot;
    }

    public JsonNode getWorkingNode() {
        return workingNode;
    }

    public void setWorkingNode(JsonNode workingNode) {
        this.workingNode = workingNode;
    }

    public JsonNode getFailedNode() {
        return failedNode;
    }

    public void setFailedNode(JsonNode failedNode) {
        this.failedNode = failedNode;
    }

    public int getArrayCounter() {
        return arrayCounter;
    }

    public void setArrayCounter(int arrayCounter) {
        this.arrayCounter = arrayCounter;
    }

    public int getEntryCounter() {
        return entryCounter;
    }

    public void setEntryCounter(int entryCounter) {
        this.entryCounter = entryCounter;
    }

    public void clear() {
        currentBufferData.clear();
        currentRoot = null;
        workingNode = null;
        failedNode = null;
        arrayCounter = 0;
        entryCounter = 0;
        failedMessage = null;
    }

    public byte[] getFailedMessage() {
        return failedMessage;
    }

    public void setFailedMessage(byte[] failedMessage) {
        this.failedMessage = failedMessage;
    }
}

