#include "qg_dlggenpath.h"
#include "ui_qg_dlggenpath.h"
#include "rs_settings.h"

namespace {
constexpr char GROUP_NAME[] = "EdmCam";
}

void QG_DlgGenPath::readSettings() {
    {
        RS_SETTINGS->beginGroup(GROUP_NAME);
        bool convOk;
        genPathData.remainHeight =
            RS_SETTINGS->readEntry("RemainHeight", "1").toDouble(&convOk);
        if (!convOk)
            genPathData.remainHeight = 1;
        genPathData.remainWidth = RS_SETTINGS->readEntry("RemainWidth", "0").toDouble(&convOk);
        if (!convOk)
            genPathData.remainWidth = 0;
        genPathData.compData[0] = RS_SETTINGS->readEntry("CompData0", "0").toDouble(&convOk);
        if (!convOk)
            genPathData.compData[0] = 0;
        genPathData.compData[1] = RS_SETTINGS->readEntry("CompData1", "0").toDouble(&convOk);
        if (!convOk)
            genPathData.compData[1] = 0;
        genPathData.compData[2] = RS_SETTINGS->readEntry("CompData2", "0").toDouble(&convOk);
        if (!convOk)
            genPathData.compData[2] = 0;
        genPathData.compData[3] = RS_SETTINGS->readEntry("CompData3", "0").toDouble(&convOk);
        if (!convOk)
            genPathData.compData[3] = 0;
        genPathData.elecData[0] = RS_SETTINGS->readNumEntry("ElecData0", 1041);
        genPathData.elecData[1] = RS_SETTINGS->readNumEntry("ElecData1", 1042);
        genPathData.elecData[2] = RS_SETTINGS->readNumEntry("ElecData2", 1043);
        genPathData.elecData[3] = RS_SETTINGS->readNumEntry("ElecData3", 1044);
        genPathData.cutTime = RS_SETTINGS->readNumEntry("CutTime", 1);
        genPathData.cutMethod =
            (CAM_GenPathConfig::CutMethod)RS_SETTINGS->readNumEntry("CutMethod", (int)CAM_GenPathConfig::CutMethodManual);
        genPathData.tapeEnabled = RS_SETTINGS->readNumEntry("TapeEnabled");
        genPathData.tapeAngle = RS_SETTINGS->readEntry("TapeAngle", "0").toDouble(&convOk);
        if (!convOk)
            genPathData.tapeAngle = 0;
        genPathData.paramUseMacro = RS_SETTINGS->readNumEntry("ParamUseMacro", true);
        genPathData.useAbsCommand = RS_SETTINGS->readNumEntry("UseAbsCommand");
        RS_SETTINGS->endGroup();
    }
}
void QG_DlgGenPath::setValidators() {
    intValidator = new QIntValidator(0, 9999, this);
    doubleValidator = new QDoubleValidator(-1000, 1000, 3, this);
    doublePosValidator = new QDoubleValidator(0, 1000, 3, this);

    ui->leTaperAngle->setValidator(doublePosValidator);
    ui->leRemainHeight->setValidator(doubleValidator);
    ui->leRemainWidth->setValidator(doublePosValidator);
    ui->leElecCut1->setValidator(intValidator);
    ui->leElecCut2->setValidator(intValidator);
    ui->leElecCut3->setValidator(intValidator);
    ui->leElecCut4->setValidator(intValidator);
    ui->leCompCut1->setValidator(doubleValidator);
    ui->leCompCut2->setValidator(doubleValidator);
    ui->leCompCut3->setValidator(doubleValidator);
    ui->leCompCut4->setValidator(doubleValidator);
}

