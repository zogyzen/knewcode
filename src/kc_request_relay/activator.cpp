#include "std.h"
#include "ctrl_request.h"

typedef TActivator<TKCCtrlWorkCommon<IKCRelayREST, CCtrlRelay>> CActivator;
KC_SET_ACTIVATOR(CActivator)
