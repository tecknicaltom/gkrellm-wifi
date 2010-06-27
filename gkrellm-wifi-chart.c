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


#define GRID_RESOLUTION_MIN 0
#define GRID_RESOLUTION_MAX 100

#define GRID_STEP0          5
#define GRID_STEP1          10
#define GRID_DIGITS         0
#define GRID_WIDTH          50

#define KILO                1e3
#define MEGA                1e6
#define GIGA                1e9


/*  prototypes  */

static gboolean chart_expose_event_callback       (GtkWidget          *widget,
                                                   GdkEventExpose     *event,
                                                   gpointer            user_data);
static gboolean chart_button_press_event_callback (GtkWidget          *widget,
                                                   GdkEventButton     *event,
                                                   gpointer            user_data);
static gboolean panel_expose_event_callback       (GtkWidget          *widget,
                                                   GdkEventExpose     *event,
                                                   gpointer            user_data);
static gboolean panel_button_press_event_callback (GtkWidget          *widget,
                                                   GdkEventButton     *event,
                                                   gpointer            user_data);

static void     reorder_charts                    (void);
static void     draw_chart                        (gpointer            user_data);
static gchar *  create_chart_text                 (GkrellmWifiMonitor *wifimon);
static gchar *  strreplace                        (const gchar        *string,
                                                   const gchar        *delimiter,
                                                   const gchar        *replacement);


/*  public functions  */

void
gkrellm_wifi_chart_create (GkrellmWifiMonitor *wifimon)
{
  GkrellmStyle *style;
  gboolean      connect = FALSE;

  g_assert (wifimon != NULL);

  if (! wifimon->chart)
    {
      wifimon->chart = gkrellm_chart_new0 ();

      wifimon->chart->panel = gkrellm_panel_new0 ();

      connect = TRUE;
    }

  gkrellm_chart_create (gkrellm_wifi_vbox,
                        gkrellm_wifi,
                        wifimon->chart,
                        &wifimon->config);

  wifimon->data = gkrellm_add_default_chartdata (wifimon->chart,
                                                 _("Link Quality Percentage"));

  gkrellm_monotonic_chartdata (wifimon->data, FALSE);

  gkrellm_set_chartdata_draw_style_default (wifimon->data,
                                            CHARTDATA_LINE);

  gkrellm_chartconfig_grid_resolution_adjustment (wifimon->config,
                                                  FALSE, 1.0,
                                                  GRID_RESOLUTION_MIN,
                                                  GRID_RESOLUTION_MAX,
                                                  GRID_STEP0,
                                                  GRID_STEP1,
                                                  GRID_DIGITS,
                                                  GRID_WIDTH);

  gkrellm_chartconfig_grid_resolution_label (wifimon->config,
                                             _("Percent"));

  gkrellm_set_draw_chart_function (wifimon->chart, draw_chart, wifimon);

  gkrellm_alloc_chartdata (wifimon->chart);

  gkrellm_panel_label_on_top_of_decals (wifimon->chart->panel, TRUE);

  style = gkrellm_panel_style (gkrellm_wifi_style_id);

  gkrellm_panel_configure (wifimon->chart->panel,
                           wifimon->interface, style);

  gkrellm_panel_create (wifimon->chart->box,
                        gkrellm_wifi,
                        wifimon->chart->panel);

  gkrellm_setup_launcher (wifimon->chart->panel,
                          &wifimon->launcher,
                          CHART_PANEL_TYPE, 0);

  reorder_charts ();

  if (connect)
    {
      g_signal_connect (wifimon->chart->drawing_area,
                        "expose-event",
                        G_CALLBACK (chart_expose_event_callback),
                        wifimon);

      g_signal_connect (wifimon->chart->drawing_area,
                        "button-press-event",
                        G_CALLBACK (chart_button_press_event_callback),
                        wifimon);

      g_signal_connect (wifimon->chart->panel->drawing_area,
                        "expose-event",
                        G_CALLBACK (panel_expose_event_callback),
                        wifimon);

      g_signal_connect (wifimon->chart->panel->drawing_area,
                        "button-press-event",
                        G_CALLBACK (panel_button_press_event_callback),
                        wifimon);
    }
}


/*  private functions  */

