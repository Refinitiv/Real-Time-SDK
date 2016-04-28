package com.thomsonreuters.upa.valueadd.reactor;

/* Information about a stream, obtained via TunnelStreamInt#getStateInfo */

class TunnelStreamStateInfo
{
	/* Possible states of the stream's stream. */
	enum TunnelStreamState
	{
		/* Stream has not been opened by the application. */
		NOT_OPEN,

		/* Stream needs to send request to open stream. */
		SEND_REQUEST,

		/* Stream is waiting for a response to establish the stream. */
		WAITING_REFRESH,

		/* Stream is open */
		STREAM_OPEN,

		/* Stream is closing **/
		CLOSING,

		/* Stream is to be closed **/
		SEND_FIN,
		
		/* Stream is to be closed **/
		WAIT_FIN_ACK,
		
		/* Stream is to be closed **/		
		SEND_FIN_ACK,
		
		/* Stream is to be closed **/	
		SEND_FINAL_FIN,
				
		/* Stream is to be closed **/	
		SEND_FINAL_FIN_ACK_AND_CLOSING,
	
		/* Stream is to be closed **/	
		WAIT_FINAL_FIN_ACK
		
	};
	
	TunnelStreamState _tunnelStreamState;
	int _outboundMsgsQueued;
	int _outboundMsgsWaitingForAck;
	int _outboundBytesOpen;
	int _inboundBytesOpen;
	
	/* Returns the current state of the tunnel stream.  */
	TunnelStreamState streamState()
	{
		return _tunnelStreamState;
	}

	/* Returns the number of messages queued. This may include messages generated internally.  */
	int outboundMsgsQueued()
	{
		return _outboundMsgsQueued;
	}

	/* Returns the number of messages that have been sent and are awaiting acknowledgement.
	 * This may include messages generated internally.  */
	int outboundMsgsWaitingForAck()
	{
		return _outboundMsgsWaitingForAck;
	}
	
    /* Returns the number of bytes available for sending data. */
	int outboundBytesOpen()
	{
		return _outboundBytesOpen;
	}

	/* Returns the number of bytes available for receiving. */
	int inboundBytesOpen()
	{
		return _inboundBytesOpen;
	}
	
	void set(TunnelStreamState tunnelStreamState,
			int outboundMsgsQueued,
			int outboundMsgsWaitingForAck,
			int outboundBytesOpen,
			int inboundBytesOpen)
	{
		_tunnelStreamState = tunnelStreamState;
		_outboundMsgsQueued = outboundMsgsQueued;
		_outboundMsgsWaitingForAck = outboundMsgsWaitingForAck;
		_outboundBytesOpen = outboundBytesOpen;
		_inboundBytesOpen = inboundBytesOpen;
	}

}
