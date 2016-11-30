package com.thomsonreuters.upa.valueadd.reactor;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.channels.OverlappingFileLockException;
import java.nio.charset.Charset;

import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamBuffer;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamPersistenceBuffer;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamPersistenceFile.FileVersion;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.queue.QueueMsgFactory;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;

/* Maintains the sequence state and outstanding messages in the substream */
class TunnelSubstream
{
    /* Tunnel stream that opened this substream. */
    TunnelStream _tunnelStream;
    
    /* Persistence file opened by this substream, if persisting locally. */
    TunnelStreamPersistenceFile _persistFile;
	
	QueueRequestImpl _queueRequest;
	QueueRefreshImpl _queueRefresh;
	QueueStatusImpl _queueStatus;
	QueueAckImpl _queueAck;
	QueueDataImpl _queueData;
	QueueDataExpiredImpl _queueDataExpired;
	QueueCloseImpl _queueClose;
	GenericMsg _genericMsg;
	
	Msg _msg;
	Msg _substreamMsg;
	DecodeIterator _dIter;

	/* Substream attributes */
	int _streamId;
	int _domainType;
	int _serviceId;
	Buffer _queueName;
    int _lastOutSeqNum;
    int _lastInSeqNum;
	
    EncodeIterator _encIter;
	DecodeIterator _decIter;
	TunnelSubstreamState _state;
    Msg _encSubMsg;
    Error _error;
    ByteBuffer _byteBuffer;
    
    /* Possible states of the substream. */
    enum TunnelSubstreamState
    {
        /* Substream has not been opened. */
        NOT_OPEN,
        
        /* Waiting for a response to establish the substream. */
        WAITING_SUBSTREAM_REFRESH,

        /* Substream is established and messages can be exchanged. */
        OPEN,
    }

	TunnelSubstream(Buffer queueName, int streamId, int domainType, int serviceId, TunnelStream tunnelStreamHandler, Error error)
	{
	    _tunnelStream = tunnelStreamHandler;
	    
		_queueName = CodecFactory.createBuffer();

		_queueName.data(ByteBuffer.allocateDirect(queueName.length()));
		queueName.copy(_queueName);

		_streamId = streamId;
		_domainType = domainType;
		_serviceId = serviceId;

        _queueRequest = (QueueRequestImpl)QueueMsgFactory.createQueueRequest();
        _queueRefresh = (QueueRefreshImpl)QueueMsgFactory.createQueueRefresh();
        _queueStatus = (QueueStatusImpl)QueueMsgFactory.createQueueStatus();
        _queueAck = (QueueAckImpl)QueueMsgFactory.createQueueAck();
        _queueData = (QueueDataImpl)QueueMsgFactory.createQueueData();
        _queueDataExpired = (QueueDataExpiredImpl)QueueMsgFactory.createQueueDataExpired();
        _queueClose = (QueueCloseImpl)QueueMsgFactory.createQueueClose();

		_substreamMsg = CodecFactory.createMsg();
		_msg = CodecFactory.createMsg();
		_dIter = CodecFactory.createDecodeIterator();
		error.errorId(ReactorReturnCodes.SUCCESS);
		_state = TunnelSubstreamState.NOT_OPEN;
        _encIter = CodecFactory.createEncodeIterator();
        _decIter = CodecFactory.createDecodeIterator();
        _encSubMsg = CodecFactory.createMsg();
        _error = TransportFactory.createError();
        _genericMsg = (GenericMsg)CodecFactory.createMsg();
        _genericMsg.msgClass(MsgClasses.GENERIC);
        _byteBuffer = ByteBuffer.allocateDirect(4);
    }
	
