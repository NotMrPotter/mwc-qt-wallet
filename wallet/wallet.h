#ifndef MWCWALLET_H
#define MWCWALLET_H

#include <QVector>
#include <QString>
#include "../core/mwcexception.h"
#include "../util/ioutils.h"
#include "../util/stringutils.h"
#include <QDateTime>
#include <QObject>

namespace wallet {

struct AccountInfo {
    QString accountName = "default";
    long height = 0;
    // in nano coins
    long total = 0;
    long awaitingConfirmation = 0;
    long lockedByPrevTransaction = 0;
    long currentlySpendable = 0;

    long mwcNodeHeight = 0;
    bool mwcServerBroken = true;

    void setData(QString account,
        long total,
        long awaitingConfirmation,
        long lockedByPrevTransaction,
        long currentlySpendable,
        long mwcNodeHeight,
        bool mwcServerBroken);
};

struct WalletConfig {
    QString dataPath = "wallet";
    QString mwcboxDomain = "mwcbox.mwc.mw";
    int     mwcboxPort = 12456;
    QString mwcNodeURI = "127.0.0.1:5432";
    QString mwcNodeSecret = "3257683476";

    void saveData( QDataStream & out) const;
    bool loadData( QDataStream & in);
};

struct NodeStatus {
    int     connection; // current number of connections
    QString tip; // Status og the current fork tip
    long    height; // Height of the tip
    long    total_difficulty; // Total difficulty accumulated on that fork since genesis block

    void setData(int   connection,
        QString tip,
        long height,
        long total_difficulty);
};

struct WalletContact {
    QString name;
    QString address;

    void setData(QString name,
        QString address);

    void saveData( QDataStream & out) const;
    bool loadData( QDataStream & in);
};

struct WalletOutput {
    enum STATUS {Unconfirmed, Confirmed};

    QString outputCommitment;
    long    MMRIndex;
    long    lockHeight;
    bool    lockedUntil;
    STATUS  status;
    bool    coinbase;
    long    numOfConfirms;
    long    valueNano;
    long    txIdx;

    void setData(QString outputCommitment,
            long    MMRIndex,
            long    lockHeight,
            bool    lockedUntil,
            STATUS  status,
            bool    coinbase,
            long    numOfConfirms,
            long    valueNano,
            long    txIdx);

    // return status value as a string
    QString getStatusStr() const;
};

struct WalletTransaction {
    enum TRANSACTION_TYPE { NONE=0, SEND=1, RECIEVE=2, CANCELLED=0x8000};

    long    txIdx;
    uint    transactionType;
    QString txid;
    QString address;
    QString creationTime;
    bool    confirmed;
    QString confirmationTime;
    long    coinNano; // Net diffrence
    bool    proof;

    void setData(long txIdx,
        uint    transactionType,
        QString txid,
        QString address,
        QString creationTime,
        bool    confirmed,
        QString confirmationTime,
        long    coinNano,
        bool    proof);

    // mark transaction as cancelled
    void cancelled() {
        transactionType |= TRANSACTION_TYPE::CANCELLED;
    }

    QString getTypeAsStr() const {
        QString res;

        if ( transactionType & TRANSACTION_TYPE::SEND )
            res += "Send";
        if ( transactionType & TRANSACTION_TYPE::RECIEVE )
            res += "Recieve";

        if ( transactionType & TRANSACTION_TYPE::CANCELLED ) {
            if (!res.isEmpty())
                res += ", ";
            res += "Cancelled";
        }

        return res;
    }

    QString toStringShort() {
        using namespace util;

        return expandStrR( QString::number(txIdx), 3) +
                expandStrR(nano2one(coinNano), 8) +
                expandStrR( string2shortStrR(txid, 12), 12) +
                " " + creationTime;
    }
};

struct WalletProofInfo {
    bool    successed = false;
    QString errorMessage;
    long    coinsNano = 0;
    QString fromAddress;
    QString toAddress;
    QString output;
    QString kernel;

    void setDataSuccess(long coinsNano,
        QString fromAddress,
        QString toAddress,
        QString output,
        QString kernel);

    void setDataFailure(QString errorMessage);
};

struct WalletUtxoSignature {
    long coinNano; // Output amount
    QString messageHash;
    QString pubKeyCompressed;
    QString messageSignature;

    void setData(long _coinNano, // Output amount
            QString _messageHash,
            QString _pubKeyCompressed,
            QString _messageSignature);
};


struct WalletNotificationMessages {
    enum LEVEL {ERROR, WARNING, INFO, DEBUG};
    LEVEL level=DEBUG;
    QString message;
    QDateTime time;

