#include <QCoreApplication>
#include <QDebug>
#include "mmolch/qtutil_mount_watcher.h"

using namespace mmolch::qtutil;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    auto mount_watcher = new MountWatcher{&app};

    QObject::connect(mount_watcher, &MountWatcher::mountAdded, [](const QString &mountPoint){
        qInfo().noquote() << "Mount added:  " << mountPoint;
    });

    QObject::connect(mount_watcher, &MountWatcher::mountRemoved, [](const QString &mountPoint){
        qInfo().noquote() << "Mount removed:" << mountPoint;
    });

    mount_watcher->setEnabled(true);

    qInfo() << "Waiting for filesystems to be mounted ot removed. Connect a removable drive for example.";

    return app.exec();
}
