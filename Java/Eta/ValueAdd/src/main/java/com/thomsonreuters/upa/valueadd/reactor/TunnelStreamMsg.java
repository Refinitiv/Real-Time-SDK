package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.RequestMsg;

/* Implements the protocol for TunnelStream messages.
 * The interfaces for each class of protocol message are members of the TunnelMsgInt
 * class to make them private to the package. */

abstract class TunnelStreamMsg
{
	class OpCodes
	{
		static final int INIT = 0;
		static final int DATA = 1;
		static final int ACK = 2;
		static final int RETRANS = 3;
		static final int REQUEST = 4;
	} 
	
	abstract int opCode();
	abstract void opCode(int opCode);

	abstract int streamId();
	abstract void streamId(int streamId);

	abstract int domainType();
	abstract void domainType(int domainType);

	abstract int serviceId();
	abstract void serviceId(int serviceId);

    abstract String name();
    abstract void name(String name);

    abstract ClassOfService classOfService();
    abstract void classOfService(ClassOfService classOfService);

	interface TunnelStreamRequest
	{
		public void clearRequest();

		/* Recommended buffer size for the populated header. */
		public int requestBufferSize();
		
        /* Encode tunnel stream request into the buffer set on the iterator. */
		public int encodeRequest(EncodeIterator encIter, RequestMsg tmpRequestMsg);
        
        /* Create RequestMsg for TunnelStream with an encoded ClassOfService. */
        public int encodeRequestAsMsg(EncodeIterator encIter, Buffer classOfServiceBuffer,
                RequestMsg requestMsg);
	}


	interface TunnelStreamData
	{
		class Flags
		{
			static final int NONE = 0x00;
			static final int FRAGMENTED = 0x01;
		} 

		public void clearData();

		/* Data message is retransmitted. */
		public void clearRetrans();

		/* Recommended buffer size for the populated header. */
		public int dataMsgBufferSize();

		public int encodeDataInit(EncodeIterator encIter);
		public int encodeDataComplete(EncodeIterator encIter);
		
	    public int seqNum();
	    public void seqNum(int seqNum);
	    
	    public int dataMsgFlag();
	    public void dataMsgFlag(int flag);

	    public long totalMsgLength();
	    public void totalMsgLength(long totalMsgLength);

	    public long fragmentNumber();
	    public void fragmentNumber(long fragmentNumber);

	    public int messageId();
	    public void messageId(int messageId);

	    public int containerType();
	    public void containerType(int containerType);
	}

	interface TunnelStreamAck
	{
	    public int seqNum();
	    public void seqNum(int seqNum);
	    
	    public int flag();

	    public int recvWindow();
	    public void recvWindow(int recvWindow);
		
		public void clearAck();

		/* Recommended buffer size for the populated header. */
		public int ackBufferSize(AckRangeList nakRangeList);
		
		public int encodeAck(EncodeIterator encIter, AckRangeList ackRangeList, AckRangeList nakRangeList, int actionOpCode);
	}

	abstract int decode(DecodeIterator decIter, GenericMsg genericMsg, AckRangeList ackRangeList, AckRangeList nakRangeList);

	abstract String xmlDumpBegin(AckRangeList ackRangeList, AckRangeList nakRangeList);
	abstract String xmlDumpEnd();
}
