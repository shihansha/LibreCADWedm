#include "rs_actioncamgenpath.h"
#include "rs_entity.h"
#include "rs_dialogfactory.h"
#include "rs_polyline.h"
#include "cam_cutdata.h"
#include "rs_dialogfactory.h"

#include "QMouseEvent"
#include "rs_preview.h"
#include "rs_line.h"
#include "rs_point.h"

struct RS_ActionCamGenPath::ContextData {
    RS_Vector threadPt;
    RS_Entity *entity;
    RS_Vector startPt;
    RS_Vector moveDir;
    RS_Vector compDir;
    RS_Vector endPt;
    RS_Vector exitPt;
    CAM_GenPathConfig genPathConfig;
    QList<RS_Vector> dirAngles;
    RS_Vector dirArrowEndPt;
    RS_Vector dirArrowUpPt;
    RS_Vector dirArrowDownPt;
    RS_Vector compArrowEndPt;
    RS_Vector compArrowUpPt;
    RS_Vector compArrowDownPt;
};

RS_ActionCamGenPath::RS_ActionCamGenPath(RS_EntityContainer &container, RS_GraphicView &graphicView)
    : RS_PreviewActionInterface("Cam Genpath", container, graphicView)
    , pContext(std::make_unique<ContextData>())
    , lastStatus(SelThreadPt)
    , highlightedEntity(nullptr)
{
    enTypeList = EntityTypeList{RS2::EntityLine, RS2::EntityArc, RS2::EntityCircle};
    setActionType(RS2::ActionEdmGenPath);
}

void RS_ActionCamGenPath::init(int status)
{
    RS_PreviewActionInterface::init(status);

    if (RS_DIALOGFACTORY->requestEdmGenPathDialog(pContext->genPathConfig)) {
        //commandMessage("ok clicked");
    }
    else {
        finish();
    }

    setSnapRestriction(RS2::RestrictNothing);

    if (status < 0) {
        deleteHighlighted();
    }
}

