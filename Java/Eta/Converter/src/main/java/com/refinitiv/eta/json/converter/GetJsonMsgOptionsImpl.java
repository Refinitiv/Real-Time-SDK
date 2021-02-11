package com.refinitiv.eta.json.converter;

class GetJsonMsgOptionsImpl implements GetJsonMsgOptions {

    private Integer streamId = null;
    private int transportProtocol;
    private int jsonProtocolType;
    private boolean solicited;
    private boolean isCloseMsg;

    GetJsonMsgOptionsImpl() {}

    public void clear() {
        streamId = null;
        transportProtocol = 0;
        jsonProtocolType = JsonProtocol.JSON_JPT_UNKNOWN;
        solicited = false;
        isCloseMsg = false;
    }

    public GetJsonMsgOptionsImpl streamId(int streamId) {
        this.streamId = streamId;
        return this;
    }

    public GetJsonMsgOptionsImpl transportProtocolType(int protocol) {
        this.transportProtocol = protocol;
        return this;
    }

    public GetJsonMsgOptionsImpl jsonProtocolType(int protocol) {
        this.jsonProtocolType = protocol;
        return this;
    }

    public GetJsonMsgOptionsImpl isSolicited(boolean solicited) {
        this.solicited = solicited;
        return this;
    }

    public GetJsonMsgOptionsImpl isCloseMsg(boolean isClose) {
        this.isCloseMsg = isClose;
        return this;
    }

    public Integer getStreamId() {
        return streamId;
    }

    public int getTransportProtocol() {
        return transportProtocol;
    }

    public int getJsonProtocolType() {
        return jsonProtocolType;
    }

    public boolean isCloseMsg() {
        return isCloseMsg;
    }

    public boolean isSolicited() {
        return solicited;
    }
}
