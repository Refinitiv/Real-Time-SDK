#|-----------------------------------------------------------------------------
#|            This source code is provided under the Apache 2.0 license
#|  and is provided AS IS with no warranty or guarantee of fit for purpose.
#|                See the project's LICENSE.md for details.
#|          Copyright (C) 2024 LSEG. All rights reserved.                    --
#|-----------------------------------------------------------------------------

header = '''/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2024 LSEG. All rights reserved.                    --
 *|-----------------------------------------------------------------------------
 */

#include "EmaConfigImpl.h"

using namespace refinitiv::ema::access;

static const char schemaData[] = {'''

print(header)

fbin = open('../../../EmaConfig.xsd', 'rb')

data = fbin.read(16)
while data:
    text = ''.join('0x{:02x}, '.format(i) for i in data)
    print(text)
    data = fbin.read(16)

fbin.close()

footer = '''};

const char* EmaConfigBaseImpl::getSchemaData()
{
	return schemaData;
}

size_t EmaConfigBaseImpl::getSchemaDataLen()
{
	return sizeof(schemaData);
}'''

print(footer)
