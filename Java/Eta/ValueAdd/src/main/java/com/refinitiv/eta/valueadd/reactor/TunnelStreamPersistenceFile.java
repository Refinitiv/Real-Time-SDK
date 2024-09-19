/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueDataFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueDataUndeliverableCode;
import com.refinitiv.eta.valueadd.reactor.TunnelSubstream.TunnelSubstreamState;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;

/* Class representing a persistence file. Different versions of persistence write to this interface. */
abstract class TunnelStreamPersistenceFile
{
	class FileVersion
	{
		static final int V1 = 1;
		static final int V2 = 33554432; /* Version 2 (read as big-endian) */
		static final int V2L = 2; /* Version 2 (read as little-endian) */
	}
    static int _defaultPersistenceVerion = FileVersion.V2;

	/* (SUBSTREAM_LINK) Pool of available persistent buffers in the persistence file,
     * if local persistence is enabled. */
	protected VaDoubleLinkList<TunnelStreamPersistenceBuffer>	_persistentBufferPool;

    /* (RETRANS_LINK) QueueData messages that were acknowledged by the QueueRefresh. 
     * Will will generate QueueAcks for these. */
    protected VaDoubleLinkList<TunnelStreamBuffer> _localQueueAckList;

    /* The persistence file. */
	protected RandomAccessFile _file;

    /* ByteBuffer mapped to the persistence file. */
    protected MappedByteBuffer _fileByteBuf;

    /* Channel for the persistence file. */
	protected FileChannel _fileChannel;

    /* Lock for the persistence file, to prevent concurrent access. */
	protected FileLock _fileLock;

    /* Substream that opened this persistence file. */
    protected TunnelSubstream _tunnelSubstream;

    /* ByteBuffer for temporary encoding/decoding */
    ByteBuffer _tmpByteBuf;

	/* Save a message to the file, if space is available. */
	abstract int saveMsg(TunnelStreamBuffer buffer, Error error);

    /* Releases active persistent buffers based on a received sequence number. */
    abstract void releasePersistenceBuffers(int seqNum);

    /* Releases the given persistence buffer. */
    abstract void releasePersistenceBuffer(TunnelStreamPersistenceBuffer persistBuffer);

	/* Used when recovering a stream. Releases any acknowledged buffers and moves the rest back for retransmission. */
	abstract int retransmitBuffers(int seqNum, Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, Error error);

    /* Mark that a buffer has been sent to the network. */
    abstract void setBufferAsTransmitted(TunnelStreamPersistenceBuffer persistenceBuffer);

    /* Save the last received sequence number. */
	abstract void lastInSeqNum(int hdrLastAckedInSeqNum);

    /* Save the last sent sequence number. */
	abstract void lastOutSeqNum(int hdrLastOutSeqNum);

    /* Returns the offset from the start of persisted buffer at which the message starts. */
    abstract int persistBufferMsgOffset();

    /* Returns the timeout of a persistence buffer. */
    abstract long persistBufferTimeoutNsec(TunnelStreamPersistenceBuffer persistBuffer);

    TunnelStreamPersistenceFile(TunnelSubstream tunnelSubstream, RandomAccessFile file, FileChannel fileChannel, FileLock fileLock)
    {
        _tunnelSubstream = tunnelSubstream;
        _file = file;
        _fileChannel = fileChannel;
        _fileLock = fileLock;
		_persistentBufferPool = new VaDoubleLinkList<TunnelStreamPersistenceBuffer>();
		_localQueueAckList = new VaDoubleLinkList<TunnelStreamBuffer>();
    }

    /* Close the persistence file and clear this object. */
    int close(Error error)
    {
        if (_fileByteBuf != null)
        {
            try
            {
                _fileLock.release();
                _fileChannel.close();
                _file.close();
            }
            catch (IOException e)
            {
                error.errorId(ReactorReturnCodes.FAILURE);
                error.text("Failed to close persistence file.");
                return ReactorReturnCodes.FAILURE;
            }
        }
        clear(error);
        return ReactorReturnCodes.SUCCESS;
    } 

