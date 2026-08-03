#include "std.h"
#include "ctrl_sqlite.h"

typedef TActivator<TKCCtrlWorkCommon<IKCSqlite, CCtrlSqlite>> CActivator;
KC_SET_ACTIVATOR(CActivator)
