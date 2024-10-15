#include "cam_utils2d.h"

CAM_Utils2D::CAM_Utils2D() {}

double CAM_Utils2D::pointDistance(const RS_Vector &point0, const RS_Vector &point1)
{
    return point0.distanceTo(point1);
}

RS_Vector CAM_Utils2D::circleCenter(const CAM_Segment &segment)
{
    RS_Vector spt = segment.getStartPt();
    RS_Vector ept = segment.getEndPt();

    double b = segment.getBulge();

    double n = (1 / b - b) / 2;

    double x = (ept.x + spt.x) / 2 - n * (ept.y - spt.y) / 2;
    double y = (ept.y + spt.y) / 2 + n * (ept.x - spt.x) / 2;

    return RS_Vector(x, y);
}

/// @brief 计算点与广义线段的距离
/// @param pt 点
/// @param segment 广义线段
/// @return 距离
double CAM_Utils2D::pointSegmentDistance(const RS_Vector &pt, const CAM_Segment &segment)
{
    RS_Vector spt = segment.getStartPt();
    RS_Vector ept = segment.getEndPt();
    double bulge = segment.getBulge();

    if (bulge != 0) {
        RS_Vector cpt = circleCenter(segment);
        double r = pointDistance(cpt, spt);

        if (pointDistance(cpt, pt) < EPSILON) {
            return r;
        }

        double acp = vectorAngle(cpt, pt); // 选取点与圆心连线的角度
        double acs = vectorAngle(cpt, spt); // 起始点与圆心连线的角度
        double ace = vectorAngle(cpt, ept); // 终点与圆心连线的角度
        double es = ace - acs;

        if ((acp - acs) * (acp - ace) * es * bulge > 0) { // 不在弧上
            double a = pointDistance(pt, spt);
            double b = pointDistance(pt, ept);
            return a < b ? a : b;
        }
        else {
            return abs(abs(pointDistance(pt, cpt)) - r);
        }
    }
    else {
        double a = pointDistance(pt, spt);
        double b = pointDistance(pt, ept);
        double c = pointDistance(spt, ept);
        double p = (a + b + c) / 2;
        double s = sqrt(p * (p - a) * (p - b) * (p - c));

        if (c < EPSILON) {
            return a < b ? a : b;
        }

        double disFoot = 2 * s / c; // 点到垂足的距离

        double disStart = sqrt(abs(a * a - disFoot * disFoot)); // 点到起始点的距离
        double disEnd = sqrt(abs(b * b - disFoot * disFoot)); // 点到终点的距离

        if (disStart > c || disEnd > c) {
            return a < b ? a : b;
        }
        else {
            return disFoot;
        }
    }
}

/// @brief 计算两个向量的夹角
/// @param startPt 向量起始点
/// @param endPt 向量终点
/// @return 夹角 [0-2PI)
double CAM_Utils2D::vectorAngle(const RS_Vector &startPt, const RS_Vector &endPt)
{
    double x = endPt.x - startPt.x;
    double y = endPt.y - startPt.y;

    double angle = atan2(y, x);
    if (angle < 0) {
        angle += 2 * M_PI;
    }

    return angle;
}

