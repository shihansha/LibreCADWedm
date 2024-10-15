#ifndef CAM_DIFFCUTDATA_H
#define CAM_DIFFCUTDATA_H

#include "cam_cutdatabase.h"
#include "cam_cutdata.h"

class CAM_DiffCutData : public CAM_CutDataBase
{
public:
    constexpr static char BLOCK_NAME_PREFIX[] = "TsinghuaDiffCutGroup";
    CAM_DiffCutData(RS_EntityContainer* container, RS_GraphicView* graphicView);
    CAM_DiffCutData(CAM_CutData mainPathCutData, CAM_CutData subPathCutData, RS_EntityContainer* container, RS_GraphicView* graphicView);

    bool isInitialized() { return initialized; }
    RS_Insert *createBlockObject();
    bool loadFromBlockObject(const RS_Insert *block);
    const CAM_CutData &getMainPathCutData() const { return mainPathCutData; }
    const CAM_CutData &getSubPathCutData() const { return subPathCutData; }

private:
    bool initialized;
    RS_EntityContainer* container;
    RS_GraphicView* graphicView;
    CAM_CutData mainPathCutData;
    CAM_CutData subPathCutData;
    CAM_Utils2D utils2d;
    QString mainPathName;
    QString subPathName;

    QString serialize();
    bool deserialize(QString data);
};

#endif // CAM_DIFFCUTDATA_H
