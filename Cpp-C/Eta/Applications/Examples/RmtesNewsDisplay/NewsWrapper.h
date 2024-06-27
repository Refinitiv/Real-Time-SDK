/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#pragma once

#include "NewsConsumer.h"

namespace NewsDisplay
{
	using namespace System;
	using namespace System::Runtime::InteropServices;
	using namespace System::Collections;
	using namespace System::Windows::Forms;

	using namespace News;
	using namespace NewsDisplay;
	

ref class Form1;

public ref class NewsHeadline : public System::Object
{
public:

	NewsHeadline( String^ time, String^ pnac,  String^ lang, String^ bcast )
   {
      this->_time = time;
      this->_pnac = pnac;
	  this->_lang = lang;
      this->_bcast = bcast;
	  this->_displayText = this->_lang + L" |" + this->_time + L" |" + this->_bcast;	
   }

   property String^ displayTextStr 
   {
      String^ get()
      {
         return _displayText;
      }
   }


	String^ _time;
	String^ _pnac;
	String^ _lang;
	String^ _bcast;
	String^ _displayText;
};

public ref class NewsStory
{
public:
	String^ _pnac;
	String^ _text;
	String^ _nextlink;
};

public ref class NewsWrapper
{
public:
	NewsWrapper(Form1^ frm);
	~NewsWrapper();

	void updateDb(String^ pnac, String^ time,
					   String^ lang, String^ bcast,
					   String^ seg, String^ nextlink );

	void openStory(Object^ obj);

	void Connect(String^ host, String^ port, String^ service, String^ item);

	void Disconnect();

private:

	void subscribe(String^ ricNm);
	void initCallBack();

private:
	Form1^ _pForm1;
	NewsConsumer^ _pConsumer;

	ListBox^ _pListHeadline;
	RichTextBox^ _pTextStory;
	TextBox^ _textStatus;
	RsslInt32 streamId;

	ArrayList^ _pArrayHeadline;
	ArrayList^ _pArrayStory;  

	statusCallBack^ _pStatusCb;
	dataCallBack^ _pDataCb;
};

}
