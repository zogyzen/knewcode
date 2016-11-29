#include "../std.h"
#include "invar_post.h"

////////////////////////////////////////////////////////////////////////////////
// TInvarPost类
KC::one_level::TInvarPost::TInvarPost(IKCActionData& act, CInnerVar& iv) : m_act(act), m_InnerVar(iv)
{
    // POST参数缓冲区
    int bufLength = m_act.GetRequestRespond().GetPostArgLength();
    char* pBuf = new char[bufLength + 1];
    boost::shared_array<char> bufPost(pBuf);
    memset(pBuf, 0, bufLength + 1);
    bufLength = m_act.GetRequestRespond().GetPostArgBuffer(pBuf, bufLength);
    // POST类型
    m_PostType = m_act.GetRequestRespond().GetPostArgType();
    vector<string> vecPostType;
    boost::split(vecPostType, m_PostType, boost::is_any_of(";"));
    m_PostType = vecPostType[0];
    // 参数组类型
    if ("application/x-www-form-urlencoded" == m_PostType)
    {
        string sPost(pBuf, bufLength);
        TInvarGet::InitParms(m_mapArg, sPost);
    }
    // 数据流类型
    else if ("multipart/form-data" == m_PostType && vecPostType.size() > 1)
    {
        string sSep = "--" + boost::replace_first_copy(boost::trim_copy(vecPostType[1]), "boundary=", "");
        DataSeparator dSep(sSep.c_str(), sSep.size());
        for (const char *head = pBuf, *next = pBuf, *tail = pBuf + bufLength; dSep.Separator(next, tail); head = next = tail, tail = pBuf + bufLength)
            this->GetOneDataArg(head + 2, next - 2);    // 去掉前后的回车换行符
    }
    // 其它类型：application/json 或 text/xml
    else
    {
        string sPost(pBuf, bufLength);
        m_mapArg.insert(make_pair(m_PostType, ArgValueT(m_PostType, sPost)));
    }
}

// 获取一个缓冲区参数
inline void KC::one_level::TInvarPost::GetOneDataArg(const char* head, const char* tail)
{
    // 查找标志位
    string sTag = "Content-Disposition: form-data;";
    const char* tag = DataSeparator::MemFind(head, tail, sTag.c_str(), sTag.size());
    if (nullptr != tag && tag < tail)
    {
        // 第1行
        const char* line_tail_1st = DataSeparator::MemFind(tag, tail, "\r\n", 2);
        string line1st(tag, line_tail_1st - 1);
        vector<string> vecLine1st;
        boost::split(vecLine1st, line1st, boost::is_any_of(";"));
        if (vecLine1st.size() > 1)
        {
            // 参数名
            string sName = boost::replace_last_copy(boost::replace_first_copy(boost::trim_copy(vecLine1st[1]), "name=\"", ""), "\"", "");
            // 参数值
            ArgValueT argVal(sName);
            // 扩展类型
            if (vecLine1st.size() > 2)
            {
                vector<string> vecExval;
                string sExval = boost::trim_copy(vecLine1st[2]);
                boost::split(vecExval, sExval, boost::is_any_of("="));
                // 文件类型
                if ("filename" == vecExval[0])
                {
                    // 文件名
                    argVal.val_type = ArgValueT::vteFile;
                    if (vecExval.size() > 1)
                    {
                        filesystem::path pathFile(boost::replace_all_copy(vecExval[1], "\"", ""));
                        argVal.val = pathFile.leaf().string();
                    }
                    // 文件缓冲区
                    const char* pBufPos = this->GetOneDataArgBuf(line_tail_1st, tail);
                    if (nullptr != pBufPos && tail > pBufPos)
                    {
                        argVal.buf_size = tail - pBufPos;
                        char* bufFile = new char[argVal.buf_size + 1];
                        boost::shared_array<char> bufPtr(bufFile);
                        memset(bufFile, 0, argVal.buf_size + 1);
                        memcpy(bufFile, pBufPos, argVal.buf_size);
                        argVal.buf = bufPtr;
                    }
                }
            }
            // 普通类型
            else
            {
                const char* pBufPos = this->GetOneDataArgBuf(line_tail_1st, tail);
                if (nullptr != pBufPos) argVal.val = string(pBufPos, tail);
            }
            // 保存参数
            m_mapArg.insert(make_pair(sName, argVal));
        }
    }
}

// 获取参数二进制缓冲区
inline const char* KC::one_level::TInvarPost::GetOneDataArgBuf(const char* head, const char* tail)
{
    const char* pResult = DataSeparator::MemFind(head, tail, "\r\n\r\n", 4);
    return nullptr != pResult && pResult + 4 < tail ? pResult + 4 : nullptr;
}

// 得到名称
const char* KC::one_level::TInvarPost::GetName(void)
{
    return TInvarPost::GetNameS();
}
const char* KC::one_level::TInvarPost::GetNameS(void)
{
    static string sName = string("Innervar-") + g_InnerVarPost;
    return sName.c_str();
}

// 得到内部变量值
bool KC::one_level::TInvarPost::Exists(boost::any arr)
{
    auto it = m_mapArg.find(TInvarGet::GetArrIndex(arr));
    return m_mapArg.end() != it;
}

// 得到内部变量值
boost::any KC::one_level::TInvarPost::GetValue(boost::any arr)
{
    return TInvarGet::GetStringValue(m_mapArg, arr);
}

// 设置内部变量值
void KC::one_level::TInvarPost::SetValue(boost::any arr, boost::any val)
{
    string sArr = TInvarGet::GetArrIndex(arr);
    // 获取参数
    auto itVal = m_mapArg.find(sArr);
    // 保存文件
    if (m_mapArg.end() != itVal && ArgValueT::vteFile == itVal->second.val_type && itVal->second.buf_size > 0)
    {
        if (val.type() != typeid(string))
            throw TInnerVarException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_InnerVar.getHint("The_wrong_file_path_type_by_inner_variable_post")) % val.type().name() % sArr % m_act.GetCurrLineID()).str(), m_InnerVar);
        string sUrlName = boost::any_cast<string>(val);
        string sFilename = m_act.GetFullPath(sUrlName);
        // 创建目录
        filesystem::path pathFile(sFilename);
        if (!filesystem::exists(pathFile.branch_path()))
            filesystem::create_directories(pathFile.branch_path());
        // 写入文件
        ofstream outfile(sFilename, ios::binary | ios::trunc);
        CAutoRelease _auto(boost::bind(&ofstream::close, &outfile));
        outfile.write(itVal->second.buf.get(), itVal->second.buf_size);
        itVal->second.val = sUrlName;
    }
}
