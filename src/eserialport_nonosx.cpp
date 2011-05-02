#include "eserialport.h"
#include <QStringList>
#include <QFile>
#include <QLibrary>
#include <hal/libhal.h>


using namespace PZST;

extern "C" {
    typedef LibHalContext* (*libhal_ctx_new_t)(void);
    typedef dbus_bool_t (*libhal_ctx_init_t)(LibHalContext*, DBusError*);
    typedef char** (*libhal_find_device_by_capability_t)(LibHalContext *, const char *, int *, DBusError *);
    typedef dbus_bool_t (*libhal_ctx_shutdown_t)(LibHalContext*, DBusError*);
    typedef dbus_bool_t (*libhal_ctx_free_t)(LibHalContext*);
    typedef char* (*libhal_device_get_property_string_t) (LibHalContext*, const char*, const char*, DBusError*);


    typedef void (*dbus_error_init_t) (DBusError*);
    typedef DBusConnection* (*dbus_bus_get_t) (DBusBusType, DBusError*);
    typedef dbus_bool_t (*dbus_error_is_set_t) (const DBusError *);
    typedef void (*dbus_error_free_t) (DBusError*);
    typedef dbus_bool_t (*libhal_ctx_set_dbus_connection_t)(LibHalContext*, DBusConnection*);

};

typedef struct {
    int initialized;
    libhal_ctx_new_t libhal_ctx_new;
    libhal_ctx_init_t libhal_ctx_init;
    libhal_find_device_by_capability_t libhal_find_device_by_capability;
    libhal_ctx_shutdown_t libhal_ctx_shutdown;
    libhal_ctx_free_t libhal_ctx_free;
    libhal_device_get_property_string_t libhal_device_get_property_string;
    dbus_error_init_t dbus_error_init;
    dbus_bus_get_t dbus_bus_get;
    dbus_error_is_set_t dbus_error_is_set;
    dbus_error_free_t dbus_error_free;
    libhal_ctx_set_dbus_connection_t libhal_ctx_set_dbus_connection;
} HALFunctions;

static HALFunctions hal = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static QLibrary halLib("hal");
static QLibrary dbusLib("dbus-1");

static bool initializeHal()
{
    if (hal.initialized == 2) return false;
    if (hal.initialized == 1) return true;

    hal.libhal_ctx_new = (libhal_ctx_new_t)halLib.resolve("libhal_ctx_new");
    if (!hal.libhal_ctx_new) {hal.initialized = 2; return false;}
    hal.libhal_ctx_init = (libhal_ctx_init_t)halLib.resolve("libhal_ctx_init");
    if (!hal.libhal_ctx_init) {hal.initialized = 2; return false;}
    hal.libhal_find_device_by_capability = (libhal_find_device_by_capability_t)halLib.resolve("libhal_find_device_by_capability");
    if (!hal.libhal_find_device_by_capability) {hal.initialized = 2; return false;}
    hal.libhal_ctx_shutdown = (libhal_ctx_shutdown_t)halLib.resolve("libhal_ctx_shutdown");
    if (!hal.libhal_ctx_shutdown) {hal.initialized = 2; return false;}
    hal.libhal_ctx_free = (libhal_ctx_free_t)halLib.resolve("libhal_ctx_free");
    if (!hal.libhal_ctx_free) {hal.initialized = 2; return false;}
    hal.libhal_ctx_set_dbus_connection = (libhal_ctx_set_dbus_connection_t)halLib.resolve("libhal_ctx_set_dbus_connection");
    if (!hal.libhal_ctx_set_dbus_connection) {hal.initialized = 2; return false;}
    hal.libhal_device_get_property_string = (libhal_device_get_property_string_t)halLib.resolve("libhal_device_get_property_string");
    if (!hal.libhal_device_get_property_string ) {hal.initialized = 2; return false;}

    hal.dbus_error_init = (dbus_error_init_t)dbusLib.resolve("dbus_error_init");
    if (!hal.dbus_error_init) {hal.initialized = 2; return false;}
    hal.dbus_bus_get = (dbus_bus_get_t)dbusLib.resolve("dbus_bus_get");
    if (!hal.dbus_bus_get) {hal.initialized = 2; return false;}
    hal.dbus_error_is_set = (dbus_error_is_set_t)dbusLib.resolve("dbus_error_is_set");
    if (!hal.dbus_error_is_set) {hal.initialized = 2; return false;}
    hal.dbus_error_free = (dbus_error_free_t)dbusLib.resolve("dbus_error_free");
    if (!hal.dbus_error_free) {hal.initialized = 2; return false;}

    return true;
}

static bool enumerateViaHAL(QStringList &ports)
{
    if (!initializeHal()) return false;
    LibHalContext* ctx;
    int nDevises;
    DBusError dbus_error;
    DBusConnection *dbus_conn;
    char ** devices;
    ctx = hal.libhal_ctx_new();
    if (ctx == NULL) {
      return false;
    }
    hal.dbus_error_init(&dbus_error);
    dbus_conn = hal.dbus_bus_get (DBUS_BUS_SYSTEM, &dbus_error);
    if (hal.dbus_error_is_set(&dbus_error)) {
      hal.dbus_error_free(&dbus_error);
      hal.libhal_ctx_free(ctx);
      return false;
    }

    hal.libhal_ctx_set_dbus_connection(ctx, dbus_conn);

    if (!(devices = hal.libhal_find_device_by_capability (ctx, "serial", &nDevises, &dbus_error))) {
       hal.dbus_error_free (&dbus_error);
       hal.libhal_ctx_shutdown (ctx, NULL);
       hal.libhal_ctx_free (ctx);
       return false;
    }

    for (int i = 0; i < nDevises; i++) {
      char *port = hal.libhal_device_get_property_string(ctx, devices[i], "serial.device", &dbus_error);
      ports << port;
    }

    hal.libhal_ctx_shutdown (ctx, NULL);
    hal.libhal_ctx_free(ctx);

    return true;
}

static void guess(QStringList &ports)
{
    QStringList templates("/dev/ttyS%1");
    templates << "/dev/ttyUSB%1";
    for (int i = 0; i < templates.size(); i++) {
        QString tpl = templates[i];
        for (int j = 0; j < 30; j++) {
            QString fName = tpl.arg(j);
            if (QFile::exists(fName)) ports << fName;
        }
    }
}

QStringList ESerialPort::enumeratePorts()
{
    QStringList ports;
    if (!enumerateViaHAL(ports)) guess(ports);
    return ports;
}

