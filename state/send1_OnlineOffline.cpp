#include "send1_OnlineOffline.h"
#include "../wallet/wallet.h"
#include "../windows/send1_onlineoffline_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"


namespace state {

SendOnlineOffline::SendOnlineOffline(const StateContext & context) :
        State(context, STATE::SEND_ONLINE_OFFLINE ) {}

SendOnlineOffline::~SendOnlineOffline() {}

NextStateRespond SendOnlineOffline::execute() {
    if ( context.appContext->getActiveWndState() != STATE::SEND_ONLINE_OFFLINE )
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    nextStep = STATE::NONE;
    wnd = new wnd::SendOnlineOffline( context.wndManager->getInWndParent(), this );
    context.wndManager->switchToWindow( wnd );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

void SendOnlineOffline::SendOnlineChosen() {
    nextStep = STATE::SEND_ONLINE;
    updateBalanceAndContinue();
}

void SendOnlineOffline::SendOfflineChosen()  {
    nextStep = STATE::SEND_OFFLINE;
    updateBalanceAndContinue();
}

void SendOnlineOffline::updateBalanceAndContinue() {
    wnd->showProgress();

    // Updating the wallet balance
    logger::logConnect("SendOnlineOffline", "onWalletBalanceUpdated" );
    slotConn = QObject::connect( context.wallet, &wallet::Wallet::onWalletBalanceUpdated,
                                 this, &SendOnlineOffline::onWalletBalanceUpdated, Qt::QueuedConnection );
    context.wallet->updateWalletBalance();
}

// Account info is updated
void SendOnlineOffline::onWalletBalanceUpdated() {
    logger::logDisconnect("SendOnlineOffline", "onWalletBalanceUpdated" );
    QObject::disconnect(slotConn);

    context.stateMachine->setActionWindow(nextStep );
}




}

