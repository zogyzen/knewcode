#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <memory>
#include <thread>
#include <deque>

#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <boost/functional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/bind/bind.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

#include <boost/process.hpp>

#include "common/base_type.h"
#include "common/knewcode_version.h"
#include "util/auto_release.h"
#include "util/load_library.h"

namespace KC
{
    // 公共函数
	struct CUtilFunc
	{
        // 版本信息
        static std::string KcVersionForPathName(void)
        {
            return (boost::format("v%d_%d_%d") % c_KnewcodeVersionMajor % c_KnewcodeVersionMinor % c_KnewcodeVersionPatch).str();
        }
        static std::string KcVersionForShow(void)
        {
            return (boost::format("v%d.%d.%d") % c_KnewcodeVersionMajor % c_KnewcodeVersionMinor % c_KnewcodeVersionPatch).str();
        }
        static std::string KcVersionForFullInfo(void)
        {
            return (boost::format("%s %s") % c_KnewcodeName % CUtilFunc::KcVersionForShow()).str();
        }

            // 格式化路径字符串
            static std::string FormatPath(std::string sPath)
            {
                boost::algorithm::trim_right_if(sPath, boost::algorithm::is_any_of("\\"));
                boost::algorithm::trim_right_if(sPath, boost::algorithm::is_any_of("/"));
                return sPath;
            }

            // 转换为绝对路径
            static std::string ToAbsPath(std::string sPath, std::string sRoot = ".")
            {
                if (!boost::filesystem::path(sPath).is_absolute())
                    sPath = (sRoot.empty() ? "" : boost::algorithm::trim_right_copy_if(sRoot, boost::is_any_of("/\\")) + "/") + sPath;
                boost::filesystem::path _path(sPath);
                // _path = boost::filesystem::system_complete(_path);
                if (boost::filesystem::exists(_path))
                    _path = boost::filesystem::canonical(_path);
                sPath = _path.string();
                return CUtilFunc::FormatPath(sPath);
            }

            // 字符串全小写
            static std::string StrToLower(std::string str)
            {
                boost::algorithm::trim(str);
                boost::algorithm::to_lower(str);
                return str;
            }

            // 安全转换字符串
            static std::string PCharSafeToStr(const char* p, const char* pDef = "")
            {
                return CUtilFunc::PCharSafeToPChar(p, pDef);
            }
            static const char* PCharSafeToPChar(const char* p, const char* pDef = "")
            {
                return nullptr == p ? pDef : p;
            }
            static std::string PCharSafeToStrBlank2Def(const char* p, const char* pDef = "")
            {
                return CUtilFunc::PCharSafeToPCharBlank2Def(p, pDef);
            }
            static const char* PCharSafeToPCharBlank2Def(const char* p, const char* pDef = "")
            {
                return nullptr == p || strlen(p) == 0 ? pDef : p;
            }

            static std::string TimeToStr(boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time(), std::string fmt = "%Y-%m-%d %H:%M:%S.%f")
            {
                boost::posix_time::time_facet *facet = new boost::posix_time::time_facet(fmt.c_str());
                std::stringstream ss;
                ss.imbue(std::locale(ss.getloc(), facet));
                ss << pt;
                return ss.str();
            }
            static std::string StdTimeToGMT(time_t tm)
            {
                std::ostringstream ss;
                ss << std::put_time(gmtime(&tm), "%F %T");
                return ss.str();
            }

            static boost::posix_time::ptime StrToTime(std::string dt, std::string fmt = "%Y-%m-%d %H:%M:%S")
            {
                boost::posix_time::time_input_facet input_facet(fmt);
                std::stringstream ss;
                ss.imbue(std::locale(ss.getloc(), &input_facet));
                ss << dt;
                boost::posix_time::ptime pResult;
                ss >> pResult;
                return pResult;
            }

            // 标准时间格式转换为GMT格式时间
            static std::string ISO2GMT(std::string dt)
            {
                boost::posix_time::ptime pt = boost::posix_time::time_from_string(dt);
                return CUtilFunc::TimeToStr(pt, "%a, %d %b %Y %H:%M:%S %z");
            }
            static std::string ISO2GMT2(std::string dt)
            {
                return CUtilFunc::StdTimeToGMT(string2time_t(dt));
            }

            //string 转换为time_t  时间格式为2020-03-28 18:25:26
            static std::time_t string2time_t(const std::string string_time)
            {
                tm tm1;
                memset(&tm1, 0, sizeof(tm1));
                sscanf(string_time.c_str(), "%d-%d-%d %d:%d:%d",
                    &(tm1.tm_year),
                    &(tm1.tm_mon),
                    &(tm1.tm_mday),
                    &(tm1.tm_hour),
                    &(tm1.tm_min),
                    &(tm1.tm_sec));
                tm1.tm_year -= 1900;
                tm1.tm_mon -= 1;
                tm1.tm_hour += 8;
                time_t time1 = mktime(&tm1);
                return time1;
            }

