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


#ifndef __GKRELLM_WIFI_H__
#define __GKRELLM_WIFI_H__


#include <gkrellm2/gkrellm.h>


#define GKRELLM_WIFI_VERSION_MAJOR         0
#define GKRELLM_WIFI_VERSION_MINOR         9
#define GKRELLM_WIFI_VERSION_MICRO         12

#define GKRELLM_WIFI_PLUGIN_NAME           "gkrellm-wifi"            /*  messages            */
#define GKRELLM_WIFI_PLUGIN_KEYWORD        GKRELLM_WIFI_PLUGIN_NAME  /*  configuration file  */

#define GKRELLM_WIFI_DEFAULT_FORMAT_STRING "\\t$Q%\\b$R\\fdB"


typedef struct {
  GkrellmChart       *chart;
  GkrellmChartdata   *data;
  GkrellmChartconfig *config;

  GkrellmLauncher     launcher;
  GtkWidget          *command_entry;
  GtkWidget          *tooltip_entry;

  GkrellmAlert       *alert;
  GtkWidget          *alert_button;

  gboolean            enabled;
  GtkWidget          *enable_button;

  gboolean            forced;
  GtkWidget          *force_button;

  gboolean            hide_text;

  gboolean            updated;

  gchar              *interface;
  gint                quality;
  guint8              quality_max;
  gint                signal;
  gint                noise;

  gint32              bitrate;

  gchar              *essid;

  gint                percent;
} GkrellmWifiMonitor;


extern GkrellmMonitor *gkrellm_wifi;
extern gint            gkrellm_wifi_style_id;
extern GtkWidget      *gkrellm_wifi_vbox;
extern GkrellmTicks   *gkrellm_wifi_ticks;

extern gchar          *gkrellm_wifi_format_string;

extern GList          *gkrellm_wifi_monitor_list;


#endif  /*  __GKRELLM_WIFI_H__  */
