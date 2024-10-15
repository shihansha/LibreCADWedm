#include "rs_actioncamutils.h"
#include "rs_dialogfactory.h"

#include "cam_version.h"

RS_ActionCamUtils::RS_ActionCamUtils(RS_EntityContainer& container, RS_GraphicView& graphicView, RS2::ActionType actionType)
    :RS_PreviewActionInterface("Cam Utils", container, graphicView)
{
    setActionType(actionType);
}

RS_ActionCamUtils::~RS_ActionCamUtils()
{

}

void RS_ActionCamUtils::init(int status)
{
    RS_PreviewActionInterface::init(status);

    if (actionType == RS2::ActionEdmUtilsTest) {
        RS_DIALOGFACTORY->commandMessage("Test echo!");
    }
    else if (actionType == RS2::ActionEdmUtilsConfigNetwork) {
        RS_DIALOGFACTORY->requestCamConfigNetworkDialog();
    }
    else if (actionType == RS2::ActionEdmUtilsVersion) {
        RS_DIALOGFACTORY->commandMessage(tr("Version: ") + CAM_VERSION);
        RS_DIALOGFACTORY->commandMessage("https://github.com/shihansha/LibreCADWedm");
    }

    finish();

    snapMode.restriction = RS2::RestrictNothing;
}
