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

#include <errno.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <linux/wireless.h>

#include "gkrellm-wifi-monitor.h"

#include "gkrellm-wifi-linux.h"


#define MAX_LINE_LENGTH     128
#define PROC_NET_WIRELESS   "/proc/net/wireless"
#define DEFAULT_QUALITY_MAX 96
#define DEFAULT_BITRATE     0


/*  the following is needed for backwards compatibility with older
    wireless extension versions - compat code inspired by the code in
    iwlib.c from wireless_tools.27 copyright (C) Jean Tourrilhes
    <jt@hpl.hp.com>  */

#define MAGIC_10_LENGTH          300

#define IW_MAX_FREQUENCIES_15    16
#define IW_MAX_BITRATES_15       8
#define IW_MAX_ENCODING_SIZES_15 8
#define IW_MAX_TXPOWER_15        8

#define IW_MAX_BITRATES_16       32
#define IW_MAX_ENCODING_SIZES_16 8
#define IW_MAX_TXPOWER_16        8
#define IW_MAX_FREQUENCIES_16    32


struct iw_freq_15
{
  __s32 m;
  __s16 e;
  __u8  i;
  __u8  pad;
};

struct iw_quality_15
{
  __u8 qual;
  __u8 level;
  __u8 noise;
  __u8 updated;
};

struct iw_range_15
{
  __u32                throughput;
  __u32                min_nwid;
  __u32                max_nwid;
  __u16                num_channels;
  __u8                 num_frequency;
  struct iw_freq_15    freq[IW_MAX_FREQUENCIES_15];
  __s32                sensitivity;
  struct iw_quality_15 max_qual;
  __u8                 num_bitrates;
  __s32                bitrate[IW_MAX_BITRATES_15];
  __s32                min_rts;
  __s32                max_rts;
  __s32                min_frag;
  __s32                max_frag;
  __s32                min_pmp;
  __s32                max_pmp;
  __s32                min_pmt;
  __s32                max_pmt;
  __u16                pmp_flags;
  __u16                pmt_flags;
  __u16                pm_capa;
  __u16                encoding_size[IW_MAX_ENCODING_SIZES_15];
  __u8                 num_encoding_sizes;
  __u8                 max_encoding_tokens;
  __u16                txpower_capa;
  __u8                 num_txpower;
  __s32                txpower[IW_MAX_TXPOWER_15];
  __u8                 we_version_compiled;
  __u8                 we_version_source;
  __u16                retry_capa;
  __u16                retry_flags;
  __u16                r_time_flags;
  __s32                min_retry;
  __s32                max_retry;
  __s32                min_r_time;
  __s32                max_r_time;
  struct iw_quality_15 avg_qual;
};

struct iw_quality_16
{
  __u8 qual;
  __u8 level;
  __u8 noise;
  __u8 updated;
};

struct iw_freq_16
{
  __s32 m;
  __s16 e;
  __u8  i;
  __u8  pad;
};

struct iw_range_16
{
  __u32                throughput;
  __u32                min_nwid;
  __u32                max_nwid;
  __u16                old_num_channels;
  __u8                 old_num_frequency;
  __s32                old_freq[6];
  __s32                sensitivity;
  struct iw_quality_16 max_qual;
  struct iw_quality_16 avg_qual;
  __u8                 num_bitrates;
  __s32                bitrate[IW_MAX_BITRATES_16];
  __s32                min_rts;
  __s32                max_rts;
  __s32                min_frag;
  __s32                max_frag;
  __s32                min_pmp;
  __s32                max_pmp;
  __s32                min_pmt;
  __s32                max_pmt;
  __u16                pmp_flags;
  __u16                pmt_flags;
  __u16                pm_capa;
  __u16                encoding_size[IW_MAX_ENCODING_SIZES_16];
  __u8                 num_encoding_sizes;
  __u8                 max_encoding_tokens;
  __u8                 encoding_login_index;
  __u16                txpower_capa;
  __u8                 num_txpower;
  __s32                txpower[IW_MAX_TXPOWER_16];
  __u8                 we_version_compiled;
  __u8                 we_version_source;
  __u16                retry_capa;
  __u16                retry_flags;
  __u16                r_time_flags;
  __s32                min_retry;
  __s32                max_retry;
  __s32                min_r_time;
  __s32                max_r_time;
  __u16                num_channels;
  __u8                 num_frequency;
  struct iw_freq_16    freq[IW_MAX_FREQUENCIES_16];
};

/*  end of compatibility cruft  */


/*  prototypes  */

static guint8  get_quality_max (const gchar *interface);
static gint32  get_bitrate     (const gchar *interface);
static gchar * get_essid       (const gchar *interface);


/*  public functions  */

