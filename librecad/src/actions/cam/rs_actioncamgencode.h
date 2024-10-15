#ifndef RS_ACTIONCAMGENCODE_H
#define RS_ACTIONCAMGENCODE_H

#include <memory>

#include "rs_previewactioninterface.h"
#include "cam_gencode.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class RS_ActionCamGencode : public RS_PreviewActionInterface
{
    Q_OBJECT
public:

    RS_ActionCamGencode(RS_EntityContainer& container, RS_GraphicView& graphicView, bool postCodeToCNC = false);
    ~RS_ActionCamGencode() override;

public:
    void init(int status) override;
    void trigger() override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
protected:
    void onMouseLeftButtonRelease(int status, QMouseEvent *e);
    void onMouseRightButtonRelease(int status, QMouseEvent *e);
    void updateMouseButtonHints() override;
private:
    enum Status {
        SelInsert,
        WaitTransmit
    };

    CAM_GenCode genCode;
    bool postCodeToCNC;
    std::unique_ptr<QNetworkAccessManager> networkManager;
    QNetworkReply *networkReply;

    QList<RS_Insert *> inserts;

    RS_Entity *lastHightlightedEntity;

    void deleteHighlighted();
    void sendToCNC(const QString& gcode);
private slots:
    void onProcessFinished();
};

#endif // RS_ACTIONCAMGENCODE_H
