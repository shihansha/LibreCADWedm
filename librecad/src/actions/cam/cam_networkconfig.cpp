#include "cam_networkconfig.h"

#include "rs_settings.h"


void CAM_NetworkConfig::loadFromSettings()
{
    RS_SETTINGS->beginGroup("EdmNetwork");
    targetAddr = RS_SETTINGS->readEntry("TargetAddr", "localhost");
    targetPort = RS_SETTINGS->readNumEntry("TargetPort", 8080);
    RS_SETTINGS->endGroup();
}
