#include "versiondialog.h"
#include "ui_versiondialog.h"

namespace core {

VersionDialog::VersionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VersionDialog)
{
    ui->setupUi(this);
}

VersionDialog::~VersionDialog()
{
    delete ui;
}

}
