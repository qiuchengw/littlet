#pragma once

// #include <stdint.h>
#include <string>
#include <vector>
#include <tchar.h>
#include "rapidjson.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include "prettywriter.h"
#include "filestream.h"

// typedef rapidjson::UTF16<wchar_t> UTF16;
// typedef rapidjson::GenericDocument<UTF16 > RJsonDocumentW;
// typedef rapidjson::GenericValue<UTF16 > RJsonValueW;
// //typedef rapidjson::Writer<UTF16 > RJsonWriterW;
// typedef rapidjson::GenericReader<UTF16 > RJsonReaderW;
// typedef rapidjson::GenericStringBuffer<UTF16 > RJsonStringBufferW;

typedef rapidjson::Document RJsonDocument;
typedef rapidjson::Value RJsonValue;
typedef rapidjson::Reader RJsonReader;
typedef rapidjson::StringBuffer RJsonStringBuffer;
//typedef rapidjson::Writer<rapidjson::UTF8> RJsonWriter;

namespace rapidjson
{
    /**
     *	保存json值到字符串里面	
     *
     *	@param
     *		-[in]
     *          d       json
     *          prety   带格式化
     *          indent  格式化缩进
     *		-[out]
     *          str     保存后的字符串
    **/
//     inline bool SaveToString(RJsonValueW* d, std::wstring& str,
//         bool prety = true, int indent = 4)
//     {
//         // 保存
//         RJsonStringBufferW sb;
//         rapidjson::PrettyWriter<RJsonStringBufferW, ::UTF16> wrt(sb);
//         if (prety)
//         {
//             wrt.SetIndent(' ', (indent <= 0) ? 4 : indent); // 4个空格
//         }
//         d->Accept(wrt);
//         str = sb.GetString();
//         return true;
//     }

    /**
    *	保存json值到字符串里面
    *
    *	@param
    *		-[in]
    *          d       json
    *          prety   带格式化
    *          indent  格式化缩进
    *		-[out]
    *          str     保存后的字符串
    **/
    template<typename _StrType> // QString / std::string
    inline bool SaveToString(RJsonValue* d, _StrType& str,
        bool prety = true, int indent = 4)
    {
        // 保存
        RJsonStringBuffer sb;
        rapidjson::PrettyWriter<RJsonStringBuffer, rapidjson::UTF8<> > wrt(sb);
        if (prety)
        {
            wrt.SetIndent(' ', (indent <= 0) ? 4 : indent); // 4个空格
        }
        d->Accept(wrt);
        str = sb.GetString();
        return true;
    }

}

#ifdef QSTRING_H

class RJsonDoc : public rapidjson::Document
{
public:
    /**
     *		
     *	@param
     *		-[in]
     *		    content     json内容
     *		    auto_reset_if_fail  如果解析错误，自动设置成解析空文件
     *
    **/
    RJsonDoc(const QString& content = "{}", bool auto_reset_if_fail = true)
    {
        MyParse(content, auto_reset_if_fail);
    }

    // false ： has error
    bool MyParse(const QString& content, bool auto_reset_if_fail)
    {
        QByteArray ar = content.toUtf8();
        Parse<0>(ar.data());

        if (HasParseError())
        {
            if (auto_reset_if_fail)
            {
                Parse<0>("{}");
            }
            return false;
        }
        return true;
    }
};

#endif // QT_DLL