void
gkrellm_wifi_wireless_info_read (void)
{
  GkrellmWifiMonitor *wifimon;
  FILE               *file;
  gchar               line[MAX_LINE_LENGTH];
  gint                lineno = 0;
  gint                items;
  gchar               interface[MAX_LINE_LENGTH];
  gint                quality;
  gint                signal;
  gint                noise;
  static gboolean     warn = TRUE;

  file = fopen (PROC_NET_WIRELESS, "r");

  if (file != NULL)
    {
      while (fgets (line, sizeof (line), file))
        {
          if (++lineno < 3)
            continue;

          items = sscanf (line, " %[^: ] : %*x %d%*[. ] %d%*[. ] %d%*[. ] %*d %*d %*d %*d %*d %*d\n",
                          interface, &quality, &signal, &noise);

          if (items == 4)
            {
              wifimon = gkrellm_wifi_monitor_find (interface);

              if (! wifimon)
                {
                  wifimon = gkrellm_wifi_monitor_create (interface);

                  wifimon->enabled = TRUE;
                }

              wifimon->quality     = quality;
              wifimon->quality_max = get_quality_max (wifimon->interface);
              wifimon->signal      = signal - 0x100;
              wifimon->noise       = noise  - 0x100;
              wifimon->bitrate     = get_bitrate (wifimon->interface);

              if (wifimon->essid)
                g_free (wifimon->essid);

              wifimon->essid = get_essid (wifimon->interface);

              if (wifimon->quality > wifimon->quality_max)
                {
                  /*  quality is in dBm  */
                  wifimon->percent = rint (100 * (wifimon->quality / wifimon->quality_max));
                }
              else
                {
                  /*  quality is relative  */
                  wifimon->percent = rint (100 * (log (wifimon->quality) / log (wifimon->quality_max)));
                }

              wifimon->percent = CLAMP (wifimon->percent, 0, 100);
              wifimon->updated = TRUE;
            }
          else
            {
              g_message (_("Parse error in %s line %d, skipping line..."),
                         PROC_NET_WIRELESS, lineno);

              continue;
            }
        }

      fclose (file);
    }
  else if (warn)
    {
      g_warning (_("Could not open %s for reading, no wireless extensions found..."),
                 PROC_NET_WIRELESS);

      warn = FALSE;
    }
}


/*  private functions  */

static guint8
get_quality_max (const gchar *interface)
{
  struct iw_range range;
  struct iwreq    request;
  gchar           buffer[sizeof (range) * 2] = { 0 };
  gint            fd;
  guint8          ret;
  gint            offset;

  g_assert (interface != NULL);

  fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd >= 0)
    {
      request.u.data.pointer = (caddr_t) &buffer;
      request.u.data.length  = sizeof (buffer);
      request.u.data.flags   = 0;

      strncpy (request.ifr_name, interface, IFNAMSIZ);

      if (ioctl (fd, SIOCGIWRANGE, &request) >= 0)
        {
          memcpy (&range, buffer, sizeof (range));

          if (request.u.data.length < MAGIC_10_LENGTH)
            range.we_version_compiled = 10;

          if (range.we_version_compiled <= 15)
            {
              offset = G_STRUCT_OFFSET (struct iw_range_15, max_qual) +
                       G_STRUCT_OFFSET (struct iw_quality_15, qual);
            }
          else
            {
              offset = G_STRUCT_OFFSET (struct iw_range_16, max_qual) +
                       G_STRUCT_OFFSET (struct iw_quality_16, qual);
            }

          memcpy (&ret, buffer + offset, sizeof (ret));
        }
      else
        {
          g_message (_("Could not get range for %s: %s"),
                     interface, g_strerror (errno));

          ret = DEFAULT_QUALITY_MAX;
        }

      close (fd);
    }
  else
    {
      g_warning (_("Could not open socket: %s"),
                 g_strerror (errno));

      ret = DEFAULT_QUALITY_MAX;
    }

  return ret;
}

static gint32
get_bitrate (const gchar *interface)
{
  struct iwreq request;
  gint         fd;
  gint         ret;

  g_assert (interface != NULL);

  fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd >= 0)
    {
      strncpy (request.ifr_name, interface, IFNAMSIZ);

      if (ioctl (fd, SIOCGIWRATE, &request) >= 0)
        ret = request.u.bitrate.value;
      else
        ret = DEFAULT_BITRATE;

      close (fd);
    }
  else
    {
      g_warning (_("Could not open socket: %s"),
                 g_strerror (errno));

      ret = DEFAULT_BITRATE;
    }

  return ret;
}

static gchar *
get_essid (const gchar *interface)
{
  struct iwreq  request;
  gchar         buffer[IW_ESSID_MAX_SIZE + 1] = { 0 };
  gint          fd;
  gchar        *ret;

  g_assert (interface != NULL);

  fd = socket (AF_INET, SOCK_DGRAM, 0);

  if (fd >= 0)
    {
      request.u.essid.pointer = (caddr_t) &buffer;
      request.u.essid.length  = sizeof (buffer);
      request.u.essid.flags   = 0;

      strncpy (request.ifr_name, interface, IFNAMSIZ);

      if (ioctl (fd, SIOCGIWESSID, &request) >= 0)
        {
          if (request.u.data.flags)
            ret = g_strdup (buffer);
          else
            ret = g_strdup (_("off/any"));
        }
      else
        {
          ret = g_strdup (_("n/a"));
        }

      close (fd);
    }
  else
    {
      g_warning (_("Could not open socket: %s"),
                 g_strerror (errno));

      ret = g_strdup (_("n/a"));
    }

  return ret;
}
