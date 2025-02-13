#include "core/mwctoolbar.h"
#include "ui_mwctoolbar.h"
#include <QPainter>
#include <QStyleOption>
#include "appcontext.h"
#include "../state/statemachine.h"
#include "../wallet/wallet.h"
#include <QDebug>

namespace core {

MwcToolbar::MwcToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MwcToolbar)
{
    ui->setupUi(this);

    //setAutoFillBackground(true);
    //setStyleSheet("background-color: #6F00D6;");
}

MwcToolbar::~MwcToolbar()
{
    delete ui;
}

void MwcToolbar::setAppEnvironment(state::StateMachine * _stateMachine, wallet::Wallet * _wallet ) {
    stateMachine = _stateMachine;
    wallet = _wallet;
    Q_ASSERT(stateMachine);
    Q_ASSERT(wallet);

    QObject::connect( wallet, &wallet::Wallet::onWalletBalanceUpdated,
                                 this, &MwcToolbar::onWalletBalanceUpdated, Qt::QueuedConnection );

}


void MwcToolbar::updateButtonsState( state::STATE state ) {
    ui->airdropToolButton->setChecked( state==state::AIRDRDOP_MAIN );
    ui->sendToolButton->setChecked( state==state::SEND_ONLINE_OFFLINE || state==state::SEND_ONLINE ||
                  state==state::SEND_OFFLINE);
    ui->recieveToolButton->setChecked( state==state::RECIEVE_COINS);
    ui->transactionToolButton->setChecked(state==state::TRANSACTIONS);
    ui->hodlToolButton->setChecked(state==state::HODL);
}

void MwcToolbar::paintEvent(QPaintEvent *)
{
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MwcToolbar::on_airdropToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::AIRDRDOP_MAIN );
}

void MwcToolbar::on_sendToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::SEND_ONLINE_OFFLINE );
}

void MwcToolbar::on_recieveToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::RECIEVE_COINS );

}

void MwcToolbar::on_transactionToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::TRANSACTIONS );
}

void MwcToolbar::on_hodlToolButton_clicked()
{
    stateMachine->setActionWindow( state::STATE::HODL );
}

// Account info is updated
void MwcToolbar::onWalletBalanceUpdated() {
    qDebug() << "get onWalletBalanceUpdated. Updating the balance";

    QVector<wallet::AccountInfo> balance = wallet->getWalletBalance();

    long mwcSum = 0;
    for ( const auto & ai : balance ) {
        mwcSum += ai.total;
    }

    ui->totalMwc->setText( util::nano2one(mwcSum) + " mwc" );
}


}


