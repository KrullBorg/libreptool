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

#ifndef __RPT_OBJ_RECT_H__
#define __RPT_OBJ_RECT_H__

#include <glib.h>
#include <glib-object.h>
#include <libxml/tree.h>

#include "rptobjectline.h"

G_BEGIN_DECLS


#define TYPE_RPT_OBJ_RECT                 (rpt_obj_rect_get_type ())
#define RPT_OBJ_RECT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_RPT_OBJ_RECT, RptObjRect))
#define RPT_OBJ_RECT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_RPT_OBJ_RECT, RptObjRectClass))
#define IS_RPT_OBJ_RECT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_RPT_OBJ_RECT))
#define IS_RPT_OBJ_RECT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_RPT_OBJ_RECT))
#define RPT_OBJ_RECT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_RPT_OBJ_RECT, RptObjRectClass))


typedef struct _RptObjRect RptObjRect;
typedef struct _RptObjRectClass RptObjRectClass;

struct _RptObjRect
	{
		RptObjLine parent;
	};

struct _RptObjRectClass
	{
		RptObjLineClass parent_class;
	};

GType rpt_obj_rect_get_type (void) G_GNUC_CONST;


RptObject *rpt_obj_rect_new (const gchar *name, RptPoint position);
RptObject *rpt_obj_rect_new_from_xml (xmlNode *xnode);

void rpt_obj_rect_get_xml (RptObject *rpt_object, xmlNode *xnode);

G_END_DECLS

#endif /* __RPT_OBJ_RECT_H__ */
