/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;

import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;

/* Class for handling a persistence file with version 2. */
class TunnelStreamPersistenceFileV2 extends TunnelStreamPersistenceFile
{
	private int _maxMsgs;
    private int _maxMsgLength;

	/* (SUBSTREAM_LINK) Persistent buffers that have been saved. */
	private VaDoubleLinkList<TunnelStreamPersistenceBuffer> _savedMsgList;
	
	/* Static class to describe the file header. */
	private class Header
	{
		private static final int LENGTH = 0
			+ 4 /* file version */
			+ 4 /* maxMsgs */
			+ 4 /* maxMsgSize */
			+ 4 /* currentMsgCount */
			+ 4 /* lastOutSeqNum */
			+ 4 /* lastInSeqNum */
			+ 4 /* Buffer pool head */
			+ 4 /* Buffer saved list head */
			+ 4 /* Flags */
            ;

		private static final int FILE_VERSION_POS = 0;
		private static final int MAX_MSGS_POS = FILE_VERSION_POS + 4;
		private static final int MAX_MSG_LENGTH_POS = MAX_MSGS_POS + 4;
		private static final int CURRENT_MSG_COUNT_POS = MAX_MSG_LENGTH_POS + 4;
		private static final int LAST_OUT_SEQ_NUM_POS = CURRENT_MSG_COUNT_POS + 4;
		private static final int LAST_IN_SEQ_NUM_POS = LAST_OUT_SEQ_NUM_POS + 4;
		private static final int POOL_HEAD_POS = LAST_IN_SEQ_NUM_POS + 4;
		private static final int SAVED_HEAD_POS = POOL_HEAD_POS + 4;
		private static final int FLAGS_POS = SAVED_HEAD_POS + 4;

		private static final int MAX_MSGS = 1024;
	}
	

    /* Static class to describe the file's msg header */
    private class MsgHeader
    {
        private static final int LENGTH = 0
            + 4 /* nextMsgPos */
            + 4 /* flags */
            + 4 /* length */
            + 8 /* time of arrival */
            + 8 /* time to live */
            + 4 /* Not used */
            ;

        static final int NEXT_MSG_POS = 0;
        static final int FLAGS_POS = NEXT_MSG_POS + 4;
        static final int MSG_LENGTH_POS = FLAGS_POS + 4;
        static final int TIME_QUEUED_POS = MSG_LENGTH_POS + 4;
        static final int TIME_TO_LIVE_POS = TIME_QUEUED_POS + 8;   
    }
    
    private class MsgHeaderFlags
    {
        private static final int NONE = 0x0;
        private static final int TRANSMITTED = 0x1;
    }

    TunnelStreamPersistenceFileV2(TunnelSubstream tunnelSubstream, RandomAccessFile file, FileChannel fileChannel, FileLock fileLock,
            Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, boolean reset, Error error)
    {
        super(tunnelSubstream, file, fileChannel, fileLock);

		_savedMsgList = new VaDoubleLinkList<TunnelStreamPersistenceBuffer>();

        try
        {
            long fileSize;
            if (reset) fileSize = Header.LENGTH + (MsgHeader.LENGTH + _tunnelSubstream._tunnelStream._classOfService.common().maxMsgSize() + SlicedBufferPool.TUNNEL_STREAM_HDR_SIZE) * Header.MAX_MSGS;
            else fileSize = fileChannel.size();

            _fileByteBuf = fileChannel.map(FileChannel.MapMode.READ_WRITE, 0, fileSize);
            _fileByteBuf.order(ByteOrder.LITTLE_ENDIAN);
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
            int transmittedMsgCount = 0;
            TunnelStreamPersistenceBuffer persistBuffer;
            
            _maxMsgs = _fileByteBuf.getInt(Header.MAX_MSGS_POS);
            _maxMsgLength = _fileByteBuf.getInt(Header.MAX_MSG_LENGTH_POS);
            _tunnelSubstream._lastOutSeqNum = _fileByteBuf.getInt(Header.LAST_OUT_SEQ_NUM_POS);
            _tunnelSubstream._lastInSeqNum = _fileByteBuf.getInt(Header.LAST_IN_SEQ_NUM_POS);

            /* Load buffer lists. */
            position = _fileByteBuf.getInt(Header.POOL_HEAD_POS);
            while (position != 0)
            {
                persistBuffer = new TunnelStreamPersistenceBuffer();
                persistBuffer.tunnelStreamHeaderLen(0);
                persistBuffer.filePosition(position);
                _persistentBufferPool.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);
                position = _fileByteBuf.getInt(position + MsgHeader.NEXT_MSG_POS);
            }

            position = _fileByteBuf.getInt(Header.SAVED_HEAD_POS);
            while (position != 0)
            {
                int entryPosition = position;
                int length;

                position = _fileByteBuf.getInt(position + MsgHeader.NEXT_MSG_POS);

                persistBuffer = new TunnelStreamPersistenceBuffer();
                length = _fileByteBuf.getInt(entryPosition + MsgHeader.MSG_LENGTH_POS);
                persistBuffer.length(length);
                persistBuffer.tunnelStreamHeaderLen(0); /* This format does not include a tunnel stream header. */
                persistBuffer.filePosition(entryPosition);
                persistBuffer.isTransmitted((_fileByteBuf.getInt(entryPosition + MsgHeader.FLAGS_POS) & MsgHeaderFlags.TRANSMITTED) != 0);
                if (persistBuffer.isTransmitted())
                    ++transmittedMsgCount;
                    
                _savedMsgList.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);
            }
                
