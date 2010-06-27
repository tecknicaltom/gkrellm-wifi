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


#ifndef __GKRELLM_WIFI_PREFERENCES_H__
#define __GKRELLM_WIFI_PREFERENCES_H__


void gkrellm_wifi_preferences_show  (GtkWidget *vbox);
void gkrellm_wifi_preferences_apply (void);
void gkrellm_wifi_preferences_save  (FILE      *file);
void gkrellm_wifi_preferences_load  (gchar     *line);


#endif  /*  __GKRELLM_WIFI_PREFERENCES_H__  */
