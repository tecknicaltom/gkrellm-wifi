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

#include "gkrellm-wifi-alert.h"


#define ALERT_MAX    100
#define ALERT_MIN    0

#define ALERT_STEP0  5
#define ALERT_STEP1  10
#define ALERT_DIGITS 0

#define DELAY_MIN    0
#define DELAY_MAX    3600
#define DELAY_STEP   1


/*  prototypes  */
static void alert_trigger_callback (GkrellmAlert *alert,
                                    gpointer      user_data);


/*  public functions  */

void
gkrellm_wifi_alert_create (GkrellmWifiMonitor *wifimon)
{
  g_assert (wifimon != NULL);

  wifimon->alert = gkrellm_alert_create (NULL,
                                         wifimon->interface,
                                         _("Link Quality Percentage"),
                                         FALSE, TRUE, TRUE,
                                         ALERT_MAX,
                                         ALERT_MIN,
                                         ALERT_STEP0,
                                         ALERT_STEP1,
                                         ALERT_DIGITS);

  gkrellm_alert_delay_config (wifimon->alert,
                              DELAY_STEP,
                              DELAY_MAX,
                              DELAY_MIN);

  gkrellm_alert_trigger_connect (wifimon->alert,
                                 alert_trigger_callback,
                                 wifimon);
}


/*  private functions  */

static void
alert_trigger_callback (GkrellmAlert *alert,
                        gpointer      user_data)
{
  GkrellmWifiMonitor *wifimon;

  g_assert (alert != NULL);
  g_assert (user_data != NULL);

  wifimon = user_data;

  alert->panel = wifimon->chart->panel;
}