void QG_DlgGenPath::setGroups() {
    cutMethodGroup = new QButtonGroup(this);
    cutMethodGroup->addButton(ui->rbCutInDirect, 0);
    cutMethodGroup->addButton(ui->rbCutInPerp, 1);
    cutMethodGroup->addButton(ui->rbCutInManual, 2);

    tapeGroup = new QButtonGroup(this);
    tapeGroup->addButton(ui->rbTaperCancel, 0);
    tapeGroup->addButton(ui->rbTaperLeftSide, 1);
    tapeGroup->addButton(ui->rbTaperRightSide, 2);

    commandGroup = new QButtonGroup(this);
    commandGroup->addButton(ui->rbMethodAbs, 0);
    commandGroup->addButton(ui->rbMethodRel, 1);

    paramModeGroup = new QButtonGroup(this);
    paramModeGroup->addButton(ui->rbUseParamIndex, 0);
    paramModeGroup->addButton(ui->rbUseParamMacro, 1);
}

void QG_DlgGenPath::initControls() {
    if (genPathData.cutMethod == CAM_GenPathConfig::CutMethodManual) ui->rbCutInManual->setChecked(true);
    else if (genPathData.cutMethod == CAM_GenPathConfig::CutMethodDirect) ui->rbCutInDirect->setChecked(true);
    else ui->rbCutInPerp->setChecked(true);

    if (!genPathData.tapeEnabled) ui->rbTaperCancel->setChecked(true);
    else if (genPathData.tapeAngle < 0) ui->rbTaperRightSide->setChecked(true);
    else ui->rbTaperLeftSide->setChecked(true);

    QString taperAngleStr;
    taperAngleStr.setNum(abs(genPathData.tapeAngle));
    ui->leTaperAngle->setText(taperAngleStr);

    if (genPathData.useAbsCommand) ui->rbMethodAbs->setChecked(true);
    else ui->rbMethodRel->setChecked(true);

    QString remainHeightStr;
    remainHeightStr.setNum(genPathData.remainHeight);
    ui->leRemainHeight->setText(remainHeightStr);

    QString remainWidthStr;
    remainWidthStr.setNum(genPathData.remainWidth);
    ui->leRemainWidth->setText(remainWidthStr);

    int cutTime = genPathData.cutTime;
    QStringList cutCountList;
    cutCountList.append(tr("1 Time"));
    cutCountList.append(tr("2 Times"));
    cutCountList.append(tr("3 Times"));
    cutCountList.append(tr("4 Times"));
    ui->cbCutCount->addItems(cutCountList);

    ui->cbCutCount->setCurrentIndex(cutTime - 1);

    if (genPathData.paramUseMacro) ui->rbUseParamMacro->setChecked(true);
    else ui->rbUseParamIndex->setChecked(true);

    QString elecData0Str;
    elecData0Str.setNum(genPathData.elecData[0]);
    ui->leElecCut1->setText(elecData0Str);
    QString elecData1Str;
    elecData1Str.setNum(genPathData.elecData[1]);
    ui->leElecCut2->setText(elecData1Str);
    QString elecData2Str;
    elecData2Str.setNum(genPathData.elecData[2]);
    ui->leElecCut3->setText(elecData2Str);
    QString elecData3Str;
    elecData3Str.setNum(genPathData.elecData[3]);
    ui->leElecCut4->setText(elecData3Str);

    QString compData0Str;
    compData0Str.setNum(genPathData.compData[0]);
    ui->leCompCut1->setText(compData0Str);
    QString compData1Str;
    compData1Str.setNum(genPathData.compData[1]);
    ui->leCompCut2->setText(compData1Str);
    QString compData2Str;
    compData2Str.setNum(genPathData.compData[2]);
    ui->leCompCut3->setText(compData2Str);
    QString compData3Str;
    compData3Str.setNum(genPathData.compData[3]);
    ui->leCompCut4->setText(compData3Str);

}

void QG_DlgGenPath::RecvCutMethodMode(int id)
{
    if (id == -1) return;
    if ((int)genPathData.cutMethod != id) {
        genPathData.cutMethod = (CAM_GenPathConfig::CutMethod)id;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("CutMethod", (int)genPathData.cutMethod);
        RS_SETTINGS->endGroup();
    }
}

