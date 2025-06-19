/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ServiceList_h
#define __refinitiv_ema_access_ServiceList_h

/**
	@class refinitiv::ema::access::ServiceList ServiceList.h "Access/Include/ServiceList.h"
	@brief The ServiceList class is used to define a ordered list of service names that is keyed by a specific name. When added to an \ref OmmConsumerConfig, this 
		   list can be specified by using \ref ReqMsg.setServiceListName.
		   
	\remark All methods in this class are \ref SingleThreaded.

	@see 
*/

#include "Access/Include/Common.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/EmaVector.h"
#include "Access/Include/EmaConfig.h"

namespace refinitiv {

namespace ema {

namespace access {

class Data;


class ServiceList
{
public :


	///@name Constructor
	//@{
	/** Create a ServiceList that defines an ordered list of service names. 
		@param[in] name The name of the service list.
	*/
	EMA_ACCESS_API ServiceList(const EmaString& name);
	//@}
	
	///@name Constructor
	//@{
	/** Create a deep copied ServiceList.
		@param[in] other ServiceList to be copied from
	*/
	EMA_ACCESS_API ServiceList(const ServiceList& other);
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	EMA_ACCESS_API virtual ~ServiceList();
	//@}

	///@name Operations
	//@{
	
	/** Clears this service list
	*/
	EMA_ACCESS_API void clear();
	
	/** Adds the indicated service name string to the service list
	*  @param[in] serviceName name of the service
	*  @return reference to this object
	*/
	EMA_ACCESS_API ServiceList& addService(const EmaString& serviceName);
	
	/** Removes the indicated service name string from the service list
	*  @param[in] serviceName name of the service
	*  @return reference to this object
	*/
	EMA_ACCESS_API ServiceList& removeService(const EmaString&);
	
	/** Gets a reference to the EmaVector containing the concrete service names
	*  @return reference to the EmaVector
	*/
	EMA_ACCESS_API EmaVector<EmaString>& concreteServiceList();
	
	/** Gets the name of this ServiceList
	*  @return reference to the EmaString name
	*/
	const EMA_ACCESS_API EmaString& name() const;

	/** Sets the name of this ServiceList
	*	@param[in] name name of this service list
	*/
	EMA_ACCESS_API ServiceList& name(const EmaString& name);
	//@}

private :
	EmaVector<EmaString> _serviceList;
    EmaString _name;

};

}

}

}

#endif // __refinitiv_ema_access_ServiceList_h
