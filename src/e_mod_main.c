#include <e.h>
#include "e_mod_main.h"

/* Local Function Prototypes */
static void _dbus_api_conf_new(void);
static void _dbus_api_conf_free(void);
static int _dbus_api_conf_timer(void *data);

/* Local Structures */
typedef struct _Instance Instance;
struct _Instance 
{
   /* evas_object used to display */
   Evas_Object *o_dbus_api;
};

/* Local Variables */
static int uuid = 0;
static Evas_List *instances = NULL;
static E_Config_DD *conf_edd = NULL;
Config *dbus_api_conf = NULL;

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "DBUS API"};

/*
 * Module Functions
*/
EAPI void *
e_modapi_init(E_Module *m) 
{
   char buf[4096];

   /* Location of theme to load for this module */
   snprintf(buf, sizeof(buf), "%s/e-module-dbus-api.edj", m->dir);

   /* Display this Modules config info in the main Config Panel */

   /* starts with a category */
   e_configure_registry_category_add("extensions", 90, ("Extensions"), NULL,
         "enlightenment/extensions");

   /* add right-side item */
   e_configure_registry_item_add("extensions/dbus_api", 110, "DBUS API",
         NULL, buf, e_int_config_dbus_api_module);

   /* Define EET Data Storage */

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);

   /* Tell E to find any existing module data. First run ? */
   dbus_api_conf = e_config_domain_load("module.dbus_api", conf_edd);
   if (dbus_api_conf) 
     {
        /* Check config version */
        if ((dbus_api_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
          {
             /* config too old */
             _dbus_api_conf_free();
	     ecore_timer_add(1.0, _dbus_api_conf_timer,
			     "DBUS API Module Configuration data needed "
			     "upgrading. Your old configuration<br> has been"
			     " wiped and a new set of defaults initialized. "
			     "This<br>will happen regularly during "
			     "development, so don't report a<br>bug. "
			     "This simply means the module needs "
			     "new configuration<br>data by default for "
			     "usable functionality that your old<br>"
			     "configuration simply lacks. This new set of "
			     "defaults will fix<br>that by adding it in. "
			     "You can re-configure things now to your<br>"
			     "liking. Sorry for the inconvenience.<br>");
          }

        /* Ardvarks */
        else if (dbus_api_conf->version > MOD_CONFIG_FILE_VERSION) 
          {
             /* config too new...wtf ? */
             _dbus_api_conf_free();
	     ecore_timer_add(1.0, _dbus_api_conf_timer, 
			     "Your DBUS API Module configuration is NEWER "
			     "than the module version. This is "
			     "very<br>strange. This should not happen unless"
			     " you downgraded<br>the module or "
			     "copied the configuration from a place where"
			     "<br>a newer version of the module "
			     "was running. This is bad and<br>as a "
			     "precaution your configuration has been now "
			     "restored to<br>defaults. Sorry for the "
			     "inconvenience.<br>");
          }
     }

   /* if we don't have a config yet, or it got erased above, 
    * then create a default one */
   if (!dbus_api_conf) _dbus_api_conf_new();

   /* create a link from the modules config to the module
    * this is not written */
   dbus_api_conf->module = m;

   int r;

   e_dbus_init();

   E_DBus_Connection *conn = e_dbus_bus_get(DBUS_BUS_SESSION);

   if (!conn)
     {
        return m;
        fprintf(stderr, "Failed to connect to bus.\n");
     }

   E_DBus_Interface *iface = e_dbus_interface_new("org.e");

   if (!iface)
     {
        return m;
        fprintf(stderr, "Failed to create an interface.\n");
     }

   r = e_dbus_interface_method_add(iface, "Restart", "", "", e_dbus_api_restart_cb);

   if (!r)
     {
        return m;
        fprintf(stderr, "Failed to add method Restart to interface.\n");
     }

   E_DBus_Object *o = e_dbus_object_add(conn, "/org/e/self", NULL);
   
   if (!o)
     {
        return m;
        fprintf(stderr, "Failed to add object.\n");
     }

   e_dbus_object_interface_attach(o, iface);
   
   e_dbus_request_name(conn, "org.e", 0, NULL, NULL);

   /* Give E the module */
   return m;
}

/*
 * Function to unload the module
 */
EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   e_dbus_shutdown();

   /* Unregister the config dialog from the main panel */
   e_configure_registry_item_del("extensions/dbus_api");

   /* Remove the config panel category if we can. E will tell us.
    category stays if other items using it */
   e_configure_registry_category_del("extensions");

   /* Kill the config dialog */
   if (dbus_api_conf->cfd) e_object_del(E_OBJECT(dbus_api_conf->cfd));
   dbus_api_conf->cfd = NULL;

   /* Tell E the module is now unloaded. */
   dbus_api_conf->module = NULL;

   /* Cleanup the main config structure */
   E_FREE(dbus_api_conf);

   /* Clean EET */
   E_CONFIG_DD_FREE(conf_edd);

   return 1;
}

/*
 * Function to Save the modules config
 */ 
EAPI int 
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.dbus_api", conf_edd, dbus_api_conf);
   return 1;
}

/* Local Functions */

/* new module needs a new config :), or config too old and we need one anyway */
static void 
_dbus_api_conf_new(void) 
{
   char buf[128];

   dbus_api_conf = E_NEW(Config, 1);
   dbus_api_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((dbus_api_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   IFMODCFGEND;

   /* update the version */
   dbus_api_conf->version = MOD_CONFIG_FILE_VERSION;

   /* setup limits on the config properties here (if needed) */

   /* save the config to disk */
   e_config_save_queue();
}

static void 
_dbus_api_conf_free(void) 
{
   E_FREE(dbus_api_conf);
}

/* timer for the config oops dialog */
static int 
_dbus_api_conf_timer(void *data) 
{
   e_util_dialog_show("DBUS API Configuration Updated", data);
   return 0;
}

DBusMessage *e_dbus_api_restart_cb(E_DBus_Object *obj, DBusMessage *message)
{
   DBusMessage *m = dbus_message_new_method_return(message);

   restart = 1;
   ecore_main_loop_quit();

   return m;
}
