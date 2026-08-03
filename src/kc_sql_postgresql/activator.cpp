#include "std.h"
#include "ctrl_postgresql.h"

typedef TActivator<TKCCtrlWorkCommon<IKCSqlPgSQL, CCtrlPgSQL>> CActivator;
KC_SET_ACTIVATOR(CActivator)
