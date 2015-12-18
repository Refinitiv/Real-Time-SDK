/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_eoa_foundation_tagtype_h
#define __thomsonreuters_eoa_foundation_tagtype_h

#include "Foundation/Include/Common.h"

namespace thomsonreuters {
	
namespace eoa {

namespace foundation {

class EoaString;

/**
	\class thomsonreuters::eoa::foundation::TagType TagType.h "Foundation/Include/TagType.h"
	\brief TagType class provides enumeration representing tag types.

	TagType combines OMM container data type and its key data type.

	\remark All methods in this class are \ref SingleThreaded.

	@see TagTypeName,
		Node,
		Leaf
*/
enum class EOA_FOUNDATION_API TagType
{
	ArrayEnum,			/*!< Represents tag type for Array container. */
	ElementListEnum,	/*!< Represents tag type for ElementList container. */
	FieldListEnum,		/*!< Represents tag type for FieldList container. */
	FilterListEnum,		/*!< Represents tag type for FilterList container. */
	MapAsciiEnum,		/*!< Represents tag type for Map container with Ascii key data type. */
	MapBufferEnum,		/*!< Represents tag type for Map container with Buffer key data type. */
	MapDateEnum,		/*!< Represents tag type for Map container with Date key data type. */
	MapDateTimeEnum,	/*!< Represents tag type for Map container with DateTime key data type. */
	MapDoubleEnum,		/*!< Represents tag type for Map container with double key data type. */
	MapEnumEnum,		/*!< Represents tag type for Map container with Enum key data type. */
	MapFloatEnum,		/*!< Represents tag type for Map container with Float key data type. */
	MapIntEnum,			/*!< Represents tag type for Map container with Int key data type. */
	MapRealEnum,		/*!< Represents tag type for Map container with Real key data type. */
	MapRmtesEnum,		/*!< Represents tag type for Map container with Rmtes key data type. */
	MapQosEnum,			/*!< Represents tag type for Map container with Qos key data type. */
	MapStateEnum,		/*!< Represents tag type for Map container with State key data type. */
	MapTimeEnum,		/*!< Represents tag type for Map container with Time key data type. */
	MapUtf8Enum,		/*!< Represents tag type for Map container with Utf8 key data type. */
	MapUIntEnum,		/*!< Represents tag type for Map container with UInt key data type. */
	SeriesEnum,			/*!< Represents tag type for Series container. */
	VectorEnum			/*!< Represents tag type for Vector container. */
};

/**
	\class thomsonreuters::eoa::foundation::TagTypeName TagType.h "Foundation/Include/TagType.h"
	\brief TagTypeName class converts TagType enumeration into a string with its name.

	\remark All methods in this class are \ref SingleThreaded.

	@see TagType
*/
class EOA_FOUNDATION_API TagTypeName
{
public :

	///@name Constructor
	//@{
	/** Constructs ComponentTypeName. This is used for string representation only.
		@param[in] tagType specifies tag type to be converted to string
	*/
	TagTypeName( TagType tagType ) throw();
	//@}

	///@name Destructor
	//@{
	/** Destructor.
	*/
	virtual ~TagTypeName() throw();
	//@}

	///@name Accessors
	//@{
	/** Returns the enum value as a string format.
		@return EoaString containing name of the data type
	*/
	const EoaString& toString() const throw();

	/** Operator const char* overload.
	*/
	operator const char* () const throw();
	//@}

private :

	TagType _value;

	TagTypeName();
	TagTypeName( const TagTypeName& );
	TagTypeName& operator=( const TagTypeName& );
};

}

}

}

#endif // __thomsonreuters_eoa_foundation_tagtype_h
