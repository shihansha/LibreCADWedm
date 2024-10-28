#include "cam_cutdata.h"

#include "rs_line.h"
#include "rs_polyline.h"
#include "rs_creation.h"
#include "rs_block.h"
#include "rs_graphic.h"

#include "rs_selection.h"

#include "math.h"

#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"

CAM_CutData::CAM_CutData(RS_EntityContainer* container, RS_GraphicView* graphicView)
    : container(container), graphicView(graphicView)
{
    cutType = CAM_CutDataBase::CutType::CutData;

    initialized = false;
}

CAM_CutData::CAM_CutData(CAM_PathData pathData, CAM_GenPathConfig genPathConfig, RS_EntityContainer* container, RS_GraphicView* graphicView)
    : pathData(pathData), genPathConfig(genPathConfig), container(container), graphicView(graphicView)
{
    cutType = CAM_CutDataBase::CutType::CutData;

    initialized = true;
}

RS_Insert *CAM_CutData::createBlockObject()
{
    if (!initialized) {
        return nullptr;
    }

    RS_Color green(0, 0xff, 0);
    RS_Color yellow(0xff, 0xff, 0);
    RS_Color magenta(0xff, 0, 0xff);
    RS_Pen greenPen(green, RS2::Width07, RS2::SolidLine);
    RS_Pen yellowPen(yellow, RS2::Width07, RS2::SolidLine);
    RS_Pen magentaPen(magenta, RS2::Width07, RS2::SolidLine);

    RS_Pen linePen = pathData.compensateDir ? greenPen : yellowPen;

    RS_Line *enterLine = new RS_Line(container, pathData.threadPt, pathData.cutOffPt);
    RS_Line *startLine = new RS_Line(container, pathData.cutOffPt, pathData.startPt);
    RS_Line *exitLine = new RS_Line(container, pathData.endPt, pathData.exitPt);
    RS_Line *cutOffLine = nullptr;
    if (pathData.isClosed && !(genPathConfig.useOrgPathAsRemainWidth && pathData.closePathExists)) {
        cutOffLine = new RS_Line(container, pathData.exitPt, pathData.cutOffPt);
    }

    CAM_Segment firstSeg = pathData.mainPath.first();
    RS_Vector t = utils2d.segmentsGetBorderTangentVector(pathData.startPt, firstSeg);
    RS_Vector normTan = t.normalized();
    RS_Vector normOffset(0.25 * -normTan.y, 0.25 * normTan.x);
    if (!pathData.compensateDir) {
        normOffset = -normOffset;
    }
    RS_Vector arrowStartPt = pathData.startPt + normOffset;
    RS_Vector arrowEndPt = arrowStartPt + normTan;
    RS_Line *arrowBody = new RS_Line(container, arrowStartPt, arrowEndPt);
    RS_Vector normTanPlus45(cos(M_PI_4) * normTan.x - sin(M_PI_4) * normTan.y, sin(M_PI_4) * normTan.x + cos(M_PI_4) * normTan.y);
    RS_Vector normTanMinus45(cos(-M_PI_4) * normTan.x - sin(-M_PI_4) * normTan.y, sin(-M_PI_4) * normTan.x + cos(-M_PI_4) * normTan.y);
    RS_Vector arrowDownPt = arrowEndPt - normTanPlus45;
    RS_Vector arrowUpPt = arrowEndPt - normTanMinus45;
    RS_Line *arrowDown = new RS_Line(container, arrowDownPt, arrowEndPt);
    RS_Line *arrowUp = new RS_Line(container, arrowUpPt, arrowEndPt);
    RS_Polyline *mainPathPolyline = new RS_Polyline(container);
    for (const CAM_Segment &seg: pathData.mainPath) {
        mainPathPolyline->addVertex(seg.getStartPt(), seg.getBulge());
    }
    mainPathPolyline->addVertex(pathData.mainPath.last().getEndPt());

    RS_Polyline *cutOffPolyline = nullptr;
    if (genPathConfig.useOrgPathAsRemainWidth && pathData.closePathExists) {
        cutOffPolyline = new RS_Polyline(container);
        for (const CAM_Segment &seg: pathData.closePath) {
            cutOffPolyline->addVertex(seg.getStartPt(), seg.getBulge());
        }
        cutOffPolyline->addVertex(pathData.closePath.last().getEndPt());
    }

    enterLine->setPen(linePen);
    startLine->setPen(linePen);
    exitLine->setPen(linePen);
    if (cutOffLine != nullptr) {
        cutOffLine->setPen(linePen);
    }
    arrowBody->setPen(magentaPen);
    arrowDown->setPen(magentaPen);
    arrowUp->setPen(magentaPen);
    mainPathPolyline->setPen(linePen);
    if (cutOffPolyline != nullptr) {
        cutOffPolyline->setPen(linePen);
    }

    container->addEntity(enterLine);
    container->addEntity(startLine);
    container->addEntity(exitLine);
    if (cutOffLine != nullptr) {
        container->addEntity(cutOffLine);
    }
    container->addEntity(arrowBody);
    container->addEntity(arrowDown);
    container->addEntity(arrowUp);
    container->addEntity(mainPathPolyline);
    if (cutOffPolyline != nullptr) {
        container->addEntity(cutOffPolyline);
    }

    RS_Selection selection(*container, graphicView);
    selection.selectSingle(enterLine);
    selection.selectSingle(startLine);
    selection.selectSingle(exitLine);
    if (cutOffLine != nullptr) {
        selection.selectSingle(cutOffLine);
    }
    selection.selectSingle(arrowBody);
    selection.selectSingle(arrowDown);
    selection.selectSingle(arrowUp);
    selection.selectSingle(mainPathPolyline);
    if (cutOffPolyline != nullptr) {
        selection.selectSingle(cutOffPolyline);
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

    RS_Creation creation(container, graphicView);
    RS_BlockData d(blockName, RS_Vector(false), false);
    RS_Block *block = creation.createBlock(&d, pathData.threadPt, true);

    RS_InsertData id(
        d.name,
        pathData.threadPt,
        RS_Vector(1.0, 1.0),
        0.0,
        1, 1, RS_Vector(0.0, 0.0)
        );
    RS_Insert *insert = creation.createInsert(&id);

    QString data = serialize();
    block->setMainString(data);

    return insert;
}

bool CAM_CutData::loadFromBlockObject(const RS_Insert *block)
{
    if (!block->getName().startsWith(BLOCK_NAME_PREFIX)) {
        return false;
    }
    RS_Block *blockData = container->getGraphic()->findBlock(block->getName());
    if (blockData == nullptr) {
        return false;
    }

    return loadFromBlockObject(blockData);
}

bool CAM_CutData::loadFromBlockObject(const RS_Block *block)
{
    QString data = block->getMainString();
    if (!deserialize(data)) {
        return false;
    }
    initialized = true;
    return true;
}

QJsonObject CAM_CutData::serializeSegment(const CAM_Segment &seg)
{
    QJsonObject obj;
    obj["x0"] = seg.getStartX();
    obj["y0"] = seg.getStartY();
    obj["x1"] = seg.getEndX();
    obj["y1"] = seg.getEndY();
    obj["b"] = seg.getBulge();
    return obj;
}

QJsonObject CAM_CutData::serializeVector(const RS_Vector &vec)
{
    QJsonObject obj;
    obj["x"] = vec.x;
    obj["y"] = vec.y;
    return obj;
}

QString CAM_CutData::serialize()
{
    QJsonObject obj;
    obj["cutTime"] = genPathConfig.cutTime;
    obj["threadPt"] = serializeVector(pathData.threadPt);
    obj["startPt"] = serializeVector(pathData.startPt);
    obj["endPt"] = serializeVector(pathData.endPt);
    obj["exitPt"] = serializeVector(pathData.exitPt);
    QJsonArray mainPathArray;
    for (const CAM_Segment &seg: pathData.mainPath) {
        mainPathArray.append(serializeSegment(seg));
    }
    obj["mainPath"] = mainPathArray;
    obj["remainHeight"] = genPathConfig.remainHeight;
    obj["remainWidth"] = genPathConfig.remainWidth;
    obj["cutOffPt"] = serializeVector(pathData.cutOffPt);
    obj["isClosed"] = pathData.isClosed;
    obj["tapeEnabled"] = genPathConfig.tapeEnabled;
    obj["tapeAngle"] = genPathConfig.tapeAngle;
    QJsonArray elecDataArray;
    for (size_t i = 0; i < sizeof(genPathConfig.elecData) / sizeof(genPathConfig.elecData[0]); i++) {
        elecDataArray.append(genPathConfig.elecData[i]);
    }
    obj["elecData"] = elecDataArray;
    QJsonArray compDataArray;
    for (size_t i = 0; i < sizeof(genPathConfig.compData) / sizeof(genPathConfig.compData[0]); i++) {
        compDataArray.append(genPathConfig.compData[i]);
    }
    obj["compData"] = compDataArray;
    obj["compensateDir"] = pathData.compensateDir;
    obj["paramUseMacro"] = genPathConfig.paramUseMacro;
    obj["useAbsCommand"] = genPathConfig.useAbsCommand;
    obj["useOrgPathAsRemainWidth"] = genPathConfig.useOrgPathAsRemainWidth;
    obj["closePathExists"] = pathData.closePathExists;
    if (genPathConfig.useOrgPathAsRemainWidth && pathData.closePathExists) {
        QJsonArray closePathArray;
        for (const CAM_Segment &seg: pathData.closePath) {
            closePathArray.append(serializeSegment(seg));
        }
        obj["closePath"] = closePathArray;
    }

    QString str = QString(QJsonDocument(obj).toJson(QJsonDocument::JsonFormat::Compact));
    return str;
}

CAM_Segment CAM_CutData::deserializeSegment(QJsonObject obj)
{
    return CAM_Segment(
        obj["x0"].toDouble(),
        obj["y0"].toDouble(),
        obj["x1"].toDouble(),
        obj["y1"].toDouble(),
        obj["b"].toDouble()
    );
}

RS_Vector CAM_CutData::deserializeVector(QJsonObject obj)
{
    return RS_Vector(obj["x"].toDouble(), obj["y"].toDouble());
}

bool CAM_CutData::deserialize(QString data) {
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject obj = doc.object();
    genPathConfig.cutTime = obj["cutTime"].toDouble();
    pathData.threadPt = deserializeVector(obj["threadPt"].toObject());
    pathData.startPt = deserializeVector(obj["startPt"].toObject());
    pathData.endPt = deserializeVector(obj["endPt"].toObject());
    pathData.exitPt = deserializeVector(obj["exitPt"].toObject());
    QJsonArray mainPathArray = obj["mainPath"].toArray();
    for (const QJsonValue &val: mainPathArray) {
        pathData.mainPath.append(deserializeSegment(val.toObject()));
    }
    genPathConfig.remainHeight = obj["remainHeight"].toDouble();
    genPathConfig.remainWidth = obj["remainWidth"].toDouble();
    pathData.cutOffPt = deserializeVector(obj["cutOffPt"].toObject());
    pathData.isClosed = obj["isClosed"].toBool();
    genPathConfig.tapeEnabled = obj["tapeEnabled"].toBool();
    genPathConfig.tapeAngle = obj["tapeAngle"].toDouble();
    QJsonArray elecDataArray = obj["elecData"].toArray();
    for (size_t i = 0; i < sizeof(genPathConfig.elecData) / sizeof(genPathConfig.elecData[0]); i++) {
        genPathConfig.elecData[i] = elecDataArray[i].toInt();
    }
    QJsonArray compDataArray = obj["compData"].toArray();
    for (size_t i = 0; i < sizeof(genPathConfig.compData) / sizeof(genPathConfig.compData[0]); i++) {
        genPathConfig.compData[i] = compDataArray[i].toDouble();
    }
    pathData.compensateDir = obj["compensateDir"].toBool();
    genPathConfig.paramUseMacro = obj["paramUseMacro"].toBool();
    genPathConfig.useAbsCommand = obj["useAbsCommand"].toBool();
    genPathConfig.useOrgPathAsRemainWidth = obj["useOrgPathAsRemainWidth"].toBool();
    pathData.closePathExists = obj["closePathExists"].toBool();
    if (genPathConfig.useOrgPathAsRemainWidth && pathData.closePathExists) {
        QJsonArray closePathArray = obj["closePath"].toArray();
        for (const QJsonValue &val: closePathArray) {
            pathData.closePath.append(deserializeSegment(val.toObject()));
        }
    }
    return true;
}

















