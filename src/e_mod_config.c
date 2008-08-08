#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data 
{
   int cd_go_parent;
   int cd_go_home;
   int rm_rf_all;
};

/* Local Function Prototypes */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

/* External Functions */
EAPI E_Config_Dialog *
e_int_config_typebuf_hooks_module(E_Container *con, const char *params __UNUSED__) 
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   char buf[4096];

   /* is this config dialog already visible ? */
   if (e_config_dialog_find("Typebuf Hooks", "_e_module_typebuf_hooks_cfg_dlg")) return NULL;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return NULL;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   /* Icon in the theme */
   snprintf(buf, sizeof(buf), "%s/e-module-typebuf-hooks.edj", typebuf_hooks_conf->module->dir);

   /* create new config dialog */
   cfd = e_config_dialog_new(con, "Typebuf Hooks Module", "Typebuf Hooks", 
                             "_e_module_typebuf_hooks_cfg_dlg", buf, 0, v, NULL);

   e_dialog_resizable_set(cfd->dia, 1);
   typebuf_hooks_conf->cfd = cfd;
   return cfd;
}

/* Local Functions */
static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata = NULL;

   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   typebuf_hooks_conf->cfd = NULL;
   E_FREE(cfdata);
}

static void 
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   /* load a temp copy of the config variables */
   cfdata->cd_go_parent = typebuf_hooks_conf->cd_go_parent;
   cfdata->cd_go_home = typebuf_hooks_conf->cd_go_home;
   cfdata->rm_rf_all = typebuf_hooks_conf->rm_rf_all;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL;

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, "Commands", 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   
   ow = e_widget_check_add(evas, "cd ..", 
                           &(cfdata->cd_go_parent));
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_check_add(evas, "cd", 
                           &(cfdata->cd_go_home));
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_check_add(evas, "rm -rf *", 
                           &(cfdata->rm_rf_all));
   e_widget_framelist_object_append(of, ow);
   
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int 
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   typebuf_hooks_conf->cd_go_parent = cfdata->cd_go_parent;
   typebuf_hooks_conf->cd_go_home = cfdata->cd_go_home;
   typebuf_hooks_conf->rm_rf_all = cfdata->rm_rf_all;
   e_config_save_queue();
   return 1;
}
