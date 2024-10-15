#ifndef QG_DLGGENPATH_H
#define QG_DLGGENPATH_H

#include <QDialog>
#include <QValidator>
#include <QButtonGroup>

#include "cam_genpathconfig.h"

namespace Ui {
class QG_DlgGenPath;
}

class QG_DlgGenPath : public QDialog
{
    Q_OBJECT

public:

    explicit QG_DlgGenPath(QWidget *parent = nullptr);
    ~QG_DlgGenPath();

    CAM_GenPathConfig getData();
private:
    CAM_GenPathConfig genPathData;
    QIntValidator *intValidator;
    QDoubleValidator *doubleValidator;
    QDoubleValidator *doublePosValidator;
    QButtonGroup *cutMethodGroup;
    QButtonGroup *tapeGroup;
    QButtonGroup *commandGroup;
    QButtonGroup *paramModeGroup;
    void readSettings();
    void setValidators();
    void setGroups();
    void initControls();
    void elecChangedHandler(int id);
    void compChangedHandler(int id);
public slots:
    void RecvCutMethodMode(int id);
    void RecvTapeMode(int id);
    void RecvCommandMode(int id);
    void RecvParamMode(int id);
private slots:
    void on_cbCutCount_currentIndexChanged(int index);

    void on_leTaperAngle_editingFinished();

    void on_leRemainHeight_editingFinished();

    void on_leRemainWidth_editingFinished();

    void on_leElecCut1_editingFinished();

    void on_leElecCut2_editingFinished();

    void on_leElecCut3_editingFinished();

    void on_leElecCut4_editingFinished();

    void on_leCompCut1_editingFinished();

    void on_leCompCut2_editingFinished();

    void on_leCompCut3_editingFinished();

    void on_leCompCut4_editingFinished();

private:
    Ui::QG_DlgGenPath *ui;
};

#endif // QG_DLGGENPATH_H
