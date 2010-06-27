/* gkrellm-wifi - A wireless link monitor plug-in for GKrellM2
 *
 * Copyright (C) 2003 Henrik Brix Andersen <brix@gimp.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "gkrellm-wifi.h"

#include "gkrellm-wifi-chart.h"
#include "gkrellm-wifi-linux.h"
#include "gkrellm-wifi-preferences.h"


#define PLUGIN_PLACEMENT MON_NET | MON_INSERT_AFTER
#define STYLE_NAME       GKRELLM_WIFI_PLUGIN_NAME

/*  prototypes  */

static void gkrellm_wifi_create (GtkWidget *vbox,
                                 gint       first_create);
static void gkrellm_wifi_update (void);


/*  global variables  */

GkrellmMonitor *gkrellm_wifi;
gint            gkrellm_wifi_style_id;
GtkWidget      *gkrellm_wifi_vbox;
GkrellmTicks   *gkrellm_wifi_ticks;

gchar          *gkrellm_wifi_format_string;

GList          *gkrellm_wifi_monitor_list = NULL;


/*  public function  */

GkrellmMonitor *
gkrellm_init_plugin (void)
{
  static GkrellmMonitor plugin_mon = {
    N_("WiFi Monitor"),             /* name, for config tab                     */
    0,                              /* id, 0 if a plugin                        */
    gkrellm_wifi_create,            /* the create_plugin() function             */
    gkrellm_wifi_update,            /* the update_plugin() function             */
    gkrellm_wifi_preferences_show,  /* the create_plugin_tab() config function  */
    gkrellm_wifi_preferences_apply, /* the apply_plugin_config() function       */

    gkrellm_wifi_preferences_save,  /* the save_plugin_config() function        */
    gkrellm_wifi_preferences_load,  /* the load_plugin_config() function        */
    GKRELLM_WIFI_PLUGIN_KEYWORD,    /* config keyword                           */

    NULL,                           /* undefined 2                              */
    NULL,                           /* undefined 1                              */
    NULL,                           /* undefined 0                              */

    PLUGIN_PLACEMENT,               /* insert plugin before this monitor        */

    NULL,                           /* handle if a plugin, filled in by GKrellM */
    NULL                            /* path if a plugin, filled in by GKrellM   */
  };

  gkrellm_wifi_format_string = g_strdup (GKRELLM_WIFI_DEFAULT_FORMAT_STRING);
  gkrellm_wifi_ticks         = gkrellm_ticks ();
  gkrellm_wifi_style_id      = gkrellm_add_chart_style (&plugin_mon, STYLE_NAME);
  gkrellm_wifi               = &plugin_mon;

  return gkrellm_wifi;
}


/*  private functions  */

static void
gkrellm_wifi_create (GtkWidget *vbox,
                     gint       first_create)
{
  GkrellmWifiMonitor *wifimon;
  GList              *list;

  g_assert (vbox != NULL);

  if (first_create)
    gkrellm_wifi_vbox = vbox;

  for (list = gkrellm_wifi_monitor_list; list; list = g_list_next (list))
    {
      wifimon = list->data;

      if (wifimon->chart)
        {
          gkrellm_wifi_chart_create (wifimon);

          gkrellm_refresh_chart (wifimon->chart);
        }
    }
}

static void
gkrellm_wifi_update (void)
{
  GkrellmWifiMonitor *wifimon;
  GList              *list;

  if (gkrellm_wifi_ticks->second_tick)
    {
      gkrellm_wifi_wireless_info_read ();

      for (list = gkrellm_wifi_monitor_list; list; list = g_list_next (list))
        {
          wifimon = list->data;

          if ((wifimon->enabled && wifimon->updated) ||
              (wifimon->enabled && wifimon->forced))
            {
              if (! wifimon->chart)
                gkrellm_wifi_chart_create (wifimon);

              if (! wifimon->updated)
                {
                  wifimon->quality     = 0;
                  wifimon->quality_max = 0;
                  wifimon->signal      = 0;
                  wifimon->noise       = 0;
                  wifimon->bitrate     = 0;
                  wifimon->percent     = 0;

                  if (wifimon->essid)
                    g_free (wifimon->essid);

                  wifimon->essid = g_strdup (_("n/a"));;
                }

              gkrellm_store_chartdata (wifimon->chart, 0, wifimon->percent);

              gkrellm_refresh_chart (wifimon->chart);

              gkrellm_check_alert (wifimon->alert, wifimon->percent);

              wifimon->updated = FALSE;
            }
          else if (wifimon->chart)
            {
              gkrellm_chart_destroy (wifimon->chart);
              wifimon->chart = NULL;
            }
        }
    }
}