    /* Clears this object's members. */
    void clear(Error tmpError)
    {
        TunnelStreamBuffer tunnelStreamBuffer;
        while ((tunnelStreamBuffer = _localQueueAckList.pop(TunnelStreamBuffer.RETRANS_LINK)) != null)
            _tunnelSubstream._tunnelStream.releaseBuffer(tunnelStreamBuffer, tmpError);
        _persistentBufferPool.clear();

        _tunnelSubstream = null;
        _fileByteBuf = null;
        _tmpByteBuf = null;
        _fileChannel = null;
        _fileChannel = null;
        _fileLock = null;
    }

	/* Moves a buffer between a persistence file's buffer lists, in-memory and in-file. */
	protected void peristenceBufferListMove(VaDoubleLinkList<TunnelStreamPersistenceBuffer> oldList, int oldListHeadPosition, 
			VaDoubleLinkList<TunnelStreamPersistenceBuffer> newList, int newListHeadPosition, int nextMsgPosOffset,
            TunnelStreamPersistenceBuffer persistBuffer)
	{
	    TunnelStreamPersistenceBuffer next;
	    TunnelStreamPersistenceBuffer prev;
		assert(newList != null);
		assert(oldList != null);

        next = TunnelStreamPersistenceBuffer.SUBSTREAM_LINK.getNext(persistBuffer);
        prev = TunnelStreamPersistenceBuffer.SUBSTREAM_LINK.getPrev(persistBuffer);

        /* Update list heads */
        if (prev != null)
        {
            /* This element was not the head, so update the previous link's next pointer */
            if (next != null)
                _fileByteBuf.putInt(prev.filePosition() + nextMsgPosOffset, next.filePosition());
            else
                _fileByteBuf.putInt(prev.filePosition() + nextMsgPosOffset, 0);
        }
        else
        {
            /* This element was the head */
            if (next != null)
                _fileByteBuf.putInt(oldListHeadPosition, next.filePosition());
            else
                _fileByteBuf.putInt(oldListHeadPosition, 0);
        }


        oldList.remove(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);
        newList.push(persistBuffer, TunnelStreamPersistenceBuffer.SUBSTREAM_LINK);

        /* Update new list file positions */
        next = TunnelStreamPersistenceBuffer.SUBSTREAM_LINK.getNext(persistBuffer);
        prev = TunnelStreamPersistenceBuffer.SUBSTREAM_LINK.getPrev(persistBuffer);

        if (next != null)
            _fileByteBuf.putInt(persistBuffer.filePosition() + nextMsgPosOffset, next.filePosition());
        else
            _fileByteBuf.putInt(persistBuffer.filePosition() + nextMsgPosOffset, 0);

        if (prev != null)
        {
            /* Update next pointer */
            _fileByteBuf.putInt(prev.filePosition() + nextMsgPosOffset, persistBuffer.filePosition());
        }
        else
        {
            /* This element is the head */
            _fileByteBuf.putInt(newListHeadPosition, persistBuffer.filePosition());
        }

	}

    /* Copies persistence buffer data to a TunnelStream buffer. */
    protected void copyToTunnelStreamBuffer(TunnelStreamPersistenceBuffer persistBuffer, TunnelStreamBuffer tunnelStreamBuffer)
    {
        int tmpLimit = _fileByteBuf.limit();
        int startPos = persistBuffer.filePosition() + persistBufferMsgOffset() + persistBuffer.tunnelStreamHeaderLen();
        int length = persistBuffer.length() - persistBuffer.tunnelStreamHeaderLen();

        _fileByteBuf.limit(startPos + length);
        _fileByteBuf.position(startPos);
        tunnelStreamBuffer.data().put(_fileByteBuf);
        _fileByteBuf.limit(tmpLimit);
        tunnelStreamBuffer.setCurrentPositionAsEndOfEncoding();
        tunnelStreamBuffer.persistenceBuffer(_tunnelSubstream, persistBuffer);
    }

