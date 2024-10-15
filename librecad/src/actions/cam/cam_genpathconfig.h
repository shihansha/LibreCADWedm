#ifndef CAM_GENPATHCONFIG_H
#define CAM_GENPATHCONFIG_H

struct CAM_GenPathConfig {
    enum CutMethod : int {
        CutMethodDirect, // 直线
        CutMethodPerp, // 垂直
        CutMethodManual, // 指定切入点
    };
    double remainHeight;
    double remainWidth;
    double compData[4];
    int elecData[4];
    int cutTime;
    CutMethod cutMethod;
    bool tapeEnabled;
    double tapeAngle;
    bool paramUseMacro;
    bool useAbsCommand;
};

#endif // CAM_GENPATHCONFIG_H
