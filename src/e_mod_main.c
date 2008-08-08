#include <e.h>
#include "e_mod_main.h"

/* Local Function Prototypes */
static void _typebuf_hooks_conf_new(void);
static void _typebuf_hooks_conf_free(void);
static int _typebuf_hooks_conf_timer(void *data);
static Config_Item *_typebuf_hooks_conf_item_get(const char *id);

/* Local Structures */
typedef struct _Instance Instance;
struct _Instance 
{
   /* evas_object used to display */
   Evas_Object *o_typebuf_hooks;

   /* Config_Item structure. Every gadget should have one :) */
   Config_Item *conf_item;
};

/* Local Variables */
static int uuid = 0;
static Evas_List *instances = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
Config *typebuf_hooks_conf = NULL;
static E_Fm_Typebuf_Hook *hook = NULL;

EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Typebuf Hooks"};

EAPI int
e_fm_typebuf_hooks_hook(void *data, Evas_Object *obj, const char *s)
{
   if(!strcmp(s, "cd ..") && typebuf_hooks_conf && typebuf_hooks_conf->cd_go_parent)
     {
        e_fm2_parent_go(obj);
        return 1;
     }
   else if(!strcmp(s, "cd") && typebuf_hooks_conf && typebuf_hooks_conf->cd_go_home)
     {
        e_fm2_path_set(obj, "/", e_user_homedir_get());
        return 1;
     }
   else if(!strcmp(s, "rm -rf *") && typebuf_hooks_conf && typebuf_hooks_conf->rm_rf_all)
     {
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "rm -rf %s/*", e_fm2_real_path_get(obj));
        ecore_exe_run(buf, NULL);
        return 1;
     }
   return 0;
}

