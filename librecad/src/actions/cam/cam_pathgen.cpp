#include "cam_pathgen.h"

#include "rs_line.h"
#include "rs_circle.h"
#include "rs_arc.h"

CAM_PathGen::CAM_PathGen()
{

}

QList<CAM_Segment> CAM_PathGen::readAllSegments(QList<RS_Entity *> entities)
{
    QList<CAM_Segment> res;
    for (auto *e: entities) {
        RS2::EntityType eType = e->rtti();
        if (eType == RS2::EntityLine) {
            RS_Vector startPt = e->getStartpoint();
            RS_Vector endPt = e->getEndpoint();
            CAM_Segment seg(startPt.x, startPt.y, endPt.x, endPt.y, 0);
            res.append(seg);
        }
        else if (eType == RS2::EntityArc) {
            RS_Arc *eArc = (RS_Arc *)e;
            RS_Vector startPt = e->getStartpoint();
            RS_Vector endPt = e->getEndpoint();
            double bulge = eArc->getBulge();
            CAM_Segment seg(startPt.x, startPt.y, endPt.x, endPt.y, bulge);
            res.append(seg);
        }
        else if (eType == RS2::EntityCircle) {
            RS_Circle *eCir = (RS_Circle *)e;
            double radius = eCir->getRadius();
            RS_Vector center = eCir->getCenter();
            CAM_Segment seg0(center.x + radius, center.y, center.x - radius, center.y, 1);
            CAM_Segment seg1(center.x - radius, center.y, center.x + radius, center.y, 1);
            res.append(seg0);
            res.append(seg1);
        }
    }
    return res;
}

bool CAM_PathGen::searchPath(const QList<CAM_Segment> &segments, RS_Vector startPt, RS_Vector dirPt, RS_Vector endPt, QList<CAM_Segment> &result)
{
    QList<CAM_Segment> startDev = utils2d.devideSegments(startPt, segments);
    QList<CAM_Segment> allDev = utils2d.devideSegments(endPt, startDev);

    RS_Vector dir = dirPt - startPt;
    RS_Vector normDir = dir.normalized();

    int segIndex;
    RS_Vector nextPt;
    segIndex = utils2d.segmentsGetNextBorderPoint(allDev, startPt, normDir, nextPt);

    if (segIndex == -1) {
        return false;
    }

    CAM_Segment startSeg = allDev[segIndex];
    RS_Vector startSegPt0 = startSeg.getStartPt();

    if (utils2d.pointDistance(startSegPt0, startPt) >= utils2d.EPSILON) {
        CAM_Segment newSeg(startSeg.getEndPt(), startSeg.getStartPt(), -startSeg.getBulge());
        startSeg = newSeg;
    }

    if (utils2d.pointDistance(nextPt, endPt) < utils2d.EPSILON) {
        result.append(startSeg);
        return true;
    }

    QList<bool> ignoreMask;
    for (int i = 0; i < allDev.size(); i++) {
        ignoreMask.append(false);
    }
    ignoreMask[segIndex] = true;

    QList<CAM_Segment> path;
    if (!utils2d.segmentsFindPath(allDev, nextPt, endPt, ignoreMask, path)) {
        return false;
    }

    result.append(startSeg);
    result.append(path);

    return true;
}


