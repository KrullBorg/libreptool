/*
 * Copyright (C) 2007-2011 Andrea Zagli <azagli@libero.it>
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
#include <gtk/gtk.h>
#include <libgda/libgda.h>
#include <libxml/tree.h>

#include "rptobject.h"

G_BEGIN_DECLS


#define TYPE_RPT_REPORT                 (rpt_report_get_type ())
#define RPT_REPORT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_RPT_REPORT, RptReport))
#define RPT_REPORT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_RPT_REPORT, RptReportClass))
#define IS_RPT_REPORT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_RPT_REPORT))
#define IS_RPT_REPORT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_RPT_REPORT))
#define RPT_REPORT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_RPT_REPORT, RptReportClass))


typedef struct _RptReport RptReport;
typedef struct _RptReportClass RptReportClass;

struct _RptReport
	{
		GObject parent;
	};

struct _RptReportClass
	{
		GObjectClass parent_class;

		guint field_request_signal_id;
	};

GType rpt_report_get_type (void) G_GNUC_CONST;


typedef enum
{
	RPTREPORT_SECTION_REPORT_HEADER,
	RPTREPORT_SECTION_REPORT_FOOTER,
	RPTREPORT_SECTION_PAGE_HEADER,
	RPTREPORT_SECTION_PAGE_FOOTER,
	RPTREPORT_SECTION_BODY
} RptReportSection;

RptReport *rpt_report_new (void);

RptReport *rpt_report_new_from_xml (xmlDoc *xdoc);
RptReport *rpt_report_new_from_file (const gchar *filename);

RptReport *rpt_report_new_from_gtktreeview (GtkTreeView *view,
                                            const gchar *title);

void rpt_report_set_output_type (RptReport *rpt_report, eRptOutputType output_type);
void rpt_report_set_output_filename (RptReport *rpt_report, const gchar *output_filename);

void rpt_report_set_copies (RptReport *rpt_report, guint copies);

const gchar *rpt_report_database_get_provider (RptReport *rpt_report);
const gchar *rpt_report_database_get_connection_string (RptReport *rpt_report);
const gchar *rpt_report_database_get_sql (RptReport *rpt_report);
void rpt_report_set_database (RptReport *rpt_report,
                              const gchar *provider_id,
                              const gchar *connection_string,
                              const gchar *sql);

void rpt_report_set_database_from_datamodel (RptReport *rpt_report, GdaDataModel *data_model);

void rpt_report_set_database_as_gtktreemodel (RptReport *rpt_report,
                                              GtkTreeModel *model,
                                              GHashTable *columns_names);

RptSize *rpt_report_get_page_size (RptReport *rpt_report);
void rpt_report_set_page_size (RptReport *rpt_report,
                               RptSize size);

void rpt_report_get_page_margins (RptReport *rpt_report,
                                  gdouble *top,
                                  gdouble *right,
                                  gdouble *bottom,
                                  gdouble *left);
void rpt_report_set_page_margins (RptReport *rpt_report,
                                  gdouble top,
                                  gdouble right,
                                  gdouble bottom,
                                  gdouble left);

RptMargin *rpt_report_get_page_margins_struct (RptReport *rpt_report);
void rpt_report_set_page_margins_struct (RptReport *rpt_report, RptMargin margin);

gdouble rpt_report_get_section_height (RptReport *rpt_report,
                                       RptReportSection section);
void rpt_report_set_section_height (RptReport *rpt_report,
                                    RptReportSection section,
                                    gdouble height);
GList *rpt_report_section_get_objects (RptReport *rpt_report,
                                       RptReportSection section);
void rpt_report_section_remove (RptReport *rpt_report, RptReportSection section);

gboolean rpt_report_get_report_header_new_page_after (RptReport *rpt_report);
void rpt_report_set_report_header_new_page_after (RptReport *rpt_report,
                                                  gboolean new_page_after);

gboolean rpt_report_get_report_footer_new_page_before (RptReport *rpt_report);
void rpt_report_set_report_footer_new_page_before (RptReport *rpt_report,
                                                   gboolean new_page_before);

gboolean rpt_report_get_page_header_first_page (RptReport *rpt_report);
gboolean rpt_report_get_page_header_last_page (RptReport *rpt_report);
void rpt_report_set_page_header_first_last_page (RptReport *rpt_report,
                                                 gboolean first_page,
                                                 gboolean last_page);

gboolean rpt_report_get_page_footer_first_page (RptReport *rpt_report);
gboolean rpt_report_get_page_footer_last_page (RptReport *rpt_report);
void rpt_report_set_page_footer_first_last_page (RptReport *rpt_report,
                                                 gboolean first_page,
                                                 gboolean last_page);

gboolean rpt_report_body_get_new_page_after (RptReport *rpt_report);
void rpt_report_body_set_new_page_after (RptReport *rpt_report, gboolean new_page_after);

gboolean rpt_report_add_object_to_section (RptReport *rpt_report,
                                           RptObject *rpt_object,
                                           RptReportSection section);
void rpt_report_remove_object (RptReport *rpt_report,
                               RptObject *rpt_object);

RptObject *rpt_report_get_object_from_name (RptReport *rpt_report,
                                            const gchar *name);

xmlDoc *rpt_report_get_xml (RptReport *rpt_report);

xmlDoc *rpt_report_get_xml_rptprint (RptReport *rpt_report);

xmlDoc *rpt_report_rptprint_new (void);
void rpt_report_rptprint_set_unit_length (xmlDoc *xdoc, eRptUnitLength unit);
void rpt_report_rptprint_set_output_type (xmlDoc *xdoc, eRptOutputType output_type);
void rpt_report_rptprint_set_output_filename (xmlDoc *xdoc, const gchar *output_filename);
void rpt_report_rptprint_set_copies (xmlDoc *xdoc, guint copies);
xmlNode *rpt_report_rptprint_page_new (xmlDoc *xdoc, RptSize *size, RptMargin *margin);
void rpt_report_rptprint_page_add_object (xmlNode *xnodepage, RptObject *rpt_object);


G_END_DECLS

#endif /* __RPT_REPORT_H__ */