/// @brief 计算广义线段上到平面上一个点距离最近的点
/// @param pt 平面上的点
/// @param segment 广义线段
/// @return 最近的点
RS_Vector CAM_Utils2D::segmentClosestPointToPoint(const RS_Vector &pt, const CAM_Segment &segment)
{
    RS_Vector spt = segment.getStartPt();
    RS_Vector ept = segment.getEndPt();
    double bulge = segment.getBulge();

    if (bulge != 0) {
        RS_Vector cpt = circleCenter(segment);
        double r = pointDistance(cpt, spt);

        if (pointDistance(cpt, pt) < EPSILON) {
            return spt;
        }

        double acp = vectorAngle(cpt, pt); // 选取点与圆心连线的角度
        double acs = vectorAngle(cpt, spt); // 起始点与圆心连线的角度
        double ace = vectorAngle(cpt, ept); // 终点与圆心连线的角度
        double es = ace - acs;

        if ((acp - acs) * (acp - ace) * es * bulge > 0) { // 不在弧上
            double a = pointDistance(pt, spt);
            double b = pointDistance(pt, ept);
            return a < b ? spt : ept;
        }
        else {
            // 计算垂足
            double dis = pointDistance(pt, cpt);
            RS_Vector res = cpt + (pt - cpt) * r / dis;
            return res;
        }
    }
    else {
        double a = pointDistance(pt, spt);
        double b = pointDistance(pt, ept);
        double c = pointDistance(spt, ept);
        double p = (a + b + c) / 2;
        double s = sqrt(p * (p - a) * (p - b) * (p - c));

        double disFoot = 2 * s / c; // 点到垂足的距禿
        double disStart;
        double disEnd;

        if (a * a - disFoot * disFoot > 0) {
            disStart = sqrt(a * a - disFoot * disFoot);
        }
        else {
            disStart = 0;
        }

        if (b * b - disFoot * disFoot > 0) {
            disEnd = sqrt(b * b - disFoot * disFoot);
        }
        else {
            disEnd = 0;
        }

        if (disStart > c || disEnd > c) {
            return a < b ? spt : ept;
        }
        else {
            return spt + (ept - spt) * disStart / c;
        }
    }
}

/// @brief 用起点终点圆心计算弧的凸度
/// @param spt 起点
/// @param ept 终点
/// @param cpt 圆心
/// @param isCCW 是否逆时针
/// @return 凸度
double CAM_Utils2D::circleBulge(const RS_Vector &spt, const RS_Vector &ept, const RS_Vector &cpt, bool isCCW)
{
    double startAngle = vectorAngle(cpt, spt);
    double endAngle = vectorAngle(cpt, ept);

    double angleDiff = endAngle - startAngle;
    if (angleDiff < 0) {
        angleDiff += 2 * M_PI;
    }

    if (!isCCW) {
        angleDiff = angleDiff - 2 * M_PI;
    }

    return tan(angleDiff / 4);
}

/// @brief 用线段上的点分割线段，不在线段上会产生未定义的结果
/// @param pt 分割点
/// @param segment 线段
/// @param firstAvailable 是否有第一段
/// @param lastAvailable 是否有第二段
/// @param firstSegment 第一段
/// @param lastSegment 第二段
void CAM_Utils2D::devideSegment(const RS_Vector &pt, const CAM_Segment &segment, bool &firstAvailable, bool &lastAvailable, CAM_Segment &firstSegment, CAM_Segment &lastSegment)
{
    firstAvailable = false;
    lastAvailable = false;

    RS_Vector spt = segment.getStartPt();
    RS_Vector ept = segment.getEndPt();
    double bulge = segment.getBulge();

    CAM_Segment first(spt, pt, 0);
    CAM_Segment last(pt, ept, 0);

    if (pointDistance(spt, pt) < EPSILON) {
        lastAvailable = true;
        lastSegment = segment;
        return;
    }
    else if (pointDistance(ept, pt) < EPSILON) {
        firstAvailable = true;
        firstSegment = segment;
        return;
    }
    else {
        firstAvailable = true;
        lastAvailable = true;
        if (bulge != 0) {
            RS_Vector cpt = circleCenter(segment);

            first.setBulge(circleBulge(spt, pt, cpt, bulge >= 0));
            last.setBulge(circleBulge(pt, ept, cpt, bulge >= 0));
        }
        firstSegment = first;
        lastSegment = last;
        return;
    }
}

QList<CAM_Segment> CAM_Utils2D::devideSegments(const RS_Vector &pt, const QList<CAM_Segment> &segments)
{
    QList<CAM_Segment> res;
    for (const CAM_Segment& segment: segments) {
        double dist = pointSegmentDistance(pt, segment);
        if (dist < EPSILON) {
            bool firstAvailable;
            bool lastAvailable;
            CAM_Segment first;
            CAM_Segment last;
            devideSegment(pt, segment, firstAvailable, lastAvailable, first, last);
            if (firstAvailable) {
                res.push_back(first);
            }
            if (lastAvailable) {
                res.push_back(last);
            }
        }
        else {
            res.push_back(segment);
        }
    }
    return res;
}

