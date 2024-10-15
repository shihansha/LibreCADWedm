#ifndef RS_ACTIONCAMUTILS_H
#define RS_ACTIONCAMUTILS_H

#include <QObject>

#include "rs_previewactioninterface.h"

class RS_ActionCamUtils : public RS_PreviewActionInterface
{
    Q_OBJECT
public:
    RS_ActionCamUtils(RS_EntityContainer& container, RS_GraphicView& graphicView, RS2::ActionType actionType);
    ~RS_ActionCamUtils() override;

public:
    void init(int status) override;
};

#endif // RS_ACTIONCAMUTILS_H
