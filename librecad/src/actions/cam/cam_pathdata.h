#ifndef CAM_PATHDATA_H
#define CAM_PATHDATA_H

#include "qlist.h"
#include "rs_vector.h"
#include "cam_segment.h"

struct CAM_PathData {
    bool isClosed = false;
    RS_Vector threadPt;
    RS_Vector startPt;
    RS_Vector endPt;
    RS_Vector exitPt;
    RS_Vector cutOffPt;
    QList<CAM_Segment> mainPath;
    bool compensateDir = false;
    bool closePathExists = false;
    QList<CAM_Segment> closePath;

};

#endif // CAM_PATHDATA_H
