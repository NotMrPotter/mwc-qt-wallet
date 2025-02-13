#include "TaskListening.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

// ------------------------------- TaskListeningListener ------------------------------------------

bool TaskListeningListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
        case S_LISTENER_MQ_ON: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(true);
            wallet713->setMwcAddress(evt.message);
            return true;
        }
        case S_LISTENER_KB_ON: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setKeybaseListeningStatus(true);
            return true;
        }
        case S_LISTENER_MQ_OFF: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(false);
            return true;
        }
        case S_LISTENER_KB_OFF: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setKeybaseListeningStatus(false);
            return true;
        }
        case S_LISTENER_MQ_LOST_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(false);
            return true;
        }
        case S_LISTENER_MQ_GET_CONNECTION: {
            qDebug() << "TaskListeningListener::processTask with events: " << printEvents(events);
            wallet713->setMwcMqListeningStatus(true);
            return true;
        }
        default:
            return false;
    }
}

// ----------------------------------- TaskListeningStart --------------------------------------

bool TaskListeningStart::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskListeningStart::processTask with events: " << printEvents(events);

    QVector< WEvent > mqStaring = filterEvents(events, WALLET_EVENTS::S_LISTENER_MQ_STARTING );
    QVector< WEvent > kbStaring = filterEvents(events, WALLET_EVENTS::S_LISTENER_KB_STARTING );

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    wallet713->setListeningStartResults( mqStaring.size()>0, kbStaring.size()>0, // what we try to start
                                         errorMessages );

    return true;
}

QString TaskListeningStart::calcCommand(bool startMq, bool startKeybase) const {
    Q_ASSERT(startMq | startKeybase);

    return QString("listen") + (startMq ? " -m" : "") + (startKeybase ? " -k" : "");
}

// -------------------------------- TaskListeningStop -------------------------------

bool TaskListeningStop::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskListeningStop::processTask with events: " << printEvents(events);

    QVector< WEvent > mqStopping = filterEvents(events, WALLET_EVENTS::S_LISTENER_MQ_STOPPING );
    QVector< WEvent > kbStopping = filterEvents(events, WALLET_EVENTS::S_LISTENER_KB_STOPPING );

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    wallet713->setListeningStopResult( mqStopping.size()>0, kbStopping.size()>0,
                                       errorMessages );

    return true;
}

QString TaskListeningStop::calcCommand(bool stopMq, bool stopKeybase) const {
    Q_ASSERT(stopMq | stopKeybase);

    return QString("stop") + (stopMq ? " -m" : "") + (stopKeybase ? " -k" : "");
}


}