static gboolean
chart_expose_event_callback (GtkWidget      *widget,
                             GdkEventExpose *event,
                             gpointer        user_data)
{
  GkrellmWifiMonitor *wifimon;

  g_assert (widget != NULL);
  g_assert (event != NULL);
  g_assert (user_data != NULL);

  wifimon = user_data;

  gdk_draw_drawable (widget->window,
                     widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                     wifimon->chart->pixmap, event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return FALSE;
}

static gboolean
chart_button_press_event_callback (GtkWidget      *widget,
                                   GdkEventButton *event,
                                   gpointer        user_data)
{
  GkrellmWifiMonitor *wifimon;

  g_assert (widget != NULL);
  g_assert (event != NULL);
  g_assert (user_data != NULL);

  wifimon = user_data;

  switch (event->button)
    {
    case 1:
      if (event->type == GDK_BUTTON_PRESS)
        {
          wifimon->hide_text = ! wifimon->hide_text;

          gkrellm_config_modified ();

          gkrellm_refresh_chart (wifimon->chart);
        }
      else if (event->type == GDK_2BUTTON_PRESS)
        {
          gkrellm_chartconfig_window_create (wifimon->chart);
        }
      break;

    case 3:
      gkrellm_chartconfig_window_create (wifimon->chart);
      break;
    }

  return FALSE;
}

static gboolean
panel_expose_event_callback (GtkWidget      *widget,
                             GdkEventExpose *event,
                             gpointer        user_data)
{
  GkrellmWifiMonitor *wifimon;

  g_assert (widget != NULL);
  g_assert (event != NULL);
  g_assert (user_data != NULL);

  wifimon = user_data;

  gdk_draw_drawable (widget->window,
                     widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                     wifimon->chart->panel->pixmap, event->area.x, event->area.y,
                     event->area.x, event->area.y,
                     event->area.width, event->area.height);

  return FALSE;
}

static gboolean
panel_button_press_event_callback (GtkWidget      *widget,
                                   GdkEventButton *event,
                                   gpointer        user_data)
{
  g_assert (widget != NULL);
  g_assert (event != NULL);
  g_assert (user_data != NULL);

  if (event->button == 3)
    gkrellm_open_config_window (gkrellm_wifi);

  return FALSE;
}

static void
reorder_charts (void)
{
  GkrellmWifiMonitor *wifimon;
  GList              *list;
  gint                i = 0;

  for (list = gkrellm_wifi_monitor_list; list; list = g_list_next (list))
    {
      wifimon = list->data;

      if (wifimon->chart)
        {
          gtk_box_reorder_child (GTK_BOX (gkrellm_wifi_vbox),
                                 GTK_WIDGET (wifimon->chart->box),
                                 i);

          i++;
        }
    }
}

static void
draw_chart (gpointer user_data)
{
  GkrellmWifiMonitor *wifimon;
  gchar              *tmp;

  g_assert (user_data != NULL);

  wifimon = user_data;

  gkrellm_draw_chartdata (wifimon->chart);

  if (! wifimon->hide_text && gkrellm_wifi_format_string)
    {
      tmp = create_chart_text (wifimon);

      gkrellm_draw_chart_text (wifimon->chart, gkrellm_wifi_style_id, tmp);

      g_free (tmp);
    }

  gkrellm_draw_chart_to_screen (wifimon->chart);
}

static gchar *
create_chart_text (GkrellmWifiMonitor *wifimon)
{
  gchar   *tmp;
  gchar   *ret;
  gchar   *value;
  gdouble  bitrate;

  g_assert (wifimon != NULL);

  ret = g_strdup (gkrellm_wifi_format_string);

  if (strstr (ret, "$M"))
    {
      value = g_strdup_printf ("%d", gkrellm_get_chart_scalemax (wifimon->chart));

      tmp = strreplace (ret, "$M", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$Q"))
    {
      value = g_strdup_printf ("%d", wifimon->percent);

      tmp = strreplace (ret, "$Q", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$q"))
    {
      value = g_strdup_printf ("%d", wifimon->quality);

      tmp = strreplace (ret, "$q", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$m"))
    {
      value = g_strdup_printf ("%d", wifimon->quality_max);

      tmp = strreplace (ret, "$m", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$s"))
    {
      value = g_strdup_printf ("%d", wifimon->signal);

      tmp = strreplace (ret, "$s", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$n"))
    {
      value = g_strdup_printf ("%d", wifimon->noise);

      tmp = strreplace (ret, "$n", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$R"))
    {
      value = g_strdup_printf ("%d", wifimon->signal - wifimon->noise);

      tmp = strreplace (ret, "$R", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (strstr (ret, "$B"))
    {
      bitrate = wifimon->bitrate;

      if (bitrate == 0)
        value = g_strdup ("0");
      else if (bitrate >= GIGA)
        value = g_strdup_printf ("%gG", bitrate / GIGA);
      else if (bitrate >= MEGA)
        value = g_strdup_printf ("%gM", bitrate / MEGA);
      else
        value = g_strdup_printf ("%gk", bitrate / KILO);

      tmp = strreplace (ret, "$B", value);
      g_free (value);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  if (wifimon->essid != NULL && strstr (ret, "$E"))
    {
      tmp = strreplace (ret, "$E", wifimon->essid);

      g_free (ret);
      ret = g_strdup (tmp);
      g_free (tmp);
    }

  return ret;
}

static gchar *
strreplace (const gchar *string,
            const gchar *delimiter,
            const gchar *replacement)
{
  gchar  *ret;
  gchar **tmp;

  g_return_val_if_fail (string != NULL, NULL);
  g_return_val_if_fail (delimiter != NULL, NULL);
  g_return_val_if_fail (replacement != NULL, NULL);

  tmp = g_strsplit (string, delimiter, 0);
  ret = g_strjoinv (replacement, tmp);
  g_strfreev (tmp);

  return ret;
}