            /* For messages that were transmitted, set the appropriate sequence number. */
            int seqNum = _tunnelSubstream._lastOutSeqNum - transmittedMsgCount;
            for (persistBuffer = _savedMsgList.start(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);
                    persistBuffer != null && persistBuffer.isTransmitted();
                    persistBuffer = _savedMsgList.forth(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK))
            {
                ++seqNum;
                persistBuffer.seqNum(seqNum);
            }

        }
        else
        {
            TunnelStreamPersistenceBuffer persistBuffer, prevPersistBuffer;

            _maxMsgs = Header.MAX_MSGS;
            _maxMsgLength = _tunnelSubstream._tunnelStream.classOfService().common().maxMsgSize();
            _tunnelSubstream._lastOutSeqNum = 0;
            _tunnelSubstream._lastInSeqNum = 0;

            _fileByteBuf.putInt(Header.FILE_VERSION_POS, FileVersion.V2L);
            _fileByteBuf.putInt(Header.MAX_MSGS_POS, _maxMsgs);
            _fileByteBuf.putInt(Header.MAX_MSG_LENGTH_POS, _maxMsgLength);
            _fileByteBuf.putInt(Header.CURRENT_MSG_COUNT_POS, 0);
            _fileByteBuf.putInt(Header.LAST_OUT_SEQ_NUM_POS, 0);
            _fileByteBuf.putInt(Header.LAST_IN_SEQ_NUM_POS, 0);
            _fileByteBuf.putInt(Header.FLAGS_POS, 0);

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
            _fileByteBuf.putInt(Header.SAVED_HEAD_POS, 0);
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
        buffer.setToInnerWriteBuffer();
        _fileByteBuf.putInt(persistenceBuffer.filePosition() + MsgHeader.FLAGS_POS, MsgHeaderFlags.NONE);
        _fileByteBuf.putInt(entryPosition + MsgHeader.MSG_LENGTH_POS, buffer.length());
        if (!buffer.timeoutIsCode())
        {
            assert(buffer.timeoutNsec() - buffer.timeQueuedNsec() > 0);
            _fileByteBuf.putLong(entryPosition + MsgHeader.TIME_TO_LIVE_POS, (buffer.timeoutNsec() - buffer.timeQueuedNsec()) / TunnelStreamUtil.NANO_PER_MILLI);
            _fileByteBuf.putLong(entryPosition + MsgHeader.TIME_QUEUED_POS, buffer.timeQueuedNsec());
        }
        else
            _fileByteBuf.putLong(entryPosition + MsgHeader.TIME_TO_LIVE_POS, buffer.timeoutNsec());

        _fileByteBuf.position(entryPosition + MsgHeader.LENGTH);
        
        buffer.copy(_fileByteBuf);

        persistenceBufferListMove(_persistentBufferPool, Header.POOL_HEAD_POS,
                _savedMsgList, Header.SAVED_HEAD_POS,
                persistenceBuffer);

        buffer.persistenceBuffer(_tunnelSubstream, persistenceBuffer);

        return ReactorReturnCodes.SUCCESS;
    }

    @Override
    void releasePersistenceBuffers(int seqNum)
    {
        TunnelStreamPersistenceBuffer persistBuffer;
        
        while ((persistBuffer = _savedMsgList.peek()) != null
                && TunnelStreamUtil.seqNumCompare(persistBuffer.seqNum(), seqNum) <= 0)
            releasePersistenceBuffer(persistBuffer);
    }

    @Override
	void releasePersistenceBuffer(TunnelStreamPersistenceBuffer persistBuffer)
	{
        persistenceBufferListMove( _savedMsgList, Header.SAVED_HEAD_POS,
                _persistentBufferPool, Header.POOL_HEAD_POS,
                persistBuffer);
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

        _fileByteBuf.putInt(persistenceBuffer.filePosition() + MsgHeader.FLAGS_POS, MsgHeaderFlags.TRANSMITTED);
        
        persistenceBuffer.isTransmitted(true);
        persistenceBuffer.seqNum(seqNum);
        lastOutSeqNum(seqNum);
        _fileByteBuf.force();
    }

    @Override
	int retransmitBuffers(int seqNum, Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, Error error)
	{
		TunnelStreamPersistenceBuffer persistBuffer;
		int ret;
		long currentTime = System.nanoTime();

        for(persistBuffer = _savedMsgList.start(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK); 
                persistBuffer != null;
                persistBuffer = _savedMsgList.forth(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK))
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

        _fileByteBuf.putInt(Header.CURRENT_MSG_COUNT_POS, _savedMsgList.count());
		_fileByteBuf.force();
    }

    @Override
    void clear(Error tmpError)
    {
        super.clear(tmpError);
        while (_savedMsgList.pop(TunnelStreamPersistenceBuffer.SUBSTREAM_LINK) != null);
        _maxMsgs = 0;
    }

    @Override
    int persistBufferMsgOffset()
    {
        return MsgHeader.LENGTH;
    }

    @Override
    long persistBufferTimeoutNsec(TunnelStreamPersistenceBuffer persistBuffer)
    {
        return _fileByteBuf.getLong(persistBuffer.filePosition() + MsgHeader.TIME_TO_LIVE_POS) * TunnelStreamUtil.NANO_PER_MILLI;
    }
}