/// @brief 计算射线与线段的最近交点
/// @param pt 射线起点
/// @param ptOnRay 射线上的点
/// @param segment 线段
/// @param result 交点
/// @return 是否有交点
bool CAM_Utils2D::raySegmentIntersect(const RS_Vector &pt, const RS_Vector &ptOnRay, const CAM_Segment &segment, RS_Vector &result)
{
    double bulge = segment.getBulge();

    double dist = pointSegmentDistance(pt, segment);
    if (dist < EPSILON) {
        result = pt;
        return true;
    }

    CAM_Segment raySeg(pt, ptOnRay, 0);

    if (bulge == 0) {
        CAM_Utils2D::LineFunc line0 = lineFunc(raySeg);
        CAM_Utils2D::LineFunc line1 = lineFunc(segment);
        RS_Vector lineLinePt = lineLineIntersect(line0, line1);
        if (onRay(pt, ptOnRay, lineLinePt) && onSegment(segment, lineLinePt)) {
            result = lineLinePt;
            return true;
        }
        else {
            return false;
        }
    }
    else {
        CAM_Utils2D::LineFunc line0 = lineFunc(raySeg);
        CAM_Utils2D::ArcFunc arc1 = arcFunc(segment);
        CAM_Utils2D::TwoPts pts = lineArcIntersect(line0, arc1);
        double x0 = pts.x0;
        double y0 = pts.y0;
        double x1 = pts.x1;
        double y1 = pts.y1;
        if (isnan(x0)) {
            return false;
        }
        RS_Vector pt0(x0, y0);
        RS_Vector pt1(x1, y1);
        bool pt0Ok = false;
        bool pt1Ok = false;
        if (onRay(pt, ptOnRay, pt0) && onSegment(segment, pt0)) {
            pt0Ok = true;
        }
        if (onRay(pt, ptOnRay, pt1) && onSegment(segment, pt1)) {
            pt1Ok = true;
        }

        if (!pt0Ok && !pt1Ok) {
            return false;
        }
        if (!(pt0Ok && pt1Ok)) {
            result = pt0Ok ? pt0 : pt1;
            return true;
        }

        // both points are on the segment
        double d0 = (pt.x - pt0.x) * (pt.x - pt0.x) + (pt.y - pt0.y) * (pt.y - pt0.y);
        double d1 = (pt.x - pt1.x) * (pt.x - pt1.x) + (pt.y - pt1.y) * (pt.y - pt1.y);
        result = d0 < d1 ? pt0 : pt1;
        return true;
    }
}

QList<CAM_Segment> CAM_Utils2D::joinSegments(const QList<CAM_Segment> &segments0, const QList<CAM_Segment> &segments1)
{
    return segments0 + segments1;
}

/// @brief 计算射线与线段集合的最近交点
/// @param pt 射线起点
/// @param ptOnRay 射线上的点
/// @param segments 线段集合
/// @param result 交点
/// @return 是否有交点
bool CAM_Utils2D::raySegmentsIntersect(const RS_Vector &pt, const RS_Vector ptOnRay, const QList<CAM_Segment> &segments, RS_Vector &result)
{
    RS_Vector temp;
    double ds = MAX_LENGTH;

    for (const CAM_Segment& segment: segments) {
        if (raySegmentIntersect(pt, ptOnRay, segment, temp)) {
            double d = pointDistance(pt, temp);
            if (d < ds) {
                ds = d;
                result = temp;
            }
        }
    }

    if (ds < MAX_LENGTH) {
        return true;
    }
    else {
        return false;
    }
}

