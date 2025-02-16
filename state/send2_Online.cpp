#include "send2_Online.h"
#include "../wallet/wallet.h"
#include "../windows/send2_online_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"

namespace state {

SendOnline::SendOnline(const StateContext &context):
        State(context, STATE::SEND_ONLINE ) {}

SendOnline::~SendOnline() {}

NextStateRespond SendOnline::execute() {
    if ( context.appContext->getActiveWndState() != STATE::SEND_ONLINE )
        return NextStateRespond(NextStateRespond::RESULT::DONE);


    wnd = new wnd::SendOnline( context.wndManager->getInWndParent(), this );

    context.wndManager->switchToWindow( wnd );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

QVector<wallet::AccountInfo> SendOnline::getWalletBalance() {
    return context.wallet->getWalletBalance();
}

QVector<wallet::WalletContact> SendOnline::getContacts() {
    return context.wallet->getContacts();
}

core::SendCoinsParams SendOnline::getSendCoinsParams() {
    return context.appContext->getSendCoinsParams();
}

void SendOnline::updateSendCoinsParams(const core::SendCoinsParams &params) {
    context.appContext->setSendCoinsParams(params);
}

// Request for MWC to send
void SendOnline::sendMwc(const wallet::AccountInfo &account, QString address, long mwcNano, QString message) {

    logger::logConnect("SendOnline", "onSend");
    sendConnect = QObject::connect(context.wallet, &wallet::Wallet::onSend,
                                               this, &SendOnline::sendRespond, Qt::QueuedConnection);
    core::SendCoinsParams prms = context.appContext->getSendCoinsParams();
    context.wallet->sendTo( account, mwcNano, address, message, prms.inputConfirmationNumber, prms.changeOutputs );
}

void SendOnline::sendRespond( bool success, QStringList errors ) {

    logger::logDisconnect("SendOnline", "onSend");
    QObject::disconnect(sendConnect);

    if (wnd) {
        wnd->sendRespond( success, errors );
    }

    if (success)
        context.stateMachine->setActionWindow(STATE::SEND_ONLINE_OFFLINE);
}


}
