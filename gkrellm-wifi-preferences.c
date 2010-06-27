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
#include "gkrellm-wifi-monitor.h"

#include "gkrellm-wifi-preferences.h"


#define TEXT_HIDE_KEYWORD    "text_hide"
#define TEXT_FORMAT_KEYWORD  "text_format"

#define CHART_ENABLE_KEYWORD "enabled"
#define CHART_FORCE_KEYWORD  "forced"

#define LAUNCHER_CMD_KEYWORD "launch"
#define LAUNCHER_TIP_KEYWORD "tooltip"


static GtkWidget *format_combo;


/*  prototypes  */

static void enable_button_toggled_callback (GtkToggleButton *togglebutton,
                                            gpointer         user_data);
static void alert_button_clicked_callback  (GtkWidget       *button,
                                            gpointer         user_data);


/*  public functions  */

void
gkrellm_wifi_preferences_show (GtkWidget *tabs_vbox)
{
  GkrellmWifiMonitor *wifimon;
  GtkWidget          *tabs;
  GtkWidget          *vbox;
  GtkWidget          *hbox;
  GtkWidget          *frame;
  GtkWidget          *text;
  GtkWidget          *label;
  gchar              *tmp;
  GList              *format_list = NULL;
  GList              *list;
  GtkWidget          *table;
  gchar              *info[] = {
    "<h>WiFi Monitor\n",
    N_("This plug-in monitors the wireless LAN cards in your computer and\n"),
    N_("displays a graph of the link quality percentage for each card.\n"),
    "\n",
    N_("The plug-in requires the kernel to have support for the Linux Wireless\n"),
    N_("Extensions (CONFIG_NET_RADIO).\n"),
    "\n",
    N_("<h>Chart Labels\n"),
    N_("Substitution variables for the chart labels format string:\n"),
    N_("\t$M\tmaximum chart value\n"),
    N_("\t$Q\tlink quality in percent\n"),
    N_("\t$q\tlink quality\n"),
    N_("\t$m\tmaximum link quality of the card\n"),
    N_("\t$s\tsignal level in dBm\n"),
    N_("\t$n\tnoise level in dBm\n"),
    N_("\t$R\tsignal to noise ratio in dB\n"),
    N_("\t$B\tbit rate in bps\n"),
    N_("\t$E\textended network name (ESSID)\n"),
  };

  g_assert (tabs_vbox != NULL);

  tabs = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (tabs_vbox), tabs, TRUE, TRUE, 0);

  /*  device tabs  */
  for (list = gkrellm_wifi_monitor_list; list; list = g_list_next (list))
    {
      wifimon = list->data;

      vbox = gkrellm_gtk_framed_notebook_page (tabs, wifimon->interface);

      hbox = gtk_hbox_new (FALSE, 0);

      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 4);

      tmp = g_strdup_printf (_("Enable %s"), wifimon->interface);

      wifimon->enable_button = gtk_check_button_new_with_label (tmp);

      g_free (tmp);

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wifimon->enable_button),
                                    wifimon->enabled);

      g_signal_connect (wifimon->enable_button,
                        "toggled",
                        G_CALLBACK (enable_button_toggled_callback),
                        wifimon);

      gtk_box_pack_start (GTK_BOX (hbox), wifimon->enable_button, FALSE, FALSE, 0);

      gkrellm_gtk_alert_button (hbox, &wifimon->alert_button,
                                FALSE, FALSE, 4, FALSE,
                                alert_button_clicked_callback, wifimon);

      gtk_widget_set_sensitive (wifimon->alert_button, wifimon->enabled);

      hbox = gtk_hbox_new (FALSE, 0);

      gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

      wifimon->force_button = gtk_check_button_new_with_label (_("Force chart to be shown even if interface is not detected"));

      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wifimon->force_button),
                                    wifimon->forced);

      gtk_widget_set_sensitive (wifimon->force_button, wifimon->enabled);

      gtk_box_pack_start (GTK_BOX (hbox), wifimon->force_button, FALSE, FALSE, 0);

      frame = gkrellm_gtk_framed_vbox_end (vbox, _("Launch Command"),
                                           4, FALSE, 0, 2);

      table = gkrellm_gtk_launcher_table_new (frame, 1);

      gkrellm_gtk_config_launcher (table, 0,
                                   &wifimon->command_entry,
                                   &wifimon->tooltip_entry,
                                   wifimon->interface,
                                   &wifimon->launcher);

      gtk_widget_set_sensitive (wifimon->command_entry, wifimon->enabled);

      gtk_widget_set_sensitive (wifimon->tooltip_entry, wifimon->enabled);
    }

  /*  setup tab  */
  vbox = gkrellm_gtk_framed_notebook_page (tabs, _("Setup"));

  frame = gkrellm_gtk_framed_vbox (vbox, _("Chart Labels Format String"),
                                   4, FALSE, 0, 4);

  format_combo = gtk_combo_new ();

  format_list = g_list_append (format_list, gkrellm_wifi_format_string);
  format_list = g_list_append (format_list, GKRELLM_WIFI_DEFAULT_FORMAT_STRING);
  format_list = g_list_append (format_list, "\\t$Q%\\t\\r$M\\b$R\\fdB");
  format_list = g_list_append (format_list, "\\t$q/$m\\b$s/$n\\fdBm");
  format_list = g_list_append (format_list, "\\t$Q%\\b$n\\fdBm\\p$s\\fdBm");
  format_list = g_list_append (format_list, "\\t$Q%\\r$B\\b$n\\fdBm\\p$s\\fdBm");
  format_list = g_list_append (format_list, "\\t$Q%\\r$B\\b$E: $R\\fdB");

  gtk_combo_set_popdown_strings (GTK_COMBO (format_combo), format_list);

  g_list_free (format_list);

  gtk_box_pack_start (GTK_BOX (frame), format_combo, TRUE, TRUE, 0);

  /*  info tab  */
  vbox = gkrellm_gtk_framed_notebook_page (tabs, _("Info"));

  text = gkrellm_gtk_scrolled_text_view (vbox, NULL, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gkrellm_gtk_text_view_append_strings (text, info, sizeof (info) / sizeof (gchar *));

  /*  about tab  */
  vbox = gkrellm_gtk_framed_notebook_page (tabs, _("About"));

  tmp = g_strdup_printf (_("%s %d.%d.%d\n"
                           "GKrellM2 Wireless Link Monitor plug-in for Linux\n\n"
                           "Copyright (C) 2003 Henrik Brix Andersen <brix@gimp.org>\n"
                           "http://brix.gimp.org\n\n"
                           "Released under the GNU General Public License"),
                         GKRELLM_WIFI_PLUGIN_NAME,
                         GKRELLM_WIFI_VERSION_MAJOR,
                         GKRELLM_WIFI_VERSION_MINOR,
                         GKRELLM_WIFI_VERSION_MICRO);

  label = gtk_label_new (tmp);
  gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, FALSE, 0);

  g_free (tmp);
}

