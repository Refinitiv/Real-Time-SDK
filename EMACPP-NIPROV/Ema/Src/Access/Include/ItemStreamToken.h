/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ItemStreamToken_h
#define __thomsonreuters_ema_access_ItemStreamToken_h

/**
	@class thomsonreuters::ema::access::ItemStreamToken ItemStreamToken.h "Access/Include/ItemStreamToken.h"
	@brief 


	@see 
*/

#include "Access/Include/EmaString.h"

namespace thomsonreuters {
	
namespace ema {

namespace access {

class EMA_ACCESS_API ItemStreamToken
{
public :

	///@name Constructor
	//@{
	/** Constructs ItemStreamToken.
	*/
	ItemStreamToken();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ItemStreamToken();
	//@}

	///@name Accessors
	//@{

	/** Returns the ServiceName within the MsgKey.
		@throw OmmInvalidUsageException if hasServiceName() returns false
		@return EmaString containing service name
	*/
	const EmaString& getServiceName() const;

	/** Returns the StreamId value as a Int.
		@return string representation of this object's NackCode
	*/
	Int64 getStreamId() const;

	/** Returns PrivateStream.
		@return true if this is a private stream; false otherwise
	*/
	bool getPrivateStream() const;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EmaString& toString() const;

	/** Indicates presence of the ServiceName within the MsgKey.
		@return true if service name is set; false otherwise
	*/
	bool hasServiceName() const;
		
	//@}

private :

	const EmaString& toString( UInt64 indent ) const;

	ItemStreamToken( const ItemStreamToken& );
	ItemStreamToken& operator=( const ItemStreamToken& );

	mutable EmaString		_toString;

	Int64			_streamId;
};

}

}

}

#endif // __thomsonreuters_ema_access_ItemStreamToken_h
