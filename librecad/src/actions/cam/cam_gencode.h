#ifndef CAM_GENCODE_H
#define CAM_GENCODE_H

#include "QList"

#include "cam_cutdatabase.h"
#include "cam_cutdata.h"
#include "cam_diffcutdata.h"

#include "rs_vector.h"
#include "cam_utils2d.h"

class CAM_GenCode
{
public:
    CAM_GenCode();

    QString generateGCode(QList<CAM_CutDataBase *> dataArr, QList<RS_Vector> startPts);
    QString generateOneCutCode(const CAM_CutData &cutdata);
    QString generateDiffCutCode(const CAM_DiffCutData &diffCutData);
private:
    CAM_Utils2D utils2d;

    double round(double num);
    QString toFixed(double num);

    QStringList genOneCutCodeAbs(const CAM_CutData &cutdata);
    QStringList genOneCutCodeRelMainPart(const CAM_CutData &cutdata, int startSubIndex);
    QStringList genOneCutCodeRelSubPart(const CAM_CutData &cutdata, int startSubIndex);
    QStringList genDiffCutCodeAbs(const CAM_DiffCutData &diffCutData);
    QStringList genDiffCutCodeRelMainPart(const CAM_DiffCutData &diffCutData, int startSubIndex);
    QStringList genDiffCutCodeRelSubPart(const CAM_DiffCutData &diffCutData, int startSubIndex);

    QList<CAM_CutDataBase *> prehandleData(QList<CAM_CutDataBase *> &dataArr, const QList<RS_Vector> &startPts, QList<RS_Vector> &outStartPts);
    QList<CAM_CutDataBase *> prehandleOneCutData(CAM_CutData *cutData);

};

#endif // CAM_GENCODE_H