            // GMT时间格式转换为标准格式时间
            static std::string GMT2ISO(std::string dt)
            {
                boost::posix_time::ptime pt = CUtilFunc::StrToTime(dt, "%a, %d %b %Y %H:%M:%S %z");
                return CUtilFunc::TimeToStr(pt, "%Y-%m-%d %H:%M:%S");
            }

            // 时间戳（秒）
            static long long GetCurrentStampS(boost::posix_time::ptime tm = boost::posix_time::second_clock::local_time())
            {
                boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
                boost::posix_time::time_duration time_from_epoch = tm - epoch;
                return time_from_epoch.total_seconds();
            }
            static boost::posix_time::ptime GetPTimeByStampS(long long s)
            {
                boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
                return epoch + boost::posix_time::seconds(s);
            }
            // 时间戳（毫秒）
            static long long GetCurrentStampMS(boost::posix_time::ptime tm = boost::posix_time::microsec_clock::local_time())
            {
                boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
                boost::posix_time::time_duration time_from_epoch = tm - epoch;
                return time_from_epoch.total_milliseconds();
            }
            static boost::posix_time::ptime GetPTimeByStampMS(long long ms)
            {
                boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
                return epoch + boost::posix_time::milliseconds(ms);
            }

            // 当前线程ID
            static int CurrThreadID(void)
            {
                try
                {
                    std::ostringstream oss;
                    oss << std::this_thread::get_id();
                    std::string stid = oss.str();
                    return std::stoi(stid);
                }
                catch (...)
                {
                    return 0;
                }
            }

            // 字符集编码转换
            static std::string ConvCharset(std::string s, std::string src, std::string dst)
            {
                return boost::locale::conv::between(s, dst, src);
            }
            static std::string GbkToUtf8Throw(std::string s)
            {
                // return boost::locale::conv::to_utf<char>(s, "GBK");
                return ConvCharset(s, "GBK", "UTF-8");
            }
            static std::string Utf8ToGbkThrow(std::string s)
            {
                return ConvCharset(s, "UTF-8", "GBK");
            }
            static std::string GbkToUtf8(std::string s)
            {
                try
                {
                    return GbkToUtf8Throw(s);
                }
                catch (...)
                {
                    std::cout << "Conv Fail - " << __CURR_CODE_PLACE_C__ << std::endl;
                }
                return s;
            }
            static std::string Utf8ToGbk(std::string s)
            {
                try
                {
                    return Utf8ToGbkThrow(s);
                }
                catch (...)
                {
                    std::cout << "Conv Fail - " << __CURR_CODE_PLACE_C__ << std::endl;
                }
                return s;
            }

            static std::wstring M2W(std::string s, std::string loc = "UTF-8")
            {
                if (s.empty()) return L"";
                else return boost::locale::conv::to_utf<wchar_t>(s, loc);
            }
            static std::string W2M(std::wstring ws, std::string loc = "UTF-8")
            {
                if (ws.empty()) return "";
                else return boost::locale::conv::from_utf(ws, loc);
            }