    WalletNotificationMessages() {time=QDateTime::currentDateTime();}
    WalletNotificationMessages(LEVEL _level, QString _message) : level(_level), message(_message) {time=QDateTime::currentDateTime();}
    WalletNotificationMessages(const WalletNotificationMessages&) = default;
    WalletNotificationMessages &operator=(const WalletNotificationMessages&) = default;

    // To debug string
    QString toString() const;
};

enum InitWalletStatus {NONE, NEED_PASSWORD, NEED_INIT, WRONG_PASSWORD, READY};
QString toString(InitWalletStatus status);

// Interface to wallet functionality
// can throw MwcException to signal errors
class Wallet : public QObject
{
    Q_OBJECT
public:
    // network: main | floo
    Wallet();
    virtual ~Wallet();

    // Generic. Reporting fatal error that somebody will process and exit app
    virtual void reportFatalError( QString message ) noexcept(false) = 0;


    // Get all notification messages
    virtual const QVector<WalletNotificationMessages> & getWalletNotificationMessages() noexcept(false) = 0;
    // Check signal: onNewNotificationMessage

    // ---- Wallet Init Phase
    virtual void start() noexcept(false)  = 0;
    virtual void loginWithPassword(QString password, QString account) noexcept(false)  = 0;
    // Check signal: onInitWalletStatus
    virtual InitWalletStatus getWalletStatus() noexcept(false) = 0;

    // Close the wallet and release the process
    virtual bool close() noexcept(false) = 0;

    // Create the wallet, generate the seed. Return the words to recover the wallet
    virtual void generateSeedForNewAccount(QString password) noexcept(false) = 0;
    // Check signal: onNewSeed( seed [] )


    // Confirm that user write the passphase
    virtual void confirmNewSeed() noexcept(false) = 0;


    // Recover the wallet with a mnemonic phrase
    // recover wallet with a passphrase:
    virtual void recover(const QVector<QString> & seed, QString password) noexcept(false)  = 0;
    // Check Signals: onRecoverProgress( int progress, int maxVal );
    // Check Signals: onRecoverResult(bool ok, QString newAddress );

    //--------------- Listening

    // Checking if wallet is listening through services
    // return:  <mwcmq status>, <keybase status>.   true mean online, false - offline
    virtual QPair<bool,bool> getListeningStatus() noexcept(false) = 0;

    // Start listening through services
    virtual void listeningStart(bool startMq, bool startKb) noexcept(false) = 0;
    // Check Signal: onListeningStartResults

    // Stop listening through services
    virtual void listeningStop(bool stopMq, bool stopKb) noexcept(false) = 0;
    // Check signal: onListeningStopResult

    // Get latest Mwc MQ address that we see
    virtual QString getLastKnownMwcBoxAddress() noexcept(false) = 0;

    // Get MWC box <address, index in the chain>
    virtual void getMwcBoxAddress() noexcept(false) = 0;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Change MWC box address to another from the chain. idx - index in the chain.
    virtual void changeMwcBoxAddress(int idx) noexcept(false) = 0;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);

    // Generate next box address
    virtual void nextBoxAddress() noexcept(false) = 0;
    // Check signal: onMwcAddressWithIndex(QString mwcAddress, int idx);


    // -------------- Accounts

    // Get all accounts with balances. Expected that Wallet allways maintain them in a cache.
    // This info needed in many cases and we don't want spend time every time for that.
    virtual QVector<AccountInfo> getWalletBalance() noexcept(false) = 0;

    virtual QString getCurrentAccountName() noexcept(false) = 0;

    // Request Wallet balance update. It is a multistep operation
    virtual void updateWalletBalance() noexcept(false) = 0;
    // Check signal: onWalletBalanceUpdated
    //          onWalletBalanceProgress
    //          onAccountSwitched - multiple calls, please ignore


    // Create another account, note no delete exist for accounts
    virtual void createAccount( const QString & accountName ) noexcept(false) = 0;
    // Check Signal:  onAccountCreated

    // Switch to different account
    virtual void switchAccount(const QString & accountName) noexcept(false) = 0;
    // Check Signal: onAccountSwitched

    // -------------- Maintaince

    // Check and repair the wallet. Will take a while
    virtual void check() noexcept(false) = 0;

    // Get current configuration of the wallet.
    virtual WalletConfig getWalletConfig() noexcept(false) = 0;
    // Update wallet config. Will be updated with non empty fields
    virtual QPair<bool, QString> setWalletConfig(const WalletConfig & config) noexcept(false) = 0;

