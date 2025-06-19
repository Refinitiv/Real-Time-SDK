/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Objects;

class RipcProtocolFunctions implements ProtocolFunctions
{
	static final int RIPC_PING_BUFFER_SIZE = 3;

	ReadBufferStateMachine _readBufferStateMachine;
	RsslSocketChannel rsslSocketChannel;
	ByteBuffer pingBuffer;

	public RipcProtocolFunctions(RsslSocketChannel rsslSocketChannel) {
		this.rsslSocketChannel = rsslSocketChannel;
		_readBufferStateMachine = rsslSocketChannel._readBufStateMachine;
		this.pingBuffer = ByteBuffer.allocate(RIPC_PING_BUFFER_SIZE);
		applyPingRipc(pingBuffer);
	}

	public void applyPingRipc(ByteBuffer pingBuffer) {
		pingBuffer.putShort(0, (short) RsslSocketChannel.RIPC_HDR_SIZE); // ripc header length
		pingBuffer.put(2, (byte)2); // ripc flag indicating data
	}

	@Override
	public int readPrependTransportHdr(int bytesRead, ReadArgsImpl readArgs, Error error) {
		return 0;
	}

	@Override
	public int prependTransportHdr(TransportBufferImpl buffer, int ripcFlags) {
		buffer.populateRipcHeader(ripcFlags);
		return 0;
	}

	@Override
	public int initChnlReadFromChannel(ByteBuffer dst, Error error) throws IOException {
		int bytesRead = rsslSocketChannel.read(dst);

		if (bytesRead > 0)
		{

			// note that we could cache the msgLen, but normally we should be reading an entire ConnectAck/ConnectNak here.
			if (dst.position() > (rsslSocketChannel.HTTP_HEADER4 + 2))
			{
				int messageLength = (dst.getShort(rsslSocketChannel.HTTP_HEADER4) & 0xFF);
				if (dst.position() >= (rsslSocketChannel.HTTP_HEADER4  + messageLength + rsslSocketChannel.CHUNKEND_SIZE))
				{
					// we have at least one complete message
					return dst.position();
				}
			}
		}
		else if (bytesRead == -1)
		{
			// The connection was closed by far end (server). Need to try another protocol.
			rsslSocketChannel._initChnlState = RsslSocketChannel.InitChnlState.RECONNECTING;
			return -1;
		}

		// we don't have a complete message, or no bytes were read.
		return 0;
	}

	@Override
	public int initChnlReadFromChannelProvider(ByteBuffer dst, Error error) throws IOException {
		int bytesRead = rsslSocketChannel.read(dst);

		if (bytesRead <= 1 && rsslSocketChannel._providerHelper != null && rsslSocketChannel._providerHelper.wininetStreamingComplete()) {
			if (RsslHttpSocketChannelProvider.debugPrint)
				System.out.println(" Got pipe notify  " + bytesRead);
			rsslSocketChannel._providerHelper._pipeNode._pipe.sink().close();
			rsslSocketChannel._providerHelper._pipeNode._pipe.source().close();
			rsslSocketChannel._providerHelper._pipeNode.returnToPool();
			// fake active for later close() to distinguish if coming from this
			// notify or timeout & INITIALING state
			rsslSocketChannel._state = ChannelState.ACTIVE;
			return -1;
		}

		if (bytesRead > 0) {
			if (dst.position() > (rsslSocketChannel.HTTP_HEADER4 + 2)) {
				if (rsslSocketChannel.checkIsProviderHTTP(dst)) {
					rsslSocketChannel._isProviderHTTP = true;
				}

				if (rsslSocketChannel._isProviderHTTP != true) {
					int messageLength = (dst.getShort(rsslSocketChannel.HTTP_HEADER4) & 0xFF);
					if (dst.position() >= (rsslSocketChannel.HTTP_HEADER4 + messageLength + rsslSocketChannel.CHUNKEND_SIZE)) {
						// we have at least one complete message
						return dst.position();
					}
				} else {
					return dst.position();
				}
			} else {
				if (RsslHttpSocketChannelProvider.debugPrint)
					System.out.println("bufferPos = " + dst.position());
			}
		} else if (bytesRead == -1) {
			rsslSocketChannel._initChnlState = RsslSocketChannel.InitChnlState.RECONNECTING;
			return -1;
		}

		return 0;
	}