    TunnelSubstream(Buffer queueName, int streamId, int domainType, int serviceId, String filePath, TunnelStream tunnelStreamHandler, Error error)
    {
		this(queueName, streamId, domainType, serviceId, tunnelStreamHandler, error);
		if (error.errorId() != ReactorReturnCodes.SUCCESS)
			return;

        _tunnelStream = tunnelStreamHandler;

		File queueFile;
		byte[] queueBytes = new byte[queueName.length()];
		int queuePos = queueName.position();
		int fileVersion;
		boolean reset = _tunnelStream.forceFileReset();

        RandomAccessFile file;
        FileChannel fileChannel;
        FileLock fileLock;

		/* Copy byte-array (can't use directly; queueName may not have byte array */
		for(int i = 0; i < queueName.length(); ++i)
			queueBytes[i] = queueName.data().get(queuePos + i);
		String queueString = new String(queueBytes, Charset.forName("UTF-8"));

		queueFile = new File(filePath, queueString);

		if (!queueFile.exists())
			reset = true;

		try
		{
			file = new RandomAccessFile(queueFile, "rw");
			fileChannel = file.getChannel();

			/* Ensure that we have exclusive file access. */
			try
			{
				if ((fileLock = fileChannel.tryLock()) == null)
				{
				    fileChannel.close();
				    file.close();
				    error.errorId(ReactorReturnCodes.FAILURE);
	                error.text("Persistence file already in use.");
	                return;
				}
			}
			catch(OverlappingFileLockException e)
			{
                fileChannel.close();
                file.close();
                error.errorId(ReactorReturnCodes.FAILURE);
                error.text("Persistence file already in use.");
                return;
			}
		}
		catch(IOException e)
		{
			error.errorId(ReactorReturnCodes.FAILURE);	
			error.text("Failed to open persistence file.");
			return;
		}

		if (!reset)
		{
            /* File version is first four bytes of file. */
		    _byteBuffer.position(0);
		    try
            {
                fileChannel.read(_byteBuffer, 0);
            }
            catch (IOException e)
            {
                error.errorId(ReactorReturnCodes.FAILURE);
                error.text("Caught IOException while reading persistence file version.");
            }
            fileVersion = _byteBuffer.getInt(0);		    
		}
		else
		    fileVersion = TunnelStreamPersistenceFile.defaultPersistenceVersion();

        switch(fileVersion)
        {
            case FileVersion.V2:
            {
                _persistFile = new TunnelStreamPersistenceFileV2(this, file, fileChannel, fileLock, _msg, _encIter, _dIter, reset, error);
                if (error.errorId() != ReactorReturnCodes.SUCCESS)
                {
                    _persistFile = null;
                    return;
                }
                break;
            }

            case FileVersion.V1:
            {
                _persistFile = new TunnelStreamPersistenceFileV1(this, file, fileChannel, fileLock, _msg, _encIter, _dIter, reset, error);
                if (error.errorId() != ReactorReturnCodes.SUCCESS)
                {
                    _persistFile = null;
                    return;
                }
                break;
            }
            default:
                try
                {
                    fileChannel.close();
                    file.close();
                    error.text("Invalid persistence file version.");
                    error.errorId(ReactorReturnCodes.FAILURE);
                    return;
                } 
                catch (IOException e)
                {
                    error.text("Caught IOException while closing persistence file (due to Invalid persistence file version).");
                    error.errorId(ReactorReturnCodes.FAILURE);
                    return;
                }
                
        }

    }

	int lastOutSeqNum() { return _lastOutSeqNum; }
	void lastOutSeqNum(int lastInSeqNum)
	{
		_lastOutSeqNum = lastInSeqNum;
		if (_persistFile != null)
			_persistFile.lastOutSeqNum(lastInSeqNum);
	}

	int lastInSeqNum() { return _lastInSeqNum; }
	void lastInSeqNum(int lastInSeqNum)
	{
		_lastInSeqNum = lastInSeqNum;
		if (_persistFile != null)
			_persistFile.lastInSeqNum(lastInSeqNum);
	}

	int close(Error error)
	{
        _state = TunnelSubstreamState.NOT_OPEN;
        if (_persistFile != null)
        {
            if (_persistFile.close(error) != ReactorReturnCodes.SUCCESS)
                return error.errorId();
            _persistFile = null;
        }

		return ReactorReturnCodes.SUCCESS;
	}