            static bool isGBK(std::string s)
            {
                for (unsigned i  = 0; i < s.size(); )
                {
                    if (s[i] <= 0x7f)
                        //编码小于等于127,只有一个字节的编码，兼容ASCII
                        ++i;
                    else
                    {
                        //大于127的使用双字节编码
                        if  (s[i] >= (char)0x81 && s[i] <= (char)0xfe && s[i + 1] >= (char)0x40 && s[i + 1] <= (char)0xfe && s[i + 1] != (char)0xf7)
                            i += 2;
                        else
                            return false;
                    }
                }
                return true;
            }
            static bool isGBKCode(const string strIn)
            {
                unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
                unsigned char chr = strIn.at(0);
                bool bAllAscii = true; //如果全部都是ASCII,

                    for (unsigned int i = 0; strIn[i] != '\0'; ++i){
                    chr = strIn.at(i);
                    if ((chr & 0x80) != 0 && nBytes == 0){// 判断是否ASCII编码,如果不是,说明有可能是GBK
                        bAllAscii = false;
                    }

                    if (nBytes == 0) {
                        if (chr >= 0x80) {
                            if (chr >= 0x81 && chr <= 0xFE){
                                nBytes = +2;
                            }
                            else{
                                return false;
                            }

                            nBytes--;
                        }
                    }
                    else{
                        if (chr < 0x40 || chr>0xFE){
                            return false;
                        }
                        nBytes--;
                    }//else end
                }

                if (nBytes != 0)  {     //违返规则
                    return false;
                }

                if (bAllAscii){ //如果全部都是ASCII, 也是GBK
                    return true;
                }

                return true;
            }
            static bool IsUTF8(const void *pBuffer, int size)
            {
                bool IsUTF8 = false;
                unsigned char *start = (unsigned char *)pBuffer;
                unsigned char *end = (unsigned char *)pBuffer + size;
                while (start < end)
                {
                    if (*start < 0x80) // (10000000): 值小于0x80的为ASCII字符
                    {
                        start++;
                    }
                    else if (*start < (0xC0)) // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
                    {
                        IsUTF8 = false;
                        break;
                    }
                    else if (*start < (0xE0)) // (11100000): 此范围内为2字节UTF-8字符
                    {
                        IsUTF8 = true;
                        if (start >= end - 1)
                            break;
                        if ((start[1] & (0xC0)) != 0x80)
                        {
                            IsUTF8 = false;
                            break;
                        }
                        start += 2;
                    }
                    else if (*start < (0xF0)) // (11110000): 此范围内为3字节UTF-8字符
                    {
                        IsUTF8 = true;
                        if (start >= end - 2) break;
                        if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
                        {
                            IsUTF8 = false; break;
                        }
                        start += 3;
                    }
                    else if (*start < (0xF8)) // (11111000): 此范围内为4字节UTF-8字符
                    {
                        IsUTF8 = true;
                        if (start >= end - 3) break;
                        if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80 || (start[3] & (0xC0)) != 0x80)
                        {
                            IsUTF8 = false; break;
                        }
                        start += 4;
                    }
                    else
                    {
                        IsUTF8 = false;
                        break;
                    }
                }
                return IsUTF8;
            }
            static std::string GbkToUtf8ByCheck(std::string s)
            {
                if (isGBKCode(s) && !IsUTF8(s.c_str(), static_cast<int>(s.size())))
                    return GbkToUtf8(s);
                return s;
            }

            static int preNUm(unsigned char byte)
            {
                unsigned char mask = 0x80;
                int num = 0;
                for (int i = 0; i < 8; ++i)
                {
                    if ((byte & mask) == mask)
                    {
                        mask = mask >> 1;
                        num++;
                    }
                    else break;
                }
                return num;
            }

            static bool isUtf8(std::string s)
            {
                int num = 0;
                for (unsigned i = 0; i < s.size(); )
                {
                    if ((s[i] & 0x80) == 0x00)
                        // 0XXX_XXXX
                        ++i;
                    else if ((num = preNUm(s[i])) > 2)
                    {
                        // 110X_XXXX 10XX_XXXX
                        // 1110_XXXX 10XX_XXXX 10XX_XXXX
                        // 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
                        // 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
                        // 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
                        // preNUm() 返回首个字节8个bits中首�?0bit前面1bit的个数，该数量也是该字符所使用的字节数
                        ++i;
                        for(int j = 0; j < num - 1; ++j)
                        {
                            //判断后面num - 1 个字节是不是都是10开
                            if ((s[i] & 0xc0) != 0x80)
                                return false;
                            ++i;
                        }
                    }
                    else return false;
                }
                return true;
            }
            static bool is_str_utf8(const char* str)
            {
              unsigned int nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
              unsigned char chr = *str;
              bool bAllAscii = true;
              for (unsigned int i = 0; str[i] != '\0'; ++i){
                chr = *(str + i);
                //判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
                if (nBytes == 0 && (chr & 0x80) != 0){
                  bAllAscii = false;
                }
                if (nBytes == 0) {
                  //如果不是ASCII码,应该是多字节符,计算字节数
                  if (chr >= 0x80) {
                    if (chr >= 0xFC && chr <= 0xFD){
                      nBytes = 6;
                    }
                    else if (chr >= 0xF8){
                      nBytes = 5;
                    }
                    else if (chr >= 0xF0){
                      nBytes = 4;
                    }
                    else if (chr >= 0xE0){
                      nBytes = 3;
                    }
                    else if (chr >= 0xC0){
                      nBytes = 2;
                    }
                    else{
                      return false;
                    }
                    nBytes--;
                  }
                }
                else{
                  //多字节符的非首字节,应为 10xxxxxx
                  if ((chr & 0xC0) != 0x80){
                    return false;
                  }
                  //减到为零为止
                  nBytes--;
                }
              }
              //违返UTF8编码规则
              if (nBytes != 0) {
                return false;
              }
              if (bAllAscii){ //如果全部都是ASCII, 也是UTF8
                return true;
              }
              return true;
            }

