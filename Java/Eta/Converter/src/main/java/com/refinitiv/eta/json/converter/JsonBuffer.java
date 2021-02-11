package com.refinitiv.eta.json.converter;

class JsonBuffer {

    JsonBuffer() {}

    JsonBuffer(int startLength) {
        data = new byte[startLength];
    }

    public byte[] data;
    public int position;

    @Override
    public String toString() {

        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < position; i++)
            sb.append((char)data[i]);

        return sb.toString();
    }
}
