/*
 * Copyright (C) 2007 Andrea Zagli <azagli@libero.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

#ifndef __RPT_REPORT_PRIV_H__
#define __RPT_REPORT_PRIV_H__

#include <glib.h>

#include "rptreport.h"

G_BEGIN_DECLS


gchar *rpt_report_get_field (RptReport *rpt_report,
                             const gchar *field_name,
                             gint row);
gchar *rpt_report_ask_field (RptReport *rpt_report,
                             const gchar *field,
                             gint row);
gchar *rpt_report_get_special (RptReport *rpt_report,
                               const gchar *special,
                               gint row);


G_END_DECLS

#endif /* __RPT_REPORT_PRIV_H__ */