void QG_DlgGenPath::RecvTapeMode(int id)
{
    if (id == -1) return;
    int curId;
    if (!genPathData.tapeEnabled) {
        curId = 0;
    }
    else {
        if (genPathData.tapeAngle < 0) {
            curId = 2;
        }
        else {
            curId = 1;
        }
    }

    if (curId == id) {
        return;
    }

    if (id == 0) {
        genPathData.tapeEnabled = false;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("TapeEnabled", false);
        RS_SETTINGS->endGroup();
    }
    else if (id == 1) {
        genPathData.tapeEnabled = true;
        genPathData.tapeAngle = abs(genPathData.tapeAngle);

        QString tapeAngleStr;
        tapeAngleStr.setNum(genPathData.tapeAngle);

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("TapeEnabled", true);
        RS_SETTINGS->writeEntry("TapeAngle", tapeAngleStr);
        RS_SETTINGS->endGroup();
    }
    else {
        genPathData.tapeEnabled = true;
        genPathData.tapeAngle = -abs(genPathData.tapeAngle);

        QString tapeAngleStr;
        tapeAngleStr.setNum(genPathData.tapeAngle);

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("TapeEnabled", true);
        RS_SETTINGS->writeEntry("TapeAngle", tapeAngleStr);
        RS_SETTINGS->endGroup();
    }
}

void QG_DlgGenPath::RecvCommandMode(int id)
{
    if (id == -1) return;
    int curId = genPathData.useAbsCommand ? 0 : 1;
    if (curId == id) return;

    if (id == 0) {
        genPathData.useAbsCommand = true;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("UseAbsCommand", true);
        RS_SETTINGS->endGroup();
    }
    else {
        genPathData.useAbsCommand = false;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("UseAbsCommand", false);
        RS_SETTINGS->endGroup();
    }
}

void QG_DlgGenPath::RecvParamMode(int id)
{
    if (id == -1) return;
    int curId = genPathData.paramUseMacro ? 1 : 0;
    if (curId == id) return;

    if (id == 0) {
        genPathData.paramUseMacro = false;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("ParamUseMacro", false);
        RS_SETTINGS->endGroup();
    }
    else {
        genPathData.paramUseMacro = true;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("ParamUseMacro", true);
        RS_SETTINGS->endGroup();
    }
}

