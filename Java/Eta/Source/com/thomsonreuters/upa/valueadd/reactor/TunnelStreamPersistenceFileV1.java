package com.thomsonreuters.upa.valueadd.reactor;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.valueadd.common.VaDoubleLinkList;

/* Class for handling a persistence file with version 1. */
class TunnelStreamPersistenceFileV1 extends TunnelStreamPersistenceFile
{
	private int _maxMsgs;
    private int _maxMsgLength;

	/* (SUBSTREAM_LINK) Persistent buffers with untransmitted outbound data. */
	private VaDoubleLinkList<TunnelStreamPersistenceBuffer>	_waitingTransmitList;

	/* (SUBSTREAM_LINK) Persistent buffers with previously transmitted outbound data. */
	private VaDoubleLinkList<TunnelStreamPersistenceBuffer>	_waitingQueueAckList;
	
	/* Static class to describe the file header. */
	private class Header
	{
		private static final int LENGTH = 0
			+ 4 /* file version */
			+ 8 /* maxMsgs */
			+ 8 /* maxMsgSize */
			+ 8 /* currentMsgCount */
			+ 4 /* lastOutSeqNum */
			+ 4 /* lastInSeqNum */
			+ 4 /* Buffer pool head */
			+ 4 /* Buffer queued list head */
			+ 4 /* Buffer wait-ack list head */
            + 28 /* Not used */
			;

		private static final int FILE_VERSION_POS = 0;
		private static final int MAX_MSGS_POS = FILE_VERSION_POS + 4;
		private static final int MAX_MSG_LENGTH_POS = MAX_MSGS_POS + 8;
		private static final int CURRENT_MSG_COUNT_POS = MAX_MSG_LENGTH_POS + 8;
		private static final int LAST_OUT_SEQ_NUM_POS = CURRENT_MSG_COUNT_POS + 8;
		private static final int LAST_IN_SEQ_NUM_POS = LAST_OUT_SEQ_NUM_POS + 4;
		private static final int POOL_HEAD_POS = LAST_IN_SEQ_NUM_POS + 4;
		private static final int WAIT_TRANSMIT_HEAD_POS = POOL_HEAD_POS + 4;
		private static final int WAITACK_LIST_HEAD_POS = WAIT_TRANSMIT_HEAD_POS + 4;

		private static final int MAX_MSGS = 1024;
	}
	

    /* Static class to describe the file's msg header */
    private class MsgHeader
    {
        private static final int LENGTH = 0
            + 4 /* _nextMsgPos */
            + 4 /* _length */
            + 2 /* _opcode */
            + 4 /* seqNum */
            + 8 /* time of arrival */
            + 8 /* time to live */
            ;

        static final int NEXT_MSG_POS = 0;
        static final int MSG_LENGTH_POS = NEXT_MSG_POS + 4;
        static final int MSG_SEQNUM_POS = MSG_LENGTH_POS + 4;
        static final int MSG_OPCODE_POS = MSG_SEQNUM_POS + 4;
        static final int TIME_QUEUED_POS = MSG_OPCODE_POS + 2;
        static final int TIME_TO_LIVE_POS = TIME_QUEUED_POS + 8;
        static final int MSG_BUFFER_POS = TIME_TO_LIVE_POS + 8;  
    }

