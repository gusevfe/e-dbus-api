#include <E_DBus.h>

#include <e.h>

#include "e_dbus_api_borders.h"

EAPI void 
e_dbus_api_borders_init() 
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

   iface = e_dbus_interface_new("org.e.borders");

   if (!iface)
     {
        fprintf(stderr, "Failed to create an interface.\n");
        return;
     }

   r = e_dbus_interface_method_add(iface, "Action", "uss", "", e_dbus_api_borders_action);

   if (!r)
     {
        fprintf(stderr, "Failed to add method Action to interface org.e.borders.\n");
        return;
     }

   o = e_dbus_object_add(conn, "/org/e/borders", NULL);
   
   if (!o)
     {
        fprintf(stderr, "Failed to add object /org/e/borders.\n");
        return;
     }

   e_dbus_object_interface_attach(o, iface);
}

EAPI void 
e_dbus_api_borders_shutdown()
{
}

EAPI DBusMessage *
e_dbus_api_borders_action(E_DBus_Object *obj, DBusMessage *message)
{
   DBusMessage *m = dbus_message_new_method_return(message);
   DBusMessageIter iter;
   DBusError error;
   int id;
   char *name, *params;
        
   if (dbus_message_get_args(message, NULL, 
            DBUS_TYPE_UINT32, &id, 
            DBUS_TYPE_STRING, &name, 
            DBUS_TYPE_STRING, &params,
            DBUS_TYPE_INVALID) == FALSE)
     {
        fprintf(stderr, "Failed to get args.\n");
        return m;
     }
        
   
   fprintf(stderr, "Border action: %x %s %s.\n", id, name, params);

   E_Border *border = e_border_find_by_client_window((Ecore_X_Window)id);

   if (!border)
     {
        fprintf(stderr, "Failed to find the border.\n");
        return m;
     }

   E_Action *action = e_action_find(name); 
   
   if (!action)
     {
        fprintf(stderr, "Failed to find action '%s'.\n", name);
        return m;
     }

   action->func.go(E_OBJECT(border), params);
   return m;
}