void RS_ActionCamGenPath::mouseMoveEvent(QMouseEvent *e)
{
    RS_Vector pt = snapPoint(e);
    //RS_Vector mouse = toGraph(e);
    RS_Entity *se;
    deleteHighlighted();
    deletePreview();

    switch (getStatus()) {
        case SelThreadPt: {

            break;
        }
        case SelEntity: {
            se = catchEntity(e, enTypeList, RS2::ResolveAll);
            if (se != nullptr) {
                RS_Vector closestPoint;
                if (pContext->genPathConfig.cutMethod == CAM_GenPathConfig::CutMethod::CutMethodManual) {
                    closestPoint = se->getNearestPointOnEntity(pt);
                }
                else if (pContext->genPathConfig.cutMethod == CAM_GenPathConfig::CutMethod::CutMethodDirect) {
                    closestPoint = se->getNearestEndpoint(pContext->threadPt);
                }
                else {
                    closestPoint = se->getNearestPointOnEntity(pContext->threadPt);
                }
                RS_Point *threadPt = new RS_Point(nullptr, pContext->threadPt);
                RS_Line *threadStart = new RS_Line(pContext->threadPt, closestPoint);
                preview->addEntity(threadPt);
                preview->addEntity(threadStart);
                se->setHighlighted(true);
                highlightedEntity = se;
            }
            drawPreview();
            break;
        }
        case SelMoveDir: {
            RS_Line *threadStart = new RS_Line(pContext->threadPt, pContext->startPt);
            preview->addEntity(threadStart);
            drawDirPreviewArrow(pContext->startPt, pt);
            drawPreview();
            break;
        }
        case SelCompDir: {
            RS_Line *threadStart = new RS_Line(pContext->threadPt, pContext->startPt);
            preview->addEntity(threadStart);
            if (pContext->dirArrowEndPt.valid) {
                RS_Line *dirArrow = new RS_Line(pContext->startPt, pContext->dirArrowEndPt);
                preview->addEntity(dirArrow);
                RS_Line *dirArrowUp = new RS_Line(pContext->dirArrowEndPt, pContext->dirArrowUpPt);
                preview->addEntity(dirArrowUp);
                RS_Line *dirArrowDown = new RS_Line(pContext->dirArrowEndPt, pContext->dirArrowDownPt);
                preview->addEntity(dirArrowDown);
            }
            else {
                RS_Line *moveDir = new RS_Line(pContext->startPt, pContext->moveDir);
                preview->addEntity(moveDir);
            }
            drawCompPreviewArrow(pContext->startPt, pt);
            drawPreview();
            break;
        }
        case SelEndPt: {
            RS_Line *threadStart = new RS_Line(pContext->threadPt, pContext->startPt);
            preview->addEntity(threadStart);
            if (pContext->dirArrowEndPt.valid) {
                RS_Line *dirArrow = new RS_Line(pContext->startPt, pContext->dirArrowEndPt);
                preview->addEntity(dirArrow);
                RS_Line *dirArrowUp = new RS_Line(pContext->dirArrowEndPt, pContext->dirArrowUpPt);
                preview->addEntity(dirArrowUp);
                RS_Line *dirArrowDown = new RS_Line(pContext->dirArrowEndPt, pContext->dirArrowDownPt);
                preview->addEntity(dirArrowDown);
            }
            else {
                RS_Line *moveDir = new RS_Line(pContext->startPt, pContext->moveDir);
                preview->addEntity(moveDir);
            }
            if (pContext->compArrowEndPt.valid) {
                RS_Line *compArrow = new RS_Line(pContext->startPt, pContext->compArrowEndPt);
                preview->addEntity(compArrow);
                RS_Line *compArrowUp = new RS_Line(pContext->compArrowEndPt, pContext->compArrowUpPt);
                preview->addEntity(compArrowUp);
                RS_Line *compArrowDown = new RS_Line(pContext->compArrowEndPt, pContext->compArrowDownPt);
                preview->addEntity(compArrowDown);
            }
            else {
                RS_Line *compDir = new RS_Line(pContext->startPt, pContext->compDir);
                preview->addEntity(compDir);
            }
            drawPreview();
            break;
        }
        case SelExitPt: {
            RS_Line *threadStart = new RS_Line(pContext->threadPt, pContext->startPt);
            preview->addEntity(threadStart);
            if (pContext->dirArrowEndPt.valid) {
                RS_Line *dirArrow = new RS_Line(pContext->startPt, pContext->dirArrowEndPt);
                preview->addEntity(dirArrow);
                RS_Line *dirArrowUp = new RS_Line(pContext->dirArrowEndPt, pContext->dirArrowUpPt);
                preview->addEntity(dirArrowUp);
                RS_Line *dirArrowDown = new RS_Line(pContext->dirArrowEndPt, pContext->dirArrowDownPt);
                preview->addEntity(dirArrowDown);
            }
            else {
                RS_Line *moveDir = new RS_Line(pContext->startPt, pContext->moveDir);
                preview->addEntity(moveDir);
            }
            if (pContext->compArrowEndPt.valid) {
                RS_Line *compArrow = new RS_Line(pContext->startPt, pContext->compArrowEndPt);
                preview->addEntity(compArrow);
                RS_Line *compArrowUp = new RS_Line(pContext->compArrowEndPt, pContext->compArrowUpPt);
                preview->addEntity(compArrowUp);
                RS_Line *compArrowDown = new RS_Line(pContext->compArrowEndPt, pContext->compArrowDownPt);
                preview->addEntity(compArrowDown);
            }
            else {
                RS_Line *compDir = new RS_Line(pContext->startPt, pContext->compDir);
                preview->addEntity(compDir);
            }
            RS_Line *endLine = new RS_Line(pContext->endPt, pt);
            preview->addEntity(endLine);
            drawPreview();
            break;
        }
    }
}

void RS_ActionCamGenPath::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        onMouseLeftButtonRelease(getStatus(), e);
    }
}

