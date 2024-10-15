#include "rs_actioncamgendiffpath.h"

#include "rs_insert.h"
#include "rs_graphicview.h"

#include "cam_cutdata.h"
#include "cam_diffcutdata.h"

#include "rs_dialogfactory.h"
#include "QMouseEvent"

struct RS_ActionCamGenDiffPath::ContextData
{
    RS_Insert *mainPathInsert;
};

RS_ActionCamGenDiffPath::RS_ActionCamGenDiffPath(RS_EntityContainer &container, RS_GraphicView &graphicView)
    : RS_PreviewActionInterface("Cam Gendiffpath", container, graphicView)
    , pContext(std::make_unique<ContextData>())
{
    setActionType(RS2::ActionEdmGenDiffPath);
}

RS_ActionCamGenDiffPath::~RS_ActionCamGenDiffPath()
{
}

void RS_ActionCamGenDiffPath::init(int status)
{
    RS_PreviewActionInterface::init(status);

    snapMode.restriction = RS2::RestrictNothing;

    if (status < 0) {
        deleteHighlighted();
    }
}

void RS_ActionCamGenDiffPath::mouseMoveEvent(QMouseEvent *e)
{
    snapPoint(e);
    RS_Entity *se;
    deleteHighlighted();

    se = catchEntity(e, RS2::EntityInsert, RS2::ResolveNone);

    switch (getStatus()) {
        case SelMainPath:
            if (se != nullptr) {
                se->setHighlighted(true);
                lastHighlightedEntities.append(se);
            }
            break;
        case SelSubPath:
            if (se != nullptr) {
                se->setHighlighted(true);
                lastHighlightedEntities.append(se);
            }
            break;
    }
}

void RS_ActionCamGenDiffPath::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        onMouseLeftButtonRelease(getStatus(), e);
    }
}

void RS_ActionCamGenDiffPath::onMouseLeftButtonRelease(int status, QMouseEvent *e)
{
    snapPoint(e);
    RS_Entity *se = catchEntity(e, RS2::EntityInsert, RS2::ResolveNone);

    switch (status) {
        case SelMainPath:
            pContext->mainPathInsert = nullptr;
            if (se != nullptr) {
                pContext->mainPathInsert = dynamic_cast<RS_Insert *>(se);
                if (pContext->mainPathInsert != nullptr) {
                    setStatus(SelSubPath);
                }
            }
            break;
        case SelSubPath:
            if (se != nullptr) {
                RS_Insert *subPathInsert = dynamic_cast<RS_Insert *>(se);
                if (pContext->mainPathInsert != nullptr && subPathInsert != nullptr) {
                    CAM_CutData mainCutData(container, graphicView);
                    CAM_CutData subCutData(container, graphicView);
                    bool mainLoadRes = mainCutData.loadFromBlockObject(pContext->mainPathInsert);
                    bool subLoadRes = subCutData.loadFromBlockObject(subPathInsert);

                    if (mainLoadRes && subLoadRes) {
                        CAM_DiffCutData diffCutData(mainCutData, subCutData, container, graphicView);
                        RS_Insert *diffInsert = diffCutData.createBlockObject();
                        if (diffInsert != nullptr) {
                            RS_DIALOGFACTORY->commandMessage(tr("diff path generated"));
                        }
                    }
                }
            }
            setStatus(-1);
            break;
    }

    updateMouseButtonHints();
    deletePreview();
    deleteHighlighted();
    graphicView->redraw();
}

void RS_ActionCamGenDiffPath::updateMouseButtonHints()
{
    switch (getStatus()) {
        case SelMainPath:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select main path"));
            break;
        case SelSubPath:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Select sub path"));
            break;
    }
}

void RS_ActionCamGenDiffPath::deleteHighlighted()
{
    if (lastHighlightedEntities.count() > 0) {
        for (RS_Entity *entity : lastHighlightedEntities) {
            entity->setHighlighted(false);
            graphicView->drawEntity(entity);
        }
        lastHighlightedEntities.clear();
    }
}