            // 函数地址转16进制字符串
            template<typename TFunc>
            static std::string GetFuncAddr(TFunc f)
            {
                char buf [32] = {0};
                sprintf(buf , "%llu", (intptr_t)(void*)(&f));
                return (boost::format("%X") % atoi(buf)).str();
            }

            static bool isEqual(double f1, double f2)
            {
                return fabs(f1 - f2) < 1e-6;
            }

            static bool isInt(double fVal)
            {
                return fabs(fVal - static_cast<int>(fVal)) < 1e-6;
            }
            static bool isLLong(double fVal)
            {
                return fabs(fVal - static_cast<long long>(fVal)) < 1e-6;
            }

            // 转16进制字符串
            static std::string TransHex(const void *inPutBuf, size_t iLen, std::string space = "")
            {
                std::string sResult;
                const unsigned char* inPut = reinterpret_cast<const unsigned char*>(inPutBuf);
                for (size_t i = 0; i < iLen; ++i)
                    sResult += (boost::format("%02X%s") % static_cast<unsigned>(inPut[i]) % space).str();
                return sResult;
            }
            static std::string TransHex(std::string sData, std::string space = "")
            {
                return CUtilFunc::TransHex(sData.c_str(), sData.size(), space);
            }
            template<typename T>
            static std::string TransHex(const T &data, std::string space = "")
            {
                return CUtilFunc::TransHex(&data, sizeof(T), space);
            }
            // 16进制字符串转整数（如，0xA5）
            static unsigned HexToUInt(std::string sHex)
            {
                return static_cast<unsigned>(std::stoi(sHex, nullptr, 16));
            }
            // 16进制字符串转整数字符串（如，0xA5 0x03 0x55）
            static std::string HexToUCharStr(std::string sHex, std::string space = "")
            {
                std::vector<unsigned char> vctResult;
                // 分解
                std::vector<std::string> hexVec;
                if (!space.empty())
                    boost::algorithm::split(hexVec, sHex, boost::is_any_of(space));
                else for (int i = 0, c = sHex.size(); i < c; i += 2)
                    hexVec.push_back(sHex.substr(i, 2));
                for(std::string s : hexVec)
                    try
                    {
                        if (!s.empty())
                            vctResult.push_back(static_cast<unsigned char>(CUtilFunc::HexToUInt(s)));
                    }
                    catch (...) {}
                // 返回
                return std::string(reinterpret_cast<char*>(vctResult.data()), vctResult.size());
            }

            // base64编解码
            static std::string Base64EncodeBase(unsigned const char *inPut, int iLen)
            {
                typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<unsigned const char *, 6, 8>> Base64EncodeIter;
                std::stringstream  result;
                copy(Base64EncodeIter(inPut),
                        Base64EncodeIter(inPut + iLen),
                        ostream_iterator<char>(result));
                return result.str();;
            }
            static std::string Base64Encode(unsigned const char *inPut, int iLen)
            {
                std::string sResult = Base64EncodeBase(inPut, iLen);
                for (size_t i = 0, Num = (3 - iLen % 3) % 3; i < Num; ++i)
                        sResult += '=';
                return sResult;
            }
            static std::string Base64Decode(unsigned const char *inPut, int iLen)
            {
                typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<unsigned const char *>, 8, 6> Base64DecodeIter;
                std::stringstream result;
                try
                {
                    copy(Base64DecodeIter(inPut),
                            Base64DecodeIter(inPut + iLen),
                            ostream_iterator<char>(result));
                }
                catch (...)
                {
                    return "";
                }
                return result.str();
            }

            // 大小端转换
            static void RotateDB(void *db, unsigned len)
            {
                std::deque<char> vec1(len), vec2;
                // char *p = static_cast<char*>(db);
                //std::copy(p, p + len, std::back_inserter(vec1));
                memcpy(&vec1[0], db, len);
                std::copy(vec1.begin(), vec1.end(), std::front_inserter(vec2));
                memcpy(db, &vec2[0], len);
            }
            template<typename T>
            static T RotateT(T i)
            {
                RotateDB(&i, sizeof (T));
                return i;
            }
            static unsigned RotateUInt(unsigned i)
            {
                /*
                union db
                {
                    unsigned i;
                    unsigned char c[4];
                };
                db d1, d2;
                d1.i = i;
                d2.c[0] = d1.c[3];
                d2.c[1] = d1.c[2];
                d2.c[2] = d1.c[1];
                d2.c[3] = d1.c[0];
                return d2.i;
                */
                return RotateT(i);
            }