void RS_ActionCamGenPath::onMouseLeftButtonRelease(int status, QMouseEvent *e)
{
    RS_Vector pt = snapPoint(e);
    RS_Entity *se;
    switch (status) {
        case SelThreadPt: {
            pContext->threadPt = pt;
            setStatus(SelEntity);
            break;
        }
        case SelEntity: {
            se = catchEntity(e, enTypeList, RS2::ResolveAll);
            if (se != nullptr) {
                RS_Vector closestPoint;
                if (pContext->genPathConfig.cutMethod == CAM_GenPathConfig::CutMethod::CutMethodManual) {
                    closestPoint = se->getNearestPointOnEntity(pt);
                }
                else if (pContext->genPathConfig.cutMethod == CAM_GenPathConfig::CutMethod::CutMethodDirect) {
                    closestPoint = se->getNearestEndpoint(pContext->threadPt);
                }
                else {
                    closestPoint = se->getNearestPointOnEntity(pContext->threadPt);
                }
                pContext->startPt = closestPoint;

                genDirAngles(closestPoint);

                setStatus(SelMoveDir); // test
            }
            break;
        }
        case SelMoveDir: {
            genDirArrow(pContext->startPt, pt);

            if (pContext->dirArrowEndPt.valid) {
                pContext->moveDir = pContext->dirArrowEndPt;
            }
            else {
                pContext->moveDir = pt;
            }
            setStatus(SelCompDir);
            break;
        }
        case SelCompDir: {
            genCompArrow(pContext->startPt, pt);

            if (pContext->compArrowEndPt.valid) {
                pContext->compDir = pContext->compArrowEndPt;
            }
            else {
                pContext->compDir = pt;
            }
            QList<CAM_Segment> allSegments = pathGen.readAllSegments(getAllBasicEntities(*container));
            QList<CAM_Segment> rawPath;
            if (!pathGen.searchPath(allSegments, pContext->startPt, pContext->moveDir, pContext->startPt, rawPath)) {
                // not closed path
                setStatus(SelEndPt);
            }
            else {
                // closed path
                handleClosedPath(pContext->threadPt, pContext->startPt, pContext->moveDir, rawPath, pContext->compDir, pContext->genPathConfig);
                setStatus(-1);
            }
            break;
        }
        case SelEndPt: {
            pContext->endPt = pt;
            setStatus(SelExitPt);
            break;
        }
        case SelExitPt: {
            pContext->exitPt = pt;
            QList<CAM_Segment> allSegments = pathGen.readAllSegments(getAllBasicEntities(*container));
            handleUnclosedPath(pContext->threadPt, pContext->startPt, pContext->moveDir, pContext->endPt, pContext->exitPt, allSegments, pContext->compDir, pContext->genPathConfig);
            setStatus(-1);
            break;
        }
    }
}

QList<RS_Entity *> RS_ActionCamGenPath::getAllBasicEntities(RS_EntityContainer& container)
{
    QList<RS_Entity *> res;

    for (RS_Entity *e: container) {
        if (!e->isVisible()) {
            continue;
        }
        if (e->isContainer()) {
            RS_EntityContainer *ec = (RS_EntityContainer *)e;
            res.append(getAllBasicEntities(*ec));
        }
        else if (enTypeList.contains(e->rtti())) {
            res.append(e);
        }
    }

    return res;
}

void RS_ActionCamGenPath::genDirAngles(const RS_Vector &startPt)
{
    QList<CAM_Segment> segs =  pathGen.readAllSegments(getAllBasicEntities(*container));
    QList<CAM_Segment> devidedSegs = utils2d.devideSegments(startPt, segs);
    QList<int> indexes;
    QList<CAM_Segment> acrossPointSegs = utils2d.segmentsAcrossPoint(startPt, devidedSegs, indexes);
    if (acrossPointSegs.isEmpty()) {
        pContext->dirAngles.clear();
        return;
    }
    QList<RS_Vector> tans;
    for (const CAM_Segment &seg: acrossPointSegs) {
        RS_Vector tan = utils2d.segmentsGetBorderTangentVector(startPt, seg);
        tans.append(tan);
    }
    pContext->dirAngles = tans;
}

