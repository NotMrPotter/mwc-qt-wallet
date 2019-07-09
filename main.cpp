#include "core/mainwindow.h"
#include <QApplication>
#include "core/windowmanager.h"
#include "wallet/mockwallet.h"
#include "wallet/mwc713.h"
#include "state/state.h"
#include "state/statemachine.h"
#include "core/appcontext.h"
#include "util/ioutils.h"
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "core/global.h"
#include "util/ioutils.h"
#include "util/Log.h"
#include "core/Config.h"
#include "util/ConfigReader.h"
#include <QFileDevice>

// Very first run - init everything
bool deployFilesFromResources() {
    QString confPath = ioutils::getAppDataPath();

    QString mwc713conf = confPath + "/wallet713v2.toml";
    QString mwcGuiWalletConf = confPath + "/mwc-gui-wallet.conf";

    bool ok = true;

    if ( !QFile::exists(mwc713conf)) {
#ifdef Q_OS_MACOS
        ok = ok && QFile::copy(":/resource/wallet713_mac.toml", mwc713conf);
#else
        ok = ok && QFile::copy(":/resource/wallet713_def.toml", mwc713conf);
#endif
        if (ok)
            QFile::setPermissions(mwc713conf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    if ( !QFile::exists(mwcGuiWalletConf)) {
        ok = ok && QFile::copy(":/resource/mwc-gui-wallet.conf", mwcGuiWalletConf);
        if (ok)
            QFile::setPermissions(mwcGuiWalletConf, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup);
    }

    // Set default values
    core::Config::setMwc713conf(mwc713conf);
    core::Config::setMwcGuiWalletConf(mwcGuiWalletConf);

    return ok;
}

// Read configs
bool readConfig(QApplication & app) {
    QCoreApplication::setApplicationName("mwc-qt-wallet");
    QCoreApplication::setApplicationVersion("v0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("GUI wallet for MWC (MimbleWimbleCoin) https://www.mwc.mw/");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOptions({
                              {{"c", "config"},
                                      "Path to the mwc-gui-wallet config ",
                                      "mwc713 path",
                                      ""},
                      });

    parser.process(app);

    QString config = parser.value("config");
    if (config.isEmpty()) {
        config = core::Config::getMwcGuiWalletConf();
    }
    else {
        core::Config::setMwcGuiWalletConf(config);
    }

    util::ConfigReader reader;
    if ( !reader.readConfig(config) ) {
        qDebug() << "Failed to read config file " << config;
        return false;
    }

    QString mwc_path = reader.getString("mwc_path");
    QString wallet713_path = reader.getString("wallet713_path");
    QString main_style_sheet = reader.getString("main_style_sheet");
    QString dialogs_style_sheet = reader.getString("dialogs_style_sheet");

    if ( mwc_path.isEmpty() || wallet713_path.isEmpty() || main_style_sheet.isEmpty() || dialogs_style_sheet.isEmpty()) {
        qDebug() << "Failed to read all expected data from config file " << config;
        return false;
    }

    if (wallet713_path == "build in") {
        wallet713_path = QCoreApplication::applicationDirPath() + "/" + "mwc713";
#ifdef Q_OS_WIN
        wallet713_path += ".exe";
#endif
    }

    if (mwc_path == "build in") {
        mwc_path = QCoreApplication::applicationDirPath() + "/" + "mwc";
#ifdef Q_OS_WIN
        mwc_path += ".exe";
#endif
    }

    core::Config::setConfigData( mwc_path, wallet713_path, main_style_sheet, dialogs_style_sheet);
    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // -------------------------------------------
    // Check envoronment variables
/*    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QStringList kk = env.keys();
        QString reportStr = "Envirinment variable for this process at starting point:\n";
        for ( const auto & k : kk ) {
            QString val = env.value( k );
            reportStr +=  "'" + k + "' => '" + val + "'\n";
        }
        QMessageBox::information(nullptr, "Environment variables", reportStr);
    }*/
    // -------------------------------------------


    logger::initLogger();

    if (!deployFilesFromResources() ) {
        QMessageBox::critical(nullptr, "Error", "Unable to provision or verify resource files during the first run");
        return 1;
    }

    if (!readConfig(app) ) {
        QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read configuration");
        return 1;
    }

    qDebug().noquote() << "Starting mwc-gui-wallet with config:\n" << core::Config::toString();


    { // Apply style sheet
        QFile file( core::Config::getMainStyleSheetPath() );
        if (file.open(QFile::ReadOnly | QFile::Text)) {
               QTextStream ts(&file);
               app.setStyleSheet(ts.readAll());
        }
        else {
            QMessageBox::critical(nullptr, "Error", "MWC GUI Wallet unable to read the stylesheet.");
            return 1;
        }
    }

    core::AppContext appContext;

    //main window has delete on close flag. That is why need to
    // create dynamically. Window will be deleted on close
    core::MainWindow * mainWnd = new core::MainWindow(nullptr);

    mwc::setApplication(&app, mainWnd);

    wallet::MWC713 wallet( core::Config::getWallet713path(), core::Config::getMwc713conf(), &appContext );
    //wallet::MockWallet wallet;

    core::WindowManager wndManager( mainWnd->getMainWindow() );

    mainWnd->show();

    state::StateContext context( &appContext, &wallet, &wndManager, mainWnd );

    state::StateMachine machine(context);
    mainWnd->setAppEnvironment(&machine, &wallet);
    machine.start();

    app.exec();
  }