            // sha1加密
            static std::string GetSHA1(std::string buffer)
            {
                // sha1摘要
                // char hash[20];
                boost::uuids::detail::sha1 boost_sha1;
                boost_sha1.process_bytes(buffer.c_str(), buffer.size());
                boost::uuids::detail::sha1::digest_type digest;
                boost_sha1.get_digest(digest);

                // 大小端转换
                for (int i = 0; i < 5; ++i)
                    digest[i] = RotateUInt(digest[i]);

                // base64编码
                return Base64Encode(reinterpret_cast<unsigned const char*>(digest), 20);
            }

            // 超时返回（true：正常返回。false：超时返回）
            template<typename F>
            static bool TimeoutRun(F f, unsigned ms)
            {
                bool bResult = true;
                std::shared_ptr<std::runtime_error> exPtr;
                auto fThrdRun = [&](void) {
                    try
                    {
                        boost::this_thread::interruption_point();
                        f();
                    }
                    catch (boost::thread_interrupted&) {}
                    catch (std::exception &ex)
                    {
                        exPtr.reset(new std::runtime_error((boost::format("%s:<%s> (%s) %s") % __CURR_CODE_PLACE_C__ % typeid(F).name() % typeid(ex).name() % ex.what()).str()));
                    }
                    catch (...)
                    {
                        exPtr.reset(new std::runtime_error((boost::format("%s:<%s> Unknown exception") % __CURR_CODE_PLACE_C__ % typeid(F).name()).str()));
                    }
                };
                boost::thread::attributes attrs;
                attrs.set_stack_size(1024 * 64);
                boost::thread thrd(attrs, boost::bind<void>(fThrdRun));
                if (thrd.joinable()) bResult = thrd.timed_join(boost::posix_time::milliseconds(ms));
                if (exPtr.get() != nullptr)
                    throw std::exception(*exPtr);
                if (thrd.joinable()) thrd.interrupt();
                boost::this_thread::sleep(boost::posix_time::milliseconds(6));
                return bResult;
            }

            // 延迟运行
            template<typename F>
            static void DelayRun(F f, unsigned ms = 66)
            {
                boost::thread([=]()mutable{
                    if (ms > 0)
                        boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
                    f();
                });
            }

            // 发送消息
            template<typename TSrv, typename TMxMQInfo>
            static int SendMxMessage(TSrv &srv, std::string name, TMxMQInfo& mxm, std::string sMsg = "")
            {
                // 发送线程
                auto fSendThrd = [&](std::shared_ptr<std::atomic_int> iResult)
                {
                    boost::this_thread::interruption_point();
                    try
                    {
                        boost::interprocess::message_queue mq(boost::interprocess::open_only, name.c_str());
                        try
                        {
                            /*
                            unsigned iCurrNum = mq.get_num_msg();
                            unsigned iMsgSZ = mq.get_max_msg_size();
                            unsigned iMaxSZ = mq.get_max_msg();
                            std::cout << "try_send: " << name << " [" << iCurrNum << "/" << iMsgSZ << "/" << iMaxSZ << "] \t" << sMsg << std::endl;
                            //mq.send(&mxm, sizeof(TMxMQInfo), 0);
                            //mq.timed_send(&mxm, size, 0, posix_time::second_clock::local_time() + posix_time::milliseconds(866));
                            if (iCurrNum < iMaxSZ && !mq.try_send(&mxm, sizeof(TMxMQInfo), 0)) throw "try_send fail";
                            */
                            mq.send(&mxm, sizeof(TMxMQInfo), 0);
                        }
                        catch(boost::interprocess::interprocess_exception &ex)
                        {
                            if (0 == *iResult) *iResult = 1;
                            srv.WriteLogDebug("SendMxMessage Fail", sMsg.c_str(), (boost::format("[%s][%s]\t\t%d: %s") % name % typeid(ex).name() % ex.get_error_code() % ex.what()).str().c_str());
                        }
                        catch (std::exception& ex)
                        {
                            if (0 == *iResult) *iResult = 1;
                            srv.WriteLogDebug("SendMxMessage Fail", sMsg.c_str(), ("[" + name + "][" + typeid(ex).name() + "]\t\t" + ex.what()).c_str());
                        }
                        catch (const char* ex)
                        {
                            if (0 == *iResult) *iResult = 1;
                            srv.WriteLogDebug("SendMxMessage Fail", sMsg.c_str(), ("[" + name + "]\t\t" + ex).c_str());
                        }
                        catch (...)
                        {
                            if (0 == *iResult) *iResult = 1;
                            srv.WriteLogDebug("SendMxMessage Fail", sMsg.c_str(), ("[" + name + "]").c_str());
                        }
                    }
                    catch(boost::interprocess::interprocess_exception &ex)
                    {
                        *iResult = -1;
                        //std::cout << "Can't Open Message Queue: " << name << "\t\t" << ex.what() << std::endl;
                        boost::interprocess::message_queue::remove(name.c_str());
                        srv.WriteLogDebug("SendMxMessage Exception", sMsg.c_str(), (boost::format("[%s][%s]\t\t%d: %s") % name % typeid(ex).name() % ex.get_error_code() % ex.what()).str().c_str());
                    }
                    catch (...)
                    {
                        if (0 == *iResult) *iResult = 1;
                        //std::cout << "Can't Open Message Queue: " << name << std::endl;
                        srv.WriteLogDebug("SendMxMessage Exception", sMsg.c_str(), "Unknown Error");
                    }
                };
                // 结果
                std::shared_ptr<std::atomic_int> iResult(new std::atomic_int);
                *iResult = 0;   // 0发送成功；1；发送失败；-1消息队列不存在
                // 超时运行
                if (!CUtilFunc::TimeoutRun(boost::bind<void>(fSendThrd, iResult), 6666))
                {
                    if (0 == *iResult) *iResult = 1;
                    //std::cout << "Remove Message Queue Begin: " << name << std::endl;
                    boost::interprocess::message_queue::remove(name.c_str());
                    //std::cout << "Remove Message Queue End: " << name << std::endl;
                    srv.WriteLogDebug("SendMxMessage Timeout", sMsg.c_str(), ("[" + name + "]").c_str());
                }
                return *iResult;
            }

