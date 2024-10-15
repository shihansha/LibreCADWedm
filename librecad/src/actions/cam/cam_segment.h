#ifndef CAM_SEGMENT_H
#define CAM_SEGMENT_H

#include "rs_vector.h"

struct CAM_Segment
{
public:
    CAM_Segment();
    CAM_Segment(double startX, double startY, double endX, double endY, double bulge = 0);
    CAM_Segment(RS_Vector start, RS_Vector end, double bulge = 0);

    const double &operator[](int index) const;
    double &operator[](int index);

    RS_Vector getStartPt() const;
    RS_Vector getEndPt() const;

    double getStartX() const;
    double getStartY() const;
    double getEndX() const;
    double getEndY() const;
    double getBulge() const;

    void setBulge(double value);

private:
    double data[5];
};

#endif // CAM_SEGMENT_H
