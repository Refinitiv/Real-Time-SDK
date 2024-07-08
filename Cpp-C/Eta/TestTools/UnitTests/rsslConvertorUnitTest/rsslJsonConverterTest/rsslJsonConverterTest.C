/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 LSEG. All rights reserved.      –
*|-----------------------------------------------------------------------------
*/

/* Unit tests for converting between RSSL and JSON. */
#include "rsslJsonConverterTestBase.h"

using namespace std;

void printHelp()
{
	cout << "\nRSSL/JSON Converter Test Options:\n"
		"  --printJsonBuffer\n      Print JSON buffer after conversion of messages from RWF to JSON.\n"
		"  --printRsslBuffer\n      Print RWF buffer as XML before conversion from RWF to JSON and after conversion from JSON to RWF.\n"
		"  -?, --help\n      Print this help."
		"\n";
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);

	cmdlPrintJsonBuffer = false;
	cmdlPrintRsslBuffer = false;

	for(int i = 1; i < argc; ++i)
	{
		if (0 == strcmp(argv[i], "--printJsonBuffer"))
			cmdlPrintJsonBuffer = true;
		else if (0 == strcmp(argv[i], "--printRsslBuffer"))
			cmdlPrintRsslBuffer = true;
		else if (0 == strcmp(argv[i], "-?")
				|| 0 == strcmp(argv[i], "-h")
				|| 0 == strcmp(argv[i], "--help"))
		{
			printHelp();
			return 0;
		}
		else
		{
			cout << "Error: Unrecognized option '" << argv[i] << "'" << endl;
			printHelp();
			return 1;
		}
	}

	MsgConversionTestBase::initTestData();

	int ret = RUN_ALL_TESTS();

	MsgConversionTestBase::cleanupTestData();
	return ret;
}

