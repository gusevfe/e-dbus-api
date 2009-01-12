#include <E_DBus.h>

#include <e.h>

#include "e_dbus_api_self.h"

EAPI void 
e_dbus_api_self_init() 
{
   E_DBus_Interface *iface = NULL;
   E_DBus_Connection *conn = NULL;
   E_DBus_Object *o = NULL;

   int r;

   e_dbus_init();

   conn = e_dbus_bus_get(DBUS_BUS_SESSION);

   if (!conn)
     {
        fprintf(stderr, "Failed to connect to bus.\n");
        return;
     }
   
   e_dbus_request_name(conn, "org.e", 0, NULL, NULL);

   iface = e_dbus_interface_new("org.e.self");

   if (!iface)
     {
        fprintf(stderr, "Failed to create an interface.\n");
        return;
     }

   r = e_dbus_interface_method_add(iface, "Restart", "", "", e_dbus_api_self_restart_cb);

   if (!r)
     {
        fprintf(stderr, "Failed to add method Restart to interface org.e.self.\n");
        return;
     }

   o = e_dbus_object_add(conn, "/org/e/self", NULL);
   
   if (!o)
     {
        fprintf(stderr, "Failed to add object /org/e/self.\n");
        return;
     }

   e_dbus_object_interface_attach(o, iface);
}

EAPI void 
e_dbus_api_self_shutdown()
{
}

EAPI DBusMessage *
e_dbus_api_self_restart_cb(E_DBus_Object *obj, DBusMessage *message)
{
   DBusMessage *m = dbus_message_new_method_return(message);

   restart = 1;
   ecore_main_loop_quit();

   return m;
}
