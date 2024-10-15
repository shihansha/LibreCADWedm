#include "cam_diffcutdata.h"
#include "rs_graphic.h"
#include "rs_creation.h"
#include "rs_block.h"
#include "rs_selection.h"
#include "QJsonDocument"
#include "QJsonObject"

CAM_DiffCutData::CAM_DiffCutData(RS_EntityContainer *container, RS_GraphicView *graphicView)
    : container(container), graphicView(graphicView), mainPathCutData(container, graphicView), subPathCutData(container, graphicView)
{
    cutType = CAM_CutDataBase::CutType::DiffCutData;
    initialized = false;
}

CAM_DiffCutData::CAM_DiffCutData(CAM_CutData mainPathCutData, CAM_CutData subPathCutData, RS_EntityContainer *container, RS_GraphicView *graphicView)
    : container(container), graphicView(graphicView), mainPathCutData(mainPathCutData), subPathCutData(subPathCutData)
{
    cutType = CAM_CutDataBase::CutType::DiffCutData;

    if (!mainPathCutData.isInitialized() || !subPathCutData.isInitialized()) {
        initialized = false;
        return;
    }
    CAM_GenPathConfig mainPathConfig = mainPathCutData.getGenPathConfig();
    CAM_GenPathConfig subPathConfig = subPathCutData.getGenPathConfig();
    CAM_PathData mainPathData = mainPathCutData.getPathData();
    CAM_PathData subPathData = subPathCutData.getPathData();

    RS_Vector mainPathThreadPt = mainPathData.threadPt;
    RS_Vector subPathThreadPt = subPathData.threadPt;
    RS_Vector mainSubOffset = mainPathThreadPt - subPathThreadPt;

    subPathData.threadPt += mainSubOffset;
    subPathData.startPt += mainSubOffset;
    subPathData.endPt += mainSubOffset;
    subPathData.exitPt += mainSubOffset;
    subPathData.cutOffPt += mainSubOffset;
    for (CAM_Segment &seg : subPathData.mainPath) {
        seg[0] += mainSubOffset.x;
        seg[1] += mainSubOffset.y;
        seg[2] += mainSubOffset.x;
        seg[3] += mainSubOffset.y;
    }

    QList<CAM_Segment> diffJoined = utils2d.segmentsDiffJoin(mainPathData.mainPath, subPathData.mainPath);
    QList<CAM_Segment> diffMain;
    QList<CAM_Segment> diffSub;
    for (int i = 0; i < diffJoined.size(); i++) {
        if (i % 2 == 0) {
            diffMain.push_back(diffJoined[i]);
        }
        else {
            diffSub.push_back(diffJoined[i]);
        }
    }
    mainPathData.mainPath = diffMain;
    subPathData.mainPath = diffSub;

    this->mainPathCutData = CAM_CutData(mainPathData, mainPathConfig, container, graphicView);
    this->subPathCutData = CAM_CutData(subPathData, subPathConfig, container, graphicView);

    initialized = true;
}

RS_Insert *CAM_DiffCutData::createBlockObject()
{
    if (!initialized) {
        return nullptr;
    }

    QString blockName;
    int counter = 0;
    do {
        blockName = BLOCK_NAME_PREFIX + QString::number(counter);
        if (container->getGraphic()->findBlock(blockName) == nullptr) {
            break;
        }
        counter++;
    } while (true);

    RS_Insert *mainPathInsert = mainPathCutData.createBlockObject();
    RS_Insert *subPathInsert = subPathCutData.createBlockObject();

    RS_Selection selection(*container, graphicView);
    selection.selectSingle(mainPathInsert);
    selection.selectSingle(subPathInsert);

    RS_Vector threadPt = mainPathCutData.getPathData().startPt;
    RS_Creation creation(container, graphicView);
    RS_BlockData d(blockName, RS_Vector(false), false);
    RS_Block *block = creation.createBlock(&d, threadPt, true);

    RS_InsertData id(
        d.name,
        threadPt,
        RS_Vector(1.0, 1.0),
        0.0,
        1, 1, RS_Vector(0.0, 0.0)
        );
    RS_Insert *insert = creation.createInsert(&id);

    mainPathName = mainPathInsert->getName();
    subPathName = subPathInsert->getName();

    QString data = serialize();
    block->setMainString(data);

    return insert;
}

bool CAM_DiffCutData::loadFromBlockObject(const RS_Insert *block)
{
    if (!block->getName().startsWith(BLOCK_NAME_PREFIX)) {
        return false;
    }
    RS_Block *blockData = container->getGraphic()->findBlock(block->getName());
    if (blockData == nullptr) {
        return false;
    }
    QString data = blockData->getMainString();
    if (!deserialize(data)) {
        return false;
    }

    RS_Block *mainPathBlock = container->getGraphic()->findBlock(mainPathName);
    RS_Block *subPathBlock = container->getGraphic()->findBlock(subPathName);

    if (mainPathBlock == nullptr || subPathBlock == nullptr) {
        return false;
    }

    bool mainPathLoadResult = mainPathCutData.loadFromBlockObject(mainPathBlock);
    bool subPathLoadResult = subPathCutData.loadFromBlockObject(subPathBlock);
    if (!mainPathLoadResult || !subPathLoadResult) {
        return false;
    }

    initialized = true;
    return true;
}

QString CAM_DiffCutData::serialize()
{
    QJsonObject obj;
    obj.insert("mainPathName", mainPathName);
    obj.insert("subPathName", subPathName);
    return QString(QJsonDocument(obj).toJson(QJsonDocument::JsonFormat::Compact));
}

bool CAM_DiffCutData::deserialize(QString data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject obj = doc.object();
    mainPathName = obj.value("mainPathName").toString();
    subPathName = obj.value("subPathName").toString();
    return true;
}














