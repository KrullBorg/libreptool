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

#ifndef __RPT_OBJ_TEXT_H__
#define __RPT_OBJ_TEXT_H__

#include <glib.h>
#include <glib-object.h>
#include <libxml/tree.h>

#include "rptobject.h"

G_BEGIN_DECLS


#define TYPE_RPT_OBJ_TEXT                 (rpt_obj_text_get_type ())
#define RPT_OBJ_TEXT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_RPT_OBJ_TEXT, RptObjText))
#define RPT_OBJ_TEXT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_RPT_OBJ_TEXT, RptObjTextClass))
#define IS_RPT_OBJ_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_RPT_OBJ_TEXT))
#define IS_RPT_OBJ_TEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_RPT_OBJ_TEXT))
#define RPT_OBJ_TEXT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_RPT_OBJ_TEXT, RptObjTextClass))


typedef struct _RptObjText RptObjText;
typedef struct _RptObjTextClass RptObjTextClass;

struct _RptObjText
	{
		RptObject parent;
	};

struct _RptObjTextClass
	{
		RptObjectClass parent_class;
	};

GType rpt_obj_text_get_type (void) G_GNUC_CONST;


RptObject *rpt_obj_text_new (const gchar *name, RptPoint position);
RptObject *rpt_obj_text_new_from_xml (xmlNode *xnode);

void rpt_obj_text_get_xml (RptObject *rpt_objtext, xmlNode *xnode);

G_END_DECLS

#endif /* __RPT_OBJ_TEXT_H__ */
