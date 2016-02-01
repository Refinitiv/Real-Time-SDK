/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_consumernodestoreclient_h
#define __thomsonreuters_eoa_foundation_consumernodestoreclient_h

#include "Foundation/Include/Common.h"

/**
	@class thomsonreuters::eoa::foundation::ConsumerNodeStoreClient Node.h "Foundation/Include/ConsumerNodeStoreClient.h"
	@brief ConsumerNodeStoreClient class is a callback client through which app gets notifications about changes to an item's payload.

	@see Node
*/

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class Node;

class EOA_FOUNDATION_API ConsumerNodeStoreClient
{
public :

	virtual void OnConsumerNodeStoreAppend( const Node& current ) {};

	virtual void OnConsumerNodeStoreReplace( const Node& current ) {};

	virtual void OnConsumerNodeStoreChange( const Node& current, const Node& change ) {};

	virtual void OnConsumerNodeStoreRemove( const Node& current ) {};
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_consumernodestoreclient_h
