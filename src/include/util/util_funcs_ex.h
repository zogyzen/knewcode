#pragma once

#include <string>

#include <boost/multiprecision/cpp_int.hpp>

#include "util/util_funcs.h"

namespace KC
{
    // 超长整型
    namespace SuperInt
    {
        using uint2048_t = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<2048, 2048, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void> >;
    }

    // 公共函数
    struct CUtilFuncEx
	{
        // 16进制长整数字符串，base编解码
        static std::string BaseXXEncode(std::string sHex, std::string sCode = c_strCodeBase62x)
        {
            // 长度不能超过256个字节
            if (sHex.size() <= 0) return "";
            else if (sHex.size() > 256) throw std::runtime_error("Out Of Range");
            // 转换为超长整数
            SuperInt::uint2048_t superUInt("0x" + sHex);
            // base62编码
            std::string sResult;
            while (superUInt > 0)
            {
                sResult = sCode[(superUInt % sCode.size()).convert_to<unsigned>()] + sResult;
                superUInt /= sCode.size();
            }
            return sResult;
        }
        static std::string BaseXXDecode(std::string sBase, std::string sCode = c_strCodeBase62x)
        {
            // 超长整数
            SuperInt::uint2048_t superUInt = 0;
            // 累加
            for (int i = 0, c = static_cast<int>(sBase.size()); i < c; ++i)
            {
                unsigned iPos = static_cast<unsigned>(sCode.find(sBase[i]));
                superUInt = superUInt * sCode.size() + iPos;
            }
            // 转换为16进制字符串
            return (boost::format("%X") % superUInt).str();
        }

        // 对字符串进行编解码
        static string BaseXXEncodeByStr(string str)
        {
            string sHex = CUtilFunc::TransHex(str, "");
            string sBase62 = CUtilFuncEx::BaseXXEncode(sHex);
            return sBase62;
        }
        static string BaseXXDecodeToStr(string sBase)
        {
            string sHex = CUtilFuncEx::BaseXXDecode(sBase);
            string sStr = CUtilFunc::HexToUCharStr(sHex, "");
            return sStr;
        }

    };
}
