///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2023 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#ifndef __refinitiv_ema_access_PackedMsg_h
#define __refinitiv_ema_access_PackedMsg_h

#include "Access/Include/Common.h"
#include "Access/Include/Msg.h"
#include "OmmProvider.h"

namespace refinitiv {

namespace ema {

namespace access {

/**
	@class refinitiv::ema::access::PackedMsg PackedMsg.h "Access/Include/PackedMsg.h"
	@brief PackedMsg class provides API to pack messages.

	\remark Thread safety of all the methods in this class depends on user's implementation.

	\code

	// create packed message
	OmmProvider provider( OmmIProviderConfig().port( "14002" ), appClient );
	PackedMsg packedMsg(provider);

	// initialize packed message
	packedMsg.initBuffer(clientHandle)

	// add user messages to the packed message
	for (Int32 i = 0; i < 10; i++)
	{
		msg.clear();
		flist.clear();
		flist.addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum);
		flist.addReal(30, 10 + i, OmmReal::Exponent0Enum);
		flist.complete();

		msg.serviceName("DIRECT_FEED").name("IBM.N");
		msg.payload(flist);

		packedMsg.addMsg(msg, itemHandle);
	}

	// submit and clear packed message
	if (packedMsg.packedMsgCount() > 0)
	{
		provider.submit(packedMsg);
		packedMsg.clear();
	}

	\endcode
*/

class PackedMsgImpl;

/**
 * PackedMsg contains a list of messages packed to be sent across the wire together.
 */

class EMA_ACCESS_API PackedMsg
{
public:
	///@name Constructor
	//@{
	/** Create an PackedMsg with OmmProvider. Set up default values for packed data variables.
		@param[in] OmmProvider instance.
		@throw OmmMemoryExhaustionException if the application runs out of memory.
	 */
	PackedMsg(OmmProvider& ommProvider);
	//@}
	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~PackedMsg();
	//@}

	////@name Operations
	//@{
	/**
		For Non-Interactive Provider applications, initialize a new write buffer.
		The size of the new packed messages write buffer will be set to its default size.
		@throw OmmInvalidUsageException if the user tries to call this method with Interactive Provider.
		@throw OmmInvalidUsageException if connection is not established.
		@throw OmmInvalidUsageException if not possible to allocate buffer for packed message.
		@return this PackedMsg.
	*/
	PackedMsg& initBuffer();
	/**
		For Non-Interactive Provider applications, initialize a new write buffer.
		Also sets the maximum size of the new packed messages write buffer.
		@param[in] UInt32 maxSize maximum size of the packed message buffer.
		@throw OmmInvalidUsageException if the user tries to call this method with Interactive Provider.
		@throw OmmInvalidUsageException if connection is not established.
		@throw OmmInvalidUsageException if not possible to allocate buffer for packed messages.
		@return this PackedMsg.
	*/
	PackedMsg& initBuffer(UInt32 maxSize);
	/**
		For Interactive Provider applications, initialize a new write buffer and sets the client handle for this PackedMsg to submit messages to.
		The size of the new packed messages write buffer will be set to its default size.
		The handle is required to be set before adding messages or submitting the packedMsg.
		The size of the packed message buffer will be set to its default size.
		@param[in] UInt64 clientHandle unique client identifier associated by EMA with a connected client.
		@throw OmmInvalidUsageException if the user tries to call this method with Non-Interactive Provider.
		@throw OmmInvalidUsageException if client handle is not valid.
		@throw OmmInvalidUsageException if not possible to allocate buffer for packed message.
		@return this PackedMsg.
	*/
	PackedMsg& initBuffer(UInt64 clientHandle);
	/**
		For Interactive Provider applications, initialize a new write buffer and sets the client handle for this PackedMsg to submit messages to.
		Also sets the maximum size of the new packed messages write buffer.
		The handle is required to be set before adding messages or submitting the packedMsg.
		Also sets the maximum size of the packed message buffer.
		@param[in] UInt64 clientHandle unique client identifier associated by EMA with a connected client.
		@param[in] UInt32 maxSize maximum size of the packed message buffer.
		@throw OmmInvalidUsageException if the user tries to call this method with Non-Interactive Provider.
		@throw OmmInvalidUsageException if client handle is not valid.
		@throw OmmInvalidUsageException if not possible to allocate buffer for packed message.
		@return this PackedMsg.
	 */
	PackedMsg& initBuffer(UInt64 clientHandle, UInt32 maxSize);
	/**
		Adds a Msg to the packed message buffer if there is enough space in the buffer to add the Msg.
		@param[in] Msg msg message to add to this packed message.
		@param[in] long itemHandle which is related to the packed message.
		@throw OmmInvalidUsageException if the connection is not established.
		@throw OmmInvalidUsageException if item handle is not set.
		@throw OmmInvalidUsageException if incoming message is empty.
		@throw OmmInvalidUsageException if not possible to allocate memory for StreamInfo Non-Interactive Provider.
		@throw OmmInvalidUsageException if fail to encode incoming message.
		@throw OmmInvalidUsageException if not possible to add messages to packed buffer.
		@return this PackedMsg.
	 */
	PackedMsg& addMsg(const Msg& msg, UInt64 itemHandle);
	/**
		Returns int value of remaining size in the buffer available for message packing.
		@return remaining size of packed buffer.
	*/
	UInt64 remainingSize() const;
	/**
		Returns int value of amount of currently packed messages in this PackedMsg object.
		@return number of packed messages.
	*/
	UInt64 packedMsgCount() const;
	/**
		Return int value of maximum size of the packed message buffer.
		@return max packed buffer size.
	*/
	UInt64 maxSize() const;
	/**
		Clears the entries in the PackedMessage.
		@return this PackedMsg
	*/
	PackedMsg& clear();
	//@}
private:
	friend class OmmNiProviderImpl;
	friend class OmmIProviderImpl;

	PackedMsgImpl* _pImpl;
};
}
}
}


#endif // __refinitiv_ema_access_PackedMsg_h