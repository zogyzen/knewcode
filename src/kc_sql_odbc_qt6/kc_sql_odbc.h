#pragma once

#include "std.h"
#include "ctrl_odbc.h"

class CKCSqlODBC : public TKCCtrlWorkCommon<IKCSqlODBC, CCtrlODBC>
{
public:
    CKCSqlODBC(const IBundle& bundle);
};