	/* Send a QueueAck for the given QueueData message to the application. Used for post-recovery case
	 * where the QueueRefresh indicates it received messages we are peristing. */
    void sendQueueAckToListener(TunnelStreamBuffer tunnelBuffer)
    {
        /* Move to start of QueueData. */
        tunnelBuffer.setAsInnerReadBuffer();
                
        /* Decode as QueueData message. */
        _decIter.clear();
        _decIter.setBufferAndRWFVersion(tunnelBuffer, _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());
        _genericMsg.decode(_decIter);
        _queueData.decode(_decIter, _genericMsg);

        /* Translate to QueueAck message (source & destination should be reversed). */
        _queueAck.clear();
        _queueAck.sourceName().data(_queueData.destName().data(), _queueData.destName().position(), _queueData.destName().length());
        _queueAck.destName().data(_queueData.sourceName().data(), _queueData.sourceName().position(), _queueData.sourceName().length());
        _queueAck.domainType(_queueData.domainType());
        _queueAck.identifier(_queueData.identifier());
        _queueAck.serviceId(_queueData.serviceId());
        _queueAck.streamId(_streamId);
        
        // encode QueueAck into buffer and decode into generic message for use by RAISE
        TunnelStreamBuffer tunnelWriteBuffer = _tunnelStream.getBuffer(_queueAck.ackMsgBufferSize(), false, false, _error);
        _encIter.clear();
        _encIter.setBufferAndRWFVersion(tunnelWriteBuffer, _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());
        _queueAck.encode(_encIter);
        _decIter.clear();
        _decIter.setBufferAndRWFVersion(tunnelWriteBuffer,  _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());
        _genericMsg.clear();
        _genericMsg.decode(_decIter);

        _tunnelStream.queueMsgAcknowledged(_queueAck,
                _genericMsg);
        
        // release buffer for generic message
        _tunnelStream.releaseBuffer(tunnelWriteBuffer, _error);
    }

	Buffer queueName() { return _queueName; }

	int streamId() { return _streamId; }

	int domainType() { return _domainType; }

	int serviceId() { return _serviceId; }
	
   int sendSubstreamRequest(QueueRequest queueRequest, Error error)
   {
       int ret;
       /* Encode request message to open substream. */
       _queueRequest.clear();

       _queueRequest.streamId(queueRequest.streamId());
       _queueRequest.domainType(queueRequest.domainType());
       _queueRequest.serviceId(_tunnelStream.serviceId());
      
       _queueRequest.lastOutSeqNum(lastOutSeqNum());
       _queueRequest.lastInSeqNum(lastInSeqNum());
       _queueRequest.sourceName(queueRequest.sourceName());
       _queueRequest.opCode(((QueueRequestImpl)queueRequest).opCode());

       TunnelStreamBuffer tunnelBuffer = _tunnelStream.getBuffer(128 + _queueRequest.requestMsgBufferSize(), false, true, error);
       if (tunnelBuffer == null)
           return error.errorId();

       _encIter.clear();
       _encIter.setBufferAndRWFVersion(tunnelBuffer, _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());

       if ((ret = _queueRequest.encode(_encIter))
               < CodecReturnCodes.SUCCESS)
       {
           _tunnelStream.releaseBuffer(tunnelBuffer, error);
           error.errorId(ret);
           error.text("Substream request encode failed");
           return ReactorReturnCodes.FAILURE;
       }

       tunnelBuffer.setCurrentPositionAsEndOfEncoding();

       if ((_tunnelStream._traceFlags & TunnelStreamTraceFlags.ACTIONS) > 0)
           System.out.println("<!-- TunnelTrace: Sending substream request. -->");
       
       _tunnelStream._outboundTransmitList.push(tunnelBuffer, TunnelStreamBuffer.RETRANS_LINK);
       _tunnelStream.tunnelStreamManager().addTunnelStreamToDispatchList(_tunnelStream);

       _state = TunnelSubstreamState.WAITING_SUBSTREAM_REFRESH;
       error.errorId(ReactorReturnCodes.SUCCESS);
       return ReactorReturnCodes.SUCCESS;
       
   }
   
