#include "send2_online_w.h"
#include "ui_send2_online.h"
#include "sendcoinsparamsdialog.h"
#include "../control/messagebox.h"
#include "../util/address.h"
#include "../state/send2_Online.h"

namespace wnd {

QString generateAmountErrorMsg( long mwcAmount, const wallet::AccountInfo & acc, const core::SendCoinsParams & sendParams ) {
        QString msg2print = "You are trying to send " + util::nano2one(mwcAmount) + " wmc, but you only have " +
                            util::nano2one(acc.currentlySpendable) + " spendable wmc.";
        if (acc.awaitingConfirmation > 0)
            msg2print += " " + util::nano2one(acc.awaitingConfirmation) + " coins are awaiting confirmation.";

        if (acc.lockedByPrevTransaction > 0)
            msg2print += " " + util::nano2one(acc.lockedByPrevTransaction) + " coins are locked.";

        if (acc.awaitingConfirmation > 0 || acc.lockedByPrevTransaction > 0) {
            if (sendParams.inputConfirmationNumber != 1) {
                if (sendParams.inputConfirmationNumber < 0)
                    msg2print += " You can modify settings to spend wmc with less than 10 confirmations (wallet default value).";
                else
                    msg2print += " You can modify settings to spend wmc with less than " +
                                 QString::number(sendParams.inputConfirmationNumber) + " confirmations.";
            }
        }
        return msg2print;
}

SendOnline::SendOnline(QWidget *parent, state::SendOnline * _state ) :
    QWidget(parent),
    ui(new Ui::SendOnline),
    state(_state)
{
    ui->setupUi(this);

    ui->progress->initLoader(false);

    // inti accounts
    accountInfo = state->getWalletBalance();
    contacts    = state->getContacts();

    int idx=0;
    for (auto & info : accountInfo) {
        ui->accountComboBox->addItem( util::expandStrR(info.accountName, 25) + " Available: " + util::nano2one(info.currentlySpendable) + " mwc", QVariant(idx++) );
    }
}

SendOnline::~SendOnline()
{
    delete ui;
}



void SendOnline::on_contactsButton_clicked()
{
    // Get the contacts
    control::MessageBox::message(this, "Not implemented", "This functionality is not implemented yet");

/*    SelectContactDlg dlg(this, contacts);
    if (dlg.exec() == QDialog::Accepted) {
        wallet::WalletContact selectedContact = dlg.getSelectedContact();
        ui->sendEdit->setText( selectedContact.address );
    }*/

}

void SendOnline::on_allAmountButton_clicked()
{
    int accountIdx = ui->accountComboBox->currentData().toInt();
    ui->amountEdit->setText( util::nano2one( accountInfo[accountIdx].currentlySpendable ) );
}

void SendOnline::on_settingsButton_clicked()
{
    core::SendCoinsParams  params = state->getSendCoinsParams();

    SendCoinsParamsDialog dlg(this, params);
    if (dlg.exec() == QDialog::Accepted) {
        state->updateSendCoinsParams( dlg.getSendCoinsParams() );
    }

}

void SendOnline::on_sendButton_clicked()
{
    int accountIdx = ui->accountComboBox->currentData().toInt();
    wallet::AccountInfo acc = accountInfo[accountIdx];

    QPair<bool,long> mwcAmount = util::one2nano(ui->amountEdit->text());
    if (!mwcAmount.first) {
        control::MessageBox::message(this, "Incorrect Input", "Please specify correct number of MWC to send");
        ui->amountEdit->setFocus();
        return;
    }

    if ( mwcAmount.second > acc.currentlySpendable ) {
        QString msg2print = generateAmountErrorMsg( mwcAmount.second, acc, state->getSendCoinsParams() );
        control::MessageBox::message(this, "Incorrect Input",
                                     msg2print );
        ui->amountEdit->setFocus();
        return;
    }

    QString sendTo = ui->sendEdit->text();
    if ( sendTo.size()>0 && sendTo[0] == '@' )
        sendTo = sendTo.right(sendTo.size()-1);

    if (sendTo.size() == 0 ) {
        control::MessageBox::message(this, "Incorrect Input",
                                     "Please specify an address of send your MWC" );
        ui->sendEdit->setFocus();
        return;
    }

    // Check the address. Try contacts first
    QString address;

    for ( auto & cnt : contacts ) {
        if (cnt.name == sendTo) {
            address = "@" + cnt.name;
            break;
        }
    }
    if (address.isEmpty()) {
        for ( auto & cnt : contacts ) {
            if (cnt.name.compare(sendTo,  Qt::CaseSensitivity::CaseInsensitive)) {
                address = "@" + cnt.name;
                break;
            }
        }
    }


    if (address.isEmpty())
        address = sendTo;

    // Let's  verify address first
    if (address[0]!='@') {
        QPair< bool, util::ADDRESS_TYPE > res = util::verifyAddress(address);
        if ( !res.first ) {
            control::MessageBox::message(this, "Incorrect Input",
                                         "Please specify correct address to send your MWC" );
            ui->sendEdit->setFocus();
            return;
        }
    }

    ui->progress->show();

    state->sendMwc( accountInfo[accountIdx], address, mwcAmount.second, ui->descriptionEdit->toPlainText() );
}

void SendOnline::sendRespond( bool success, const QStringList & errors ) {
    ui->progress->hide();

    if (success) {
        control::MessageBox::message(this, "Success", "Your mwc was successfully sent to recipient");
        ui->sendEdit->setText("");
        ui->amountEdit->setText("");
        ui->descriptionEdit->setText("");
        return;
    }

    QString errMsg = util::formatErrorMessages(errors);

    if (errMsg.isEmpty())
        errMsg = "Your send request was failed by some reasons";
    else
        errMsg = "Your send request was failed:\n" + errMsg;

    control::MessageBox::message( this, "Send request was failed", errMsg );
}


}

