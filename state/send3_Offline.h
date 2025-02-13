#ifndef MWC_QT_WALLET_SEND3_OFFLINE_H
#define MWC_QT_WALLET_SEND3_OFFLINE_H

#include "state.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"

namespace wnd {
class SendOfflineFiles;
}

namespace state {

class SendOffline : public QObject, public State {
    Q_OBJECT
public:
    SendOffline(const StateContext & context);
    virtual ~SendOffline() override;

    QVector<wallet::AccountInfo> getWalletBalance();

    core::SendCoinsParams getSendCoinsParams();
    void updateSendCoinsParams( const core::SendCoinsParams  & params );

    void prepareSendMwcOffline( const wallet::AccountInfo & account, QString message );

    QString getFileGenerationPath();
    void updateFileGenerationPath(QString path);

    void sendToFile(long nanoCoins, QString fileName);
    //void signTransaction( QString fileName );
    void publishTransaction( QString fileName );

    void deletedSendOfflineFiles() {sendFilesWnd = nullptr;}
protected:
    virtual NextStateRespond execute() override;

    void respSendFile( bool success, QStringList errors, QString fileName );
    //void respReceiveFile( bool success, QStringList errors, QString inFileName );
    void respFinalizeFile( bool success, QStringList errors, QString fileName );

private:
    QMetaObject::Connection sendConnect;
    wnd::SendOfflineFiles * sendFilesWnd = nullptr;
    QString message;
};

}



#endif //MWC_QT_WALLET_SEND3_OFFLINE_H
