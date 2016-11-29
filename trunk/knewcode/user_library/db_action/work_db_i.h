#pragma once

namespace KC
{
    // 数据集
    typedef boost::shared_ptr<session> TKCDBSessionPtr;
    typedef rowset_iterator<row> TKCDBRowPos;
    typedef rowset<row> TKCDBSet;
    typedef boost::shared_ptr<TKCDBSet> TKCDBSetPtr;
}
