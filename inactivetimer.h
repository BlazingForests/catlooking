#ifndef INACTIVETIMER_H
#define INACTIVETIMER_H

#include <QObject>
#include <QTimer>

class InactiveTimer : public QObject
{
    Q_OBJECT
public:
    explicit InactiveTimer(int msecInactivityTimeout, QObject *parent);
    bool isActionPresence();

public slots:
    void startTicker();
    void notifyActivity();

signals:
    void inactivityDetected();

private:
    int inactivityTimeout;
    QTimer *ticker;
    int currentTicker;
    int static const totalTickerSteps;
    void clearTickerSteps();

private slots:
    void tick();

};

#endif // INACTIVETIMER_H
