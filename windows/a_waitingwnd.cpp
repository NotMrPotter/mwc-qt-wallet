#include "a_waitingwnd.h"
#include "ui_a_waitingwnd.h"
#include <QMovie>
#include <QLabel>

namespace wnd {

WaitingWnd::WaitingWnd(QWidget *parent, QString title, QString progressMessage) :
    QWidget(parent),
    ui(new Ui::WaitingWnd)
{
    ui->setupUi(this);

    updateTitle(title);
    updateProgressMessage(progressMessage);

    ui->progress->initLoader(true);
}

WaitingWnd::~WaitingWnd()
{
    delete ui;
}

void WaitingWnd::updateTitle(QString title) {
    ui->titleLabel->setText("<html><body><p><span style=\" font-size:18pt; font-weight:600;\">" + title + "</span></p></body></html>");
}

void WaitingWnd::updateProgressMessage( QString message ) {
    ui->progressMsg->setText(message);
}

}