/// @brief 给定点和线段集合求最近顶点，注意不是线段上的点而是顶点
/// @param pt 给定点
/// @param segments 线段集合
/// @return 最近顶点
RS_Vector CAM_Utils2D::segmentsClosestVertexToPoint(const RS_Vector &pt, const QList<CAM_Segment> &segments)
{
    RS_Vector res;
    double ds = MAX_LENGTH;

    for (const CAM_Segment& segment: segments) {
        double d0 = pointDistance(pt, segment.getStartPt());
        double d1 = pointDistance(pt, segment.getEndPt());
        if (d0 < ds) {
            ds = d0;
            res = segment.getStartPt();
        }
        if (d1 < ds) {
            ds = d1;
            res = segment.getEndPt();
        }
    }

    return res;
}

CAM_Segment CAM_Utils2D::segmentsClosestSegmentToPoint(const RS_Vector &pt, const QList<CAM_Segment> &segments)
{
    CAM_Segment res;
    double ds = MAX_LENGTH;

    for (const CAM_Segment& segment: segments) {
        double d = pointSegmentDistance(pt, segment);
        if (d < ds) {
            ds = d;
            res = segment;
        }
    }

    return res;
}

/// @brief 寻找所有通过给定点的线段
/// @param pt 给定点
/// @param segments 线段集合
/// @param outSegIndexArr 通过点的线段索引
/// @return 通过点的线段
QList<CAM_Segment> CAM_Utils2D::segmentsAcrossPoint(const RS_Vector &pt, const QList<CAM_Segment> &segments, QList<int> &outSegIndexArr)
{
    QList<CAM_Segment> res;
    for (int i = 0; i < segments.size(); i++) {
        if (segments[i].getStartPt().distanceTo(pt) < EPSILON || segments[i].getEndPt().distanceTo(pt) < EPSILON) {
            res.push_back(segments[i]);
            outSegIndexArr.push_back(i);
        }
    }
    return res;
}

/// @brief 计算线段的切线向量
/// @param pt 给定点
/// @param segment 线段
/// @return 切线向量
RS_Vector CAM_Utils2D::segmentsGetTangentVector(const RS_Vector &pt, const CAM_Segment &segment)
{
    double bulge = segment.getBulge();
    if (bulge != 0) {
        RS_Vector cpt = circleCenter(segment);
        return RS_Vector(cpt.y - pt.y, pt.x - cpt.x).normalized();
    }
    else {
        return RS_Vector(segment.getStartPt() - segment.getEndPt()).normalized();
    }
}

/// @brief 获得线段边缘点指向线段内部的切向量方向
/// @param pt 给定点
/// @param segment 线段
/// @return 切向量
RS_Vector CAM_Utils2D::segmentsGetBorderTangentVector(const RS_Vector &pt, const CAM_Segment &segment)
{
    double bulge = segment.getBulge();
    if (bulge != 0) {
        RS_Vector segTan = segmentsGetTangentVector(pt, segment);
        RS_Vector invTan = RS_Vector(-segTan.x, -segTan.y);

        RS_Vector cpt = circleCenter(segment);
        RS_Vector cToPt = pt - cpt;
        double cross = cToPt.x * segTan.y - cToPt.y * segTan.x;

        RS_Vector startPt = segment.getStartPt();
        bool isStartPt = pointDistance(startPt, pt) < EPSILON;

        if (bulge > 0) {
            if (isStartPt) {
                if (cross > 0) {
                    return segTan;
                }
                else {
                    return invTan;
                }
            }
            else {
                if (cross > 0) {
                    return invTan;
                }
                else {
                    return segTan;
                }
            }
        }
        else {
            if (isStartPt) {
                if (cross > 0) {
                    return invTan;
                }
                else {
                    return segTan;
                }
            }
            else {
                if (cross > 0) {
                    return segTan;
                }
                else {
                    return invTan;
                }
            }
        }
    }
    else {
        RS_Vector segPt0 = segment.getStartPt();
        if (pointDistance(pt, segPt0) < EPSILON) {
            segPt0 = segment.getEndPt();
        }

        return (segPt0 - pt).normalized();
    }
}

