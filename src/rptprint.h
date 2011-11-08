/*
 * Copyright (C) 2006-2011 Andrea Zagli <azagli@inwind.it>
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

#ifndef __RPT_PRINT_H__
#define __RPT_PRINT_H__

#include <glib.h>
#include <glib-object.h>
#include <libxml/tree.h>
#include <gtk/gtk.h>

#include "rptcommon.h"

G_BEGIN_DECLS


#define TYPE_RPT_PRINT                 (rpt_print_get_type ())
#define RPT_PRINT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_RPT_PRINT, RptPrint))
#define RPT_PRINT_COMMON_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_RPT_PRINT, RptPrintClass))
#define IS_RPT_PRINT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_RPT_PRINT))
#define IS_RPT_PRINT_COMMON_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_RPT_PRINT))
#define RPT_PRINT_COMMON_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_RPT_PRINT, RptPrintClass))


typedef struct _RptPrint RptPrint;
typedef struct _RptPrintClass RptPrintClass;

struct _RptPrint
	{
		GObject parent;
	};

struct _RptPrintClass
	{
		GObjectClass parent_class;
	};

GType rpt_print_get_type (void) G_GNUC_CONST;


RptPrint *rpt_print_new_from_xml (xmlDoc *xdoc);
RptPrint *rpt_print_new_from_file (const gchar *filename);

void rpt_print_set_output_type (RptPrint *rpt_print, eRptOutputType output_type);
void rpt_print_set_output_filename (RptPrint *rpt_print, const gchar *output_filename);

void rpt_print_set_gtkprintsettings (RptPrint *rpt_print, GtkPrintSettings *settings);
void rpt_print_set_copies (RptPrint *rpt_print, guint copies);
void rpt_print_set_translation (RptPrint *rpt_print, RptTranslation *translation);

void rpt_print_print (RptPrint *rpt_print, GtkWindow *transient);


G_END_DECLS

#endif /* __RPT_PRINT_H__ */
