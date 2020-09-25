/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ComplexType_h
#define __refinitiv_ema_access_ComplexType_h

/**
	@class rtsdk::ema::access::ComplexType ComplexType.h "Access/Include/ComplexType.h"
	@brief ComplexType class is a parent class from whom all complex data types inherit.

	ComplexType class represents all OMM Data constructs set-able as message payload.

	\remark Objects of this class are intended to be short lived or rather transitional.
	\remark Objects of this class are not cache-able.
	\remark All methods in this class are \ref SingleThreaded.

	@see Data
*/

#include "Access/Include/Data.h"

namespace rtsdk {

namespace ema {

namespace access {

class EMA_ACCESS_API ComplexType : public Data
{
public :

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~ComplexType();
	//@}

protected :

	ComplexType();

private :

	ComplexType( const ComplexType& );
	ComplexType& operator=( const ComplexType& );
};

}

}

}

#endif // __refinitiv_ema_access_ComplexType_h