    /* Retransmits, locally acknowledges, or locally expires a buffer
     * loaded from the persistence file. */
    protected int retransmitBuffer(TunnelStreamPersistenceBuffer persistBuffer, int seqNum, long currentTime, Msg tmpMsg, EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, Error error)
    {
        int ret;
        TunnelStreamBuffer tunnelStreamBuffer;

        /* If message is too large to persist, don't try to get a buffer for it.
         * We won't be sending it (it will be locally acknowledged or expired). */
        if (persistBuffer.length() - persistBuffer.tunnelStreamHeaderLen() > _tunnelSubstream._tunnelStream._classOfService.common().maxMsgSize()) 
        {
            if ((tunnelStreamBuffer = _tunnelSubstream._tunnelStream._tunnelStreamBufferPool.pop(TunnelStreamBuffer.RETRANS_LINK)) == null)
                tunnelStreamBuffer = new TunnelStreamBuffer();

            tunnelStreamBuffer.clear(persistBuffer.length());
            tunnelStreamBuffer.persistenceBuffer(_tunnelSubstream, persistBuffer);
        }
        else
        {
            tunnelStreamBuffer = _tunnelSubstream._tunnelStream.getBuffer(persistBuffer.length() - persistBuffer.tunnelStreamHeaderLen(), false, true, error);

            if (tunnelStreamBuffer == null)
            {
                error.text("Failed to get TunnelStream buffer while retransmitting QueueData message: " + error.text());
                return ReactorReturnCodes.FAILURE;
            }

            /* Copy the message from the file to the TunnelStream buffer. */
            copyToTunnelStreamBuffer(persistBuffer, tunnelStreamBuffer);
        }

        if (seqNum == 0)
        {
            if (tunnelStreamBuffer.data() == null)
            {
                /* Expire large messages. */
                _localQueueAckList.push(tunnelStreamBuffer, TunnelStreamBuffer.RETRANS_LINK);
                return ReactorReturnCodes.SUCCESS;
            }

            /* If the Provider sent 0 as last received sequence number,
             * it may be because it lost its persistence and/or doesn't recognize us.
             * Resend all buffers, marking them as possible duplicates. */
            tunnelStreamBuffer.setAsInnerReadBuffer();
            TunnelStreamUtil.replaceQueueDataFlags(tunnelStreamBuffer.data(), QueueDataFlags.POSSIBLE_DUPLICATE);
            tunnelStreamBuffer.setAsFullReadBuffer();
        }
        else if (TunnelStreamUtil.seqNumCompare(persistBuffer.seqNum(), seqNum) <= 0)
        {
            /* Message was already received. Generate QueueAck for it and release the buffer. */
            tunnelStreamBuffer.isForLocalAck(true);
            _localQueueAckList.push(tunnelStreamBuffer, TunnelStreamBuffer.RETRANS_LINK);
            return ReactorReturnCodes.SUCCESS;
        }
        else if (tunnelStreamBuffer.data() == null)
        {
            /* Expire large messages. */
            _localQueueAckList.push(tunnelStreamBuffer, TunnelStreamBuffer.RETRANS_LINK);
            return ReactorReturnCodes.SUCCESS;
        }

        // replace queue data stream id if changed
        tunnelStreamBuffer.setAsInnerReadBuffer();
        tmpDecodeIter.clear();
        tmpDecodeIter.setBufferAndRWFVersion(tunnelStreamBuffer, _tunnelSubstream._tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelSubstream._tunnelStream.classOfService().common().protocolMinorVersion());  
        if (tmpDecodeIter.extractStreamId() != _tunnelSubstream._streamId)
        {
            tmpEncodeIter.clear();
            tmpEncodeIter.setBufferAndRWFVersion(tunnelStreamBuffer, _tunnelSubstream._tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelSubstream._tunnelStream.classOfService().common().protocolMinorVersion());
            if ((ret = tmpEncodeIter.replaceStreamId(_tunnelSubstream._streamId)) != CodecReturnCodes.SUCCESS)
            {
                error.errorId(ret);
                error.text("Failed to update stream id from " + tmpDecodeIter.extractStreamId() + " to " + _tunnelSubstream._streamId + " on QueueData message.");
                return ReactorReturnCodes.FAILURE;
            }
        }

        tunnelStreamBuffer.timeoutNsec(persistBufferTimeoutNsec(persistBuffer));
        if (tunnelStreamBuffer.timeoutNsec() > 0)
        {
            tunnelStreamBuffer.timeoutIsCode(false);
            if (!persistBuffer.isTransmitted())
                _tunnelSubstream._tunnelStream.insertTimeoutBuffer(tunnelStreamBuffer, currentTime);
        }
        else
        {
            tunnelStreamBuffer.timeoutIsCode(true);
        }

        tunnelStreamBuffer.isQueueData(true);

        _tunnelSubstream._tunnelStream._outboundTransmitList.push(tunnelStreamBuffer, TunnelStreamBuffer.RETRANS_LINK);

        return ReactorReturnCodes.SUCCESS;
    }            


