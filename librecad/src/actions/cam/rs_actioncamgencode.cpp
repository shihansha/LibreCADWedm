#include "rs_actioncamgencode.h"
#include "rs_graphicview.h"

#include "rs_insert.h"
#include "rs_block.h"
#include "rs_selection.h"

#include "cam_cutdata.h"
#include "cam_diffcutdata.h"

#include "QFileDialog"

#include "cam_networkconfig.h"

#include "QMouseEvent"

#include "rs_dialogfactory.h"

RS_ActionCamGencode::RS_ActionCamGencode(RS_EntityContainer &container, RS_GraphicView &graphicView, bool postCodeToCNC)
    :RS_PreviewActionInterface("Cam Gencode", container, graphicView), postCodeToCNC(postCodeToCNC), lastHightlightedEntity(nullptr)
{
    setActionType(RS2::ActionEdmGenCode);

    networkManager = std::make_unique<QNetworkAccessManager>(this);
}

void RS_ActionCamGencode::init(int status)
{
    RS_PreviewActionInterface::init(status);

    snapMode.restriction = RS2::RestrictNothing;

    if (status < 0) {
        deleteHighlighted();
    }
}

void RS_ActionCamGencode::deleteHighlighted() {
    if (lastHightlightedEntity != nullptr) {
        lastHightlightedEntity->setHighlighted(false);
        lastHightlightedEntity = nullptr;
    }
}
void RS_ActionCamGencode::trigger() {
    if (getStatus() == SelInsert) {
        if (inserts.count() != 0) {
            QList<CAM_CutDataBase *> cutDataList;
            QList<RS_Vector> startPts;
            for (const RS_Insert *insert : inserts) {
                RS_Block *blk = insert->getBlockForInsert();
                if (blk != nullptr) {
                    if (blk->getName().startsWith(CAM_CutData::BLOCK_NAME_PREFIX)) {
                        CAM_CutData *cutData = new CAM_CutData(container, graphicView);
                        if (cutData->loadFromBlockObject(insert)) {
                            cutDataList.append(cutData);
                            startPts.append(insert->getInsertionPoint());
                        } else {
                            delete cutData;
                        }
                    } else if (blk->getName().startsWith(
                                   CAM_DiffCutData::BLOCK_NAME_PREFIX)) {
                        CAM_DiffCutData *diffCutData =
                            new CAM_DiffCutData(container, graphicView);
                        if (diffCutData->loadFromBlockObject(insert)) {
                            cutDataList.append(diffCutData);
                            startPts.append(insert->getInsertionPoint());
                        } else {
                            delete diffCutData;
                        }
                    }
                }
            }

            if (cutDataList.count() != 0) {
                if (postCodeToCNC) {
                    sendToCNC(genCode.generateGCode(cutDataList, startPts));
                    setStatus(WaitTransmit);
                } else {
                    // open save file dialog
                    QString fileName =
                        QFileDialog::getSaveFileName(graphicView, tr("Save G-code file"),
                                                                    "", tr("G-code files (*.txt)"));
                    if (!fileName.isEmpty()) {
                        QFile file(fileName);
                        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            QTextStream out(&file);
                            out << genCode.generateGCode(cutDataList, startPts);
                            file.close();
                            RS_DIALOGFACTORY->commandMessage(tr("G-code file saved"));
                        } else {
                            RS_DIALOGFACTORY->commandMessage(
                                tr("Failed to save G-code file"));
                        }
                    }
                    setStatus(-1);
                }

                for (CAM_CutDataBase *cutData : cutDataList) {
                    delete cutData;
                }
            }
        }

        inserts.clear();
    }

    deletePreview();
    deleteHighlighted();
    graphicView->redraw();
}

void RS_ActionCamGencode::mouseMoveEvent(QMouseEvent *e)
{
    snapPoint(e);

    //RS_Vector mouse = toGraph(e);
    RS_Entity *se = catchEntity(e, RS2::EntityInsert, RS2::ResolveNone);

    deleteHighlighted();

    if (se != nullptr && se->rtti() == RS2::EntityInsert) {
        se->setHighlighted(true);
        lastHightlightedEntity = se;
    }
}

void RS_ActionCamGencode::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        onMouseLeftButtonRelease(getStatus(), e);
    }
    else if (e->button() == Qt::RightButton) {
        onMouseRightButtonRelease(getStatus(), e);
    }
}

void RS_ActionCamGencode::onMouseLeftButtonRelease([[maybe_unused]]int status, QMouseEvent *e)
{
    RS_Entity *se = catchEntity(e, RS2::EntityInsert, RS2::ResolveNone);
    if (se != nullptr) {
        RS_Insert *insert = dynamic_cast<RS_Insert *>(se);
        if (insert != nullptr) {
            RS_Selection s(*container, graphicView);
            s.selectSingle(insert);

            this->inserts.append(insert);
        }
    }
}

void RS_ActionCamGencode::onMouseRightButtonRelease([[maybe_unused]] int status, [[maybe_unused]] QMouseEvent *e) {
    deletePreview();
    if (getStatus() == SelInsert) {
        if (inserts.count() != 0) {
            trigger();
        }
        else {
            setStatus(-1);
        }
    }
}

void RS_ActionCamGencode::updateMouseButtonHints()
{
    RS_DIALOGFACTORY->updateMouseWidget(tr("Select insert with cut data"));
}

void RS_ActionCamGencode::sendToCNC(const QString &gcode)
{
    CAM_NetworkConfig networkConfig;
    networkConfig.loadFromSettings();

    QNetworkRequest req;
    req.setRawHeader("Content-Type", "text/plain");
    QString url = QString("http://%1:%2/api/cad/postcode").arg(networkConfig.targetAddr, QString::number(networkConfig.targetPort));
    req.setUrl(QUrl(url));
    networkReply = networkManager->post(req, (gcode + "\0").toUtf8());
    connect(networkReply, &QNetworkReply::finished, this, &RS_ActionCamGencode::onProcessFinished);
}

void RS_ActionCamGencode::onProcessFinished()
{
    if (networkReply->error() == QNetworkReply::NoError) {
        RS_DIALOGFACTORY->commandMessage(tr("G-code sent to CNC"));
    }
    else {
        RS_DIALOGFACTORY->commandMessage(tr("Failed to send G-code to CNC: ") + networkReply->errorString());
    }
    setStatus(-1);
}

RS_ActionCamGencode::~RS_ActionCamGencode() = default;