/// @brief 寻找路径
/// @param segments 线段集合
/// @param startPt 起点
/// @param endPt 终点
/// @param ignoreMask 忽略掩码
/// @param result 结果
/// @return 是否找到路径
bool CAM_Utils2D::segmentsFindPath(const QList<CAM_Segment> &segments, const RS_Vector &startPt, const RS_Vector &endPt, QList<bool>& ignoreMask, QList<CAM_Segment> &result)
{
    QList<CAM_Segment> stack;

    int stackIndex = -1;

    CAM_Segment con;
    if (!ptFindConnectedSegment(segments, startPt, ignoreMask, con)) {
        return false;
    }

    stack.push_back(con);
    stackIndex++;

    do {
        RS_Vector cur = stack.last().getEndPt();
        if (pointDistance(cur, endPt) < EPSILON) {
            result = stack;
            return true;
        }

        if (!ptFindConnectedSegment(segments, cur, ignoreMask, con)) {
            stackIndex--;
            stack.pop_back();
        }
        else {
            stackIndex++;
            stack.push_back(con);
        }
    } while (stackIndex >= 0);

    return false;
}

int CAM_Utils2D::segmentsGetNextBorderPoint(const QList<CAM_Segment> &segments, const RS_Vector &startBorderPoint, const RS_Vector &normDir, RS_Vector &result)
{
    QList<int> crossSegsIndexArr;
    QList<CAM_Segment> crossedSegs = segmentsAcrossPoint(startBorderPoint, segments, crossSegsIndexArr);

    if (crossedSegs.size() == 0) {
        return -1;
    }

    QList<RS_Vector> crossedSegTan;

    for (int i = 0; i < crossedSegs.size(); i++) {
        crossedSegTan.push_back(segmentsGetBorderTangentVector(startBorderPoint, crossedSegs[i]));
    }

    CAM_Segment mostMatchedSeg;
    double mostMatchedDot;
    mostMatchedDot = -2;
    int outSegIndex = -1;

    for (int i = 0; i < crossedSegs.size(); i++) {
        double dotVal = normDir.x * crossedSegTan[i].x + normDir.y * crossedSegTan[i].y;
        if (dotVal > mostMatchedDot) {
            mostMatchedDot = dotVal;
            mostMatchedSeg = crossedSegs[i];
            outSegIndex = crossSegsIndexArr[i];
        }
    }

    RS_Vector resultPt = mostMatchedSeg.getStartPt();
    if (pointDistance(resultPt, startBorderPoint) < EPSILON) {
        resultPt = mostMatchedSeg.getEndPt();
    }
    result = resultPt;
    return outSegIndex;
}

