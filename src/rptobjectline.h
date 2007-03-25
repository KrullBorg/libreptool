/*
 * Copyright (C) 2007 Andrea Zagli <azagli@inwind.it>
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

#ifndef __RPT_OBJ_LINE_H__
#define __RPT_OBJ_LINE_H__

#include <glib.h>
#include <glib-object.h>
#include <libxml/tree.h>

#include "rptobject.h"

G_BEGIN_DECLS


#define TYPE_RPT_OBJ_LINE                 (rpt_obj_line_get_type ())
#define RPT_OBJ_LINE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_RPT_OBJ_LINE, RptObjLine))
#define RPT_OBJ_LINE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_RPT_OBJ_LINE, RptObjLineClass))
#define IS_RPT_OBJ_LINE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_RPT_OBJ_LINE))
#define IS_RPT_OBJ_LINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_RPT_OBJ_LINE))
#define RPT_OBJ_LINE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_RPT_OBJ_LINE, RptObjLineClass))


typedef struct _RptObjLine RptObjLine;
typedef struct _RptObjLineClass RptObjLineClass;

struct _RptObjLine
	{
		RptObject parent;
	};

struct _RptObjLineClass
	{
		RptObjectClass parent_class;
	};

GType rpt_obj_line_get_type (void) G_GNUC_CONST;


RptObject *rpt_obj_line_new (const gchar *name, RptPoint position);
RptObject *rpt_obj_line_new_from_xml (xmlNode *xnode);

void rpt_obj_line_get_xml (RptObject *rpt_objline, xmlNode *xnode);

G_END_DECLS

#endif /* __RPT_OBJ_LINE_H__ */
