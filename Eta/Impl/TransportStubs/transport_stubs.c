/*
 *|---------------------------------------------------------------
 *|					Copyright (C) 2003 Reuters,                 --
 *|         1400 Kensington Road, Oak Brook, IL. 60523          --
 *| All rights reserved. Duplication or distribution prohibited --
 *|---------------------------------------------------------------
 */
#include "rtr/rsslTransport.h"


/**
 * @brief Gets the IP address of a hostname
 *
 * Typical use:<BR>
 * This function gets the IP address of a hostname in host byte order.
 *
 * @param hostName Hostname to get IP address for
 * @param ipAddr IP address of the hostname
 * @return RsslRet RSSL return value
 */
RsslRet rsslHostByName(RsslBuffer *hostName, RsslUInt32 *ipAddr)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Gets the user name
 *
 * Typical use:<BR>
 * This function gets the user name that the owner of the current process is
 * logged in under.  The returned user name is truncated if length of user name
 * from machine is greater than length of userName->length.
 *
 * @param userName User name of user
 * @return RsslRet RSSL return value
 */
RsslRet rsslGetUserName(RsslBuffer *userName)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Initializes the RSSL API and all internal members
 *
 * Typical use:<BR>
 * This is the first function called when using the RSSL. It 
 * initializes internal data structures and pre-allocates some memory.
 *
 * @param rsslLocking Method of locking used.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or RsslInitRets value.
 * @see RsslReturnCodes
 * @see RsslLockingTypes
 */
RsslRet rsslInitialize(RsslLockingTypes rsslLocking, RsslError *error)
{
	return RSSL_RET_FAILURE;
}
											
/**
 * @brief Initializes the RSSL API and all internal members
 *
 * Typical use:<BR>
 * This is the first function called when using the RSSL. It 
 * initializes internal data structures and pre-allocates some memory.
 *
 * @param rsslInitOpts Initialize options.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or RsslInitRets value.
 * @see RsslReturnCodes
 * @see RsslInitializeExOpts
 */