QList<CAM_Segment> CAM_Utils2D::segmentsDiffJoin(const QList<CAM_Segment> &segs0, const QList<CAM_Segment> &segs1)
{
    int mainSegLen = segs0.size();
    int subSegLen = segs1.size();

    QList<double> mainLengthArr;
    QList<double> subLengthArr;
    double mainLengthSum = 0;
    double subLengthSum = 0;
    for (int i = 0; i < mainSegLen; i++) {
        double len = segmentLength(segs0[i]);
        mainLengthArr.push_back(len);
        mainLengthSum += len;
    }
    for (int i = 0; i < subSegLen; i++) {
        double len = segmentLength(segs1[i]);
        subLengthArr.push_back(len);
        subLengthSum += len;
    }

    QList<double> mainRateArr;
    QList<double> subRateArr;
    for (int i = 0; i < mainSegLen; i++) {
        if (mainLengthSum < EPSILON) {
            mainRateArr.push_back(0);
        }
        else {
            mainRateArr.push_back(mainLengthArr[i] / mainLengthSum);
        }
    }
    for (int i = 0; i < subSegLen; i++) {
        if (subLengthSum < EPSILON) {
            subRateArr.push_back(0);
        }
        else {
            subRateArr.push_back(subLengthArr[i] / subLengthSum);
        }
    }

    int mainCurIndex = 0;
    int subCurIndex = 0;
    double mainCurSum = 0;
    double subCurSum = 0;

    RS_Vector lastXY;
    RS_Vector lastUV;
    lastXY = segs0[0].getStartPt();
    lastUV = segs1[0].getStartPt();
    QList<CAM_Segment> result;

    while (mainCurIndex < mainSegLen || subCurIndex < subSegLen) {
        double mainNextSum;
        double subNextSum;
        if (mainCurIndex < mainSegLen) {
            mainNextSum = mainRateArr[mainCurIndex] + mainCurSum;
        }
        else {
            mainNextSum = 2;
        }
        if (subCurIndex < subSegLen) {
            subNextSum = subRateArr[subCurIndex] + subCurSum;
        }
        else {
            subNextSum = 2;
        }

        CAM_Segment nextMain;
        CAM_Segment nextSub;

        if (mainNextSum <= subNextSum) {
            // 推进main,拆分sub
            CAM_Segment mainCurSeg = segs0[mainCurIndex];

            double bulge;
            if (mainCurSeg.getBulge() == 0) {
                bulge = 0;
            }
            else {
                RS_Vector cpt = circleCenter(mainCurSeg);
                bulge = circleBulge(lastXY, mainCurSeg.getEndPt(), cpt, mainCurSeg.getBulge() >= 0);
            }
            nextMain = CAM_Segment(lastXY, mainCurSeg.getEndPt(), bulge);

            lastXY = mainCurSeg.getEndPt();

            if (subCurIndex >= subSegLen) {
                nextSub = CAM_Segment(lastUV, lastUV, 0);
            }
            else {
                CAM_Segment subSeg = segs1[subCurIndex];

                double takeRate;
                if (subRateArr[subCurIndex] > EPSILON / 100) {
                    takeRate = (mainNextSum - subCurSum) / subRateArr[subCurIndex];
                }
                else {
                    takeRate = 0;
                }

                double bulge;
                RS_Vector endUV;
                if (subSeg.getBulge() == 0) {
                    endUV = subSeg.getStartPt() * (1 - takeRate) + subSeg.getEndPt() * takeRate;

                    bulge = 0;
                }
                else {
                    RS_Vector subCirCenter = circleCenter(subSeg);
                    double subRad = atan(subSeg.getBulge()) * 4;
                    double subEndRad = subRad * takeRate;
                    RS_Vector subCs = subSeg.getStartPt() - subCirCenter;
                    double subCex = cos(subEndRad) * subCs.x - sin(subEndRad) * subCs.y;
                    double subCey = sin(subEndRad) * subCs.x + cos(subEndRad) * subCs.y;
                    RS_Vector subCe = RS_Vector(subCex, subCey);
                    endUV = subCirCenter + subCe;

                    bulge = circleBulge(lastUV, endUV, subCirCenter, subRad >= 0);
                }

                nextSub = CAM_Segment(lastUV, endUV, bulge);

                lastUV = endUV;
            }

            mainCurIndex++;
            mainCurSum = mainNextSum;
        }
        else {
            // 推进sub,拆分main
            CAM_Segment subCurSeg = segs1[subCurIndex];

            double bulge;
            if (subCurSeg.getBulge() == 0) {
                bulge = 0;
            }
            else {
                RS_Vector cpt = circleCenter(subCurSeg);
                bulge = circleBulge(lastUV, subCurSeg.getEndPt(), cpt, subCurSeg.getBulge() >= 0);
            }
            nextSub = CAM_Segment(lastUV, subCurSeg.getEndPt(), bulge);

            lastUV = subCurSeg.getEndPt();

            if (mainCurIndex >= mainSegLen) {
                nextMain = CAM_Segment(lastXY, lastXY, 0);
            }
            else {
                CAM_Segment mainSeg = segs0[mainCurIndex];

                double takeRate;
                if (mainRateArr[mainCurIndex] > EPSILON / 100) {
                    takeRate = (subNextSum - mainCurSum) / mainRateArr[mainCurIndex];
                }
                else {
                    takeRate = 0;
                }

                double bulge;
                RS_Vector endXY;
                if (mainSeg.getBulge() == 0) {
                    endXY = mainSeg.getStartPt() * (1 - takeRate) + mainSeg.getEndPt() * takeRate;

                    bulge = 0;
                }
                else {
                    RS_Vector mainCirCenter = circleCenter(mainSeg);
                    double mainRad = atan(mainSeg.getBulge()) * 4;
                    double mainEndRad = mainRad * takeRate;
                    RS_Vector mainCs = mainSeg.getStartPt() - mainCirCenter;
                    double mainCex = cos(mainEndRad) * mainCs.x - sin(mainEndRad) * mainCs.y;
                    double mainCey = sin(mainEndRad) * mainCs.x + cos(mainEndRad) * mainCs.y;
                    RS_Vector mainCe = RS_Vector(mainCex, mainCey);
                    endXY = mainCirCenter + mainCe;

                    bulge = circleBulge(lastXY, endXY, mainCirCenter, mainRad >= 0);
                }

                nextMain = CAM_Segment(lastXY, endXY, bulge);

                lastXY = endXY;
            }

            subCurIndex++;
            subCurSum = subNextSum;
        }

        result.push_back(nextMain);
        result.push_back(nextSub);
    }

    return result;
}

