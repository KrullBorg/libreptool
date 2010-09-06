/*
 * Copyright (C) 2007-2010 Andrea Zagli <azagli@inwind.it>
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

#include "rptobjectellipse.h"
#include "rptcommon.h"

enum
{
	PROP_0
};

static void rpt_obj_ellipse_class_init (RptObjEllipseClass *klass);
static void rpt_obj_ellipse_init (RptObjEllipse *rpt_obj_ellipse);

static void rpt_obj_ellipse_set_property (GObject *object,
                                          guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec);
static void rpt_obj_ellipse_get_property (GObject *object,
                                          guint property_id,
                                          GValue *value,
                                          GParamSpec *pspec);


#define RPT_OBJ_ELLIPSE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_OBJ_ELLIPSE, RptObjEllipsePrivate))

typedef struct _RptObjEllipsePrivate RptObjEllipsePrivate;
struct _RptObjEllipsePrivate
	{
		gpointer foo;
	};

GType
rpt_obj_ellipse_get_type (void)
{
	static GType rpt_obj_ellipse_type = 0;

	if (!rpt_obj_ellipse_type)
		{
			static const GTypeInfo rpt_obj_ellipse_info =
			{
				sizeof (RptObjEllipseClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_obj_ellipse_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptObjEllipse),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_obj_ellipse_init,
				NULL
			};

			rpt_obj_ellipse_type = g_type_register_static (TYPE_RPT_OBJ_RECT, "RptObjEllipse",
			                                               &rpt_obj_ellipse_info, 0);
		}

	return rpt_obj_ellipse_type;
}

static void
rpt_obj_ellipse_class_init (RptObjEllipseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	RptObjectClass *rptobject_class = RPT_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjEllipsePrivate));

	object_class->set_property = rpt_obj_ellipse_set_property;
	object_class->get_property = rpt_obj_ellipse_get_property;

	rptobject_class->get_xml = rpt_obj_ellipse_get_xml;
}

static void
rpt_obj_ellipse_init (RptObjEllipse *rpt_obj_ellipse)
{
	RptObjEllipsePrivate *priv = RPT_OBJ_ELLIPSE_GET_PRIVATE (rpt_obj_ellipse);
}

/**
 * rpt_obj_ellipse_new:
 * @name: the #RptObjEllipse's name.
 * @position: an #RptPoint.
 *
 * Creates a new #RptObjEllipse object and sets its position to @position.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_ellipse_new (const gchar *name, RptPoint position)
{
	RptObject *rpt_obj_ellipse = NULL;

	gchar *name_ = g_strstrip (g_strdup (name));

	if (strcmp (name_, "") != 0)
		{
			rpt_obj_ellipse = RPT_OBJECT (g_object_new (rpt_obj_ellipse_get_type (), NULL));

			g_object_set (G_OBJECT (rpt_obj_ellipse),
	                      "name", name_,
	                      "position", &position,
	                      NULL);
		}

	return rpt_obj_ellipse;
}

/**
 * rpt_obj_ellipse_new_from_xml:
 * @xnode:
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_ellipse_new_from_xml (xmlNode *xnode)
{
	gchar *name;
	RptObject *rpt_obj_ellipse = NULL;

	name = g_strdup ((gchar *)xmlGetProp (xnode, "name"));
	if (name != NULL && strcmp (g_strstrip (name), "") != 0)
		{
			RptPoint *position;
			RptObjEllipsePrivate *priv;

			position = rpt_common_get_position (xnode);

			rpt_obj_ellipse = rpt_obj_ellipse_new ((const gchar *)name, *position);

			if (rpt_obj_ellipse != NULL)
				{
					const gchar *prop;
					RptSize *size;
					RptStroke *stroke;

					priv = RPT_OBJ_ELLIPSE_GET_PRIVATE (rpt_obj_ellipse);

					size = rpt_common_get_size (xnode);
					stroke = rpt_common_get_stroke (xnode);
					g_object_set (G_OBJECT (rpt_obj_ellipse),
					              "size", size,
					              "stroke", stroke,
					              NULL);

					prop = (const gchar *)xmlGetProp (xnode, "fill-color");
					if (prop != NULL)
						{
							RptColor *color;

							color = rpt_common_parse_color (prop);
							g_object_set (rpt_obj_ellipse, "fill-color", color, NULL);
						}
				}
		}

	return rpt_obj_ellipse;
}

/**
 * rpt_obj_ellipse_get_xml:
 * @rpt_object:
 * @xnode:
 *
 */
void
rpt_obj_ellipse_get_xml (RptObject *rpt_object, xmlNode *xnode)
{
	RptObjEllipsePrivate *priv = RPT_OBJ_ELLIPSE_GET_PRIVATE (RPT_OBJ_ELLIPSE (rpt_object));

	rpt_obj_rect_get_xml (rpt_object, xnode);

	xmlNodeSetName (xnode, "ellipse");
}

static void
rpt_obj_ellipse_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObjEllipse *rpt_obj_ellipse = RPT_OBJ_ELLIPSE (object);

	RptObjEllipsePrivate *priv = RPT_OBJ_ELLIPSE_GET_PRIVATE (rpt_obj_ellipse);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_obj_ellipse_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptObjEllipse *rpt_obj_ellipse = RPT_OBJ_ELLIPSE (object);

	RptObjEllipsePrivate *priv = RPT_OBJ_ELLIPSE_GET_PRIVATE (rpt_obj_ellipse);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
