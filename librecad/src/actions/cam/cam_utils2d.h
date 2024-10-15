#ifndef CAM_UTILS2D_H
#define CAM_UTILS2D_H

#include "math.h"

#include "rs_vector.h"
#include "cam_segment.h"

#include <QList>

class CAM_Utils2D
{
public:
    CAM_Utils2D();

    double EPSILON = 0.001;
    double MAX_LENGTH = 10000000;

    double pointDistance(const RS_Vector& point0, const RS_Vector& point1);
    RS_Vector circleCenter(const CAM_Segment& segment);
    double pointSegmentDistance(const RS_Vector& pt, const CAM_Segment& segment);
    double vectorAngle(const RS_Vector& startPt, const RS_Vector& endPt);
    RS_Vector segmentClosestPointToPoint(const RS_Vector& pt, const CAM_Segment& segment);
    double circleBulge(const RS_Vector& spt, const RS_Vector& ept, const RS_Vector& cpt, bool isCCW);
    void devideSegment(const RS_Vector& pt, const CAM_Segment& segment, bool& firstAvailable, bool& lastAvailable, CAM_Segment& firstSegment, CAM_Segment& lastSegment);
    QList<CAM_Segment> devideSegments(const RS_Vector& pt, const QList<CAM_Segment>& segments);
    bool raySegmentIntersect(const RS_Vector& pt, const RS_Vector& ptOnRay, const CAM_Segment& segment, RS_Vector& result);
    QList<CAM_Segment> joinSegments(const QList<CAM_Segment>& segments0, const QList<CAM_Segment>& segments1);
    bool raySegmentsIntersect(const RS_Vector& pt, const RS_Vector ptOnRay, const QList<CAM_Segment>& segments, RS_Vector& result);
    RS_Vector segmentsClosestVertexToPoint(const RS_Vector& pt, const QList<CAM_Segment>& segments);
    CAM_Segment segmentsClosestSegmentToPoint(const RS_Vector& pt, const QList<CAM_Segment>& segments);
    QList<CAM_Segment> segmentsAcrossPoint(const RS_Vector& pt, const QList<CAM_Segment>& segments, QList<int>& outSegIndexArr);
    RS_Vector segmentsGetTangentVector(const RS_Vector& pt, const CAM_Segment& segment);
    RS_Vector segmentsGetBorderTangentVector(const RS_Vector& pt, const CAM_Segment& segment);
    bool segmentsFindPath(const QList<CAM_Segment>& segments, const RS_Vector& startPt, const RS_Vector& endPt, QList<bool>& ignoreMask, QList<CAM_Segment>& result);
    int segmentsGetNextBorderPoint(const QList<CAM_Segment>& segments, const RS_Vector& startBorderPoint, const RS_Vector& normDir, RS_Vector& result);
    QList<CAM_Segment> segmentsDiffJoin(const QList<CAM_Segment>& segs0, const QList<CAM_Segment>& segs1);
    double segmentLength(const CAM_Segment& segment);

private:
    struct LineFunc {
        double a;
        double b;
        double c;
    };
    struct ArcFunc {
        double a;
        double b;
        double r;
    };
    struct TwoPts {
        double x0;
        double y0;
        double x1;
        double y1;
    };

    bool ptFindConnectedSegment(const QList<CAM_Segment>& segments, const RS_Vector& startPt, QList<bool>& ignoreMask, CAM_Segment& result);
    LineFunc lineFunc(const CAM_Segment& segment);
    ArcFunc arcFunc(const CAM_Segment& segment);
    RS_Vector lineLineIntersect(const LineFunc& line0, const LineFunc& line1);
    TwoPts lineArcIntersect(const LineFunc& line, const ArcFunc& arc);
    bool onSegment(const CAM_Segment& s, const RS_Vector& pt);
    bool onRay(const RS_Vector& rayPt, const RS_Vector& ptOnRay, const RS_Vector& pt);
};

#endif // CAM_UTILS2D_H