   int readMsg(Msg deliveredMsg, Error error)
   {
       int ret;
       
       switch (_state)
       {
           case WAITING_SUBSTREAM_REFRESH:
           {    
               State state;
               
               if (deliveredMsg.containerType() != DataTypes.MSG)
               {
                   error.errorId(ReactorReturnCodes.FAILURE);
                   error.text("Received unexpected container type " + deliveredMsg.containerType()
                           + " while establishing substream.");
                   return ReactorReturnCodes.FAILURE;
               }

               _decIter.clear();
               _decIter.setBufferAndRWFVersion(deliveredMsg.encodedDataBody(), _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());
               if ((ret = _encSubMsg.decode(_decIter)) < CodecReturnCodes.SUCCESS)
               {
                   error.errorId(ret);
                   error.text("Failed to decode substream Msg.");
                   return ReactorReturnCodes.FAILURE;
               }
               
               switch (_encSubMsg.msgClass())
               {
                   case MsgClasses.REFRESH:
                   {
                       RefreshMsg refreshMsg = (RefreshMsg)_encSubMsg;
                       state = refreshMsg.state();

                       switch(state.streamState())
                       {
                           case StreamStates.OPEN:
                               if (state.dataState() != DataStates.OK)
                                   return ReactorReturnCodes.SUCCESS;
                               else
                                   _state = TunnelSubstreamState.OPEN;
                               break;

                           default:
                               _tunnelStream._streamIdtoQueueSubstreamTable.remove(_streamId);
                               if ((ret = close(error)) != ReactorReturnCodes.SUCCESS)
                                   return ret;
                               break;
                       }

                       if ((ret = _queueRefresh.decode(_decIter, refreshMsg)) != CodecReturnCodes.SUCCESS)
                       {
                           error.errorId(ret);
                           error.text("Failed to decode substream refresh header.");
                           return ReactorReturnCodes.FAILURE;
                       }

                       /* If existing substream was closed, stop here. */
                       if (state.streamState() != StreamStates.OPEN)
                       {
                           _tunnelStream.queueMsgReceived(_queueRefresh, refreshMsg);
                           return ReactorReturnCodes.SUCCESS;
                       }

                       /* Now that stream is open again, sequence numbers in refresh 
                        * will indicate if we have any messages that need to be resent. */
                       if (_persistFile != null)
                       {
                           if ((ret = _persistFile.retransmitBuffers(_queueRefresh.lastInSeqNum(), _msg, _encIter, _dIter, error))
                                   != ReactorReturnCodes.SUCCESS)
                               return ret;
                       }

                       /* Resent whatever we can, so take current state from the refresh. */
                       lastOutSeqNum(_queueRefresh.lastInSeqNum());
                       
                       /* if last received sequence number is 0 or last sent one in refresh is 0,
                        * use last sent one from refresh */
                       if (lastInSeqNum() == 0 || _queueRefresh.lastOutSeqNum() == 0)
                       {
                           lastInSeqNum(_queueRefresh.lastOutSeqNum());
                       }

                       /* callback listener with RDM QueueRefresh */
                       _tunnelStream.queueMsgReceived(_queueRefresh, refreshMsg);
                       if (state.streamState() != StreamStates.OPEN)
                           return ReactorReturnCodes.SUCCESS;

                       /* Generate QueueAcks for any messages that the QueueRefresh freed. */
                       if (_persistFile != null && (ret = _persistFile.sendLocalQueueAcks(_encIter, _decIter, error)) !=
                                   ReactorReturnCodes.SUCCESS)
                           return ret;
                       
                       break;
                   }

                   case MsgClasses.STATUS:

                       StatusMsg statusMsg = (StatusMsg)_encSubMsg;
                       state = statusMsg.state();
                       
                       if (statusMsg.checkHasState())
                       {
                           switch(state.streamState())
                           {
                               case StreamStates.OPEN:
                                   if (state.dataState() != DataStates.OK)
                                       return ReactorReturnCodes.SUCCESS;
                                   break;

                               default:
                                   _tunnelStream._streamIdtoQueueSubstreamTable.remove(_streamId);
                                   if ((ret = close(error)) != ReactorReturnCodes.SUCCESS)
                                       return ret;
                                   break;
                           }

                           if ((ret = _queueStatus.decode(_decIter, statusMsg)) != CodecReturnCodes.SUCCESS)
                           {
                               error.errorId(ret);
                               error.text("Failed to decode substream refresh header.");
                               return ReactorReturnCodes.FAILURE;
                           }
                           
                           /* callback listener with RDM QueueStatus */
                           _tunnelStream.queueMsgReceived(_queueStatus, statusMsg);

                           /* If existing substream was closed, stop here. */
                           if (state.streamState() != StreamStates.OPEN)
                               return ReactorReturnCodes.SUCCESS;
                       }
                       else
                       {
                           /* No state change; just deliver the message. */
                           _queueStatus.clear();
                           _queueStatus.streamId(statusMsg.streamId());
                           _queueStatus.domainType(statusMsg.domainType());
                           _tunnelStream.queueMsgReceived(_queueStatus, statusMsg);
                       }

                       break;

                   default:
                       error.errorId(ReactorReturnCodes.FAILURE);
                       error.text("Received unexpected substream MsgClass " + _encSubMsg.msgClass()
                               + " while establishing substream.");
                       return ReactorReturnCodes.FAILURE;
               }
               
               if ((_tunnelStream._traceFlags & TunnelStreamTraceFlags.ACTIONS) > 0)
               {
                   System.out.println("<!-- TunnelTrace: Substream established on stream "
                           + deliveredMsg.streamId() + ", queue ready -->");
               }

               _tunnelStream._recvLastSeqNum = ((TunnelStreamMsg.TunnelStreamData)_tunnelStream._tunnelStreamMsg).seqNum();

               /* Send any queued buffers. */
               _tunnelStream.tunnelStreamManager().addTunnelStreamToDispatchList(_tunnelStream);
               return ReactorReturnCodes.SUCCESS;
           }

           case OPEN:
           {
               if (deliveredMsg.containerType() != DataTypes.MSG)
               {
                   error.errorId(ReactorReturnCodes.FAILURE);
                   error.text("Unexpected container type: " + deliveredMsg.containerType());
                   return ReactorReturnCodes.FAILURE;
               }

               _decIter.clear();
               _decIter.setBufferAndRWFVersion(deliveredMsg.encodedDataBody(), _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());
               if ((ret = _encSubMsg.decode(_decIter)) < CodecReturnCodes.SUCCESS)
               {
                   error.errorId(ret);
                   error.text("Failed to decode substream message.");
                   return ReactorReturnCodes.FAILURE;
               }

               switch(_encSubMsg.msgClass())
               {
                   case MsgClasses.GENERIC:

                       int opcode = getSubstreamOpcode(_encSubMsg);
                       
                       switch(opcode)
                       {
                           case QueueMsgImpl.OpCodes.DATA:
                           {
                               if ((ret = _queueData.decode(_decIter, _encSubMsg)) != CodecReturnCodes.SUCCESS)
                               {
                            	   error.errorId(ret);
                            	   error.text("Substream header decode failed");
                            	   return ReactorReturnCodes.FAILURE;
                               }
                               
                               lastInSeqNum(_queueData.seqNum());
                               error.errorId(CodecReturnCodes.SUCCESS);

                               _tunnelStream.queueMsgReceived(_queueData, _encSubMsg);

                               TunnelStreamBuffer ackBuffer;

                               /* Encode the ack substream header and add it to the outbound queue. */

                               /* This should be the next message in order. */
                               _queueAck.clear();                                      
                               _queueAck.streamId(streamId());
                               _queueAck.domainType(domainType());
                               _queueAck.serviceId(_tunnelStream.serviceId());
                               _queueAck.seqNum(_queueData.seqNum());
                               _queueAck.sourceName().data(_queueData.destName().data().duplicate(), _queueData.destName().position(), _queueData.destName().length()); 
                               _queueAck.destName().data(_queueData.sourceName().data().duplicate(), _queueData.sourceName().position(), _queueData.sourceName().length()); 
                               _queueAck.identifier(_queueData.identifier());

                               if ((ackBuffer = _tunnelStream.getBuffer(_queueAck.ackMsgBufferSize(), false, true, error)) == null)
                                   return error.errorId();

                               ackBuffer.isApplicationBuffer(false);

                               _encIter.clear();
                               _encIter.setBufferAndRWFVersion(ackBuffer, _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());

                               //subAckHeader
                               if ((ret = _queueAck.encode(_encIter)) != CodecReturnCodes.SUCCESS)
                               {
                                   _tunnelStream.releaseBuffer(ackBuffer, error);
                                   error.errorId(ret);
                                   error.text("Substream ack header encode failed.");
                                   return ReactorReturnCodes.FAILURE;
                               }
                               
                               ackBuffer.setCurrentPositionAsEndOfEncoding();
                               _tunnelStream._outboundTransmitList.push(ackBuffer, TunnelStreamBuffer.RETRANS_LINK);
                               _tunnelStream.tunnelStreamManager().addTunnelStreamToDispatchList(_tunnelStream);

                               return ReactorReturnCodes.SUCCESS;
                           }

                           case QueueMsgImpl.OpCodes.DEAD_LETTER:
                           {
                              if ((ret = _queueDataExpired.decode(_decIter, _encSubMsg)) != CodecReturnCodes.SUCCESS)
                               {
                            	   error.errorId(ret);
                            	   error.text("Substream header decode failed");
                            	   return ReactorReturnCodes.FAILURE;
                               }

                               lastInSeqNum(_queueDataExpired.seqNum());

                               error.errorId(CodecReturnCodes.SUCCESS);
                               _tunnelStream.queueMsgReceived(_queueDataExpired, _encSubMsg);                               

                               TunnelStreamBuffer ackBuffer;

                               /* Encode the ack substream header and add it to the outbound queue. */

                               /* This should be the next message in order. */
                               _queueAck.clear();
                               
                               _queueAck.streamId(streamId());
                               _queueAck.domainType(domainType());
                               _queueAck.serviceId(_tunnelStream.serviceId());
                               _queueAck.seqNum(_queueDataExpired.seqNum());
                               _queueAck.sourceName().data((queueName().data()));

                               _queueAck.destName().data(_queueDataExpired.sourceName().data(), _queueDataExpired.sourceName().position(),
                                       _queueDataExpired.sourceName().length());
                               _queueAck.identifier(_queueDataExpired.identifier());
                               
                               if ((ackBuffer = _tunnelStream.getBuffer(_queueAck.ackMsgBufferSize(), false, true, error)) == null)
                                   return error.errorId();

                               ackBuffer.isApplicationBuffer(false);

                               _encIter.clear();
                               _encIter.setBufferAndRWFVersion(ackBuffer, _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());

                               //subAckHeader
                               if ((ret = _queueAck.encode(_encIter)) != CodecReturnCodes.SUCCESS)
                               {
                                   _tunnelStream.releaseBuffer(ackBuffer, error);
                                   error.errorId(ret);
                                   error.text("Substream ack header encode failed.");
                                   return ReactorReturnCodes.FAILURE;
                               }
                               
                               ackBuffer.setCurrentPositionAsEndOfEncoding();
                               _tunnelStream._outboundTransmitList.push(ackBuffer, TunnelStreamBuffer.RETRANS_LINK);
                               _tunnelStream.tunnelStreamManager().addTunnelStreamToDispatchList(_tunnelStream);

                               return ReactorReturnCodes.SUCCESS;
                           }

                           case QueueMsgImpl.OpCodes.ACK:
                           {
                        	   
                               if ((ret = _queueAck.decode(_decIter, _encSubMsg)) != CodecReturnCodes.SUCCESS)
                               {
                                   error.errorId(ret);
                                   error.text("Substream header decode failed");
                                   return ReactorReturnCodes.FAILURE;
                               }
                               
                               int seqNum = _queueAck.seqNum();

                               /* Notify application that queue provider has received this message. */
                               _tunnelStream.queueMsgAcknowledged(
                                       _queueAck,
                                       _encSubMsg);
                               
                               if (_persistFile != null)
                                   _persistFile.releasePersistenceBuffers(seqNum);

                               break;
                           }
                           
                           case MsgClasses.STATUS:
                               break;

                           default:
                               error.errorId(ReactorReturnCodes.FAILURE);
                               error.text("Unhandled substream header opcode.");
                               return ReactorReturnCodes.FAILURE;
                       }
                       break;

                   default:
                       error.errorId(ReactorReturnCodes.FAILURE);
                       error.text("Unhandled substream message class.");
                       return ReactorReturnCodes.FAILURE;
               }

               /* Need to send acknowledgement */
               _tunnelStream.tunnelStreamManager().addTunnelStreamToDispatchList(_tunnelStream);
               return ReactorReturnCodes.SUCCESS;
           }
           
           case NOT_OPEN:

               error.errorId(ReactorReturnCodes.SUCCESS);
               return ReactorReturnCodes.SUCCESS;

           default:
               error.errorId(ReactorReturnCodes.FAILURE);
               error.text("Unknown queue substream state.");
               return ReactorReturnCodes.FAILURE;

       }
   }