void RS_ActionCamGenPath::drawDirPreviewArrow(const RS_Vector &startPt, const RS_Vector &endPt)
{
    if (pContext->dirAngles.count() != 0) {
        double maxDot = -2;
        RS_Vector bestDir;
        for (const RS_Vector &tan: pContext->dirAngles) {
            RS_Vector dir = (endPt - startPt).normalized();
            double dot = dir.dotP(tan);
            if (dot > maxDot) {
                maxDot = dot;
                bestDir = tan;
            }
        }

        RS_Vector normTan = bestDir.normalized();
        RS_Vector arrowUpDir(cos(M_PI_4) * normTan.x - sin(M_PI_4) * normTan.y, sin(M_PI_4) * normTan.x + cos(M_PI_4) * normTan.y);
        RS_Vector arrowDownDir(cos(-M_PI_4) * normTan.x - sin(-M_PI_4) * normTan.y, sin(-M_PI_4) * normTan.x + cos(-M_PI_4) * normTan.y);
        RS_Vector arrowStartPt = startPt;
        RS_Vector arrowEndPt = startPt + normTan * 2;

        RS_Line *dirArrow = new RS_Line(arrowStartPt, arrowEndPt);
        preview->addEntity(dirArrow);
        RS_Line *dirArrowUp = new RS_Line(arrowEndPt, arrowEndPt - arrowUpDir);
        preview->addEntity(dirArrowUp);
        RS_Line *dirArrowDown = new RS_Line(arrowEndPt, arrowEndPt - arrowDownDir);
        preview->addEntity(dirArrowDown);
    }
}

void RS_ActionCamGenPath::drawCompPreviewArrow(const RS_Vector &startPt, const RS_Vector &endPt)
{
    if (pContext->dirArrowEndPt.valid) {
        RS_Vector dir = pContext->dirArrowEndPt - startPt;
        RS_Vector dirRot90(-dir.y, dir.x);
        RS_Vector compDir = endPt - startPt;
        double dot = compDir.dotP(dirRot90);
        if (dot < 0) {
            dirRot90 = -dirRot90;
        }
        RS_Vector normDirRot90 = dirRot90.normalized();
        RS_Vector arrowUpDir(cos(M_PI_4) * normDirRot90.x - sin(M_PI_4) * normDirRot90.y, sin(M_PI_4) * normDirRot90.x + cos(M_PI_4) * normDirRot90.y);
        RS_Vector arrowDownDir(cos(-M_PI_4) * normDirRot90.x - sin(-M_PI_4) * normDirRot90.y, sin(-M_PI_4) * normDirRot90.x + cos(-M_PI_4) * normDirRot90.y);
        RS_Vector arrowStartPt = startPt;
        RS_Vector arrowEndPt = startPt + dirRot90;

        RS_Line *compArrow = new RS_Line(arrowStartPt, arrowEndPt);
        preview->addEntity(compArrow);
        RS_Line *compArrowUp = new RS_Line(arrowEndPt, arrowEndPt - arrowUpDir);
        preview->addEntity(compArrowUp);
        RS_Line *compArrowDown = new RS_Line(arrowEndPt, arrowEndPt - arrowDownDir);
        preview->addEntity(compArrowDown);
    }
}

