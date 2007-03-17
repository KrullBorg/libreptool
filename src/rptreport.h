/*
 * Copyright (C) 2006-2007 Andrea Zagli <azagli@inwind.it>
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

#ifndef __RPT_REPORT_H__
#define __RPT_REPORT_H__

#include <glib.h>
#include <glib-object.h>
#include <libxml/tree.h>

G_BEGIN_DECLS


#define TYPE_RPT_REPORT                 (rpt_report_get_type ())
#define RPT_REPORT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_RPT_REPORT, RptReport))
#define RPT_REPORT_COMMON_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_RPT_REPORT, RptReportClass))
#define IS_RPT_REPORT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_RPT_REPORT))
#define IS_RPT_REPORT_COMMON_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_RPT_REPORT))
#define RPT_REPORT_COMMON_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_RPT_REPORT, RptReportClass))


typedef struct _RptReport RptReport;
typedef struct _RptReportClass RptReportClass;

struct _RptReport
	{
		GObject parent;
	};

struct _RptReportClass
	{
		GObjectClass parent_class;
	};

GType rpt_report_get_type (void) G_GNUC_CONST;


RptReport *rpt_report_new_from_xml (xmlDoc *xdoc);
RptReport *rpt_report_new_from_file (const gchar *filename);

xmlDoc *rpt_report_get_xml (RptReport *rptreport);

xmlDoc *rpt_report_get_rptprint (RptReport *rptreport);


G_END_DECLS

#endif /* __RPT_REPORT_H__ */
