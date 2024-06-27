/*
 * This source code is provided under the Apache 2.0 license and is provided
 * AS IS with no warranty or guarantee of fit for purpose.  See the project's 
 * LICENSE.md for details. 
 * Copyright (C) 2019 LSEG. All rights reserved.
*/



#pragma once

#include "NewsWrapper.h"

namespace NewsDisplay {

	using namespace News;
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ConnectForm
	/// </summary>
	public ref class ConnectForm : public System::Windows::Forms::Form
	{
	public:

		ConnectForm(NewsWrapper^ newsWrapper)
		{
			InitializeComponent();
			_pNewsWrapper = newsWrapper;
		}



	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ConnectForm()
		{
			if (components)
			{
				delete components;
			}
		}


	protected:
	NewsWrapper^ _pNewsWrapper;

	private: System::Windows::Forms::Label^  HostLabel;
	private: System::Windows::Forms::Label^  PortLabel;
	private: System::Windows::Forms::Button^  ConnectButton;
	private: System::Windows::Forms::Label^  label1;

	private: System::Windows::Forms::TextBox^  HostBox;
	private: System::Windows::Forms::TextBox^  PortBox;
	private: System::Windows::Forms::TextBox^  ServiceBox;
	private: System::Windows::Forms::TextBox^  ItemBox;

	private: System::Windows::Forms::Label^  ItemLabel;
	private: System::Windows::Forms::Button^  CancelButton;



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
			this->HostLabel = (gcnew System::Windows::Forms::Label());
			this->PortLabel = (gcnew System::Windows::Forms::Label());
			this->ConnectButton = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->HostBox = (gcnew System::Windows::Forms::TextBox());
			this->PortBox = (gcnew System::Windows::Forms::TextBox());
			this->ServiceBox = (gcnew System::Windows::Forms::TextBox());
			this->ItemBox = (gcnew System::Windows::Forms::TextBox());
			this->ItemLabel = (gcnew System::Windows::Forms::Label());
			this->CancelButton = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// HostLabel
			// 
			this->HostLabel->AutoSize = true;
			this->HostLabel->Location = System::Drawing::Point(12, 16);
			this->HostLabel->Name = L"HostLabel";
			this->HostLabel->Size = System::Drawing::Size(60, 13);
			this->HostLabel->TabIndex = 3;
			this->HostLabel->Text = L"Host Name";
			// 
			// PortLabel
			// 
			this->PortLabel->AutoSize = true;
			this->PortLabel->Location = System::Drawing::Point(224, 16);
			this->PortLabel->Name = L"PortLabel";
			this->PortLabel->Size = System::Drawing::Size(66, 13);
			this->PortLabel->TabIndex = 4;
			this->PortLabel->Text = L"Port Number";
			// 
			// ConnectButton
			// 
			this->ConnectButton->Location = System::Drawing::Point(12, 133);
			this->ConnectButton->Name = L"ConnectButton";
			this->ConnectButton->Size = System::Drawing::Size(75, 23);
			this->ConnectButton->TabIndex = 5;
			this->ConnectButton->Text = L"Connect";
			this->ConnectButton->UseVisualStyleBackColor = true;
			this->ConnectButton->Click += gcnew System::EventHandler(this, &ConnectForm::ConnectButton_Click);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(9, 71);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(74, 13);
			this->label1->TabIndex = 7;
			this->label1->Text = L"Service Name";
			// 
			// HostBox
			// 
			this->HostBox->Location = System::Drawing::Point(15, 32);
			this->HostBox->Name = L"HostBox";
			this->HostBox->Size = System::Drawing::Size(181, 20);
			this->HostBox->TabIndex = 8;
			// 
			// PortBox
			// 
			this->PortBox->Location = System::Drawing::Point(227, 32);
			this->PortBox->Name = L"PortBox";
			this->PortBox->Size = System::Drawing::Size(181, 20);
			this->PortBox->TabIndex = 9;
			// 
			// ServiceBox
			// 
			this->ServiceBox->Location = System::Drawing::Point(12, 87);
			this->ServiceBox->Name = L"ServiceBox";
			this->ServiceBox->Size = System::Drawing::Size(181, 20);
			this->ServiceBox->TabIndex = 10;
			// 
			// ItemBox
			// 
			this->ItemBox->Location = System::Drawing::Point(227, 87);
			this->ItemBox->Name = L"ItemBox";
			this->ItemBox->Size = System::Drawing::Size(181, 20);
			this->ItemBox->TabIndex = 12;
			this->ItemBox->Text = L"N2_UBMS";
			// 
			// ItemLabel
			// 
			this->ItemLabel->AutoSize = true;
			this->ItemLabel->Location = System::Drawing::Point(224, 71);
			this->ItemLabel->Name = L"ItemLabel";
			this->ItemLabel->Size = System::Drawing::Size(58, 13);
			this->ItemLabel->TabIndex = 11;
			this->ItemLabel->Text = L"Item Name";
			// 
			// CancelButton
			// 
			this->CancelButton->Location = System::Drawing::Point(118, 133);
			this->CancelButton->Name = L"CancelButton";
			this->CancelButton->Size = System::Drawing::Size(75, 23);
			this->CancelButton->TabIndex = 13;
			this->CancelButton->Text = L"Cancel";
			this->CancelButton->UseVisualStyleBackColor = true;
			this->CancelButton->Click += gcnew System::EventHandler(this, &ConnectForm::CancelButton_Click);
			// 
			// ConnectForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(437, 187);
			this->Controls->Add(this->CancelButton);
			this->Controls->Add(this->ItemBox);
			this->Controls->Add(this->ItemLabel);
			this->Controls->Add(this->ServiceBox);
			this->Controls->Add(this->PortBox);
			this->Controls->Add(this->HostBox);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->ConnectButton);
			this->Controls->Add(this->PortLabel);
			this->Controls->Add(this->HostLabel);
			this->Name = L"ConnectForm";
			this->Text = L"Connect";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


private: System::Void ConnectButton_Click(System::Object^  sender, System::EventArgs^  e) {
			 _pNewsWrapper->Connect(HostBox->Text, PortBox->Text, ServiceBox->Text, ItemBox->Text);
			 this->Hide();
		 }
private: System::Void CancelButton_Click(System::Object^  sender, System::EventArgs^  e) {
			 this->Hide();
		 }
};
}
