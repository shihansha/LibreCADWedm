#ifndef RS_ACTIONCAMGENPATH_H
#define RS_ACTIONCAMGENPATH_H

#include <memory>

#include "rs_previewactioninterface.h"

#include "cam_utils2d.h"
#include "cam_pathgen.h"

#include "cam_genpathconfig.h"

class RS_ActionCamGenPath : public RS_PreviewActionInterface
{
    Q_OBJECT
public:
    RS_ActionCamGenPath(RS_EntityContainer& container, RS_GraphicView& graphicView);
    ~RS_ActionCamGenPath() override;
public:
    void init(int status) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void onMouseLeftButtonRelease(int status, QMouseEvent *e);

protected:
    void updateMouseButtonHints() override;
private:
    CAM_Utils2D utils2d;
    CAM_PathGen pathGen;

    enum Status {
        SelThreadPt,
        SelEntity,
        SelMoveDir,
        SelCompDir,
        SelEndPt,
        SelExitPt
    };
    struct ContextData;
    std::unique_ptr<ContextData> pContext;
    Status lastStatus;
    EntityTypeList enTypeList;
    RS_Entity *highlightedEntity;
    void deleteHighlighted();

    QList<RS_Entity *> getAllBasicEntities(RS_EntityContainer& container);
    void genDirAngles(const RS_Vector& startPt);
    void drawDirPreviewArrow(const RS_Vector& startPt, const RS_Vector& endPt);
    void drawCompPreviewArrow(const RS_Vector& startPt, const RS_Vector& endPt);
    void genDirArrow(const RS_Vector& startPt, const RS_Vector& endPt);
    void genCompArrow(const RS_Vector& startPt, const RS_Vector& endPt);
    void segmentsToPolyline(QList<CAM_Segment> segments, RS_EntityContainer& container);
    bool handleClosedPath(const RS_Vector& threadPt, const RS_Vector& startPt, const RS_Vector& dirPt, const QList<CAM_Segment>& rawPath, const RS_Vector& compPt, const CAM_GenPathConfig& setupArgs);
    bool handleUnclosedPath(const RS_Vector& threadPt, const RS_Vector& startPt, const RS_Vector& dirPt, const RS_Vector& endPt, const RS_Vector& exitPt, const QList<CAM_Segment>& rawPath, const RS_Vector& compPt, const CAM_GenPathConfig& setupArgs);
};

#endif // RS_ACTIONCAMGENPATH_H