            // 唯一的链接编号
            static std::string GetUniqueConnID(unsigned srvid, long wcid)
            {
                return (boost::format("%d_%d_%ld") % srvid % boost::this_process::get_id() % wcid).str();
            }

            // 自动分配线程内持久字符串空间（为C语言提供）
            static char* MakeKeepStr(unsigned len)
            {
                static thread_local std::list<std::string> s_list(1);
                static thread_local auto s_iter = s_list.begin();
                s_list.push_back(std::string(len + 1, '\0'));
                ++s_iter;
                return s_iter->data();
            }

            // 其他类型转线程内持久字符串
            template<int flag, typename T>
            static const char* ToKeepStr(T v)
            {
                static thread_local std::string str;
                str.clear();
                try
                {
                    // str = boost::lexical_cast<std::string>(v);
                    str = std::to_string(v);
                }
                catch (...) {}
                return str.c_str();
            }
            template<int flag = 0>
            static const char* ToKeepStr(std::string v)
            {
                static thread_local std::string str;
                str = v;
                return str.c_str();
            }
            template<int flag = 0>
            static const char* ToKeepStr(boost::format& v)
            {
                static thread_local std::string str;
                str = v.str();
                return str.c_str();
            }
            template<int flag = 0>
            static const char* ToKeepStr(const char* v)
            {
                return nullptr == v ? nullptr : ToKeepStr<flag>(std::string(v));
            }

            // 通过管道获取命令行输出
            static std::string GetCmdResult(std::string cmd)
            {
            #ifdef WIN32    // Windows环境
                #define POPEN _popen
                #define PCLOSE _pclose
            #else           // linux环境
                #define POPEN popen
                #define PCLOSE pclose
            #endif
                std::string strResult;
                auto file = POPEN(cmd.c_str(), "r");
                if (nullptr != file)
                {
                    CAutoRelease _autoClose([&](){ PCLOSE(file); });
                    while (!feof(file))
                    {
                        char buffer[130] = { 0 };
                        if (fgets(buffer, 128, file) != NULL)
                            strResult += buffer;
                    }
                }
                return strResult;
            }
            static int GetCmdResult(std::string cmd, std::string &strResult)
            {
                namespace bp = boost::process;
                std::error_code ec;
                bp::ipstream is;
                int iResult = bp::system(cmd, bp::std_out > is, ec);
                if (!ec)
                {
                    char result[1024] = { 0 };
                    is.read(result, 1024);
                    strResult += result;
                }
                return iResult;
            }

