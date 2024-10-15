#ifndef QG_DLGCONFIGNETWORK_H
#define QG_DLGCONFIGNETWORK_H

#include <QDialog>
#include <QValidator>

#include "cam_networkconfig.h"

namespace Ui {
class QG_DlgConfigNetwork;
}

class QG_DlgConfigNetwork : public QDialog
{
    Q_OBJECT

public:
    explicit QG_DlgConfigNetwork(QWidget *parent = nullptr);
    ~QG_DlgConfigNetwork();

private:
    Ui::QG_DlgConfigNetwork *ui;

    QIntValidator *intValidator;
    CAM_NetworkConfig networkConfig;

    void initControls();
    void setValidators();
private slots:
    void targetAddressChanged();
    void targetPortChanged();
};

#endif // QG_DLGCONFIGNETWORK_H