void
gkrellm_wifi_preferences_apply (void)
{
  GkrellmWifiMonitor *wifimon;
  GList   *list;

  if (gkrellm_wifi_format_string)
    g_free (gkrellm_wifi_format_string);

  gkrellm_wifi_format_string = g_strdup (gkrellm_gtk_entry_get_text (&GTK_COMBO (format_combo)->entry));

  for (list = gkrellm_wifi_monitor_list; list; list = g_list_next (list))
    {
      wifimon = list->data;

      wifimon->enabled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wifimon->enable_button));
      wifimon->forced  = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wifimon->force_button));

      if (wifimon->chart)
        {
          gkrellm_apply_launcher (&wifimon->command_entry,
                                  &wifimon->tooltip_entry,
                                  wifimon->chart->panel,
                                  &wifimon->launcher,
                                  gkrellm_launch_button_cb);

          gkrellm_refresh_chart (wifimon->chart);
        }
      else
        {
          if (wifimon->launcher.command)
            g_free (wifimon->launcher.command);

          wifimon->launcher.command = g_strdup (gkrellm_gtk_entry_get_text (&wifimon->command_entry));

          if (wifimon->launcher.tooltip_comment)
            g_free (wifimon->launcher.tooltip_comment);

          wifimon->launcher.tooltip_comment = g_strdup (gkrellm_gtk_entry_get_text (&wifimon->tooltip_entry));
        }
    }
}

void
gkrellm_wifi_preferences_save (FILE *file)
{
  GkrellmWifiMonitor *wifimon;
  GList   *list;

  g_assert (file != NULL);

  for (list = gkrellm_wifi_monitor_list; list; list = g_list_next (list))
    {
      wifimon = list->data;

      gkrellm_save_chartconfig (file, wifimon->config,
                                GKRELLM_WIFI_PLUGIN_KEYWORD, wifimon->interface);

      gkrellm_save_alertconfig (file, wifimon->alert,
                                GKRELLM_WIFI_PLUGIN_KEYWORD, wifimon->interface);

      fprintf (file, "%s %s %s %d\n",
               GKRELLM_WIFI_PLUGIN_KEYWORD,
               TEXT_HIDE_KEYWORD,
               wifimon->interface,
               wifimon->hide_text);

      fprintf (file, "%s %s %s %d\n",
               GKRELLM_WIFI_PLUGIN_KEYWORD,
               CHART_ENABLE_KEYWORD,
               wifimon->interface,
               wifimon->enabled);

      fprintf (file, "%s %s %s %d\n",
               GKRELLM_WIFI_PLUGIN_KEYWORD,
               CHART_FORCE_KEYWORD,
               wifimon->interface,
               wifimon->forced);

      if (wifimon->launcher.command &&
          strlen (wifimon->launcher.command) > 0)
        {
          fprintf (file, "%s %s %s %s\n",
                   GKRELLM_WIFI_PLUGIN_KEYWORD,
                   LAUNCHER_CMD_KEYWORD,
                   wifimon->interface,
                   wifimon->launcher.command);
        }

      if (wifimon->launcher.tooltip_comment &&
          strlen (wifimon->launcher.tooltip_comment) > 0)
        {
          fprintf (file, "%s %s %s %s\n",
                   GKRELLM_WIFI_PLUGIN_KEYWORD,
                   LAUNCHER_TIP_KEYWORD,
                   wifimon->interface,
                   wifimon->launcher.tooltip_comment);
        }
    }

  if (gkrellm_wifi_format_string)
    {
      fprintf (file, "%s %s %s\n",
               GKRELLM_WIFI_PLUGIN_KEYWORD,
               TEXT_FORMAT_KEYWORD,
               gkrellm_wifi_format_string);
    }
}