        // 保存文件
        static void SaveFile(std::string sFile, std::string sBuf)
        {
            std::ofstream fOut(sFile, std::ios::binary | std::ios::out);
            if (!fOut.is_open()) throw std::runtime_error("Can't Open - " + sFile);
            CAutoRelease _auto([&](){ fOut.close(); });
            fOut.write(sBuf.c_str(), sBuf.size());
        }
        // 读取文件
        static std::string ReadFile(std::string sFile)
        {
            int iSz = static_cast<int>(boost::filesystem::file_size(sFile));
            std::ifstream fIn(sFile, std::ios::binary);
            if (!fIn.is_open()) throw std::runtime_error("Can't Open - " + sFile);
            CAutoRelease _auto([&](){ fIn.close(); });
            char *pBuf = new char[iSz + 1] { 0 };
            fIn.read(pBuf, iSz);
            std::string sBuf(pBuf, iSz);
            return sBuf;
        }
        static std::string ReadTxtFile(std::string sFile)
        {
            std::ifstream kcFile;
            kcFile.open(sFile.c_str());
            if (!kcFile.is_open()) throw std::runtime_error("Can't Open - " + sFile);
            CAutoRelease _auto(boost::bind(&std::ifstream::close, &kcFile));
            // 读取文件内容
            std::stringstream bufStream;
            bufStream << kcFile.rdbuf();
            return bufStream.str();
        }
        static std::string ReadFile2(std::string sFile)
        {
            int iSz = static_cast<int>(boost::filesystem::file_size(sFile));
            boost::iostreams::stream<boost::iostreams::file_source> file_stream(sFile);
            if (!file_stream.is_open()) throw std::runtime_error("Can't Open - " + sFile);
            CAutoRelease _auto([&](){ file_stream.close(); });
            char *pBuf = new char[iSz + 1] { 0 };
            file_stream.read(pBuf, iSz);
            std::string sBuf(pBuf, iSz);
            return sBuf;
        }
        static std::string ReadFile3(std::string sFile)
        {
            boost::iostreams::mapped_file_source file(sFile); // 映射文件到内存中
            std::string content(static_cast<const char*>(file.data()), file.size());
            return content;
        }

        // 获取字符串某行字符串
        static std::string GetStrLine(const std::string& text, int lineNumber)
        {
            std::istringstream stream(text);
            std::string line;
            int currentLine = 0;
            while (std::getline(stream, line))
            {
                if (currentLine == lineNumber - 1) // 注意行号是从0开始的，所以这里是lineNumber - 1
                    return line;
                currentLine++;
            }
            return ""; // 如果指定的行号超出范围，返回空字符串
        }

        // 分解键值对
        static std::pair<std::string, std::string> splitKeyValue(const std::string& str)
        {
            std::pair<std::string, std::string> result;
            std::stringstream ss(str);
            std::getline(ss, result.first, '=');
            std::getline(ss, result.second, '\0');
            result.first = boost::trim_copy(result.first);
            result.second = boost::trim_copy(result.second);
            return result;
        }
        // 分解Url上的get参数
        static void splitUrlGetParm(std::map<std::string, std::string> &getParms, std::string get)
        {
            getParms.clear();
            std::string sGet = boost::algorithm::trim_copy(get);
            if (!sGet.empty())
            {
                std::vector<std::string> getsVec;
                boost::algorithm::split(getsVec, sGet, boost::is_any_of("&"));
                for (std::string gt : getsVec)
                {
                    // vector<string> getVec;
                    // algorithm::split(getVec, gt, is_any_of("="));
                    // if (getVec.size() > 1)
                    //     getParms.insert(make_pair(getVec[0], getVec[1]));
                    auto prParm = CUtilFunc::splitKeyValue(boost::algorithm::trim_left_copy(gt));
                    if (getParms.find(prParm.first) == getParms.end())
                        getParms.insert(prParm);
                }
            }
        }

        // 获取Uri路径部分
        static const char* GetUrlPagePath(const char* uri)
        {
            static thread_local string sResult;
            sResult = uri;
            auto iPos = sResult.find_last_of("/\\");
            if (string::npos == iPos) sResult = "/";
            else sResult = sResult.substr(0, iPos);
            return sResult.c_str();
        }

        // 获取url虚拟路径对应的本地路径
        template<typename TPath>
        static std::string TransVPathToLocal(const map<std::string, TPath>& vPaths, std::string sUri)
        {
            std::string sLocalFile;
            for (auto &pth : vPaths)
            {
                std::string sVirPth = pth.first;
                if (sUri == sVirPth)
                    sLocalFile = pth.second;
                else if (boost::algorithm::istarts_with(sUri, sVirPth + "/"))
                    sLocalFile = boost::algorithm::ireplace_first_copy(sUri, sVirPth, std::string(pth.second));
            }
            return sLocalFile;
        }

        template<typename TPath>
        static std::string GetUrlRootByVPath(const map<std::string, TPath>& vPaths, std::string sUri)
        {
            std::string sResult = sUri;
            for (auto it = vPaths.find(sResult); vPaths.end() == it; it = vPaths.find(sResult))
            {
                sResult = boost::filesystem::path(sResult).parent_path().string();
                if ("/" == sResult || sResult.empty()) break;
            }
            return sResult;
        }

