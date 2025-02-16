#include "a_initaccount_w.h"
#include "ui_a_initaccount.h"
#include "../util/passwordanalyser.h"
#include "../state/a_initaccount.h"
#include "../util/widgetutils.h"
#include "../control/messagebox.h"
#include <QShortcut>
#include <QKeyEvent>
#include "../core/global.h"

namespace wnd {

InitAccount::InitAccount(QWidget *parent, state::InitAccount * _state) :
    QWidget(parent),
    ui(new Ui::InitAccount),
    state(_state)
{
    ui->setupUi(this);

    state->setWindowTitle( mwc::VERSION + " " + mwc::APP_NAME );

    util::PasswordAnalyser pa( ui->password1Edit->text() );
    ui->strengthLabel->setText(pa.getPasswordQualityStr());

    ui->submitButton->setEnabled( pa.isPasswordOK() );

    ui->password1Edit->installEventFilter(this);

    utils::defineDefaultButtonSlot(this, SLOT(on_submitButton_clicked()) );
}

InitAccount::~InitAccount()
{
    delete ui;
}

void InitAccount::on_password1Edit_textChanged(const QString &text)
{
    util::PasswordAnalyser pa(text);
    ui->strengthLabel->setText(pa.getPasswordQualityStr());
    ui->submitButton->setEnabled( pa.isPasswordOK() );
}

void InitAccount::on_submitButton_clicked()
{
    QString pswd1 = ui->password1Edit->text();
    QString pswd2 = ui->password2Edit->text();
    util::PasswordAnalyser pa(pswd1);

    if (!pa.isPasswordOK())
        return;

    if (pswd1!=pswd2) {
        control::MessageBox::message(this, "Password", "Password doesn't match confirm string. Please retype the password correctly");
        return;
    }

    if (! pa.isPasswordOK() ) {
        control::MessageBox::message(this, "Password", "Your password is not strong enough. Please input stronger password");
        return;
    }

    state->setPassword(pswd1);
}

void InitAccount::on_instancesButton_clicked()
{
    control::MessageBox::message(this, "Not implemented", "Here we will have a dialog or a page where it will be possible to specify the directory with wallet data. Somilar to electrum");
}

}
