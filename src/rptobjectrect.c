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
 */

#include "rptobjectrect.h"
#include "rptcommon.h"

enum
{
	PROP_0,
	PROP_FILL_COLOR
};

static void rpt_obj_rect_class_init (RptObjRectClass *klass);
static void rpt_obj_rect_init (RptObjRect *rpt_obj_rect);

static void rpt_obj_rect_set_property (GObject *object,
                                       guint property_id,
                                       const GValue *value,
                                       GParamSpec *pspec);
static void rpt_obj_rect_get_property (GObject *object,
                                       guint property_id,
                                       GValue *value,
                                       GParamSpec *pspec);


#define RPT_OBJ_RECT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_OBJ_RECT, RptObjRectPrivate))

typedef struct _RptObjRectPrivate RptObjRectPrivate;
struct _RptObjRectPrivate
	{
		RptColor *fill_color;
	};

GType
rpt_obj_rect_get_type (void)
{
	static GType rpt_obj_rect_type = 0;

	if (!rpt_obj_rect_type)
		{
			static const GTypeInfo rpt_obj_rect_info =
			{
				sizeof (RptObjRectClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_obj_rect_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptObjRect),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_obj_rect_init,
				NULL
			};

			rpt_obj_rect_type = g_type_register_static (TYPE_RPT_OBJ_LINE, "RptObjRect",
			                                            &rpt_obj_rect_info, 0);
		}

	return rpt_obj_rect_type;
}

static void
rpt_obj_rect_class_init (RptObjRectClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	RptObjectClass *rptobject_class = RPT_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjRectPrivate));

	object_class->set_property = rpt_obj_rect_set_property;
	object_class->get_property = rpt_obj_rect_get_property;

	rptobject_class->get_xml = rpt_obj_rect_get_xml;

	g_object_class_install_property (object_class, PROP_FILL_COLOR,
	                                 g_param_spec_pointer ("fill-color",
	                                                       "Fill Color",
	                                                       "The object's fill color.",
	                                                       G_PARAM_READWRITE));
}

static void
rpt_obj_rect_init (RptObjRect *rpt_obj_rect)
{
	RptObjRectPrivate *priv = RPT_OBJ_RECT_GET_PRIVATE (rpt_obj_rect);

	priv->fill_color = NULL;
}

/**
 * rpt_obj_rect_new:
 * @name: the #RptObjRect's name.
 * @position: an #RptPoint.
 *
 * Creates a new #RptObjRect object and sets its position to @position.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_rect_new (const gchar *name, RptPoint position)
{
	RptObject *rpt_obj_rect = NULL;

	gchar *name_ = g_strstrip (g_strdup (name));

	if (strcmp (name_, "") != 0)
		{
			rpt_obj_rect = RPT_OBJECT (g_object_new (rpt_obj_rect_get_type (), NULL));

			g_object_set (G_OBJECT (rpt_obj_rect),
	                      "name", name_,
	                      "position", &position,
	                      NULL);
		}

	return rpt_obj_rect;
}

/**
 * rpt_obj_rect_new_from_xml:
 * @xnode:
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_rect_new_from_xml (xmlNode *xnode)
{
	gchar *name;
	RptObject *rpt_obj_rect = NULL;

	name = g_strdup ((gchar *)xmlGetProp (xnode, "name"));
	if (name != NULL && strcmp (g_strstrip (name), "") != 0)
		{
			RptPoint *position;
			RptObjRectPrivate *priv;

			position = rpt_common_get_position (xnode);

			rpt_obj_rect = rpt_obj_rect_new ((const gchar *)name, *position);

			if (rpt_obj_rect != NULL)
				{
					const gchar *prop;
					RptSize *size;
					RptRotation *rotation;
					RptStroke *stroke;

					priv = RPT_OBJ_RECT_GET_PRIVATE (rpt_obj_rect);

					size = rpt_common_get_size (xnode);
					rotation = rpt_common_get_rotation (xnode);
					stroke = rpt_common_get_stroke (xnode);
					g_object_set (G_OBJECT (rpt_obj_rect),
					              "size", size,
					              "stroke", stroke,
					              "rotation", rotation,
					              NULL);

					prop = (const gchar *)xmlGetProp (xnode, "fill-color");
					if (prop != NULL)
						{
							priv->fill_color = rpt_common_parse_color (prop);
						}
				}
		}

	return rpt_obj_rect;
}

/**
 * rpt_obj_rect_get_xml:
 * @rpt_object:
 * @xnode:
 *
 */
void
rpt_obj_rect_get_xml (RptObject *rpt_object, xmlNode *xnode)
{
	RptObjRectPrivate *priv = RPT_OBJ_RECT_GET_PRIVATE (RPT_OBJ_RECT (rpt_object));

	rpt_obj_line_get_xml (rpt_object, xnode);

	xmlNodeSetName (xnode, "rect");

	if (priv->fill_color != NULL)
		{
			xmlSetProp (xnode, "fill-color", rpt_common_rptcolor_to_string (priv->fill_color));
		}
}

static void
rpt_obj_rect_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObjRect *rpt_obj_rect = RPT_OBJ_RECT (object);

	RptObjRectPrivate *priv = RPT_OBJ_RECT_GET_PRIVATE (rpt_obj_rect);

	switch (property_id)
		{
			case PROP_FILL_COLOR:
				priv->fill_color = g_memdup (g_value_get_pointer (value), sizeof (RptColor));
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_obj_rect_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptObjRect *rpt_obj_rect = RPT_OBJ_RECT (object);

	RptObjRectPrivate *priv = RPT_OBJ_RECT_GET_PRIVATE (rpt_obj_rect);

	switch (property_id)
		{
			case PROP_FILL_COLOR:
				if (priv->fill_color == NULL)
					{
						RptColor *color = rpt_common_rptcolor_new ();

						color->r = 1.0;
						color->g = 1.0;
						color->b = 1.0;
						color->a = 1.0;

						g_value_set_pointer (value, g_memdup (color, sizeof (RptColor)));
					}
				else
					{
						g_value_set_pointer (value, g_memdup (priv->fill_color, sizeof (RptColor)));
					}
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
