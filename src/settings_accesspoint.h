#pragma once
#include "openchessboard.h"

extern void AP_setup(WiFiServer &APserver);
extern bool handleAPClientRequest(WiFiClient &client);
extern void run_APsettings(void);