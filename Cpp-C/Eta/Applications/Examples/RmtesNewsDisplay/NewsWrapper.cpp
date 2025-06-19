/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NewsWrapper.h"
#include "Form1.h"


using namespace NewsDisplay;
using namespace News;


NewsWrapper::NewsWrapper(Form1^ frm):
_pForm1(frm)
{

	_pConsumer = nullptr;

	_pStatusCb = gcnew statusCallBack(_pForm1, &Form1::processStatusCallback);
	_pDataCb = gcnew dataCallBack(_pForm1, &Form1::processDataCallback);
	
	streamId = 10;
	_pListHeadline = _pForm1->getListHeadline();
	_pTextStory = _pForm1->getTextStory();
	_textStatus = _pForm1->getTextStatus();

	_pArrayHeadline = gcnew ArrayList(50);
	_pArrayStory = gcnew ArrayList(10);
}

NewsWrapper::~NewsWrapper()
{
	if(_pConsumer)
		delete _pConsumer;
}

void NewsWrapper::initCallBack()
{
	_pStatusCb = gcnew statusCallBack(_pForm1, &Form1::processStatusCallback);
	

	_pDataCb = gcnew dataCallBack(_pForm1, &Form1::processDataCallback);
	
}

void NewsWrapper::Connect(String^ host, String^ port, String^ service, String^ item)
   {
	_pConsumer = gcnew NewsConsumer();
	
	_pConsumer->setStatusCallBackFn(gcnew statusCallBack(_pForm1, &Form1::processStatusCallback));
	_pConsumer->setDataCallBackFn(gcnew dataCallBack(_pForm1, &Form1::processDataCallback));
	_pConsumer->setConnectionStatusCallbackFn(gcnew connectionStatusCallback(_pForm1, &Form1::processConnectionCallback));

	_pConsumer->Connect(host, port, service, item);

	return;
   }

void NewsWrapper::Disconnect()
   {
	if(_pConsumer)
      {
		delete _pConsumer;
		_pConsumer = nullptr;
      }


	// Connection has gone down, so clear headline cache and story cache, but do not clear the current displayed story(if there is one)

	_pListHeadline->BeginUpdate();

	if(_pListHeadline->Items->Count > 0)
		_pListHeadline->Items->Clear();

	if(_pArrayHeadline->Count > 0)
		_pArrayHeadline->Clear();

	if(_pArrayStory->Count > 0)
		_pArrayStory->Clear();

	_pListHeadline->EndUpdate();

	
   }



void NewsWrapper::subscribe(String^ ricNm)
{
	if ( _pConsumer )
	{
		_pTextStory->Text = L"";
		String^ str(L"Subscribing  ");
		str += ricNm;
		_textStatus->Text = str;
		if ( ricNm->Length == 0 )
{
			_pForm1->processInternalStatusCallback(L"Invalid input name for request.");
			return;
		}

		char* ricNames = (char*)((void*)Marshal::StringToHGlobalAnsi(ricNm));
			
		_pConsumer->sendRequest(ricNames, streamId, RSSL_FALSE); 
		streamId++;
		if(streamId == 100)
			streamId = 10;

		Marshal::FreeHGlobal((IntPtr)(void*)ricNames);
	}
}

void NewsWrapper::updateDb(String^ pnac, String^ time,
					   String^ lang, String^ bcast,
					String^ seg, String^ nextlink )
{
	if( !_pArrayHeadline->Count )
		_textStatus->Text = L"";
	if ( seg->Length == 0 && pnac->Length != 0 ) //headline
	{
		NewsHeadline^ aHL = gcnew NewsHeadline(time,pnac,lang,bcast);
    	_pArrayHeadline->Add(aHL);

		_pListHeadline->BeginUpdate();

		_pListHeadline->Items->Insert(0,aHL);
		_pListHeadline->DisplayMember = "displayTextStr";

		//no need to save so many news in list. 
		UInt32 totalNews = _pListHeadline->Items->Count;
		if ( totalNews > 200)
		{
			UInt32 totalRemoved = 50; 
			while (  totalRemoved-- > 0 )
			{
				NewsHeadline^ newheadline = (NewsHeadline^)(_pArrayHeadline[0]);
				_pArrayHeadline->Remove(newheadline);
				_pListHeadline->Items->Remove(newheadline);
			}
		}
		_pListHeadline->EndUpdate();

	}
	else if (seg->Length != 0 ) //story
	{

		UInt32 sCount = _pArrayStory->Count;
		UInt32 sFound = 0;
		UInt32 index = 0; 
		for (;index < sCount; ++index)
		{
			if ( ((NewsStory^)_pArrayStory[index])->_nextlink == pnac )
			{
				sFound = 1;
				break;
			}
		}

		NewsStory^ aStory;
		if ( sFound == 1 )
		{
			aStory = ((NewsStory^)_pArrayStory[index]);
			aStory->_text += seg;
			aStory->_nextlink = nextlink;
		}
		else 
		{
			aStory = gcnew NewsStory();
			aStory->_pnac = pnac;
			aStory->_text = seg;
			aStory->_nextlink = nextlink; 
			_pArrayStory->Add(aStory);
		}

		if ( aStory->_nextlink->Length != 0 )
			subscribe(aStory->_nextlink);
		else
		{
			_pTextStory->Text = aStory->_text;
			_textStatus->Text = L"Done";
		}

	}
	else 
		return;
}

void NewsWrapper::openStory(Object^ obj)
{
	NewsHeadline^ aNH = (NewsHeadline^)obj;
	_pTextStory->Text = L"";

	//first need to find pnac
	UInt32 found = 0;
	UInt32 headlineIndex = 0;
	UInt32 storyIndex = 0;
	UInt32 count = _pArrayHeadline->Count;
	for (; headlineIndex < count; ++headlineIndex)
	{
		if ( ((NewsHeadline^)_pArrayHeadline[headlineIndex])== aNH )
		{
			found = 1;
			break;
}
	}

	//check to see if there is one open already
	if ( found == 1)
	{
		UInt32 sCount = _pArrayStory->Count;
		UInt32 sFound = 0;
		for (;storyIndex < sCount; ++storyIndex)
		{
			if ( ((NewsStory^)_pArrayStory[storyIndex])->_pnac == ((NewsHeadline^)_pArrayHeadline[headlineIndex])->_pnac )
			{
				sFound = 1;
				break;
			}
		}
			
		if ( sFound == 0 )//not find, open request
			subscribe(((NewsHeadline^)_pArrayHeadline[headlineIndex])->_pnac);
		else //found, display story
			_pTextStory->Text = ((NewsStory^)_pArrayStory[storyIndex])->_text;
	}

}

