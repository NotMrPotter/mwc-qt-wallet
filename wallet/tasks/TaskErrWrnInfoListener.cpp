#include "TaskErrWrnInfoListener.h"
#include <QDebug>
#include "../mwc713.h"

namespace wallet {

bool TaskErrWrnInfoListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];

    switch (evt.event) {
        case S_ERROR:
        case S_GENERIC_ERROR: {
            qDebug() << "TaskErrWrnInfoListener::processTask with events: " << printEvents(events);
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::CRITICAL, MWC713::MESSAGE_ID::GENERIC, evt.message );
            return true;
        }
        case S_GENERIC_WARNING: {
            qDebug() << "TaskErrWrnInfoListener::processTask with events: " << printEvents(events);
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::WARNING, MWC713::MESSAGE_ID::GENERIC, evt.message );
            return true;
        }
        case S_GENERIC_INFO: {
            qDebug() << "TaskErrWrnInfoListener::processTask with events: " << printEvents(events);
            wallet713->appendNotificationMessage(MWC713::MESSAGE_LEVEL::INFO, MWC713::MESSAGE_ID::GENERIC, evt.message );
            return true;
        }
        default:
            return false;
    }
}

}
