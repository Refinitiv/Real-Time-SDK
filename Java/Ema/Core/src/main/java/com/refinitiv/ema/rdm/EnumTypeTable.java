///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.rdm;

import java.util.List;

/**
* A table of enumerated types.  A field that uses this table will contain a value
* corresponding to an enumerated type in this table.
*/

public interface EnumTypeTable
{	
	/**
	* Returns the list of EnumType that is belonged to this EnumTypeTable.
	* @return the list of EnumType
	*/
	public List<EnumType> enumTypes();

	/**
	* Returns the list of Field ID that references to this EnumTypeTable.
	* @return the list of FID
	*/
	public List<Integer> FidReferences();
	
	/**
     * Convert information contained in the table of enumerated types to a string.
     * 
     * @return the string representation of this {@link EnumTypeTable}
     */
     public String toString();
}