RsslRet rsslInitializeEx(RsslInitializeExOpts *rsslInitOpts, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Uninitializes the RSSL API and all internal members
 *
 * Typical use:<BR>
 * This is the last function called when using the RSSL.  It 
 * uninitializes internal data structures and deletes any allocated
 * memory.
 *
 * @return RsslRet RSSL return value
 */
RsslRet rsslUninitialize()
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Gets information about the channel
 *
 * Typical use:<BR>
 * If information about the RsslChannel is needed, such as maxFragmentSize or 
 * maxOutputBuffers, this function can be called to retrieve this information.
 *
 * @param chnl RSSL Channel to get information about
 * @param info RSSL Channel Info structure to be populated
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RsslRet rsslGetChannelInfo(RsslChannel *chnl, RsslChannelInfo *info, RsslError *error)
{
	return RSSL_RET_FAILURE;
}
											   
/**
 * @brief Allows changing some I/O values programmatically
 * 
 * Typical use:<BR>
 * If an I/O value needs to be changed, this is used
 *
 * @param chnl RSSL Channel to change I/O value for
 * @param code Code of I/O Option to change
 * @param value Value to change Option to
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RsslRet rsslIoctl(RsslChannel *chnl, RsslIoctlCodes code, void *value, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Returns the total number of used buffers for this channel
 *
 * Typical use: <BR>
 * This function can be called to find out the number of used buffers
 * for the calling channel.  This, in combination with the maxOutputBuffers
 * obtained from the RsslChannelInfo call, can be used to monitor and 
 * potentially throttle buffer usage.
 *
 * @param chnl RSSL Channel to obtain buffer usage for
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslInt32 If less than 0, this is an RsslRet value, otherwise
 *					 it is the total number of buffers in use by this channel
 */
RsslInt32 rsslBufferUsage(RsslChannel *chnl, RsslError *error)
{
	return 0;
}

/**
 * @brief Connects a client to a listening server
 *
 * Typical use:<BR>
 * 1. Initialize RsslConnectOptions<BR>
 * 2. Set RsslConnectOptions to desired values<BR>
 * 3. Call rsslConnect to create RsslChannel<BR>
 * 4. Read or write with the RsslChannel<BR>
 *
 * @param opts Options used when connecting
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslChannel* Connected RSSL channel or NULL
 */
RsslChannel* rsslConnect(RsslConnectOptions *opts, RsslError *error)
{
	return NULL;
}

/**
 * @brief Reconnects a client to a listening server - used for tunneling connection types
 *
 * Typical use:<BR>
 * 1. Call rsslReconnectClient with already active RsslChannel
 *
 * @note This function only performs a reconnection for client connections and
 * only when tunnelType is enabled.  This function is designed so the application
 * can proactively bridge a connection to keep data flow alive through proxy servers.
 *
 * @param chnl RSSL Channel to reconnect
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet of Success or failure
 */
RsslRet rsslReconnectClient(RsslChannel *chnl, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Gets information about the server
 *
 * Typical use:<BR>
 * If information about the RsslServer is needed, such as peakBufferUsage,
 * this function can be called to retrieve this information.
 *
 * @param srvr RSSL Server to get information about
 * @param info RSSL Server Info structure to be populated
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RsslRet rsslGetServerInfo(RsslServer *srvr, RsslServerInfo *info, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Allows changing some I/O values programmatically for an Rssl Server
 *
 * Typical use:<BR>
 * If an I/O value needs to be changed for a server, this is 
 * used. Currently this only supports changing the shared pool size.
 *
 * @param srvr RSSL Server to change I/O value for
 * @param code Code of I/O Option to change
 * @param value Value to change Option to
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RsslRet rsslServerIoctl(RsslServer *srvr, RsslIoctlCodes code, void *value, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Returns the total number of used buffers for the server
 *
 * Typical use: <BR>
 * This function can be called to find out the number of used buffers
 * for the server.  This, in combination with the bufferPoolSize
 * used as input to the rsslBind call, can be used to monitor and 
 * potentially throttle buffer usage.
 *
 * @param srvr RSSL Server to obtain buffer usage for
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslInt32 If less than 0, this is an RsslRet value, otherwise
 *					 it is the total number of buffers in use by the server
 */
RsslInt32 rsslServerBufferUsage(RsslServer *srvr, RsslError *error)
{
	return 0;
}

/**
 * @brief Creates an RSSL Server by binding to a port 
 *
 * Typical use:<BR>
 * 1. Initialize RsslBindOptions<BR>
 * 2. Set RsslBindOptions to desired values<BR>
 * 3. Call rsslBind to create RsslServer<BR>
 *
 * @param opts Options used when binding
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslServer* Bound RSSL server or NULL
 */
RsslServer* rsslBind(RsslBindOptions *opts, RsslError *error)
{
	return NULL;
}

/**
 * @brief Accepts an incoming connection and returns a channel which corresponds to it
 *
 * Typical use:<BR>
 * After a server is created using the rsslBind call, the rsslAccept call can
 * be made.  When the socketId of the server detects something to be read, 
 * this will check for incoming client connections.  When a client
 * successfully connects, a RsslChannel is returned which corresponds to this
 * connection.  This channel can be used to read or write with the connected 
 * client. If a clients connect message is not accepted (nakMount = RSSL_TRUE),
 * a negative acknowledgment can be sent when the rsslInitChannel handshake
 * is completed. 
 *
 * @param srvr RSSL Server to accept incoming connections on
 * @param opts RSSL Accept Options 
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslChannel* Accepted RSSL channel or NULL in failure. 
 */
RsslChannel* rsslAccept(RsslServer *srvr, RsslAcceptOptions *opts, RsslError *error)
{
	return NULL;
}

/**
 * @brief Closes an RSSL Server
 * 
 * Typical use:<BR>
 * When done using a Server, this call closes it.  
 * Active channels connected to this server will not be closed; 
 * this allows them to continue receiving data even if the server
 * is not accepting more connections.  
 *
 * @param srvr RSSL Server to close
 * @param error RSSL Error, to be populated in event of an error 
 * @return RsslRet RSSL return value
 */
RsslRet rsslCloseServer(RsslServer *srvr, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Continues channel initialization for non-blocking channels
 *
 * Typical use:<BR>
 * 1. Connect using rsslConnect()
 * 2. While Channel state is ::RSSL_CH_STATE_INITIALIZING and the channels socketId
 *    detects data to read,  call rsslInitChannel()
 *
 * @note This is not necessary for blocking channels, which will return in the ::RSSL_CH_STATE_ACTIVE state after the rsslConnect() call.  
 *
 * @param chnl RSSL Channel to continue initialization on
 * @param inProg InProg Info for compatibility mode reconnections
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or RsslReturnCodes value
 * @see RsslReturnCodes, RsslInProgInfo
 */
RsslRet rsslInitChannel(RsslChannel *chnl, RsslInProgInfo *inProg, RsslError *error)
{
	return RSSL_RET_FAILURE;
}
											 			   
/**
 * @brief Closes an RSSL Channel
 *
 * Typical use:<BR>
 * When done using a Channel, this call closes it.
 *
 * @param chnl RSSL Channel to close
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RsslRet rsslCloseChannel(RsslChannel *chnl, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Reads on a given channel
 *
 * Typical use:<BR>
 * rsslRead is called and returns buffer with any 
 * data read from the channels socketId.  The buffer is only good
 * until the next time rsslRead is called. The buffer used for reading
 * is populated by this call, it is not necessary to use rsslGetBuffer
 * to create a buffer. rsslRead will assign readRet a positive value if there is more
 * data to read, ::RSSL_RET_READ_WOULD_BLOCK if the read call is blocked, or a failure
 * code. 
 *
 * @param chnl RSSL Channel to read from
 * @param readRet RsslRet value or RsslReturnCodes value which is the return value of read.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslBuffer Buffer that contains data read from network 
 * @see RsslReturnCodes
 */
RsslBuffer* rsslRead(RsslChannel *chnl, RsslRet *readRet, RsslError *error)
{
	return NULL;
}

/**
 * @brief Extended reads on a given channel
 *
 * Typical use:<BR>
 * rsslReadEx is called and returns buffer with any 
 * data read from the channels socketId.  The buffer is only good
 * until the next time rsslReadEx is called. The buffer used for reading
 * is populated by this call, it is not necessary to use rsslGetBuffer
 * to create a buffer. rsslReadEx will assign readRet a positive value if there is more
 * data to read, ::RSSL_RET_READ_WOULD_BLOCK if the read call is blocked, or a failure
 * code. 
 *
 * @param chnl RSSL Channel to read from
 * @param RsslReadInArgs input arguments (RsslReadInArgs)
 * @param RsslReadOutArgs various output values from the read
 * @param readRet RsslRet value or RsslReturnCodes value which is the return value of read
 * @return RsslBuffer Buffer that contains data read from network 
 * @see RsslReturnCodes
 */
RsslBuffer* rsslReadEx(RsslChannel *chnl, RsslReadInArgs *readInArgs, RsslReadOutArgs *readOutArgs, RsslRet *readRet, RsslError *error)
{
	return NULL;
}

/**
 * @brief Retrieves a RsslBuffer for use
 *
 * Typical use: <BR>
 * This is called when a buffer is needed to write data to.  Generally,
 * the user will populate the RsslBuffer structure and then pass it to
 * the rsslWrite function. 
 *
 * @param chnl RSSL Channel who requests the buffer
 * @param size Size of the requested buffer
 * @param packedBuffer Set to RSSL_TRUE if you plan on packing multiple messages into the same buffer
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslBuffer RSSL buffer to be filled in with valid memory
 * @see RsslReturnCodes
 */
RsslBuffer* rsslGetBuffer(	RsslChannel *chnl, RsslUInt32 size, RsslBool  packedBuffer, RsslError *error)
{
	return NULL;
}


/**
 * @brief Releases a RsslBuffer after use
 * 
 * Typical use: <BR>
 * This is called when a buffer is done being used.  
 * The rsslWrite function will release the buffer if it 
 * successfully writes.  The user should only need to use 
 * this function when they get a buffer that they do not need
 * or when rsslWrite fails.
 *
 * @param buffer RSSL buffer to be released
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value
 */
RsslRet rsslReleaseBuffer(RsslBuffer *buffer, RsslError *error)
{
	return RSSL_RET_FAILURE;
}


/**
 * @brief Allows user to pack multiple RSSL encoded messages into the same RSSL Buffer
 *
 * Typical use: <BR>
 * This is called when the application wants to perform message packing.
 * Call rsslGetBuffer to get a new buffer
 * Call necessary encode functions
 * Call rsslPackBuffer to write size of packed buffer and prepare for next packed buffer
 * Repeat above two steps until ready to write
 * Call rsslWrite with buffer
 *
 * @param chnl RsslChannel that the message is being sent on.
 * @param buffer Rssl buffer to be packed (length must be set to encoded size)
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslBuffer Buffer pointing to new position to encode data to
 */
RsslBuffer* rsslPackBuffer(RsslChannel *chnl, RsslBuffer *buffer, RsslError *error)
{
	return NULL;
}

/**
 * @brief Writes on a given channel
 *
 * Typical use:<BR>
 * rsslWrite is called after buffer is populated with a message.
 * This message is then written to the channels internal write buffer.  If write
 * is successful, the passed in buffer will be released automatically.  In the
 * event of a failure the user needs to call rsslReleaseBuffer.  In the 
 * success case, rsslWrite will return the number of bytes pending flush.  
 *
 * @note Data is not written across the network until rsslFlush is called. 
 *
 * @param chnl RSSL Channel to write to
 * @param buffer Buffer to write to the network
 * @param rsslPriority Priority to flush the message (high, medium, or low) 
 * @param writeFlags Flags for writing the buffer (RsslWriteFlags)
 * @param bytesWritten Returns total number of bytes written.
 * @param uncompressedBytesWritten Returns number of bytes written before compression.
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value, RsslReturnCodes value, or the number of bytes pending flush 
 * @see RsslReturnCodes, RsslWriteFlags
 */
RsslRet rsslWrite(RsslChannel *chnl, RsslBuffer *buffer, RsslWritePriorities rsslPriority, RsslUInt8	writeFlags, RsslUInt32 *bytesWritten, RsslUInt32 *uncompressedBytesWritten, RsslError *error)
{
	return RSSL_RET_FAILURE;
}	

/**
 * @brief Extended writes on a given channel
 *
 * Typical use:<BR>
 * rsslWrite is called after buffer is populated with a message.
 * This message is then written to the channels internal write buffer.  If write
 * is successful, the passed in buffer will be released automatically.  In the
 * event of a failure the user needs to call rsslReleaseBuffer.  In the 
 * success case, rsslWrite will return the number of bytes pending flush.  
 *
 * @note Data is not written across the network until rsslFlush is called. 
 *
 * @param chnl RSSL Channel to write to
 * @param buffer Buffer to write to the network
 * @param writeInArgs input arguments to the function (RsslWriteInArgs)
 * @param writeOutArgs various output values following the write(RsslWriteFlags)
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value, RsslReturnCodes value, or the number of bytes pending flush 
 * @see RsslReturnCodes, RsslWriteFlags
 */
RsslRet rsslWriteEx(RsslChannel *chnl, RsslBuffer *buffer, RsslWriteInArgs *writeInArgs, RsslWriteOutArgs *writeOutArgs, RsslError	*error)
{
	return RSSL_RET_FAILURE;
}


/**
 * @brief Flushes data waiting to be written on a given channel
 *
 * Typical use:<BR>
 * rsslFlush pushes the data in the write buffer out to the network.  
 * This should be called when write returns that there is data to flush.
 * Under certain circumstances, write will automatically flush data.
 *
 * @param chnl RSSL Channel to attempt flush on
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or the number of bytes pending flush
 */
RsslRet rsslFlush(RsslChannel *chnl, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Sends a heartbeat message
 *
 * Typical use:<BR>
 * rsslPing is called to send some type of ping or heartbeat message.
 * rsslPing will send only the message header across the network.  
 * This helps reduce overhead on the network, and does not incur any
 * cost of parsing or assembling a special ping message type.  
 * It is the user's responsibility to send the ping message  
 * in the correct time frame.  Since it is assumed 
 * a ping or heartbeat is only sent when other writing is not taking place,
 * rsslFlush is automatically called once.  The return value will be the number
 * of bytes left to flush.
 *
 * @param chnl RSSL Channel to send ping or heartbeat on
 * @param error RSSL Error, to be populated in event of an error
 * @return RsslRet RSSL return value or the number of bytes pending flush
 */
RsslRet rsslPing(RsslChannel *chnl, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/** 
 * @brief Programmatically extracts library and product version information that is compiled into this library
 *
 * User can call this function to programmatically extract version information, or <BR>
 * query version information externally (via 'strings' command or something similar<BR>
 * and grep for the following tags:<BR>
 * 'VERSION' - contains internal library version information such as node number (e.g. rssl1.4.F2)<BR>
 * 'PRODUCT' - contains product information such as load/package naming (e.g. upa7.0.0.L1)<BR>
 * @param pVerInfo RsslLibraryVersionInfo structure to populate with library version information
 * @see RsslLibraryVersionInfo
 */
void rsslQueryTransportLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	return;
}

/**
 * @brief Calculates necessary buffer size to encrypt content into
 *
 * Typical use:<BR>
 * User populates bufferToEncrypt with content they want to encrypt.
 * Calling this method will return the size of the buffer needed to contain the
 * encrypted output.  User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToEncrypt and the buffer for encrypted
 * output can be passed to rsslEncryptBuffer().  
 * 
 * @param bufferToEncrypt RsslBuffer populated with content the user wants to calculate encrypted size of.  buffer->length should represent the number of bytes contained in buffer->data. 
 * @return RsslUInt32 Number of bytes needed in an RsslBuffer to encrypt contents into
 */
RsslUInt32 rsslCalculateEncryptedSize(const RsslBuffer *bufferToEncrypt)
{
	return 0;
}

/**
 * @brief If possible, will perform weak encryption of passed in contents
 *
 * Typical use:<BR>
 * User populates bufferToEncrypt with content they want to encrypt.
 * User calculates necessary size for encrypted output buffer by calling rsslCalculateEncryptedSize()
 * User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToEncrypt and the buffer for encrypted
 * output can be passed to rsslEncryptBuffer(), which will attempt encryption.  
 * If the RsslChannel has a valid encryption key and the passed in buffers are valid, contents will be encrypted.
 * If the RsslChannel does not have a valid encryption key, most likely due to the connection not having the ability
 * to perform proper key exchange, contents will not be encrypted and an error will be returned.  
 *
 * @param chnl RsslChannel to encrypt contents for transmission on.  Each RsslChannel may have a different encryption key, including no encryption key.
 * @param bufferToEncrypt RsslBuffer populated with content the user wants to encrypt.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should hold unencrypted content.
 * @param encryptedOutput RsslBuffer with appropriate space to encrypt into.  buffer->length should represent the number of bytes available in buffer->data.
 * @param error RsslError, to be populated in event of an error
 * @return RsslRet RsslRet return value indicating success or failure type.
 */
RsslRet rsslEncryptBuffer(const RsslChannel *chnl, const RsslBuffer* bufferToEncrypt, RsslBuffer* encryptedOutput, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief If possible, will perform decryption of passed in encrypted contents.
 *
 * Typical use:<BR>
 * User populates bufferToDecrypt with encrypted content they want to decrypt.
 * User will populated a buffer to decrypt into with sufficient space to properly decrypt,
 * generally bufferToDecrypt->length bytes will be sufficient space for decryptedOutput buffer to 
 * perform proper decryption.
 * bufferToDecrypt and the buffer for decrypted output 
 * can be passed to to rsslDecryptBuffer(), which will attempt decryption.  
 * If the RsslChannel has a valid decryption key and the passed in buffers are valid, contents will be decrypted.
 * If the RsslChannel does not have a valid decryption key, most likely due to the connection not having the ability
 * to perform proper key exchange, contents will not be decrypted and an error will be returned.  
 *
 * @param chnl RsslChannel to decrypt contents for transmission on.  Each RsslChannel may have a different decryption key, including no decryption key.
 * @param bufferToDecrypt RsslBuffer populated with content the user wants to decrypt.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should contain encrypted content.
 * @param decryptedOutput RsslBuffer with appropriate space to decrypt into.  buffer->length should represent the number of bytes available in buffer->data.
 * @param error RsslError, to be populated in event of an error
 * @return RsslRet RsslRet return value indicating success or failure type.
 */
RsslRet rsslDecryptBuffer(const RsslChannel *chnl, const RsslBuffer* bufferToDecrypt, RsslBuffer* decryptedOutput, RsslError *error)
{
	return RSSL_RET_FAILURE;
}

/**
 * @brief Calculates necessary buffer size to convert the buffer to a hex dump output
 *
 * Typical use:<BR>
 * User populates bufferToHexDump with content they want to dump as hex.
 * Calling this method will return the size of the buffer needed to contain the
 * hex dump output.  User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToHexDump and the buffer for the hex dump
 * output can be passed to rsslBufferToHexDump().  
 * 
 * @param bufferToHexDump RsslBuffer populated with content the user wants to calculate hex dump size of.  buffer->length should represent the number of bytes contained in buffer->data. 
 * @param valuesPerLine Numeric value indicating how many hex values to represent per line in the formatted hex dump output.
 * @return RsslUInt32 Number of bytes needed in an RsslBuffer to successfully dump hex formatted contents into
 */
RsslUInt32 rsslCalculateHexDumpOutputSize(const RsslBuffer *bufferToHexDump, RsslUInt32 valuesPerLine)
{
	return 0;
}

/**
 * @brief Will convert buffer's contents to a formatted hex dump
 *
 * Typical use:<BR>
 * User populates bufferToHexDump with content they want to encrypt.
 * User calculates necessary size for hex dump output buffer by calling rsslCalculateHexDumpOutputSize()
 * User will then typically get or create a buffer of this 
 * size.  Once buffer is obtained, bufferToHexDump and the buffer for the hex dump formatted
 * output can be passed to rsslBufferToHexDump(), which will dump the formatted hex.  
 *
 * @param bufferToHexDump RsslBuffer populated with content the user wants to dump as formatted hex.  buffer->length should represent the number of bytes contained in buffer->data and buffer->data should hold unencrypted content.
 * @param hexDumpOutput RsslBuffer with appropriate space to dump formatted hex into.  buffer->length should represent the number of bytes available in buffer->data.
 * @param valuesPerLine Numeric value indicating how many hex values to represent per line in the formatted hex dump output.
 * @param error RsslError, to be populated in event of an error
 * @return RsslRet RsslRet return value indicating success or failure type.
 */
RsslRet rsslBufferToHexDump(const RsslBuffer* bufferToHexDump, RsslBuffer* hexDumpOutput, RsslUInt32 valuesPerLine, RsslError *error)
{
	return RSSL_RET_FAILURE;
}