QG_DlgGenPath::QG_DlgGenPath(QWidget *parent)
    : QDialog(parent), ui(new Ui::QG_DlgGenPath) {
    ui->setupUi(this);

    readSettings();

    setValidators();

    setGroups();

    initControls();

    connect(cutMethodGroup, SIGNAL(idPressed(int)), this, SLOT(RecvCutMethodMode(int)));
    connect(tapeGroup, SIGNAL(idPressed(int)), this, SLOT(RecvTapeMode(int)));
    connect(commandGroup, SIGNAL(idPressed(int)), this, SLOT(RecvCommandMode(int)));
    connect(paramModeGroup, SIGNAL(idPressed(int)), this, SLOT(RecvParamMode(int)));

    connect(ui->dialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->dialogButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QG_DlgGenPath::~QG_DlgGenPath()
{
    delete paramModeGroup;
    delete commandGroup;
    delete tapeGroup;
    delete cutMethodGroup;
    delete doublePosValidator;
    delete doubleValidator;
    delete intValidator;
    delete ui;
}

CAM_GenPathConfig QG_DlgGenPath::getData()
{
    return genPathData;
}

void QG_DlgGenPath::on_cbCutCount_currentIndexChanged(int index)
{
    if (index == -1) return;
    if (genPathData.cutTime != index + 1) {
        genPathData.cutTime = index + 1;

        RS_SETTINGS->beginGroup(GROUP_NAME);
        RS_SETTINGS->writeEntry("CutTime", index + 1);
        RS_SETTINGS->endGroup();
    }
}


void QG_DlgGenPath::on_leTaperAngle_editingFinished()
{
    QString curText = ui->leTaperAngle->text();
    bool convOk;
    double tapeAngle = curText.toDouble(&convOk);
    if (genPathData.tapeAngle < 0) {
        tapeAngle = -tapeAngle;
    }

    if (convOk) {
        if (genPathData.tapeAngle != tapeAngle) {
            genPathData.tapeAngle = tapeAngle;

            QString tapeAngleStr;
            tapeAngleStr.setNum(tapeAngle);

            RS_SETTINGS->beginGroup(GROUP_NAME);
            RS_SETTINGS->writeEntry("TapeAngle", tapeAngleStr);
            RS_SETTINGS->endGroup();
        }
    }
}


void QG_DlgGenPath::on_leRemainHeight_editingFinished()
{
    QString curText = ui->leRemainHeight->text();
    bool convOk;
    double remainHeight = curText.toDouble(&convOk);
    if (convOk) {
        if (genPathData.remainHeight != remainHeight) {
            genPathData.remainHeight = remainHeight;


            RS_SETTINGS->beginGroup(GROUP_NAME);
            RS_SETTINGS->writeEntry("RemainHeight", curText);
            RS_SETTINGS->endGroup();
        }
    }
}

void QG_DlgGenPath::on_leRemainWidth_editingFinished()
{
    QString curText = ui->leRemainWidth->text();
    bool convOk;
    double remainWidth = curText.toDouble(&convOk);
    if (convOk) {
        if (genPathData.remainWidth != remainWidth) {
            genPathData.remainWidth = remainWidth;

            RS_SETTINGS->beginGroup(GROUP_NAME);
            RS_SETTINGS->writeEntry("RemainWidth", curText);
            RS_SETTINGS->endGroup();
        }
    }
}

void QG_DlgGenPath::elecChangedHandler(int id) {
    QString curText;
    if (id == 0) curText = ui->leElecCut1->text();
    else if (id == 1) curText = ui->leElecCut2->text();
    else if (id == 2) curText = ui->leElecCut3->text();
    else if (id == 3) curText = ui->leElecCut4->text();
    else return;

    bool convOk;
    int elecIndex = curText.toInt(&convOk);
    if (convOk) {
        if (genPathData.elecData[id] != elecIndex) {
            genPathData.elecData[id] = elecIndex;

            RS_SETTINGS->beginGroup(GROUP_NAME);
            RS_SETTINGS->writeEntry(tr("ElecData%1").arg(id), elecIndex);
            RS_SETTINGS->endGroup();
        }
    }
}

void QG_DlgGenPath::compChangedHandler(int id) {
    QString curText;
    if (id == 0) curText = ui->leCompCut1->text();
    else if (id == 1) curText = ui->leCompCut2->text();
    else if (id == 2) curText = ui->leCompCut3->text();
    else if (id == 3) curText = ui->leCompCut4->text();
    else return;

    bool convOk;
    double compValue = curText.toDouble(&convOk);
    if (convOk) {
        if (genPathData.compData[id] != compValue) {
            genPathData.compData[id] = compValue;

            RS_SETTINGS->beginGroup(GROUP_NAME);
            RS_SETTINGS->writeEntry(tr("CompData%1").arg(id), curText);
            RS_SETTINGS->endGroup();
        }
    }
}

void QG_DlgGenPath::on_leElecCut1_editingFinished()
{
    elecChangedHandler(0);
}


void QG_DlgGenPath::on_leElecCut2_editingFinished()
{
    elecChangedHandler(1);
}


void QG_DlgGenPath::on_leElecCut3_editingFinished()
{
    elecChangedHandler(2);
}


void QG_DlgGenPath::on_leElecCut4_editingFinished()
{
    elecChangedHandler(3);
}


void QG_DlgGenPath::on_leCompCut1_editingFinished()
{
    compChangedHandler(0);
}


void QG_DlgGenPath::on_leCompCut2_editingFinished()
{
    compChangedHandler(1);
}


void QG_DlgGenPath::on_leCompCut3_editingFinished()
{
    compChangedHandler(2);
}


void QG_DlgGenPath::on_leCompCut4_editingFinished()
{
    compChangedHandler(3);
}

