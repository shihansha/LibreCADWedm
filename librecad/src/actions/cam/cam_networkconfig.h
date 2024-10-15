#ifndef CAM_NETWORKCONFIG_H
#define CAM_NETWORKCONFIG_H

#include <qstring.h>

struct CAM_NetworkConfig {
    QString targetAddr;
    int targetPort;

    void loadFromSettings();
};

#endif // CAM_NETWORKCONFIG_H