double CAM_Utils2D::segmentLength(const CAM_Segment &segment)
{
    if (segment.getBulge() == 0) {
        double xLen = segment.getStartX() - segment.getEndX();
        double yLen = segment.getStartY() - segment.getEndY();
        return sqrt(xLen * xLen + yLen * yLen);
    }
    else {
        double rad = atan(segment.getBulge()) * 4;
        RS_Vector c = circleCenter(segment);
        double r = pointDistance(c, segment.getStartPt());
        return abs(r * rad);
    }
}

bool CAM_Utils2D::ptFindConnectedSegment(const QList<CAM_Segment> &segments, const RS_Vector &startPt, QList<bool>& ignoreMask, CAM_Segment &result)
{
    for (int i = 0; i < segments.size(); i++) {
        if (ignoreMask[i]) {
            continue;
        }

        CAM_Segment s = segments[i];

        RS_Vector segPt0 = s.getStartPt();
        RS_Vector segPt1 = s.getEndPt();

        if (pointDistance(startPt, segPt0) < EPSILON) {
            ignoreMask[i] = true;
            result = s;
            return true;
        }
        else if (pointDistance(startPt, segPt1) < EPSILON) {
            ignoreMask[i] = true;
            result = CAM_Segment(segPt1, segPt0, -s.getBulge());
            return true;
        }
    }

    return false;
}

CAM_Utils2D::LineFunc CAM_Utils2D::lineFunc(const CAM_Segment &segment)
{
    double a = -(segment.getStartY() - segment.getEndY());
    double b = segment.getStartX() - segment.getEndX();
    double d = sqrt(a * a + b * b);
    a /= d;
    b /= d;
    double c = -a * segment.getStartX() - b * segment.getStartY();
    return {a, b, c};
}

CAM_Utils2D::ArcFunc CAM_Utils2D::arcFunc(const CAM_Segment &segment)
{
    double n = (1 / segment.getBulge() - segment.getBulge()) / 2;
    double cx = (segment.getStartX() + segment.getEndX()) / 2 - n * (segment.getEndY() - segment.getStartY()) / 2;
    double cy = (segment.getStartY() + segment.getEndY()) / 2 + n * (segment.getEndX() - segment.getStartX()) / 2;
    double rx = segment.getStartX() - cx;
    double ry = segment.getStartY() - cy;
    double r = sqrt(rx * rx + ry * ry);
    return {cx, cy, r};
}

RS_Vector CAM_Utils2D::lineLineIntersect(const LineFunc &line0, const LineFunc &line1)
{
    double m = 1 / (line0.a * line1.b - line1.a * line0.b);
    double x = -(line1.b * line0.c - line0.b * line1.c) * m;
    double y = -(line0.a * line1.c - line1.a * line0.c) * m;
    return RS_Vector(x, y);
}