/*
 * Module Functions
*/
EAPI void *
e_modapi_init(E_Module *m) 
{
   char buf[4096];

   /* Location of theme to load for this module */
   snprintf(buf, sizeof(buf), "%s/e-module-typebuf-hooks.edj", m->dir);

   /* Display this Modules config info in the main Config Panel */

   /* starts with a category */
   e_configure_registry_category_add("fileman", 80, "File Manager",
         NULL, "enlightenment/fileman");

   /* add right-side item */
   e_configure_registry_item_add("fileman/typebuf_hooks", 110, "Typebuf Hooks",
         NULL, buf, e_int_config_typebuf_hooks_module);

   /* Define EET Data Storage */

   conf_item_edd = E_CONFIG_DD_NEW("Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, switch2, INT);

   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, cd_go_parent, UCHAR); /* our var from header */
   E_CONFIG_VAL(D, T, cd_go_home, UCHAR); /* our var from header */
   E_CONFIG_VAL(D, T, rm_rf_all, UCHAR); /* our var from header */
   E_CONFIG_LIST(D, T, conf_items, conf_item_edd); /* the list */

   /* Tell E to find any existing module data. First run ? */
   typebuf_hooks_conf = e_config_domain_load("module.typebuf_hooks", conf_edd);
   if (typebuf_hooks_conf) 
     {
        /* Check config version */
        if ((typebuf_hooks_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH) 
          {
             /* config too old */
             _typebuf_hooks_conf_free();
	     ecore_timer_add(1.0, _typebuf_hooks_conf_timer,
			     "Typebuf Hooks Module Configuration data needed "
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
        else if (typebuf_hooks_conf->version > MOD_CONFIG_FILE_VERSION) 
          {
             /* config too new...wtf ? */
             _typebuf_hooks_conf_free();
	     ecore_timer_add(1.0, _typebuf_hooks_conf_timer, 
			     "Your Typebuf Hooks Module configuration is NEWER "
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
   if (!typebuf_hooks_conf) _typebuf_hooks_conf_new();

   /* create a link from the modules config to the module
    * this is not written */
   typebuf_hooks_conf->module = m;

   hook = e_fm_typebuf_hook_register(e_fm_typebuf_hooks_hook, NULL);

   /* Give E the module */
   return m;
}

/*
 * Function to unload the module
 */
EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   /* Unregister the config dialog from the main panel */
   e_configure_registry_item_del("fileman/typebuf_hooks");

   /* Remove the config panel category if we can. E will tell us.
    category stays if other items using it */
   e_configure_registry_category_del("fileman");

   /* Kill the config dialog */
   if (typebuf_hooks_conf->cfd) e_object_del(E_OBJECT(typebuf_hooks_conf->cfd));
   typebuf_hooks_conf->cfd = NULL;

   /* Tell E the module is now unloaded. */
   typebuf_hooks_conf->module = NULL;

   /* Cleanup our item list */
   while (typebuf_hooks_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        /* Grab an item from the list */
        ci = typebuf_hooks_conf->conf_items->data;
        /* remove it */
        typebuf_hooks_conf->conf_items = 
          evas_list_remove_list(typebuf_hooks_conf->conf_items, 
                                typebuf_hooks_conf->conf_items);
        /* cleanup stringshares !! ) */
        if (ci->id) evas_stringshare_del(ci->id);
        /* keep the planet green */
        E_FREE(ci);
     }

   /* Cleanup the main config structure */
   E_FREE(typebuf_hooks_conf);

   /* Clean EET */
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);

   e_fm_typebuf_hook_unregister(hook);
   return 1;
}

/*
 * Function to Save the modules config
 */ 
EAPI int 
e_modapi_save(E_Module *m) 
{
   e_config_domain_save("module.typebuf_hooks", conf_edd, typebuf_hooks_conf);
   return 1;
}

/* Local Functions */

/* new module needs a new config :), or config too old and we need one anyway */
static void 
_typebuf_hooks_conf_new(void) 
{
   Config_Item *ci = NULL;
   char buf[128];

   typebuf_hooks_conf = E_NEW(Config, 1);
   typebuf_hooks_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((typebuf_hooks_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   typebuf_hooks_conf->cd_go_parent = 1;
   typebuf_hooks_conf->cd_go_home = 1;
   typebuf_hooks_conf->rm_rf_all = 1;
   _typebuf_hooks_conf_item_get(NULL);
   IFMODCFGEND;

   /* update the version */
   typebuf_hooks_conf->version = MOD_CONFIG_FILE_VERSION;

   /* setup limits on the config properties here (if needed) */

   /* save the config to disk */
   e_config_save_queue();
}

static void 
_typebuf_hooks_conf_free(void) 
{
   /* cleanup any stringshares here */
   while (typebuf_hooks_conf->conf_items) 
     {
        Config_Item *ci = NULL;

        ci = typebuf_hooks_conf->conf_items->data;
        typebuf_hooks_conf->conf_items = 
          evas_list_remove_list(typebuf_hooks_conf->conf_items, 
                                typebuf_hooks_conf->conf_items);
        /* EPA */
        if (ci->id) evas_stringshare_del(ci->id);
        E_FREE(ci);
     }

   E_FREE(typebuf_hooks_conf);
}

/* timer for the config oops dialog */
static int 
_typebuf_hooks_conf_timer(void *data) 
{
   e_util_dialog_show("Typebuf Hooks Configuration Updated", data);
   return 0;
}

/* function to search for any Config_Item struct for this Item
 * create if needed */
static Config_Item *
_typebuf_hooks_conf_item_get(const char *id) 
{
   Evas_List *l = NULL;
   Config_Item *ci = NULL;
   char buf[128];

   if (!id) 
     {
        /* nothing passed, return a new id */
        snprintf(buf, sizeof(buf), "%s.%d", "typebuf_hooks", ++uuid);
        id = buf;
     }
   else 
     {
        for (l = typebuf_hooks_conf->conf_items; l; l = l->next) 
          {
             if (!(ci = l->data)) continue;
             if ((ci->id) && (!strcmp(ci->id, id))) return ci;
          }
     }
   ci = E_NEW(Config_Item, 1);
   ci->id = evas_stringshare_add(id);
   ci->switch2 = 0;
   typebuf_hooks_conf->conf_items = evas_list_append(typebuf_hooks_conf->conf_items, ci);
   return ci;
}
