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
 */

#include "rptobjecttext.h"
#include "rptcommon.h"

enum
{
	PROP_0,
	PROP_POSITION,
	PROP_SIZE
};

static void rpt_obj_text_class_init (RptObjTextClass *klass);
static void rpt_obj_text_init (RptObjText *rpt_obj_text);

static void rpt_obj_text_set_property (GObject *object,
                                    guint property_id,
                                    const GValue *value,
                                    GParamSpec *pspec);
static void rpt_obj_text_get_property (GObject *object,
                                    guint property_id,
                                    GValue *value,
                                    GParamSpec *pspec);


#define RPT_OBJ_TEXT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_OBJ_TEXT, RptObjTextPrivate))

typedef struct _RptObjTextPrivate RptObjTextPrivate;
struct _RptObjTextPrivate
	{
		RptPoint *position;
		RptSize *size;

		gchar *text;
	};

GType
rpt_obj_text_get_type (void)
{
	static GType rpt_obj_text_type = 0;

	if (!rpt_obj_text_type)
		{
			static const GTypeInfo rpt_obj_text_info =
			{
				sizeof (RptObjTextClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_obj_text_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptObjText),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_obj_text_init,
				NULL
			};

			rpt_obj_text_type = g_type_register_static (TYPE_RPT_OBJECT, "RptObjText",
			                                            &rpt_obj_text_info, 0);
		}

	return rpt_obj_text_type;
}

static void
rpt_obj_text_class_init (RptObjTextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjTextPrivate));

	object_class->set_property = rpt_obj_text_set_property;
	object_class->get_property = rpt_obj_text_get_property;
}

static void
rpt_obj_text_init (RptObjText *rpt_obj_text)
{
}

/**
 * rpt_obj_text_new:
 * @name: the #RptObjText's name.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_text_new (const gchar *name)
{
	RptObject *rpt_obj_text = RPT_OBJ_TEXT (g_object_new (rpt_obj_text_get_type (), NULL));

	g_object_set (rpt_obj_text, "name", name, NULL);

	return rpt_obj_text;
}

/**
 * rpt_obj_text_new_from_xml:
 * @xnode:
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_text_new_from_xml (xmlNode *xnode)
{
	gchar *name;
	RptObject *rpt_obj_text = NULL;

	name = (gchar *)xmlGetProp (xnode, "name");
	if (strcmp (g_strstrip (name), "") != 0)
		{
			RptObjTextPrivate *priv;

			rpt_obj_text = rpt_obj_text_new (name);

			priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

			rpt_common_get_position (xnode, priv->position);
			rpt_common_get_size (xnode, priv->size);
		}

	return rpt_obj_text;
}

static void
rpt_obj_text_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObjText *rpt_obj_text = RPT_OBJ_TEXT (object);

	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_obj_text_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptObjText *rpt_obj_text = RPT_OBJ_TEXT (object);

	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