	@Override
	public int prependInitChnlHdr(ByteBuffer sourceData, ByteBuffer destinationData) {
		if (Objects.nonNull(destinationData) && !Objects.equals(sourceData, destinationData)) {
			destinationData.put(sourceData);
			sourceData.clear();
		}
		return 0;
	}

	@Override
	public int messageLength() {
		return _readBufferStateMachine._currentMsgRipcLen;
	}

	@Override
	public void unsetMessageLength() {
		_readBufferStateMachine._currentMsgRipcLen = ReadBufferStateMachine.UNKNOWN_LENGTH;
	}

	@Override
	public int additionalHdrLength() {
		return 0;
	}

	@Override
	public int estimateHeaderLength()
	{
		return RsslSocketChannel.RIPC_HDR_SIZE;
	}

	@Override
	public int entireHeaderLength() {
		return RsslSocketChannel.RIPC_HDR_SIZE;
	}

	@Override
	public int writeAdditionalMessagePrefix(TransportBufferImpl buffer) {
		return 0;
	}

	@Override
	public int packBuffer(TransportBufferImpl packedBuffer, boolean reserveNextPackedHdr, Channel chnl, Error error) {

		return TransportBufferImpl.packBuffer(packedBuffer, reserveNextPackedHdr, chnl, error);
	}

    @Override
    public int ping(Error error) throws IOException {
		// send ping buffer
		pingBuffer.rewind();
        return RsslSocketChannel.RIPC_HDR_SIZE - rsslSocketChannel._scktChannel.write(pingBuffer);
    }

	@Override
	public int pong(Error error) throws IOException {
		return ping(error);
	}

	@Override
	public int closeChannel(Error error) {
		return rsslSocketChannel.closeSocketChannel(error);
	}

	@Override
	public boolean isRWFProtocol() {
		return true;
	}

	@Override
	public int writeCompressed(TransportBufferImpl buffer, WriteArgs writeArgs, Error error)
	{
		return rsslSocketChannel.writeNormalCompressed(buffer, writeArgs, error);
	}

	@Override
	public int populateFragment(BigBuffer bigBuffer, boolean firstFragment, TransportBufferImpl writeBuffer, int flags, WriteArgs writeArgs) 
	{
		return writeBuffer.populateFragment(bigBuffer, firstFragment, flags, writeArgs);
	}

	@Override
	public int writeFragmentCompressed(BigBuffer bigBuffer, TransportBufferImpl fragment, WriteArgs writeArgs, boolean firstFragment, Error error)
	{
		return rsslSocketChannel.writeFragmentCompressed(bigBuffer, fragment, writeArgs, firstFragment, error);
	}

	@Override
	public TransportBufferImpl getBigBuffer(int size)
	{
		return rsslSocketChannel.getBigBuffer(size);
	}

	@Override
	public int writeFragmentSuffix(BigBuffer bigBuffer)
	{
		return 0;
	}

	@Override
	public boolean checkCompressionFragmentedMsg(int messageSize) {
	
		return (messageSize < rsslSocketChannel._sessionCompLowThreshold);
	}

	@Override
	public boolean isPingMessage() {
		return (_readBufferStateMachine.currentMessageLength() == Ripc.Lengths.HEADER);
	}
	
	@Override
	public boolean writeAsFragmentedMessage(TransportBufferImpl buffer)
	{
		return ((buffer._data.position() + estimateHeaderLength()) > rsslSocketChannel._internalMaxFragmentSize);
	}

	@Override
	public int totalPayloadSize() {
		return 0;
	}

	@Override
	public int remaingBytesAfterPausing(BigBuffer bigBuffer) {
		return bigBuffer._data.limit() - bigBuffer._data.position();
	}
}