    TunnelStreamPersistenceFileV1(TunnelSubstream tunnelSubstream, RandomAccessFile file, FileChannel fileChannel, FileLock fileLock,
            Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, boolean reset, Error error)
    {
        super(tunnelSubstream, file, fileChannel, fileLock);

        _waitingTransmitList = new VaDoubleLinkList<TunnelStreamPersistenceBuffer>();
        _waitingQueueAckList = new VaDoubleLinkList<TunnelStreamPersistenceBuffer>();

        try
        {
            long fileSize;
            if (reset) fileSize = Header.LENGTH + (MsgHeader.LENGTH + _tunnelSubstream._tunnelStream._classOfService.common().maxMsgSize() + SlicedBufferPool.TUNNEL_STREAM_HDR_SIZE) * Header.MAX_MSGS;
            else fileSize = fileChannel.size();
            _fileByteBuf = fileChannel.map(FileChannel.MapMode.READ_WRITE, 0, fileSize);
        }
        catch(IOException e)
        {
            error.errorId(ReactorReturnCodes.FAILURE);
            error.text("Failed to map persistence file to ByteBuffer");
        }

        // check if file version is valid
        // proceed if valid,  return error if invalid
        if (!reset)
        {
            int position;

            _maxMsgs = _fileByteBuf.getInt(Header.MAX_MSGS_POS);
            _maxMsgLength = _fileByteBuf.getInt(Header.MAX_MSG_LENGTH_POS);
            _tunnelSubstream._lastOutSeqNum = _fileByteBuf.getInt(Header.LAST_OUT_SEQ_NUM_POS);
            _tunnelSubstream._lastInSeqNum = _fileByteBuf.getInt(Header.LAST_IN_SEQ_NUM_POS);

            /* Load buffer lists. */
            position = _fileByteBuf.getInt(Header.POOL_HEAD_POS);
            while (position != 0)
            {
                TunnelStreamPersistenceBuffer persistBuffer = new TunnelStreamPersistenceBuffer();
                persistBuffer.filePosition(position);
                _persistentBufferPool.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);
                position = _fileByteBuf.getInt(position + MsgHeader.NEXT_MSG_POS);
            }

            position = _fileByteBuf.getInt(Header.WAIT_TRANSMIT_HEAD_POS);
            while (position != 0)
            {
                int tmpPosition = position;
                position = _fileByteBuf.getInt(position + MsgHeader.NEXT_MSG_POS);
                if (loadMsg(tmpPosition, false, tmpMsg, tmpEncodeIter, tmpDecodeIter, error) != ReactorReturnCodes.SUCCESS)
                    return;
            }

            position = _fileByteBuf.getInt(Header.WAITACK_LIST_HEAD_POS);
            while (position != 0)
            {
                int tmpPosition = position;
                position = _fileByteBuf.getInt(position + MsgHeader.NEXT_MSG_POS);
                if (loadMsg(tmpPosition, true, tmpMsg, tmpEncodeIter, tmpDecodeIter, error) != ReactorReturnCodes.SUCCESS)
                    return;
            }
        }
        else
        {
            TunnelStreamPersistenceBuffer persistBuffer, prevPersistBuffer;

            _maxMsgs = Header.MAX_MSGS;
            _maxMsgLength = _tunnelSubstream._tunnelStream.classOfService().common().maxMsgSize();
            _tunnelSubstream._lastOutSeqNum = 0;
            _tunnelSubstream._lastInSeqNum = 0;

            _fileByteBuf.putInt(Header.FILE_VERSION_POS, FileVersion.V1);
            _fileByteBuf.putInt(Header.MAX_MSGS_POS, _maxMsgs);
            _fileByteBuf.putInt(Header.MAX_MSG_LENGTH_POS, _maxMsgLength);
            _fileByteBuf.putInt(Header.CURRENT_MSG_COUNT_POS, 0);
            _fileByteBuf.putInt(Header.LAST_OUT_SEQ_NUM_POS, 0);
            _fileByteBuf.putInt(Header.LAST_IN_SEQ_NUM_POS, 0);

            /* Populate buffer pool. */
            prevPersistBuffer = null;
            persistBuffer = null;
            for(int i = 0; i < Header.MAX_MSGS; ++i)
            {
                persistBuffer = new TunnelStreamPersistenceBuffer();
                persistBuffer.filePosition(Header.LENGTH + i * (MsgHeader.LENGTH + _maxMsgLength + SlicedBufferPool.TUNNEL_STREAM_HDR_SIZE));
                _persistentBufferPool.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);

                if (prevPersistBuffer != null)
                    _fileByteBuf.putInt(prevPersistBuffer.filePosition() + MsgHeader.NEXT_MSG_POS,
                            persistBuffer.filePosition());

                prevPersistBuffer = persistBuffer;
            }

            /* Set tail element's next pointer to null. */
            _fileByteBuf.putInt(persistBuffer.filePosition() + MsgHeader.NEXT_MSG_POS, 0);