void RS_ActionCamGenPath::genDirArrow(const RS_Vector &startPt, const RS_Vector &endPt)
{
    pContext->dirArrowEndPt.valid = false;
    pContext->dirArrowUpPt.valid = false;
    pContext->dirArrowDownPt.valid = false;

    if (pContext->dirAngles.count() != 0) {
        double maxDot = -2;
        RS_Vector bestDir;
        for (const RS_Vector &tan: pContext->dirAngles) {
            RS_Vector dir = (endPt - startPt).normalized();
            double dot = dir.dotP(tan);
            if (dot > maxDot) {
                maxDot = dot;
                bestDir = tan;
            }
        }

        RS_Vector normTan = bestDir.normalized();
        RS_Vector arrowUpDir(cos(M_PI_4) * normTan.x - sin(M_PI_4) * normTan.y, sin(M_PI_4) * normTan.x + cos(M_PI_4) * normTan.y);
        RS_Vector arrowDownDir(cos(-M_PI_4) * normTan.x - sin(-M_PI_4) * normTan.y, sin(-M_PI_4) * normTan.x + cos(-M_PI_4) * normTan.y);
        RS_Vector arrowEndPt = startPt + normTan * 2;

        pContext->dirArrowEndPt = arrowEndPt;
        pContext->dirArrowUpPt = arrowEndPt - arrowUpDir;
        pContext->dirArrowDownPt = arrowEndPt - arrowDownDir;
    }
}

void RS_ActionCamGenPath::genCompArrow(const RS_Vector &startPt, const RS_Vector &endPt)
{
    pContext->compArrowEndPt.valid = false;
    pContext->compArrowUpPt.valid = false;
    pContext->compArrowDownPt.valid = false;

    if (pContext->dirArrowEndPt.valid) {
        RS_Vector dir = pContext->dirArrowEndPt - startPt;
        RS_Vector dirRot90(-dir.y, dir.x);
        RS_Vector compDir = endPt - startPt;
        double dot = compDir.dotP(dirRot90);
        if (dot < 0) {
            dirRot90 = -dirRot90;
        }
        RS_Vector normDirRot90 = dirRot90.normalized();
        RS_Vector arrowUpDir(cos(M_PI_4) * normDirRot90.x - sin(M_PI_4) * normDirRot90.y, sin(M_PI_4) * normDirRot90.x + cos(M_PI_4) * normDirRot90.y);
        RS_Vector arrowDownDir(cos(-M_PI_4) * normDirRot90.x - sin(-M_PI_4) * normDirRot90.y, sin(-M_PI_4) * normDirRot90.x + cos(-M_PI_4) * normDirRot90.y);
        RS_Vector arrowEndPt = startPt + dirRot90;

        pContext->compArrowEndPt = arrowEndPt;
        pContext->compArrowUpPt = arrowEndPt - arrowUpDir;
        pContext->compArrowDownPt = arrowEndPt - arrowDownDir;
    }
}

void RS_ActionCamGenPath::segmentsToPolyline(QList<CAM_Segment> segments, RS_EntityContainer &container)
{
    RS_Polyline *pl = new RS_Polyline(&container);
    for (const CAM_Segment &seg: segments) {
        pl->addVertex(seg.getStartPt(), seg.getBulge());
    }
    pl->addVertex(segments.last().getEndPt());
    container.addEntity(pl);
    // set color
}

