#pragma once

#include <QLoggingCategory>
#include <QObject>
#include <QReadWriteLock>
#include <QStringList>
#include <QThread>

class QSocketNotifier;

namespace mmolch::qtutil {

Q_DECLARE_LOGGING_CATEGORY(lcMountWatcher)

class MountWatcher : public QObject
{
    Q_OBJECT
public:
    explicit MountWatcher(QObject *parent = nullptr);
    ~MountWatcher();

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    QStringList readMounts() const;
    void updateMounts();

    QStringList mounts() const;

signals:
    void mountAdded(const QString &mountPoint);
    void mountRemoved(const QString &mountPoint);

private:
    bool m_enabled = false;
    int m_fd = -1;
    QSocketNotifier* m_notifier = nullptr;
    QStringList m_mounts;
    mutable QReadWriteLock m_mountsLock;
};

class MountWatcherThread : public QObject
{
    Q_OBJECT
public:
    explicit MountWatcherThread(QObject* parent = nullptr);
    ~MountWatcherThread();

    MountWatcher* watcher() const { return m_watcher; }

    void start();
    void stop();

private:
    QThread m_thread;
    MountWatcher* m_watcher = nullptr;
};

} // namespace mmolch::qtutil
