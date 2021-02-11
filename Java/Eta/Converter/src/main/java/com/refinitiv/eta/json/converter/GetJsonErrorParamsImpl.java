package com.refinitiv.eta.json.converter;

class GetJsonErrorParamsImpl implements GetJsonErrorParams {

    private String file;
    private int line;
    private int streamId;
    private String text;

    GetJsonErrorParamsImpl() {}

    @Override
    public void setFile(String file) {
        this.file = file;
    }

    @Override
    public void setLine(int line) {
        this.line = line;
    }

    @Override
    public void setText(String text) {
        this.text = text;
    }

    @Override
    public void setStreamId(int streamId) {
        this.streamId = streamId;
    }

    @Override
    public String getFile() {
        return file;
    }

    @Override
    public String getText() {
        return text;
    }

    @Override
    public int getLine() {
        return line;
    }

    @Override
    public int getStreamId() {
        return streamId;
    }

    @Override
    public void fillParams(JsonConverterError error, int streamId) {

        line = error.getLine();
        file = error.getFile();
        text = error.getText();
        this.streamId = streamId;
    }

    @Override
    public void clear() {
        text = null;
        file = null;
        line = EMPTY_LINE_VALUE;
        streamId = 0;
    }
}
