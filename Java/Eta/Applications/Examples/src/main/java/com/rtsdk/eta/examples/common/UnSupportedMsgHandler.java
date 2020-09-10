package com.rtsdk.eta.examples.common;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StatusMsg;
import com.rtsdk.eta.codec.StatusMsgFlags;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.transport.Channel;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;
import com.rtsdk.eta.transport.TransportReturnCodes;
import com.rtsdk.eta.shared.ProviderSession;

/**
 * This is the implementation of sending unsupported status message for the
 * interactive provider and generic provider applications.
 */
public class UnSupportedMsgHandler
{
    private static final int MSG_SIZE = 1024;
    
    private ProviderSession _providerSession;
    private StatusMsg _statusMsg =  (StatusMsg)CodecFactory.createMsg();
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();

    public UnSupportedMsgHandler(ProviderSession providerClientSession)
    {
        _providerSession = providerClientSession;
    }

	public int processRequest(Channel chnl, Msg msg, Error error)
	{
		switch(msg.msgClass())
		{
			case MsgClasses.REQUEST:
				return sendStatus(chnl, msg, error);

			case MsgClasses.CLOSE:
				System.out.println("Received close message with streamId=" + msg.streamId() + " and unsupported Domain '" + msg.domainType() + "'");
				return CodecReturnCodes.SUCCESS;

			default:
				System.out.println("Received unhandled Msg Class: " + MsgClasses.toString(msg.msgClass()) + " with streamId=" + msg.streamId() + " and unsupported Domain '" + msg.domainType() + "'");
				return CodecReturnCodes.SUCCESS;
		}
	}

    /*
     * Encodes the not supported status. Returns success if encoding succeeds or
     * failure if encoding fails. chnl - The channel to send not supported
     * status message to msg - The partially decoded request message msgBuf -
     * The message buffer to encode the not supported status into
     */
    private int encodeStatus(Channel chnl, Msg requestMsg, TransportBuffer msgBuf, Error error)
    {
        _statusMsg.clear();
   
        /* set-up message */
        _statusMsg.msgClass(MsgClasses.STATUS);
        _statusMsg.streamId(requestMsg.streamId());
        _statusMsg.domainType(requestMsg.domainType());
        _statusMsg.containerType(DataTypes.NO_DATA);
        _statusMsg.flags(StatusMsgFlags.HAS_STATE);
        _statusMsg.state().streamState(StreamStates.CLOSED);
        _statusMsg.state().dataState(DataStates.SUSPECT);
        _statusMsg.state().code(StateCodes.USAGE_ERROR);
        _statusMsg.state().text().data("Request rejected for stream id '" + requestMsg.streamId() + "' - domain type '" + requestMsg.domainType() + "' is not supported");

        /* clear encode iterator */
        _encodeIter.clear();

        /* encode message */
        
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(), chnl.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            error.text("EncodeIterator.setBufferAndRWFVersion() failed with return code: " + ret);
            return ret;
        }

        ret = _statusMsg.encode(_encodeIter);
        if (ret  != CodecReturnCodes.SUCCESS)
        {
            error.text("StatusMsg.encode() failed with return code: " + CodecReturnCodes.toString(ret));
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /**
     * Encodes and sends the not supported status. Returns success if encoding
     * succeeds or failure if encoding fails.
     * 
     * @param channel - The channel to send not supported status message to
     * @param requestMsg - The partially decoded request message
     * @param error - UPA error when encoding or sending of status message
     *            fails.
     * @return {@link CodecReturnCodes}
     */
    public int sendStatus(Channel channel, Msg requestMsg, Error error)
    {
        /* get a buffer for the not supported status */
        TransportBuffer msgBuf = channel.getBuffer(MSG_SIZE, false, error);
        if (msgBuf == null)
        {
            return TransportReturnCodes.FAILURE;
        }

        /* encode not supported status */
        int ret = encodeStatus(channel, requestMsg, msgBuf, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            return ret;
        }

        System.out.println("Rejecting Item Request with streamId=" + requestMsg.streamId() + " Reason: Domain '" + requestMsg.domainType() + "' Not Supported");

        /* send not supported status */
        return _providerSession.write(channel, msgBuf, error);
    }
}
