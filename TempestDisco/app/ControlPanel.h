
#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "TempestIO/AbstractTempestIO.h"

void InitializeControlPanel(void);
void ServiceControlPanel(void);
bool GetButton(ButtonFlag button);

#endif
