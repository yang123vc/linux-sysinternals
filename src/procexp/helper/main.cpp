#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <iostream>
#include <unistd.h>

#include "procconnector.h"
#include "procexphelper.h"
#include "ProcexpHelperAdaptor.h"
#include "ProcexpHelperInterface.h"

#define CONNECTOR_SERVICE "com.procexp.helper"
#define CONNECTOR_PATH "/"

static com::procexp::helper* iface = nullptr;

void handler(struct proc_event event)
{
    switch (event.what) {
    case event.PROC_EVENT_NONE:
        break;
    case event.PROC_EVENT_FORK:
        std::cout << event.event_data.fork.child_pid << std::endl;
        emit iface->fork(
               event.event_data.fork.parent_pid,
               event.event_data.fork.parent_tgid,
               event.event_data.fork.child_pid,
               event.event_data.fork.child_tgid);
        break;
    case event.PROC_EVENT_EXEC:
        emit iface->exec(
               event.event_data.exec.process_pid,
               event.event_data.exec.process_tgid);
        break;
    case event.PROC_EVENT_UID:
        emit iface->uid(
               event.event_data.id.process_pid,
               event.event_data.id.process_tgid,
               event.event_data.id.r.ruid,
               event.event_data.id.e.euid);
        break;
    case event.PROC_EVENT_GID:
        emit iface->gid(
               event.event_data.id.process_pid,
               event.event_data.id.process_tgid,
               event.event_data.id.r.rgid,
               event.event_data.id.e.egid);
        break;
    case event.PROC_EVENT_EXIT:
        emit iface->exit(
               event.event_data.exit.process_pid,
               event.event_data.exit.process_tgid,
               event.event_data.exit.exit_code);
        break;
    default:
        break;
    }

}

int main(int argc, char* argv[])
{
    QDBusConnection connection = QDBusConnection::sessionBus();

    ProcexpHelper* conn = new ProcexpHelper();
    ProcexpHelperAdaptor* adaptor = new ProcexpHelperAdaptor(conn);

    if (!connection.registerService(CONNECTOR_SERVICE))
        qFatal("Could not register the service");

    if (!connection.registerObject(CONNECTOR_PATH, conn))
        qFatal("Could not register the connector object");

    iface = new com::procexp::helper(CONNECTOR_SERVICE, CONNECTOR_PATH, connection);

    while(1)
    {
        emit iface->fork(1, 2, 3, 4);
        sleep(2);
    }

    // ProcConnector connector = ProcConnector();
    // connector.addCallback(handler);
    // connector.listen();
}
