#include "qg_dlgconfignetwork.h"
#include "ui_qg_dlgconfignetwork.h"

#include "rs_settings.h"
#include "cam_networkconfig.h"

QG_DlgConfigNetwork::QG_DlgConfigNetwork(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QG_DlgConfigNetwork)
{
    ui->setupUi(this);

    networkConfig.loadFromSettings();

    initControls();
    setValidators();
}

QG_DlgConfigNetwork::~QG_DlgConfigNetwork()
{
    delete intValidator;
    delete ui;
}

void QG_DlgConfigNetwork::initControls()
{
    ui->leTargetAddr->setText(networkConfig.targetAddr);
    ui->leTargetPort->setText(QString::number(networkConfig.targetPort));
}

void QG_DlgConfigNetwork::setValidators()
{
    intValidator = new QIntValidator(0, 65535, this);
    ui->leTargetPort->setValidator(intValidator);
}

void QG_DlgConfigNetwork::targetAddressChanged()
{
    QString addr = ui->leTargetAddr->text();
    if (addr != networkConfig.targetAddr) {
        networkConfig.targetAddr = addr;

        RS_SETTINGS->beginGroup("EdmNetwork");
        RS_SETTINGS->writeEntry("TargetAddr", addr);
        RS_SETTINGS->endGroup();
    }
}

void QG_DlgConfigNetwork::targetPortChanged()
{
    bool ok;
    int port = ui->leTargetPort->text().toInt(&ok);
    if (ok) {
        if (port != networkConfig.targetPort) {
            networkConfig.targetPort = port;

            RS_SETTINGS->beginGroup("EdmNetwork");
            RS_SETTINGS->writeEntry("TargetPort", port);
            RS_SETTINGS->endGroup();
        }
    }
}
