/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */



#include "Form1.h"

namespace NewsDisplay {

	using namespace NewsDisplay;

	void Form1::processStatusCallback(String^ statusText)
	{
		if ( _destroy == 1 )
			return;
		array<System::Object^>^ args = {this->txtStatus, statusText};
		updateStatusTextDelegate^ del = gcnew updateStatusTextDelegate(this, &Form1::updateStatusTextSafely);
		this->Invoke(del, args);
	}

	void Form1::processInternalStatusCallback(String^ statusText)
	{
		txtStatus->Text = statusText;
	}

	void Form1::processConnectionCallback(bool connStatus)
	{
		array<bool^>^ args = {gcnew Boolean(connStatus)};
		clearDataDelegate^ del = gcnew clearDataDelegate(this, &Form1::clearDataSafely);
		this->Invoke(del, args);

	}

	void Form1::clearDataSafely(bool^ connStatus)
	{
		if(*connStatus == true)
		{
			ConnectButton->Text = L"Disconnect";
			_Connected = true;
		}
		else
		{
			if(_Connected == true)
			{
				_pNewsWrapper->Disconnect();
			}

			ConnectButton->Text = L"Connect";
			_Connected = false;
		}
	}


	void Form1::updateStatusTextSafely(System::Windows::Forms::TextBox^ txtStatus,  String^ text)
	{
		txtStatus->Text = text; 
	}

	void Form1::processDataCallback(String^ pnac, String^ time,
					                String^ lang, wchar_t* bcast,
					                wchar_t* seg, String^ nextlink  )
	{
		if ( _destroy == 1 )
			return;


		array<String^>^ args = {pnac, time, lang, gcnew String(bcast), gcnew String(seg), nextlink};
		updateDataDelegate^ del = gcnew updateDataDelegate(this, &Form1::updateDataSafely);
		this->Invoke(del, args);
	}

	void Form1::updateDataSafely(String^ pnac, String^ time,
								 String^ lang, String^ bcast,
								 String^ seg, String^ nextlink)
	{
		if(_destroy == 1)
			return;

		_pNewsWrapper->updateDb(pnac, time, lang, bcast, seg, nextlink);
	}
}
