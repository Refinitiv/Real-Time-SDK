/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_omminaccessiblelogfileexception_h
#define __thomsonreuters_eoa_foundation_omminaccessiblelogfileexception_h

/**
	@class thomsonreuters::eoa::foundation::OmmInaccessibleLogFileException OmmInaccessibleLogFileException.h "Foundation/Include/OmmInaccessibleLogFileException.h"
	@brief OmmInaccessibleLogFileException is thrown when EOA log file can not be created.

	OmmInaccessibleLogFileException is thrown when user running EOA application has no
	proper file writing permissions in a configured folder / directory.

	\remark All methods in this class are \ref SingleThreaded.

	@see OmmException
*/

#include "Foundation/Include/OmmException.h"
#include "Foundation/Include/EoaString.h"

namespace thomsonreuters {

namespace eoa {

namespace foundation {

class EOA_FOUNDATION_API OmmInaccessibleLogFileException : public OmmException
{
public:

	///@name Accessors
	//@{
	/** Returns ExceptionType.
		@return OmmInaccessibleLogFileExceptionEnum
	*/
	OmmException::ExceptionType getExceptionType() const override;

	/** Returns Text.
		@return string with exception text information
	*/
	const EoaString& getText() const override;

	/** Returns a string representation of the class instance.
		@return string representation of the class instance
	*/
	const EoaString& toString() const override;

	/** Returns the inaccessible file name.
		@return directory and file name that caused this exception
	*/
	const EoaString& getFilename() const;
	//@}

	///@name Destructor
	//@{
	/** Destructor
	*/
	virtual ~OmmInaccessibleLogFileException();
	//@}

protected:

	OmmInaccessibleLogFileException( const EoaString& );

	OmmInaccessibleLogFileException( const OmmInaccessibleLogFileException& );
	OmmInaccessibleLogFileException& operator=( const OmmInaccessibleLogFileException& );

	EoaString				_fileName;
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_omminaccessiblelogfileexception_h