    /* Used when QueueRefresh is received. Generates QueueAcks for any messages that
     * the QueueRefresh acknowledged. */
    protected int sendLocalQueueAcks(EncodeIterator tmpEncodeIter, DecodeIterator tmpDecodeIter, Error error)
    {
        int ret;
		TunnelStreamBuffer tunnelStreamBuffer;
        while ((tunnelStreamBuffer = _localQueueAckList.pop(TunnelStreamBuffer.RETRANS_LINK)) != null) 
        {
            TunnelStreamPersistenceBuffer persistBuffer = tunnelStreamBuffer.persistenceBuffer();
            if (tunnelStreamBuffer.data() == null)
            {
                /* No data -- buffer is in the list because was larger than the max message size,
                 * so load data into temporary memory. */
                if (_tmpByteBuf == null || _tmpByteBuf.capacity() < persistBuffer.length())
                    _tmpByteBuf = ByteBuffer.allocateDirect(persistBuffer.length());

                _tmpByteBuf.position(0);
                _tmpByteBuf.limit(persistBuffer.length());
                tunnelStreamBuffer.data(_tmpByteBuf);
                copyToTunnelStreamBuffer(persistBuffer, tunnelStreamBuffer);
                
            }

            // replace queue data stream id if changed
            tunnelStreamBuffer.setAsInnerReadBuffer();
            tmpDecodeIter.clear();
            tmpDecodeIter.setBufferAndRWFVersion(tunnelStreamBuffer, _tunnelSubstream._tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelSubstream._tunnelStream.classOfService().common().protocolMinorVersion());  
            if (tmpDecodeIter.extractStreamId() != _tunnelSubstream._streamId)
            {
                tmpEncodeIter.clear();
                tmpEncodeIter.setBufferAndRWFVersion(tunnelStreamBuffer, _tunnelSubstream._tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelSubstream._tunnelStream.classOfService().common().protocolMinorVersion());
                if ((ret = tmpEncodeIter.replaceStreamId(_tunnelSubstream._streamId)) != CodecReturnCodes.SUCCESS)
                {
                    error.errorId(ret);
                    error.text("Failed to update stream id from " + tmpDecodeIter.extractStreamId() + " to " + _tunnelSubstream._streamId + " on QueueData message.");
                    return ReactorReturnCodes.FAILURE;
                }
            }

            if (tunnelStreamBuffer.isForLocalAck())
            {
                /* Buffer is in this list to be acknowledged. */
                _tunnelSubstream.sendQueueAckToListener(tunnelStreamBuffer);
                releasePersistenceBuffer(persistBuffer);
                _tunnelSubstream._tunnelStream.releaseBuffer(tunnelStreamBuffer, error);
            }
            else
            {
                _tunnelSubstream._tunnelStream.queueMsgExpired(tunnelStreamBuffer, null,  QueueDataUndeliverableCode.MAX_MSG_SIZE);
                releasePersistenceBuffer(persistBuffer);
                tunnelStreamBuffer.clear(0);
                _tunnelSubstream._tunnelStream._tunnelStreamBufferPool.push(tunnelStreamBuffer, TunnelStreamBuffer.RETRANS_LINK);
            }

            if (_tunnelSubstream._state != TunnelSubstreamState.OPEN)
                return ReactorReturnCodes.SUCCESS;
        }

        return ReactorReturnCodes.SUCCESS;
    }

    /* For testing only */
    static void defaultPersistenceVersion(int defaultPersistenceVersion)
    {
        _defaultPersistenceVerion = defaultPersistenceVersion;
    }

    static int defaultPersistenceVersion()
    {
        return _defaultPersistenceVerion;
    }

}
