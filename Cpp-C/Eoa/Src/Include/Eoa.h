/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_eoa_h
#define __thomsonreuters_eoa_eoa_h

/**
	@file Eoa.h "Include/Eoa.h"
	@brief Eoa.h file is a collection of all the EOA public header files.
	
	For simplicity EOA applications may just include this one file instead of listing
	individual header files.
*/

#include "Foundation/Include/Common.h"

#include "Foundation/Include/OmmException.h"
#include "Foundation/Include/OmmInvalidUsageException.h"
#include "Foundation/Include/OmmMemoryExhaustionException.h"
#include "Foundation/Include/OmmOutOfRangeException.h"
#include "Foundation/Include/OmmUnsupportedDomainTypeException.h"

#include "Foundation/Include/Action.h"
#include "Foundation/Include/ComponentType.h"
#include "Foundation/Include/DataType.h"
#include "Foundation/Include/FilterType.h"
#include "Foundation/Include/TagType.h"

#include "Foundation/Include/EoaBuffer.h"
#include "Foundation/Include/EoaString.h"
#include "Foundation/Include/RmtesBuffer.h"

#include "Foundation/Include/OmmDate.h"
#include "Foundation/Include/OmmDateTime.h"
#include "Foundation/Include/OmmQos.h"
#include "Foundation/Include/OmmReal.h"
#include "Foundation/Include/OmmState.h"
#include "Foundation/Include/OmmTime.h"

#include "Foundation/Include/Component.h"
#include "Foundation/Include/Leaf.h"
#include "Foundation/Include/Node.h"
#include "Foundation/Include/Tag.h"

#include "Foundation/Include/ConsumerItem.h"
#include "Foundation/Include/ConsumerItemClient.h"
#include "Foundation/Include/ReqSpec.h"
#include "Foundation/Include/ConsumerService.h"
#include "Foundation/Include/RefreshInfo.h"
#include "Foundation/Include/StatusInfo.h"
#include "Foundation/Include/UpdateInfo.h"

#include "Foundation/Include/ConsumerNodeStoreClient.h"

#endif // __thomsonreuters_eoa_eoa_h
