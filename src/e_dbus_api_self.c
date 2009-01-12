#include <E_DBus.h>

#include <e.h>

#include "e_dbus_api_self.h"

#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__ )

#define ACTION_CALLBACK_BEGIN(name) EAPI DBusMessage *                      \
   e_dbus_api_self_##name##_cb(E_DBus_Object *obj, DBusMessage *message)    \
   {                                                                        \
       DBusMessage *m = dbus_message_new_method_return(message);            \
       E_Action *action = e_action_find(#name);                             \
                                                                            \
       LOG("Action %s called.", #name);                                     \
                                                                            \
       if (!(action && action->func.go))                                    \
        {                                                                   \
           LOG("Action %s not found.", #name);                              \
           return m;                                                        \
        }


#define ACTION_CALLBACK_END                                                 \
       return m;                                                            \
   }

ACTION_CALLBACK_BEGIN(restart)
   action->func.go(NULL, NULL); 
ACTION_CALLBACK_END

ACTION_CALLBACK_BEGIN(exit)

   char *when;

   if (dbus_message_get_args(message, NULL,
            DBUS_TYPE_STRING, &when,
            DBUS_TYPE_INVALID) == FALSE)
   {
      LOG("Failed to get args in Exit method.");
      return m;
   }

   LOG("Args: %s", when);

   action->func.go(NULL, when);
   
ACTION_CALLBACK_END

#undef ACTION_CALLBACK_BEGIN
#undef ACTION_CALLBACK_END

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
        LOG("Failed to connect to bus.");
        return;
     }
   
   e_dbus_request_name(conn, "org.e", 0, NULL, NULL);

   iface = e_dbus_interface_new("org.e.self");

   if (!iface)
     {
        LOG("Failed to create an interface.");
        return;
     }

#define ACTION_METHOD_ADD(name, action, signature)                                              \
   r = e_dbus_interface_method_add(iface, name, signature, "", e_dbus_api_self_##action##_cb);  \
   if (!r)                                                                                      \
     {                                                                                          \
        LOG("Failed to add method %s to interface org.e.self.", name);                          \
        return;                                                                                 \
     }

   ACTION_METHOD_ADD("Restart", restart, "");
   ACTION_METHOD_ADD("Exit", exit, "s");

#undef ACTION_METHOD_ADD

   o = e_dbus_object_add(conn, "/org/e/self", NULL);
   
   if (!o)
     {
        LOG("Failed to add object /org/e/self.");
        return;
     }

   e_dbus_object_interface_attach(o, iface);
}

EAPI void 
e_dbus_api_self_shutdown()
{
}
