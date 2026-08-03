#pragma once

extern int KC_HasLoad(void);
extern const char* KC_GetLastError(void);
extern const char* KC_LoadLib(const char* strLibFileName);
extern void KC_UnLoadLib(void);
extern void* KC_GetLibFunc(const char* strFuncName);

extern const char *c_LoadOutLibExtName;
