#ifndef CAM_CUTDATA_H
#define CAM_CUTDATA_H

#include "cam_pathdata.h"
#include "cam_genpathconfig.h"
#include "rs_graphicview.h"
#include "rs_insert.h"
#include "cam_utils2d.h"

#include "cam_cutdatabase.h"

class CAM_CutData : public CAM_CutDataBase
{
public:
    constexpr static char BLOCK_NAME_PREFIX[] = "TsinghuaCutGroup";

    CAM_CutData(RS_EntityContainer* container, RS_GraphicView* graphicView);
    CAM_CutData(CAM_PathData pathData, CAM_GenPathConfig genPathConfig, RS_EntityContainer* container, RS_GraphicView* graphicView);

    RS_Insert *createBlockObject();
    bool loadFromBlockObject(const RS_Insert *block);
    bool loadFromBlockObject(const RS_Block *block);

    bool isInitialized() { return initialized; }
    const CAM_PathData &getPathData() const { return pathData; }
    const CAM_GenPathConfig &getGenPathConfig() const { return genPathConfig; }
private:
    bool initialized;
    CAM_PathData pathData;
    CAM_GenPathConfig genPathConfig;
    RS_EntityContainer* container;
    RS_GraphicView* graphicView;
    CAM_Utils2D utils2d;

    QJsonObject serializeSegment(const CAM_Segment& seg);
    QJsonObject serializeVector(const RS_Vector& vec);
    QString serialize();
    CAM_Segment deserializeSegment(QJsonObject obj);
    RS_Vector deserializeVector(QJsonObject obj);
    bool deserialize(QString data);
};

#endif // CAM_CUTDATA_H
