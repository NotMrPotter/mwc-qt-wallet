#include "c_createwithseed.h"
#include "../wallet/wallet.h"
#include "../windows/c_enterseed.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../control/messagebox.h"
#include "../windows/c_progresswnd.h"
#include "../util/Log.h"
#include <QDebug>
#include <QThread>
#include "../control/messagebox.h"

namespace state {


CreateWithSeed::CreateWithSeed(const StateContext & context) :
    State(context, STATE::CREATE_WITH_SEED) {
}

CreateWithSeed::~CreateWithSeed() {
}

NextStateRespond CreateWithSeed::execute() {
    QString withSeed = context.appContext->pullCookie<QString>("withSeed");
    if (withSeed.length()==0)
        return NextStateRespond(NextStateRespond::RESULT::DONE);

    context.wndManager->switchToWindow(
                new wnd::EnterSeed( context.wndManager->getInWndParent(), this ) );

    return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
}

// Second Step, switching to the progress and starting this process at mwc713
void CreateWithSeed::createWalletWithSeed( QVector<QString> seed ) {
    seed2recover = seed;
    pass2recover = context.appContext->getCookie<QString>(COOKIE_PASSWORD);
    Q_ASSERT(pass2recover.length() > 0);

    auto walletListenerStatus = context.wallet->getListeningStatus();
    mwcMqOriginalState = walletListenerStatus.first;
    keybaseOriginalState = walletListenerStatus.second;

    // switching to a progress Wnd
    progressWnd = new wnd::ProgressWnd(context.wndManager->getInWndParent(), "Recovering account from the passphrase", "",
                                       "");
    context.wndManager->switchToWindow(progressWnd);

    // Creating connections...
    logger::logConnect("CreateWithSeed", "onListeningStopResult");
    connListeningStopResult = QObject::connect(context.wallet, &wallet::Wallet::onListeningStopResult,
                                               this, &CreateWithSeed::onListeningStopResult, Qt::QueuedConnection);

    logger::logConnect("CreateWithSeed", "onRecoverProgress");
    connRecoverProgress = QObject::connect(context.wallet, &wallet::Wallet::onRecoverProgress,
                                           this, &CreateWithSeed::onRecoverProgress, Qt::QueuedConnection);

    logger::logConnect("CreateWithSeed", "onRecoverResult");
    connRecoverResult = QObject::connect(context.wallet, &wallet::Wallet::onRecoverResult,
                                         this, &CreateWithSeed::onRecoverResult, Qt::QueuedConnection);

    // Stopping listeners first. Not checking if they are running.
    progressWnd->setMsgPlus("Preparing for recovery...");
    qDebug() << "Stopping MQ listener...";
    context.wallet->listeningStop( true, false );
    // continue at onListeningStopResult
}

void CreateWithSeed::onListeningStopResult( bool mqTry, bool kbTry, // what we try to stop
                           QStringList errorMessages ) {
    Q_UNUSED(errorMessages);
    qDebug() << "Stopping listener get: mqTry=" << mqTry << " kbTry=" << kbTry;
    if (mqTry) {
        // continue with keybase
        qDebug() << "Stopping KeyBase listener...";
        context.wallet->listeningStop( false, true );
    }
    else {
        // Stoping is done. Let's start recovery
        QThread::sleep(1); // Let's wait for mwc713 (1 second is enough)
        qDebug() << "Starting recovery";
        progressWnd->setMsgPlus("Recovering your wallet...");
        context.wallet->recover(seed2recover, pass2recover);
    }
}

void CreateWithSeed::onRecoverProgress( int progress, int maxVal ) {
    progressMaxVal = maxVal;
    progressWnd->initProgress(0, maxVal);

    QString msgProgress = "Recovering..." + QString::number(progress * 100 / maxVal) + "%";
    progressWnd->updateProgress(progress, msgProgress);
    progressWnd->setMsgPlus("");
}

void CreateWithSeed::onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages) {
    // start listening first since it will be async and we don't need to wait
    if (mwcMqOriginalState)
        context.wallet->listeningStart( true, false );
    if (keybaseOriginalState)
        context.wallet->listeningStart( false, true );

    // Removing connections...
    logger::logDisconnect("CreateWithSeed", "onListeningStopResult");
    QObject::disconnect(connListeningStopResult);

    logger::logDisconnect("CreateWithSeed", "onRecoverProgress");
    QObject::disconnect(connRecoverProgress);

    logger::logDisconnect("CreateWithSeed", "onRecoverResult");
    QObject::disconnect(connRecoverResult);

    if (finishedWithSuccess)
        progressWnd->updateProgress(progressMaxVal, "Done");

    QString errorMsg;
    if (errorMessages.size()>0) {
        errorMsg += "\nErrors:";
        for (auto & s : errorMessages)
            errorMsg += "\n" + s;
    }

    bool success = false;

    if (!started) {
        control::MessageBox::message(nullptr, "Recover failure", "Account recovery failed to start." + errorMsg);
    }
    else if (!finishedWithSuccess) {
        control::MessageBox::message(nullptr, "Recover failure", "Account recovery failed to finish." + errorMsg);
    }
    else {
        success = true;
        control::MessageBox::message(nullptr, "Success", "Your account was successfully recovered from the passphrase." + errorMsg);
    }

    if (success) {
        // Great, we are done here.
        // Must wait for balance update

        // Start Balance update
        logger::logConnect("CreateWithSeed", "onWalletBalanceUpdated");
        connWalletBalanceUpdated = QObject::connect(context.wallet, &wallet::Wallet::onWalletBalanceUpdated,
                                                    this, &CreateWithSeed::onWalletBalanceUpdated, Qt::QueuedConnection);

        context.wallet->updateWalletBalance();

        progressWnd->updateProgress(0,"");
        progressWnd->setMsgPlus("Recovering your wallet...");

        return;
    }
    else {
        // switch back to the seed window
        context.wndManager->switchToWindow(
                new wnd::EnterSeed( context.wndManager->getInWndParent(), this ) );
        return;
    }
}

// Account info is updated
void CreateWithSeed::onWalletBalanceUpdated() {
    logger::logDisconnect("InputPassword", "onWalletBalanceUpdated" );
    QObject::disconnect(connWalletBalanceUpdated);

    context.stateMachine->executeFrom(STATE::CREATE_WITH_SEED);
}

void CreateWithSeed::cancel() {
    context.stateMachine->executeFrom(STATE::NEW_WALLET);
}



}