void
gkrellm_wifi_preferences_load (gchar *line)
{
  GkrellmWifiMonitor  *wifimon;
  gchar    *keyword;
  gchar    *data;
  gchar    *interface;
  gchar    *config;
  gboolean  choice;
  gint      items;

  g_assert (line != NULL);

  keyword   = g_new (gchar, strlen (line) + 1);
  data      = g_new (gchar, strlen (line) + 1);
  interface = g_new (gchar, strlen (line) + 1);
  config    = g_new (gchar, strlen (line) + 1);

  items = sscanf (line, "%s %[^\n]", keyword, data);

  if (items == 2)
    {
      if (g_ascii_strcasecmp (keyword, GKRELLM_CHARTCONFIG_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %[^\n]", interface, config);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              gkrellm_load_chartconfig (&wifimon->config, config, 1);
            }
        }
      if (g_ascii_strcasecmp (keyword, GKRELLM_ALERTCONFIG_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %[^\n]", interface, config);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              if (! wifimon->alert)
                gkrellm_wifi_alert_create (wifimon);

              gkrellm_load_alertconfig (&wifimon->alert, config);
            }
        }
      else if (g_ascii_strcasecmp (keyword, TEXT_HIDE_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %d\n", interface, &choice);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              wifimon->hide_text = choice;
            }
        }
      else if (g_ascii_strcasecmp (keyword, CHART_ENABLE_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %d\n", interface, &choice);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              wifimon->enabled = choice;
            }
        }
      else if (g_ascii_strcasecmp (keyword, CHART_FORCE_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %d\n", interface, &choice);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);


              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              wifimon->forced = choice;
            }
        }
      else if (g_ascii_strcasecmp (keyword, LAUNCHER_CMD_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %[^\n]", interface, config);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              if (wifimon->launcher.command)
                g_free (wifimon->launcher.command);

              wifimon->launcher.command = g_strdup (config);
            }
        }
      else if (g_ascii_strcasecmp (keyword, LAUNCHER_TIP_KEYWORD) == 0)
        {
          items = sscanf (data, "%s %[^\n]", interface, config);

          if (items == 2)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                wifimon = gkrellm_wifi_monitor_create (interface);

              if (wifimon->launcher.tooltip_comment)
                g_free (wifimon->launcher.tooltip_comment);

              wifimon->launcher.tooltip_comment = g_strdup (config);
            }
        }
      else if (g_ascii_strcasecmp (keyword, TEXT_FORMAT_KEYWORD) == 0)
        {
          if (gkrellm_wifi_format_string)
            g_free (gkrellm_wifi_format_string);

          gkrellm_wifi_format_string = g_strdup (data);
        }
    }

  g_free (keyword);
  g_free (data);
  g_free (interface);
  g_free (config);
}


/*  private functions  */

static void
enable_button_toggled_callback (GtkToggleButton *togglebutton,
                                gpointer         user_data)
{
  GkrellmWifiMonitor *wifimon;
  gboolean            sensitive;

  g_assert (togglebutton != NULL);
  g_assert (user_data != NULL);

  wifimon = user_data;

  sensitive = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wifimon->enable_button));

  gtk_widget_set_sensitive (wifimon->force_button, sensitive);
  gtk_widget_set_sensitive (wifimon->alert_button, sensitive);
  gtk_widget_set_sensitive (wifimon->command_entry, sensitive);
  gtk_widget_set_sensitive (wifimon->tooltip_entry, sensitive);
}

static void
alert_button_clicked_callback (GtkWidget *button,
                               gpointer   user_data)
{
  GkrellmWifiMonitor *wifimon;

  g_assert (button != NULL);
  g_assert (user_data != NULL);

  wifimon = user_data;

  if (! wifimon->alert)
    gkrellm_wifi_alert_create (wifimon);

  gkrellm_alert_config_window (&wifimon->alert);
}
