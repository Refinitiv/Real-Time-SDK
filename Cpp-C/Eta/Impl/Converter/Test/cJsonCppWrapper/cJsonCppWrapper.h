/*|-----------------------------------------------------------------------------
*| This source code is provided under the Apache 2.0 license –
*| and is provided AS IS with no warranty or guarantee of fit for purpose. –
*| See the project's LICENSE.md for details. –
*| Copyright (C) 2020 Refinitiv. All rights reserved. –
*|-----------------------------------------------------------------------------
*/

#ifndef _json_hpp__
#define _json_hpp__

#include <cjson/cJSON.h>
#include <string>
#include <iostream>

namespace json {

  class Document;

  class Value
  {
  private:
    cJSON *jsonValue;

  public:

    Value():jsonValue(NULL){}
    Value(cJSON *data) :jsonValue(data) {}
    Value(const Value& data) :jsonValue(data.jsonValue) {}

    void Set(cJSON  *data) { jsonValue = data; }
    bool IsNull() const { return (jsonValue->type == cJSON_NULL); }
    bool IsBool() const { return ((jsonValue->type == cJSON_True) || (jsonValue->type == cJSON_False)); }
    bool IsNumber() const { return (jsonValue->type == cJSON_Number); }
    bool IsInt() const { return (jsonValue->type == cJSON_Number); }
    bool IsString() const { return (jsonValue->type == cJSON_String); }
    bool IsArray() const { return (jsonValue->type == cJSON_Array); }
    bool IsObject() const{ return (jsonValue->type == cJSON_Object); }
    bool IsRaw() const { return (jsonValue->type == cJSON_Raw);}
    bool HasMember(const std::string& key) const { return (cJSON_HasObjectItem(jsonValue, key.c_str())); }

    const char* GetString() const
    {
      if (!IsString())
      {
        throw (std::bad_cast());
      }
      return (jsonValue->valuestring);
    }

    const size_t GetStringLength() const
    {
      if (!IsString())
      {
        throw (std::bad_cast());
      }
      return (strlen(jsonValue->valuestring) + 1);
    }

    const bool GetBool() const
    {
      if (!IsBool())
      {
        throw (std::bad_cast());
      }
      return (jsonValue->valueint == 0 ? false : true);
    }

    const int GetInt() const
    {
      if (!IsNumber())
      {
        throw (std::bad_cast());
      }
      return (jsonValue->valueint);
    }

    const double  GetDouble() const
    {
      if (!IsNumber())
      {
        throw (std::bad_cast());
      }
      return (jsonValue->valuedouble);
    }

    const float  GetFloat() const
    {
      if (!IsNumber())
      {
        throw (std::bad_cast());
      }
      return (static_cast<float>(jsonValue->valuedouble));
    }

    const int Size() const
    {
      if (!IsArray())
      {
        throw (std::bad_cast());
      }
      return (cJSON_GetArraySize(jsonValue));
    }

    Value& operator=(const Value & data)
    {
      jsonValue = data.jsonValue;
      return (*this);
    }

    Value operator[] (const int& key) const
    {
      cJSON * item = cJSON_GetArrayItem(jsonValue, key);
      if (NULL == item) {
        throw (std::exception());
      }
      return (Value(item));
    }

    Value operator[] (const std::string& key) const
    {
      cJSON *item = cJSON_GetObjectItem(jsonValue, key.c_str());
      if (NULL == item) {
        throw (std::exception());
      }
      return (Value(item));
    }

    Value operator[] (const char key) const
    {
      cJSON *item = cJSON_GetObjectItem(jsonValue, &key);
      if (NULL == item) {
        throw (std::exception());
      }
      return (Value(item));
    }

    friend class Document;
  };

  class Document
  {
  private:

    cJSON *jsonDocuments;
    Value value;

  public:

    Document():jsonDocuments(NULL){}

    virtual ~Document()
    {
      if (NULL != jsonDocuments)cJSON_Delete(jsonDocuments);
    }

    void Parse(const std::string& text)
    {
      cJSON *const item = cJSON_Parse(text.c_str());
      if( NULL == item)
      {
        // cJSON_GetErrorPtr()
        throw (std::exception());
      }
        jsonDocuments = item;
        value.Set(item);
    }

    const char* GetParseError() { return (cJSON_GetErrorPtr()); }

    void SetObject()
    {
      cJSON *item = cJSON_CreateObject();
      if (NULL == item)
      {
        throw (std::bad_alloc());
      }
      jsonDocuments = item;
      value.Set(item);
    };

    bool IsNull() const { return value.IsNull(); }
    bool IsBool() const { return value.IsBool(); }
    bool IsNumber() const { return value.IsNumber(); }
    bool IsInt() const { return value.IsInt(); }
    bool IsString() const { return value.IsString(); }
    bool IsArray() const { return value.IsArray(); }
    bool IsObject() const { return value.IsObject(); }
    bool IsRaw() const { return value.IsRaw(); }
    bool HasMember(const std::string& key) const { return value.HasMember(key); }
    const char* GetString() const { return value.GetString(); }
    const size_t GetStringLength() const { return value.GetStringLength(); }
    const bool GetBool() const { return value.GetBool(); }
    const int GetInt() const { return value.GetInt(); }
    const double GetDouble() const { return value.GetDouble(); }
    const float GetFloat() const { return value.GetFloat(); }
    const int Size() const { return value.Size(); }

    operator Value() const { return Value(jsonDocuments); }

    Value operator[] (const int& key) const
    {
      cJSON * item = cJSON_GetArrayItem(value.jsonValue, key);
      if (NULL == item) {
        throw (std::exception());
      }
      return (Value(item));
    }

    Value operator[] (const std::string& key) const
    {
      cJSON *item = cJSON_GetObjectItem(value.jsonValue, key.c_str());
      if (NULL == item) {
        throw (std::exception());
      }
      return (Value(item));
    }

    Value operator[] (const char key) const
    {
      cJSON *item = cJSON_GetObjectItem(value.jsonValue, &key);
      if (NULL == item) {
        throw (std::exception());
      }
      return (Value(item));
    }
  };
}

#endif /* _json_hpp__ */