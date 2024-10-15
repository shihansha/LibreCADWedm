#include "cam_segment.h"

#include "math.h"

CAM_Segment::CAM_Segment() {}

CAM_Segment::CAM_Segment(double startX, double startY, double endX, double endY, double bulge)
{
    data[0] = startX;
    data[1] = startY;
    data[2] = endX;
    data[3] = endY;
    data[4] = bulge;
}

CAM_Segment::CAM_Segment(RS_Vector start, RS_Vector end, double bulge)
{
    data[0] = start.x;
    data[1] = start.y;
    data[2] = end.x;
    data[3] = end.y;
    data[4] = bulge;
}

const double &CAM_Segment::operator[](int index) const
{
    return data[index];
}

double &CAM_Segment::operator[](int index)
{
    return data[index];
}

RS_Vector CAM_Segment::getStartPt() const
{
    return RS_Vector(data[0], data[1]);
}

RS_Vector CAM_Segment::getEndPt() const
{
    return RS_Vector(data[2], data[3]);
}

double CAM_Segment::getStartX() const {
    return data[0];
}

double CAM_Segment::getStartY() const {
    return data[1];
}

double CAM_Segment::getEndX() const {
    return data[2];
}

double CAM_Segment::getEndY() const {
    return data[3];
}

double CAM_Segment::getBulge() const {
    return data[4];
}

void CAM_Segment::setBulge(double value)
{
    data[4] = value;
}


