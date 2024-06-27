/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/

#pragma once

#include "NewsWrapper.h"
#include "ConnectForm.h"

namespace NewsDisplay
{
using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;



/// <summary>
/// Summary for Form1
///
/// WARNING: If you change the name of this class, you will need to change the
///          'Resource File Name' property for the managed resource compiler tool
///          associated with all .resx files this class depends on.  Otherwise,
///          the designers will not be able to interact properly with localized
///          resources associated with this form.
/// </summary>
public ref class Form1 : public System::Windows::Forms::Form
{

public:
	Form1(void)
	{
		InitializeComponent();

		 _pNewsWrapper = gcnew NewsWrapper(this);

		_openHeadline = 1;
		_destroy = 0;
		_Connected = false;

	}
		
	void processStatusCallback(String^ statusText);
	void processInternalStatusCallback(String^ statusText);
	void processDataCallback(String^ pnac, String^ time,
					            String^ lang, wchar_t* bcast,
					            wchar_t* seg, String^ nextlink );
	void processConnectionCallback(bool connStatus); 


	delegate void updateStatusTextDelegate(TextBox^, String^);
	void updateStatusTextSafely(TextBox^ txtBox,  String^ text);

	
	delegate void updateDataDelegate(String^ pnac, String^ time,
					                    String^ lang, String^ bcast,
					                    String^ seg, String^ nextlink );

	void updateDataSafely(String^ pnac, String^ time,
					        String^ lang, String^ bcast,
					        String^ seg, String^ nextlink );

	delegate void clearDataDelegate(bool^ connStatus);
	void clearDataSafely(bool^ connStatus);

	ListBox^ getListHeadline() { return listNewsHeadline; }
	RichTextBox^ getTextStory() { return richTextStory; }
	TextBox^ getTextStatus() { return txtStatus; }

protected:
	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	~Form1()
	{
		_destroy = 1;
		if (_pNewsWrapper)
			delete _pNewsWrapper;


		if (components)
		{
			delete components;
		}
	}
	
protected: 

	NewsWrapper^ _pNewsWrapper;
	ConnectForm^ _pConnect;
	UInt32 _openHeadline;
	UInt32 _destroy;
	bool _Connected;

	
private: System::Windows::Forms::TextBox^  txtStatus;
private: System::Windows::Forms::Label^  label3;
private: System::Windows::Forms::ListBox^  listNewsHeadline;
private: System::Windows::Forms::RichTextBox^  richTextStory;
private: System::Windows::Forms::Button^  ConnectButton;



private:
	/// <summary>
	/// Required designer variable.
	/// </summary>
	System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		this->txtStatus = (gcnew System::Windows::Forms::TextBox());
		this->label3 = (gcnew System::Windows::Forms::Label());
		this->listNewsHeadline = (gcnew System::Windows::Forms::ListBox());
		this->richTextStory = (gcnew System::Windows::Forms::RichTextBox());
		this->ConnectButton = (gcnew System::Windows::Forms::Button());
		this->SuspendLayout();
		// 
		// txtStatus
		// 
		this->txtStatus->Location = System::Drawing::Point(37, 436);
		this->txtStatus->Name = L"txtStatus";
		this->txtStatus->Size = System::Drawing::Size(560, 20);
		this->txtStatus->TabIndex = 2;
		// 
		// label3
		// 
		this->label3->AutoSize = true;
		this->label3->Location = System::Drawing::Point(34, 420);
		this->label3->Name = L"label3";
		this->label3->Size = System::Drawing::Size(37, 13);
		this->label3->TabIndex = 7;
		this->label3->Text = L"Status";
		// 
		// listNewsHeadline
		// 
		this->listNewsHeadline->Font = (gcnew System::Drawing::Font(L"Arial Unicode MS", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->listNewsHeadline->FormattingEnabled = true;
		this->listNewsHeadline->HorizontalScrollbar = true;
		this->listNewsHeadline->ItemHeight = 18;
		this->listNewsHeadline->Location = System::Drawing::Point(38, 12);
		this->listNewsHeadline->Name = L"listNewsHeadline";
		this->listNewsHeadline->ScrollAlwaysVisible = true;
		this->listNewsHeadline->Size = System::Drawing::Size(559, 130);
		this->listNewsHeadline->TabIndex = 8;
		this->listNewsHeadline->DoubleClick += gcnew System::EventHandler(this, &Form1::listNewsHeadline_DoubleClick);
		// 
		// richTextStory
		// 
		this->richTextStory->BackColor = System::Drawing::SystemColors::Window;
		this->richTextStory->Font = (gcnew System::Drawing::Font(L"Arial Unicode MS", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
			static_cast<System::Byte>(0)));
		this->richTextStory->Location = System::Drawing::Point(37, 161);
		this->richTextStory->Name = L"richTextStory";
		this->richTextStory->ReadOnly = true;
		this->richTextStory->Size = System::Drawing::Size(559, 246);
		this->richTextStory->TabIndex = 10;
		this->richTextStory->Text = L"";
		// 
		// ConnectButton
		// 
		this->ConnectButton->Location = System::Drawing::Point(37, 491);
		this->ConnectButton->Name = L"ConnectButton";
		this->ConnectButton->Size = System::Drawing::Size(75, 23);
		this->ConnectButton->TabIndex = 11;
		this->ConnectButton->Text = L"Connect";
		this->ConnectButton->UseVisualStyleBackColor = true;
		this->ConnectButton->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
		// 
		// Form1
		// 
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->ClientSize = System::Drawing::Size(638, 541);
		this->Controls->Add(this->ConnectButton);
		this->Controls->Add(this->richTextStory);
		this->Controls->Add(this->listNewsHeadline);
		this->Controls->Add(this->label3);
		this->Controls->Add(this->txtStatus);
		this->Name = L"Form1";
		this->Text = L"NewsDisplay";
		this->ResumeLayout(false);
		this->PerformLayout();

	}
#pragma endregion
	
private: System::Void listNewsHeadline_DoubleClick( Object^, System::EventArgs^ )
{
	if ( listNewsHeadline->SelectedIndex != -1 )
	{
		Object^ obj = listNewsHeadline->SelectedItem;
		_pNewsWrapper->openStory(obj);
	}
}

private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 if(_Connected == false)
			 {
				 _pConnect = gcnew ConnectForm(_pNewsWrapper);
			 _pConnect->Show();
			 }
			 else
			 {
				_pNewsWrapper->Disconnect();
				_Connected = false;
				ConnectButton->Text = L"Connect";
			 }
		 }
};


}