CAM_Utils2D::TwoPts CAM_Utils2D::lineArcIntersect(const LineFunc &line, const ArcFunc &arc)
{
    double a = line.a;
    double b = line.b;
    double c = line.c;
    double d = arc.a;
    double e = arc.b;
    double f = arc.r;

    double yaa = a * a + b * b;
    double ybb = 2 * ((c + a * d) * b - a * a * e);
    double ycc = (a * d + c) * (a * d + c) - a * a * f * f + a * a * e * e;

    double ydelta = ybb * ybb - 4 * yaa * ycc;
    if (ydelta < 0 && ydelta > -0.0001 * 0.0001)
    {
        ydelta = 0;
    }
    double y0 = (-ybb + sqrt(ydelta)) / (2 * yaa);
    double y1 = (-ybb - sqrt(ydelta)) / (2 * yaa);

    double xaa = a * a + b * b;
    double xbb = 2 * ((c + b * e) * a - b * b * d);
    double xcc = (b * e + c) * (b * e + c) - b * b * f * f + b * b * d * d;
    double xdelta = xbb * xbb - 4 * xaa * xcc;
    if (xdelta < 0 && xdelta > -0.0001 * 0.0001)
    {
        xdelta = 0;
    }
    double x0 = (-xbb + sqrt(xdelta)) / (2 * xaa);
    double x1 = (-xbb - sqrt(xdelta)) / (2 * xaa);

    double vec0x = x0 - x1;
    double vec0y = y0 - y1;
    double vec1x = x1 - x0;
    double vec1y = y0 - y1;

    double dot0 = -line.b * vec0x + line.a * vec0y;
    double dot1 = -line.b * vec1x + line.a * vec1y;
    if (abs(dot0) > abs(dot1))
    {
        return {x0, y0, x1, y1};
    }
    else
    {
        return {x1, y0, x0, y1};
    }
}

bool CAM_Utils2D::onSegment(const CAM_Segment &s, const RS_Vector &pt)
{
    if (abs(s.getStartX() - pt.x) < EPSILON && abs(s.getStartY() - pt.y) < EPSILON)
    {
        return true;
    }
    if (abs(s.getEndX() - pt.x) < EPSILON && abs(s.getEndY() - pt.y) < EPSILON)
    {
        return true;
    }
    if (s.getBulge() != 0)
    {
        auto arc = arcFunc(s);
        double cx = arc.a;
        double cy = arc.b;
        double acp = atan2(pt.y - cy, pt.x - cx);
        double acs = atan2(s.getStartY() - cy, s.getStartX() - cx);
        double ace = atan2(s.getEndY() - cy, s.getEndX() - cx);

        if (acp < 0)
        {
            acp += M_PI * 2;
        }
        if (acs < 0)
        {
            acs += M_PI * 2;
        }
        if (ace < 0)
        {
            ace += M_PI * 2;
        }

        double es = ace - acs;

        return (acp - acs) * (acp - ace) * es * s.getBulge() <= 0;
    }
    else
    {
        double p2sx = s.getStartX() - pt.x;
        double p2sy = s.getStartY() - pt.y;
        double p2ex = s.getEndX() - pt.x;
        double p2ey = s.getEndY() - pt.y;

        return p2sx * p2ex + p2sy * p2ey <= 0;
    }

}

bool CAM_Utils2D::onRay(const RS_Vector &rayPt, const RS_Vector &ptOnRay, const RS_Vector &pt)
{
    if (abs(rayPt.x - pt.x) < EPSILON && abs(rayPt.y - pt.y) < EPSILON)
    {
        return true;
    }

    RS_Vector rayDir = ptOnRay - rayPt;
    RS_Vector ptDir = pt - rayPt;
    if (rayDir.x * ptDir.x + rayDir.y * ptDir.y < 0)
    {
        return false;
    }
    return true;
}