   int saveMsg(TunnelStreamBuffer buffer, Error error)
   {
       if (_persistFile != null)
           return _persistFile.saveMsg(buffer, error);
        else
        {
            buffer.persistenceBuffer(this, null);
            return ReactorReturnCodes.SUCCESS;
        }
   }
   
   int setBufferAsTransmitted(TunnelStreamBuffer tunnelBuffer, Error error)
   {
       TunnelStreamPersistenceBuffer persistenceBuffer;
       int seqNum;
       int ret;

       seqNum = lastOutSeqNum() + 1;
       
       if ((persistenceBuffer = tunnelBuffer.persistenceBuffer()) != null)
       {
           _persistFile.setBufferAsTransmitted(persistenceBuffer);

           /* Don't need to reference persistence buffer anymore (nothing more to update). */
           tunnelBuffer.persistenceBuffer(null, null);
       }
       else
       {
           /* Increment sequence number and use that. */
           seqNum = lastOutSeqNum() + 1;
       }
       
       lastOutSeqNum(seqNum);

       /* Set buffer to queue message and update its sequence number / stream ID */
       tunnelBuffer.setAsInnerReadBuffer();

       /* Assign sequence number and update it in substream header. */
       _encIter.clear();
       _encIter.setBufferAndRWFVersion(tunnelBuffer, _tunnelStream.classOfService().common().protocolMajorVersion(), _tunnelStream.classOfService().common().protocolMinorVersion());
       if ((ret = _encIter.replaceSeqNum(seqNum)) != CodecReturnCodes.SUCCESS)
       {
           error.errorId(ret);
           error.text("Failed to update sequence number on substream message.");
           return ReactorReturnCodes.FAILURE;
       }

       /* Update timeout. */
       if (!tunnelBuffer.timeoutIsCode())
       {
           long newTimeoutNsec = tunnelBuffer.timeoutNsec() - System.nanoTime();
           if (newTimeoutNsec < 1 * TunnelStreamUtil.NANO_PER_MILLI)
               newTimeoutNsec = 1 * TunnelStreamUtil.NANO_PER_MILLI;

           TunnelStreamUtil.replaceQueueDataTimeout(tunnelBuffer.data(),
                   newTimeoutNsec / TunnelStreamUtil.NANO_PER_MILLI);                                   
       }

       tunnelBuffer.setToFullWritebuffer();

       return ReactorReturnCodes.SUCCESS;
   }

   void releasePersistenceBuffer(TunnelStreamPersistenceBuffer persistBuffer)
   {
       if (_persistFile != null)
           _persistFile.releasePersistenceBuffer(persistBuffer);
   }
   
   private int getSubstreamOpcode(Msg substreamMsg)
   {
       int opcode = 0;

       if (substreamMsg.extendedHeader() != null)
       {
           opcode = substreamMsg.extendedHeader().data().get(substreamMsg.extendedHeader().position());
       }
       
       return opcode;
   }

}
