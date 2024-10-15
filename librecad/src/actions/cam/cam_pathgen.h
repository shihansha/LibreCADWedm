#ifndef CAM_PATHGEN_H
#define CAM_PATHGEN_H

#include "rs_entity.h"
#include "cam_segment.h"
#include "cam_utils2d.h"

class CAM_PathGen
{
public:
    CAM_PathGen();

    QList<CAM_Segment> readAllSegments(QList<RS_Entity *> entities);
    bool searchPath(const QList<CAM_Segment> &segments, RS_Vector startPt, RS_Vector dirPt, RS_Vector endPt, QList<CAM_Segment> &result);
private:
    CAM_Utils2D utils2d;
};

#endif // CAM_PATHGEN_H
