#include "mmolch/qtutil_mount_watcher.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>

#include <fcntl.h>
#include <unistd.h>

namespace mmolch::qtutil {

MountWatcher::MountWatcher(QObject* parent)
    : QObject(parent)
{
    m_fd = ::open("/proc/self/mountinfo", O_RDONLY);
    if (m_fd == -1) {
        qFatal("Failed to open /proc/self/mountinfo: %s", strerror(errno));
    }
}

MountWatcher::~MountWatcher()
{
    setEnabled(false);
    if (m_fd != -1)
        ::close(m_fd);
}

void MountWatcher::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;

    if (enabled) {
        m_mounts = readMounts();

        m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Exception, this);
        connect(m_notifier, &QSocketNotifier::activated,
                this, &MountWatcher::updateMounts);
    } else {
        if (m_notifier) {
            m_notifier->setEnabled(false);
            m_notifier->deleteLater();
            m_notifier = nullptr;
        }
    }
}

QStringList MountWatcher::mounts() const
{
    QReadLocker lock{&m_mountsLock};
    return m_mounts;
}

namespace {
QString unescapeMountField(const QByteArray &field)
{
    QByteArray out;
    out.reserve(field.size());

    for (int i = 0; i < field.size(); ++i) {
        if (field[i] == '\\' && i + 3 < field.size()) {
            // Try to parse \XYZ (octal)
            char c1 = field[i+1];
            char c2 = field[i+2];
            char c3 = field[i+3];

            if (c1 >= '0' && c1 <= '7' &&
                c2 >= '0' && c2 <= '7' &&
                c3 >= '0' && c3 <= '7') {

                int value = (c1 - '0') * 64 +
                            (c2 - '0') * 8 +
                            (c3 - '0');

                out.append(char(value));
                i += 3;
                continue;
            }
        }

        out.append(field[i]);
    }

    return QString::fromLocal8Bit(out);
}
}

QStringList MountWatcher::readMounts() const
{
    QStringList result;

    QFile file(QStringLiteral("/proc/self/mountinfo"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    for (QByteArray line = file.readLine(); !line.isEmpty(); line = file.readLine()) {
        line = line.trimmed();
        if (line.isEmpty())
            continue;

        QList<QByteArray> parts = line.split(' ');
        if (parts.size() < 5)
            continue;

        const QByteArray mountField = parts[4];
        result << unescapeMountField(mountField);
    }

    return result;
}

void MountWatcher::updateMounts()
{
    const QStringList current = readMounts();

    for (const QString& m : current) {
        if (!m_mounts.contains(m))
            emit mountAdded(m);
    }

    for (const QString& m : std::as_const(m_mounts)) {
        if (!current.contains(m))
            emit mountRemoved(m);
    }

    QWriteLocker lock{&m_mountsLock};
    m_mounts = current;
}

// ---------------------------------------------------------------------------
// MountWatcherThread
// ---------------------------------------------------------------------------

MountWatcherThread::MountWatcherThread(QObject* parent)
    : QObject(parent)
{
    m_watcher = new MountWatcher();
    m_watcher->moveToThread(&m_thread);

    connect(&m_thread, &QThread::started,
            m_watcher, [this](){ m_watcher->setEnabled(true); });

    connect(&m_thread, &QThread::finished,
            m_watcher, [this](){ m_watcher->setEnabled(false); });

    connect(&m_thread, &QThread::finished,
            m_watcher, &QObject::deleteLater);
}

MountWatcherThread::~MountWatcherThread()
{
    stop();
}

void MountWatcherThread::start()
{
    if (!m_thread.isRunning())
        m_thread.start();
}

void MountWatcherThread::stop()
{
    if (m_thread.isRunning()) {
        m_thread.quit();
        m_thread.wait();
    }
}

} // namespace mmolch::qtutil