    // Status of the node
    virtual NodeStatus getNodeStatus() noexcept(false) = 0;

    // -------------- Transactions

    // Get wallet balance
    // Cancel transaction
    virtual bool cancelTransacton(QString transactionID) noexcept(false) = 0;

    // Generating transaction proof for mwcbox transaction. This transaction must be broadcasted to the chain
    virtual WalletProofInfo  generateMwcBoxTransactionProof( long transactionId, QString resultingFileName ) noexcept(false) = 0;
    // Verify the proof for transaction
    virtual WalletProofInfo  verifyMwcBoxTransactionProof( QString proofFileName ) noexcept(false) = 0;

    // Init send transaction with file output
    // Check signal:  onSendFile
    virtual void sendFile( long coinNano, QString fileTx ) noexcept(false) = 0;
    // Recieve transaction. Will generate *.response file in the same dir
    // Check signal:  onReceiveFile
    virtual void receiveFile( QString fileTx) noexcept(false) = 0;
    // finalize transaction and broadcast it
    // Check signal:  onFinalizeFile
    virtual void finalizeFile( QString fileTxResponse ) noexcept(false) = 0;

    // Send some coins to address.
    // Before send, wallet always do the switch to account to make it active
    virtual void sendTo( const wallet::AccountInfo &account, long coinNano, const QString & address, QString message="", int inputConfirmationNumber=-1, int changeOutputs=-1 ) noexcept(false) = 0;
    // Check signal:  onSend

    // Show outputs for the wallet
    virtual QVector<WalletOutput> getOutputs() noexcept(false) = 0;
    // Show all transactions for current account
    virtual void getTransactions() noexcept(false) = 0;
    // Check Signal: onTransactions( QString account, long height, QVector<WalletTransaction> Transactions)

    // -------------- Contacts

    // Get the contacts
    virtual QVector<WalletContact> getContacts() noexcept(false) = 0;
    // Add new contact
    virtual QPair<bool, QString> addContact( const WalletContact & contact ) noexcept(false) = 0;
    // Remove contact. return false if not found
    virtual QPair<bool, QString> deleteContact( const QString & name ) noexcept(false) = 0;

    // ----------- HODL
    // https://github.com/mimblewimble/grin/pull/2374

    // sign output commitment utxo. Hash must be provivded by party that want to verify the signature.
    // Late this signature can be used for verification of ounewship
    virtual WalletUtxoSignature sign_utxo( const QString & utxo, const QString & hash ) = 0;

private:
signals:
    // Notification/error message
    void onNewNotificationMessage(WalletNotificationMessages::LEVEL level, QString message);

    // Update of the wallet status
    void onInitWalletStatus(InitWalletStatus status);

    // Get MWC updated address. Normally you don't need that
    void onMwcAddress(QString mwcAddress);

    // Get MWC MQ address with index
    void onMwcAddressWithIndex(QString mwcAddress, int idx);

    // New seed generated by the wallet
    void onNewSeed(QVector<QString> seed);

    void onRecoverProgress( int progress, int maxVal );
    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

    // Listening, you will not be able to get a results
    void onListeningStartResults( bool mqTry, bool kbTry, // what we try to start
                                   QStringList errorMessages ); // error messages, if get some

    void onListeningStopResult(bool mqTry, bool kbTry, // what we try to stop
                                QStringList errorMessages );

    // Account info is updated
    void onWalletBalanceUpdated();
    // Progress for balance update
    void onWalletBalanceProgress( int progress, int maxVal );

    void onAccountSwitched(QString currentAccountName);
    void onAccountCreated( QString newAccountName);

    // Send results
    void onSend( bool success, QStringList errors );

    // Files operations
    void onSendFile( bool success, QStringList errors, QString fileName );
    void onReceiveFile( bool success, QStringList errors, QString inFileName, QString outFn );
    void onFinalizeFile( bool success, QStringList errors, QString fileName );

    // Transactions
    void onTransactions( QString account, long height, QVector<WalletTransaction> Transactions);

    // Listener status listeners...
    void onMwcMqListenerStatus(bool online);
    void onKeybaseListenerStatus(bool online);
};

}

Q_DECLARE_METATYPE(wallet::WalletNotificationMessages::LEVEL);
Q_DECLARE_METATYPE(wallet::InitWalletStatus);
Q_DECLARE_METATYPE(wallet::WalletTransaction);

#endif // MWCWALLET_H