            _fileByteBuf.putInt(Header.POOL_HEAD_POS, _persistentBufferPool.peek().filePosition());
            _fileByteBuf.putInt(Header.WAIT_TRANSMIT_HEAD_POS, 0);
            _fileByteBuf.putInt(Header.WAITACK_LIST_HEAD_POS, 0);

        }
        error.errorId(ReactorReturnCodes.SUCCESS);
    }

    @Override
	void lastOutSeqNum(int lastOutSeqNum)
	{
        _fileByteBuf.putInt(Header.LAST_OUT_SEQ_NUM_POS, lastOutSeqNum);
	}

    @Override
	void lastInSeqNum(int lastInSeqNum)
	{
        _fileByteBuf.putInt(Header.LAST_IN_SEQ_NUM_POS, lastInSeqNum);
	}

    @Override
	int saveMsg(TunnelStreamBuffer buffer, Error error)
    {
        int entryPosition;
        int seqNum = _tunnelSubstream._lastOutSeqNum + 1; 
        TunnelStreamPersistenceBuffer persistenceBuffer;

        assert (buffer.length() <= _tunnelSubstream._tunnelStream._classOfService.common().maxMsgSize());

        /* Stop if no buffers are free. */
        if ((persistenceBuffer = _persistentBufferPool.peek()) == null)
        {
            error.errorId(ReactorReturnCodes.PERSISTENCE_FULL);
            error.text("Local persistence file is full. Space may become available later as delivered messages are acknowledged.");
            return ReactorReturnCodes.PERSISTENCE_FULL;
        }

        entryPosition = persistenceBuffer.filePosition();

        /* Store message in file. */
        buffer.setToFullWritebuffer();
        _fileByteBuf.putInt(entryPosition + MsgHeader.MSG_LENGTH_POS, buffer.length());
        _fileByteBuf.putInt(entryPosition + MsgHeader.MSG_SEQNUM_POS, seqNum);
        _fileByteBuf.putShort(entryPosition + MsgHeader.MSG_OPCODE_POS, (short)TunnelStreamMsg.OpCodes.DATA);
        if (!buffer.timeoutIsCode())
        {
            assert(buffer.timeoutNsec() - buffer.timeQueuedNsec() > 0);
            _fileByteBuf.putLong(entryPosition + MsgHeader.TIME_TO_LIVE_POS, buffer.timeoutNsec() - buffer.timeQueuedNsec());
            _fileByteBuf.putLong(entryPosition + MsgHeader.TIME_QUEUED_POS, buffer.timeQueuedNsec());
        }
        else
            _fileByteBuf.putLong(entryPosition + MsgHeader.TIME_TO_LIVE_POS, buffer.timeoutNsec());

        _fileByteBuf.position(entryPosition + MsgHeader.MSG_BUFFER_POS);
        buffer.copyFullBuffer(_fileByteBuf);

        persistenceBufferListMove(_persistentBufferPool, Header.POOL_HEAD_POS,
                _waitingTransmitList, Header.WAIT_TRANSMIT_HEAD_POS,
                persistenceBuffer);


        buffer.persistenceBuffer(_tunnelSubstream, persistenceBuffer);
        persistenceBuffer.tunnelStreamHeaderLen(buffer.tunnelStreamHeaderLen());

        return ReactorReturnCodes.SUCCESS;
    }

    /* Loads a persisted message found in the file. */
    Buffer _tmpBuf = CodecFactory.createBuffer();
	private int loadMsg(int entryPosition, boolean isTransmitted, Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, Error error)
	{
		int length;
		int ret;

        TunnelStreamPersistenceBuffer persistBuffer = new TunnelStreamPersistenceBuffer();
		length = _fileByteBuf.getInt(entryPosition + MsgHeader.MSG_LENGTH_POS);
        persistBuffer.length(length);
		persistBuffer.filePosition(entryPosition);
		persistBuffer.seqNum(_fileByteBuf.getInt(entryPosition + MsgHeader.MSG_SEQNUM_POS));
		persistBuffer.isTransmitted(isTransmitted);

        /* Get TunnelStreamHeader length. */
		_tmpBuf.clear();
        _tmpBuf.data(_fileByteBuf, entryPosition + MsgHeader.LENGTH, persistBuffer.length());
        tmpDecodeIter.clear();
        tmpDecodeIter.setBufferAndRWFVersion(_tmpBuf, _tunnelSubstream._tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelSubstream._tunnelStream.classOfService().common().protocolMinorVersion());  
        if ((ret = tmpMsg.decode(tmpDecodeIter)) != CodecReturnCodes.SUCCESS)
        {
            error.errorId(ret);
            error.text("Failed to decode message while loading message from file.");
            return ReactorReturnCodes.FAILURE;
        }
        persistBuffer.tunnelStreamHeaderLen(tmpMsg.encodedDataBody().position() - (entryPosition + MsgHeader.LENGTH));

        if (isTransmitted)
            _waitingQueueAckList.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);
        else
            _waitingTransmitList.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);

		return ReactorReturnCodes.SUCCESS;
	}

    @Override
    void releasePersistenceBuffers(int seqNum)
    {
        TunnelStreamPersistenceBuffer persistBuffer;
        
        while ((persistBuffer = _waitingQueueAckList.peek()) != null
                && TunnelStreamUtil.seqNumCompare(persistBuffer.seqNum(), seqNum) <= 0)
            releasePersistenceBuffer(persistBuffer);
    }

    @Override
	void releasePersistenceBuffer(TunnelStreamPersistenceBuffer persistBuffer)
	{
        if (persistBuffer.isTransmitted())
        {
            persistenceBufferListMove( _waitingQueueAckList, Header.WAITACK_LIST_HEAD_POS,
                    _persistentBufferPool, Header.POOL_HEAD_POS,
                    persistBuffer);
        }
        else
        {
            persistenceBufferListMove( _waitingTransmitList, Header.WAIT_TRANSMIT_HEAD_POS,
                    _persistentBufferPool, Header.POOL_HEAD_POS,
                    persistBuffer);
        }
        persistBuffer.reset();
	}

    @Override
    void setBufferAsTransmitted(TunnelStreamPersistenceBuffer persistenceBuffer)
    {
        if (persistenceBuffer.isTransmitted())
            return;
        
        int seqNum;
        /* Assign a sequence number to the buffer and mark it as 
         * transmitted. */
        seqNum = _tunnelSubstream._lastOutSeqNum + 1;

        _fileByteBuf.putInt(persistenceBuffer.filePosition() + MsgHeader.MSG_SEQNUM_POS, seqNum);

        persistenceBuffer.isTransmitted(true);
        persistenceBuffer.seqNum(seqNum);
        persistenceBufferListMove(_waitingTransmitList, Header.WAIT_TRANSMIT_HEAD_POS,
                _waitingQueueAckList, Header.WAITACK_LIST_HEAD_POS, persistenceBuffer);
        lastOutSeqNum(seqNum);
    }

    @Override
	int retransmitBuffers(int seqNum, Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, Error error)
	{
		TunnelStreamPersistenceBuffer persistBuffer;
		int ret;
		long currentTime = System.nanoTime();

        for(persistBuffer = _waitingQueueAckList.start(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK); 
                persistBuffer != null;
                persistBuffer = _waitingQueueAckList.forth(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK))
            if ((ret = retransmitBuffer(persistBuffer, seqNum, currentTime, tmpMsg, tmpEncodeIter, tmpDecodeIter, error)) != ReactorReturnCodes.SUCCESS)
                return ret;
        
        for(persistBuffer = _waitingTransmitList.start(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK); 
                persistBuffer != null;
                persistBuffer = _waitingTransmitList.forth(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK))
            if ((ret = retransmitBuffer(persistBuffer, seqNum, currentTime, tmpMsg, tmpEncodeIter, tmpDecodeIter, error)) != ReactorReturnCodes.SUCCESS)
                return ret;


        return ReactorReturnCodes.SUCCESS;

	}

	/* Moves a buffer between a persistence file's buffer lists, in-memory and in-file. */
	private void persistenceBufferListMove(VaDoubleLinkList<TunnelStreamPersistenceBuffer> oldList, int oldListHeadPosition, 
			VaDoubleLinkList<TunnelStreamPersistenceBuffer> newList, int newListHeadPosition, TunnelStreamPersistenceBuffer persistBuffer)
	{
        super.peristenceBufferListMove(oldList, oldListHeadPosition, newList, newListHeadPosition,
                MsgHeader.NEXT_MSG_POS, persistBuffer);

        _fileByteBuf.putInt(Header.CURRENT_MSG_COUNT_POS, _waitingTransmitList.count() + _waitingQueueAckList.count());
		_fileByteBuf.force();
	}

    @Override
    void clear(Error tmpError)
    {
        super.clear(tmpError);
        while (_waitingTransmitList.pop(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK) != null);
        while (_waitingQueueAckList.pop(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK) != null);
        _maxMsgs = 0;
        _tmpBuf.clear();
    }

    @Override
    int persistBufferMsgOffset()
    {
        return MsgHeader.LENGTH;
    }

    @Override
    long persistBufferTimeoutNsec(TunnelStreamPersistenceBuffer persistBuffer)
    {
        return _fileByteBuf.getLong(persistBuffer.filePosition() + MsgHeader.TIME_TO_LIVE_POS);
    }
}
