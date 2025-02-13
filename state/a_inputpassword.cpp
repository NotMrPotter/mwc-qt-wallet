#include "state/a_inputpassword.h"
#include "windows/a_inputpassword_w.h"
#include "../wallet/wallet.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"

namespace state {

InputPassword::InputPassword(const StateContext & context) :
    State(context, STATE::INPUT_PASSWORD)
{
}

InputPassword::~InputPassword() {
}

NextStateRespond InputPassword::execute() {
    auto status = context.wallet->getWalletStatus();
    if ( status == wallet::InitWalletStatus::NEED_PASSWORD ||
            status == wallet::InitWalletStatus::WRONG_PASSWORD )
    {
        wnd = new wnd::InputPassword( context.wndManager->getInWndParent(), this );
        context.wndManager->switchToWindow(wnd);

        logger::logConnect("InputPassword", "onInitWalletStatus" );
        slotConn = QObject::connect( context.wallet, &wallet::Wallet::onInitWalletStatus, this, &InputPassword::onInitWalletStatus, Qt::QueuedConnection );

        return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void InputPassword::submitPassword(const QString & password) {
    Q_ASSERT(wnd != nullptr);
    if (wnd) {
        wnd->startWaiting();
    }

    context.wallet->loginWithPassword(password, context.appContext->getCurrentAccount());
}

void InputPassword::onInitWalletStatus( wallet::InitWalletStatus  status ) {
    logger::logRecieve("InputPassword", "onInitWalletStatus", toString(status) );

    if (status == wallet::InitWalletStatus::WRONG_PASSWORD ) {
        Q_ASSERT(wnd != nullptr);
        if (wnd) {
            wnd->stopWaiting();
            wnd->reportWrongPassword();
        }
    } else if (status == wallet::InitWalletStatus::READY ) {
        // Great, login is done. Now we can use the wallet
        Q_ASSERT(context.wallet->getWalletStatus() == wallet::InitWalletStatus::READY);
        logger::logDisconnect("InputPassword", "onInitWalletStatus" );
        QObject::disconnect(slotConn);

        // Start listening, no feedback interested
        context.wallet->listeningStart(true, false);
        context.wallet->listeningStart(false, true);

        // Updating the wallet balance
        logger::logConnect("InputPassword", "onWalletBalanceUpdated" );
        slotConn = QObject::connect( context.wallet, &wallet::Wallet::onWalletBalanceUpdated, this, &InputPassword::onWalletBalanceUpdated, Qt::QueuedConnection );
        context.wallet->updateWalletBalance();

    }
    else {
        // seems like fatal error. Unknown state
        context.wallet->reportFatalError("Internal error. Wallet Password verification invalid state.");
    }
}

// Account info is updated
void InputPassword::onWalletBalanceUpdated() {
    logger::logDisconnect("InputPassword", "onWalletBalanceUpdated" );
    QObject::disconnect(slotConn);

    context.stateMachine->executeFrom(STATE::INPUT_PASSWORD);
}


}
