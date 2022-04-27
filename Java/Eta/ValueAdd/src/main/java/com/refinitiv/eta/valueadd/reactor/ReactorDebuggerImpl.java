package com.refinitiv.eta.valueadd.reactor;

import java.io.ByteArrayOutputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

class ReactorDebuggerImpl implements ReactorDebugger {

    private final static int SB_START_CAPACITY = 512;

    private OutputStream _stream;
    private int _numOfCloseCalls;
    private int _numOfDispatchCalls;
    private SimpleDateFormat _formatter = new SimpleDateFormat("HH:mm:ss.SSS");
    private boolean _streamProvided;
    private StringBuilder sb = new StringBuilder(SB_START_CAPACITY);
    private int _capacity;
    private boolean _noLimitSet;


    ReactorDebuggerImpl(OutputStream outputStream, int streamCapacity) {
        if (outputStream != null) {
            _stream = outputStream;
            _streamProvided = true;
        } else {
            _noLimitSet = streamCapacity == ReactorDebuggerOptions.NO_LIMIT_SET;
            _capacity = Math.max(ReactorDebuggerOptions.DEFAULT_CAPACITY, streamCapacity);
            _stream = new ByteArrayOutputStream(_capacity);
        }
    }

    ReactorDebuggerImpl(int streamCapacity) {
        _noLimitSet = streamCapacity == ReactorDebuggerOptions.NO_LIMIT_SET;
        _capacity = Math.max(ReactorDebuggerOptions.DEFAULT_CAPACITY, streamCapacity);;
        _stream = new ByteArrayOutputStream(_capacity);
    }

    ReactorDebuggerImpl(OutputStream outputStream) {
        if (outputStream != null) {
            _stream = outputStream;
            _streamProvided = true;
        } else {
            _capacity = ReactorDebuggerOptions.DEFAULT_CAPACITY;
            _stream = new ByteArrayOutputStream(ReactorDebuggerOptions.DEFAULT_CAPACITY);
        }
    }

    @Override
    public synchronized void writeDebugInfo(String msg, Object... args) {
        try {
            sb.delete(0, sb.length());
            sb.append('[');
            sb.append(_formatter.format(new Date()));
            sb.append(" : ");
            sb.append(String.format(msg, args));
            byte[] bytes = sb.toString().getBytes();
            if (_streamProvided || _noLimitSet || (bytes.length + ((ByteArrayOutputStream)_stream).size()) <= _capacity) {
                _stream.write(bytes);
                _stream.flush();
            }
        } catch (Exception e) {
            System.out.println("Error writing debug to OutputStream: " + e.getMessage());
        }
    }

    @Override
    public synchronized void incNumOfCloseCalls() {
        _numOfCloseCalls++;
    }

    @Override
    public synchronized int getNumOfCloseCalls() {
        return _numOfCloseCalls;
    }

    @Override
    public synchronized void incNumOfDispatchCalls() {
        _numOfDispatchCalls++;
    }

    @Override
    public synchronized int getNumOfDispatchCalls() {
        return _numOfDispatchCalls;
    }

    @Override
    public OutputStream getOutputStream() {
        return _stream;
    }

    @Override
    public synchronized byte[] toByteArray() {
        if (_streamProvided && !(_stream instanceof ByteArrayOutputStream)) {
            return null;
        } else {
            ByteArrayOutputStream byteStream = (ByteArrayOutputStream)_stream;
            byte[] result = byteStream.toByteArray();
            byteStream.reset();
            return result;
        }
    }
}
