#include "std.h"
#include "ctrl_oracle.h"

typedef TActivator<TKCCtrlWorkCommon<IKCSqlOracle, CCtrlOracle>> CActivator;
KC_SET_ACTIVATOR(CActivator)