bool RS_ActionCamGenPath::handleClosedPath(const RS_Vector &threadPt, const RS_Vector &startPt, const RS_Vector &dirPt, const QList<CAM_Segment> &rawPath, const RS_Vector &compPt, const CAM_GenPathConfig &setupArgs)
{
    // 计算偏移方向
    RS_Vector inVec = startPt - threadPt;
    RS_Vector normedInVec = inVec.normalized();

    RS_Vector offsetVec(-normedInVec.y, normedInVec.x);

    RS_Vector dirVec = dirPt - startPt;
    RS_Vector normedDirVec = dirVec.normalized();

    if (offsetVec.dotP(normedDirVec) > 0) {
        offsetVec = -offsetVec;
    }

    // 计算残留宽 残留高
    double remainHeight = setupArgs.remainHeight;
    double remainWidth = setupArgs.remainWidth;

    RS_Vector exitPt = startPt - normedInVec * remainHeight + offsetVec * remainWidth;
    RS_Vector ptOnExitToEndRay = exitPt + normedInVec * remainHeight;
    RS_Vector endPt;
    if (remainWidth == 0) {
        endPt = startPt;
    }
    else {
        if (remainHeight == 0) {
            ptOnExitToEndRay = exitPt + normedInVec;
        }
        if (!utils2d.raySegmentsIntersect(exitPt, ptOnExitToEndRay, rawPath, endPt)) {
            // 无效的残留宽 残留高
            return false;
        }
    }
    RS_Vector cutOffPt = exitPt - offsetVec * remainWidth;

    QList<CAM_Segment> path;
    if (!pathGen.searchPath(rawPath, startPt, dirPt, endPt, path)) {
        return false;
    }

    QList<CAM_Segment> closePath; // 关闭路径
    bool closePathExists = false;
    if (remainWidth != 0) {
        closePathExists = pathGen.searchPath(rawPath, startPt, startPt * 2 - dirPt, endPt, closePath);
        if (closePathExists) {
            // 反向路径
            QList<CAM_Segment> reversedClosePath;
            for (int i = closePath.size() - 1; i >= 0; i--) {
                CAM_Segment seg = closePath[i];
                reversedClosePath.append(CAM_Segment(seg.getEndPt(), seg.getStartPt(), -seg.getBulge()));
            }
            closePath = reversedClosePath;
        }
    }

    RS_Vector compVec = compPt - startPt;
    double dirInvInCross = dirVec.x * compVec.y - dirVec.y * compVec.x;
    bool compDir = dirInvInCross >= 0;

    CAM_PathData pathData;
    pathData.isClosed = true;
    pathData.threadPt = threadPt;
    pathData.startPt = startPt;
    pathData.endPt = endPt;
    pathData.exitPt = exitPt;
    pathData.cutOffPt = cutOffPt;
    pathData.mainPath = path;
    pathData.compensateDir = compDir;
    pathData.closePathExists = closePathExists;
    pathData.closePath = closePath;

    CAM_CutData cutData(pathData, setupArgs, container, graphicView);
    cutData.createBlockObject();

    return true;
}

bool RS_ActionCamGenPath::handleUnclosedPath(const RS_Vector &threadPt, const RS_Vector &startPt, const RS_Vector &dirPt, const RS_Vector &endPt, const RS_Vector &exitPt, const QList<CAM_Segment> &rawPath, const RS_Vector &compPt, const CAM_GenPathConfig &setupArgs)
{
    QList<CAM_Segment> path;
    if (!pathGen.searchPath(rawPath, startPt, dirPt, endPt, path)) {
        return false;
    }

    RS_Vector inVec = startPt - threadPt;
    RS_Vector normedInVec = inVec.normalized();

    RS_Vector cutOffPt = startPt - normedInVec * setupArgs.remainHeight;

    RS_Vector compVec = compPt - startPt;
    RS_Vector dirVec = dirPt - startPt;
    double dirInvInCross = dirVec.x * compVec.y - dirVec.y * compVec.x;
    bool compDir = dirInvInCross >= 0;

    CAM_PathData pathData;
    pathData.isClosed = false;
    pathData.threadPt = threadPt;
    pathData.startPt = startPt;
    pathData.endPt = endPt;
    pathData.exitPt = exitPt;
    pathData.cutOffPt = cutOffPt;
    pathData.mainPath = path;
    pathData.compensateDir = compDir;
    pathData.closePathExists = false;

    CAM_CutData cutData(pathData, setupArgs, container, graphicView);
    cutData.createBlockObject();

    return true;
}

void RS_ActionCamGenPath::updateMouseButtonHints()
{
    switch (getStatus()) {
        case SelThreadPt: {
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select thread point"));
            break;
        }
        case SelEntity: {
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select entity"));
            break;
        }
        case SelMoveDir: {
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select move direction"));
            break;
        }
        case SelCompDir: {
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select comp direction"));
            break;
        }
        case SelEndPt: {
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select end point"));
            break;
        }
        case SelExitPt: {
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select exit point"));
            break;
        }
    }
}

void RS_ActionCamGenPath::deleteHighlighted()
{
    if (highlightedEntity != nullptr) {
        highlightedEntity->setHighlighted(false);
        highlightedEntity = nullptr;
    }
}

RS_ActionCamGenPath::~RS_ActionCamGenPath() = default;
