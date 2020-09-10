package com.rtsdk.eta.examples.genericprov;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileChannel;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.GenericMsg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.examples.common.GenericResponseStatusFlags;
import com.rtsdk.eta.shared.ProviderSession;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;
import com.rtsdk.eta.transport.TransportFactory;
import com.rtsdk.eta.transport.TransportReturnCodes;

/** 
 * This class sends a file response for the file name requested.
 */
public class SendFileResponseThread implements Runnable
{
    private static final int MAX_MSG_OVERHEAD = 512;

    private Channel _channel;
    private String _fileName;
    private int _streamId;
    private int _domain;
    private ProviderSession _providerSession;
    private EncodeIterator _encIter;
    private GenericMsg _fileResponseMsg;
    private Error _error;

    public SendFileResponseThread(Channel channel, String fileName, int streamId, int domain, ProviderSession providerSession)
    {
        _channel = channel;
        _fileName = fileName;
        _streamId = streamId;
        _domain = domain;
        _providerSession = providerSession;
        _encIter = CodecFactory.createEncodeIterator();
        _fileResponseMsg = (GenericMsg)CodecFactory.createMsg();
        _error = TransportFactory.createError();
    }
    
    @Override
    /** Runs the thread to send a file response. */
    public void run()
    {
        // use MsgKey to retrieve file name
        if (_fileName.length() > 0)
        {
            // send file to generic consumer
            sendFileResponse(_channel, _fileName);
        }
        else
        {
            // send error response
            sendErrorResponse(_channel, null, "Invalid file name");
        }
    }
    
    /* Send a file response. */
    private void sendFileResponse(Channel channel, String fileName)
    {
        try
        {
            int ret = 0;
            RandomAccessFile file = new RandomAccessFile(fileName, "r");
            FileChannel fileInput = file.getChannel();
            TransportBuffer buffer = channel.getBuffer((int)(file.length() + MAX_MSG_OVERHEAD), false, _error);
            
            _encIter.clear();
            _encIter.setBufferAndRWFVersion(buffer, channel.majorVersion(), channel.minorVersion());
            _fileResponseMsg.clear();
            _fileResponseMsg.msgClass(MsgClasses.GENERIC);
            _fileResponseMsg.domainType(_domain);
            _fileResponseMsg.streamId(_streamId);
            _fileResponseMsg.applyMessageComplete();
            // use MsgKey for file name in response
            _fileResponseMsg.applyHasMsgKey();
            _fileResponseMsg.msgKey().applyHasName();
            _fileResponseMsg.msgKey().name().data(fileName);
            _fileResponseMsg.containerType(DataTypes.OPAQUE);
            if ((ret = _fileResponseMsg.encodeInit(_encIter, 0)) != CodecReturnCodes.ENCODE_CONTAINER)
            {
                System.out.println("GenericMsg encoding failure: " + ret);
                System.exit(-1);
            }
            
            // encode generic response status flag of success first
            buffer.data().put((byte)GenericResponseStatusFlags.SUCCESS);
            
            // encode file next
            fileInput.read(buffer.data());
            
            if ((ret = _fileResponseMsg.encodeComplete(_encIter, true)) != CodecReturnCodes.SUCCESS)
            {
                System.out.println("GenericMsg encoding failure: " + ret);
                System.exit(-1);
            }
            
            // send message
            if (( ret = _providerSession.write(channel, buffer, _error)) != TransportReturnCodes.SUCCESS)
            {
                System.out.println("GenericMsg write failure: " + ret);
                System.exit(-1);
            }
            
            file.close();
            fileInput.close();
        }
        catch (IOException e)
        {
            sendErrorResponse(channel, fileName, e.getLocalizedMessage());
        }
    }

    /* Send an error response. */
    private void sendErrorResponse(Channel channel, String fileName, String errorText)
    {
        int ret = 0;
        TransportBuffer buffer = channel.getBuffer(errorText.length() + MAX_MSG_OVERHEAD, false, _error);
        
        _encIter.clear();
        _encIter.setBufferAndRWFVersion(buffer, channel.majorVersion(), channel.minorVersion());
        _fileResponseMsg.clear();
        _fileResponseMsg.msgClass(MsgClasses.GENERIC);
        _fileResponseMsg.domainType(_domain);
        _fileResponseMsg.streamId(_streamId);
        _fileResponseMsg.applyMessageComplete();
        // use MsgKey for file name in response
        if (fileName != null)
        {
            _fileResponseMsg.applyHasMsgKey();
            _fileResponseMsg.msgKey().applyHasName();
            _fileResponseMsg.msgKey().name().data(fileName);
        }
        _fileResponseMsg.containerType(DataTypes.OPAQUE);
        if ((ret = _fileResponseMsg.encodeInit(_encIter, 0)) != CodecReturnCodes.ENCODE_CONTAINER)
        {
            System.out.println("GenericMsg encoding failure: " + ret);
            System.exit(-1);
        }
        
        // encode generic response status flag of failure first
        buffer.data().put((byte)GenericResponseStatusFlags.FAILURE);
        
        // encode error text next
        for (int i = 0; i < errorText.length(); i++)
        {
            buffer.data().put((byte)errorText.charAt(i));
        }

        if ((ret = _fileResponseMsg.encodeComplete(_encIter, true)) != CodecReturnCodes.SUCCESS)
        {
            System.out.println("GenericMsg encoding failure: " + ret);
            System.exit(-1);
        }
        
        // send message
        if (( ret = _providerSession.write(channel, buffer, _error)) != TransportReturnCodes.SUCCESS)
        {
            System.out.println("GenericMsg write failure: " + ret);
            System.exit(-1);
        }
    }
}
