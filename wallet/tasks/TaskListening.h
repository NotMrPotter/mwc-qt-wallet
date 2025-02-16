#ifndef MWC_QT_WALLET_TASKLISTENING_H
#define MWC_QT_WALLET_TASKLISTENING_H

#include "../mwc713task.h"

namespace wallet {

// It is listener task. No input can be defined.
// Listening for MWC MQ & keybase connection statuses
class TaskListeningListener : public Mwc713Task {
public:
    const static long TIMEOUT = 3600*1000*5; // 5 hours should be enough

    TaskListeningListener( MWC713 *wallet713 ) :
            Mwc713Task("TaskListeningListener", "", wallet713,"") {}

    virtual ~TaskListeningListener() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>();}
};


class TaskListeningStart : public Mwc713Task {
public:
    const static long TIMEOUT = 7000;

    // Start one listen per request. mwc713 doesn't support both
    TaskListeningStart(MWC713 *wallet713, bool startMq, bool startKeybase) :
            Mwc713Task("TaskListeningStart", calcCommand(startMq, startKeybase), wallet713,"") {
        Q_ASSERT(startMq|startKeybase); Q_ASSERT( (startMq &&startKeybase) == false);
    }

    virtual ~TaskListeningStart() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString calcCommand(bool startMq, bool startKeybase) const;
};



class TaskListeningStop : public Mwc713Task {
public:
    const static long TIMEOUT = 8000;

    // Start one listen per request. mwc713 doesn't support both
    TaskListeningStop(MWC713 *wallet713, bool stopMq, bool stopKeybase) :
            Mwc713Task("TaskListeningStop", calcCommand(stopMq, stopKeybase), wallet713,"") {
        Q_ASSERT(stopMq | stopKeybase); Q_ASSERT( (stopMq && stopKeybase) == false);
    }

    virtual ~TaskListeningStop() override {}

    virtual bool processTask(const QVector<WEvent> &events) override;

    virtual QSet<WALLET_EVENTS> getReadyEvents() override {return QSet<WALLET_EVENTS>{ WALLET_EVENTS::S_READY };}
private:
    QString calcCommand(bool stopMq, bool stopKeybase) const;
};


}

#endif //MWC_QT_WALLET_TASKLISTENING_H
