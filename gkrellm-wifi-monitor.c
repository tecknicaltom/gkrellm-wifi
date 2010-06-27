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

#include "gkrellm-wifi-monitor.h"


GkrellmWifiMonitor *
gkrellm_wifi_monitor_create (const gchar *interface)
{
  GkrellmWifiMonitor *wifimon;

  g_assert (interface != NULL);

  wifimon = g_new0 (GkrellmWifiMonitor, 1);

  wifimon->interface = g_strdup (interface);

  gkrellm_wifi_monitor_list = g_list_insert_sorted (gkrellm_wifi_monitor_list,
                                                    wifimon,
                                                    gkrellm_wifi_monitor_compare);

  return wifimon;
}

GkrellmWifiMonitor *
gkrellm_wifi_monitor_find (const gchar *interface)
{
  GkrellmWifiMonitor *wifimon;
  GList              *list;

  g_assert (interface != NULL);

  wifimon = g_new0 (GkrellmWifiMonitor, 1);

  wifimon->interface = g_strdup (interface);

  list = g_list_find_custom (gkrellm_wifi_monitor_list,
                             wifimon,
                             gkrellm_wifi_monitor_compare);

  if (list)
    {
      g_free (wifimon->interface);
      g_free (wifimon);

      wifimon = list->data;
    }
  else
    {
      g_free (wifimon->interface);
      g_free (wifimon);

      wifimon = NULL;
    }

  return wifimon;
}

gint
gkrellm_wifi_monitor_compare (gconstpointer a,
                              gconstpointer b)
{
  const GkrellmWifiMonitor *alpha;
  const GkrellmWifiMonitor *bravo;

  g_assert (a != NULL);
  g_assert (b != NULL);

  alpha = a;
  bravo = b;

  return g_ascii_strcasecmp (alpha->interface,
                             bravo->interface);
}
