#ifndef RS_ACTIONCAMGENDIFFPATH_H
#define RS_ACTIONCAMGENDIFFPATH_H

#include <memory>

#include "rs_previewactioninterface.h"

class RS_ActionCamGenDiffPath : public RS_PreviewActionInterface
{
    Q_OBJECT
public:
    RS_ActionCamGenDiffPath(RS_EntityContainer& container, RS_GraphicView& graphicView);
    ~RS_ActionCamGenDiffPath() override;
public:
    void init(int status) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void onMouseLeftButtonRelease(int status, QMouseEvent *e);
protected:
    void updateMouseButtonHints() override;
private:

    enum Status {
        SelMainPath,
        SelSubPath
    };

    struct ContextData;
    std::unique_ptr<ContextData> pContext;
    QList<RS_Entity *> lastHighlightedEntities;
    void deleteHighlighted();
};

#endif // RS_ACTIONCAMGENDIFFPATH_H
