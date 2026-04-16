/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_GlobalConfig_h
#define __refinitiv_ema_access_GlobalConfig_h

#include "Common.h"

/**
	@file GlobalConfig.h "Access/Include/GlobalConfig.h"
	@brief GlobalConfig.h file provides functions to modify global configuration options.
*/

namespace refinitiv
{

namespace ema
{

namespace access
{

class EMA_ACCESS_API GlobalConfig final
{
public:

	GlobalConfig() = delete;

	///@name Object Pools
	//@{
	/** Limit number of objects in the message object pools.

		@param[in] limit number of objects in the pools above which returned objects are deleted,
					 instead of being reused

		@see getMsgTypePoolLimit(), Msg
	 */
	static void setMsgTypePoolLimit(UInt32 limit);

	/** Limit number of objects in the complex data types object pools.

		@param[in] limit number of objects in the pools above which returned objects are deleted,
					 instead of being reused

		@see getComplexTypePoolLimit(), ComplexType
	 */
	static void setComplexTypePoolLimit(UInt32 limit);

	/** Limit number of objects in the data type object pools.

		@param[in] limit number of objects in the pools above which returned objects are deleted,
					 instead of being reused

		@see getDataTypePoolLimit(), Data
	 */
	static void setDataTypePoolLimit(UInt32 limit);

	/** Get the message object pools capacity.
		@return maximum number of objects in the message object pools stored for reuse
	 */
	static UInt32 getMsgTypePoolLimit();

	/** Get the complex types object pools capacity.
		@return maximum number of objects in the complex types object pools stored for reuse
	 */
	static UInt32 getComplexTypePoolLimit();

	/** Get the data types object pools capacity.
		@return maximum number of objects in the data types object pools stored for reuse
	 */
	static UInt32 getDataTypePoolLimit();
	//@}

	///@name Monitor Object Pools Utilization
	//@{
	/** Returns number of AckMsg objects in the object pool at the moment.

		@return number of items of type AckMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getAckMsgInPoolCount();

	/** Returns number of GenericMsg objects in the object pool at the moment.

		@return number of items of type GenericMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getGenericMsgInPoolCount();

	/** Returns number of PostMsg objects in the object pool at the moment.

		@return number of items of type PostMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getPostMsgInPoolCount();

	/** Returns number of ReqMsg objects in the object pool at the moment.

		@return number of items of type ReqMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getReqMsgInPoolCount();

	/** Returns number of RefreshMsg objects in the object pool at the moment.

		@return number of items of type RefreshMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getRefreshMsgInPoolCount();

	/** Returns number of StatusMsg objects in the object pool at the moment.

		@return number of items of type StatusMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getStatusMsgInPoolCount();

	/** Returns number of UpdateMsg objects in the object pool at the moment.

		@return number of items of type UpdateMsg in the object pool.

		@see getMsgTypePoolLimit(), setMsgTypePoolLimit()
	*/
	static UInt32 getUpdateMsgInPoolCount();
	//@}
};

} // namespace access

} // namespace ema

} // namespace refinitiv
#endif // __refinitiv_ema_access_GlobalConfig_h
