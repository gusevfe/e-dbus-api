#ifndef __E_DBUS_API_BORDERS_H
#define __E_DBUS_API_BORDERS_H

#include <E_DBus.h>

EAPI void e_dbus_api_borders_init();
EAPI void e_dbus_api_borders_shutdown();

EAPI DBusMessage *e_dbus_api_borders_action(E_DBus_Object *obj, DBusMessage *message);

#endif