        // 格式化字符串
        template <typename T>
        static boost::format FormatStrWithArgs(boost::format fmt, const T& val)
        {
            try
            {
                return fmt % val;
            }
            catch (...) {}
            return fmt;
        }
        template <typename T, typename... Args>
        static boost::format FormatStrWithArgs(boost::format fmt, const T& val, Args... args)
        {
            auto newFmt = FormatStrWithArgs(fmt, val);     // 拆开第1个
            return FormatStrWithArgs(newFmt, args...);     // 余下的，递归
        }
        template <typename... Args>
        static std::string FormatStrWithArgs(std::string fmt, std::string def, Args... args)
        {
            try
            {
                return FormatStrWithArgs(boost::format(fmt), args...).str();
            }
            catch (...) {}
            return def;
        }
    };

    // 任意类型转换为字符串
    template<typename T, typename... TArgs>
    struct TBoostAnyToStdString
    {
        static bool To(boost::any val, std::string &sResult)
        {
            return TBoostAnyToStdString<T>::To(val, sResult) || TBoostAnyToStdString<TArgs...>::To(val, sResult);
        }
    };
    template<typename T>
    struct TBoostAnyToStdString<T>
    {
        static bool To(boost::any val, std::string &sResult)
        {
            bool bResult = val.type() == typeid(T);
            if (bResult) sResult = std::to_string(boost::any_cast<T>(val));
            return bResult;
        }
    };
    template<>
    struct TBoostAnyToStdString<std::string>
    {
        static bool To(boost::any val, std::string &sResult)
        {
            bool bResult = val.type() == typeid(std::string);
            if (bResult) sResult = boost::any_cast<std::string>(val);
            return bResult;
        }
    };
    template<typename ...TArgs>
    static std::string BoostAnyToStdString(boost::any val, std::string sDef = "")
    {
        std::string sResult = sDef;
        try
        {
            TBoostAnyToStdString<TArgs...>::To(val, sResult);
        }
        catch (boost::bad_any_cast& ex)
        {
            std::cout << val.type().name() << ": " << ex.what() << std::endl;
        }
        catch (std::exception& ex)
        {
            std::cout << val.type().name() << ": " << typeid(ex).name() << " - " << ex.what() << std::endl;
        }
        return sResult;
    }
    static std::string BoostAnyToStdString(boost::any val, std::string sDef = "")
    {
        return BoostAnyToStdString<std::string, int, unsigned, long, unsigned long, long long, unsigned long long,
                short, unsigned short, char, unsigned char, bool, float, double, long double>(val, sDef);
    }

    // 任意类型转换为指定类型
    template<typename TResult, typename TFirst, typename... TArgs>
    struct TCastBoostAnyTo
    {
        static bool To(boost::any val, TResult &result)
        {
            return TCastBoostAnyTo<TResult, TFirst>::To(val, result) || TCastBoostAnyTo<TResult, TArgs...>::To(val, result);
        }
    };
    template<typename TResult, typename TFirst>
    struct TCastBoostAnyTo<TResult, TFirst>
    {
        static bool To(boost::any val, TResult &result)
        {
            bool bResult = val.type() == typeid(TFirst);
            if (bResult) result = static_cast<TResult>(boost::any_cast<TFirst>(val));
            return bResult;
        }
    };
    template<typename TFirst, typename ...TArgs>
    static TFirst CastBoostAnyTo(boost::any val, TFirst def = TFirst())
    {
        TFirst result = def;
        try
        {
            TCastBoostAnyTo<TFirst, TFirst, TArgs...>::To(val, result);
        }
        catch (boost::bad_any_cast& ex)
        {
            std::cout << val.type().name() << ": " << ex.what() << std::endl;
        }
        catch (std::exception& ex)
        {
            std::cout << val.type().name() << ": " << typeid(ex).name() << " - " << ex.what() << std::endl;
        }
        return result;
    }
    // 任意类型转换为整型
    static long long CastBoostAnyToInt(boost::any val, long long def = 0)
    {
        return CastBoostAnyTo<long long, int, unsigned, long, unsigned long, unsigned long long,
                short, unsigned short, char, unsigned char, bool>(val, def);
    }
    // 任意类型转换为浮点型
    static double CastBoostAnyToFloat(boost::any val, double def = 0)
    {
        return CastBoostAnyTo<double, float, long double>(val, def);
    }

    // 用于map的比较算法
    template<typename TStr>
    struct TKcNameLess
    {
        const bool m_caseSensitive = false;
        TKcNameLess(bool caseSensitive = false) : m_caseSensitive(caseSensitive) {}
        bool operator()(const TStr &l, const TStr &r) const
        {
            std::string sLeft = l.c_str();
            std::string sRight = r.c_str();
            if (!m_caseSensitive)
            {
                boost::algorithm::to_upper(sLeft);
                boost::algorithm::to_upper(sRight);
            }
            return sLeft > sRight;
        }
    };
    typedef TKcNameLess<std::string> TLessStr;
}
