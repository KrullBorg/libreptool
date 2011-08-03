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

#include "rptobjectline.h"
#include "rptcommon.h"

enum
{
	PROP_0,
	PROP_SIZE,
	PROP_ROTATION,
	PROP_STROKE
};

static void rpt_obj_line_class_init (RptObjLineClass *klass);
static void rpt_obj_line_init (RptObjLine *rpt_obj_line);

static void rpt_obj_line_set_property (GObject *object,
                                       guint property_id,
                                       const GValue *value,
                                       GParamSpec *pspec);
static void rpt_obj_line_get_property (GObject *object,
                                       guint property_id,
                                       GValue *value,
                                       GParamSpec *pspec);


#define RPT_OBJ_LINE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_OBJ_LINE, RptObjLinePrivate))

typedef struct _RptObjLinePrivate RptObjLinePrivate;
struct _RptObjLinePrivate
	{
		RptSize *size;
		RptRotation *rotation;
		RptStroke *stroke;
	};

GType
rpt_obj_line_get_type (void)
{
	static GType rpt_obj_line_type = 0;

	if (!rpt_obj_line_type)
		{
			static const GTypeInfo rpt_obj_line_info =
			{
				sizeof (RptObjLineClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_obj_line_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptObjLine),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_obj_line_init,
				NULL
			};

			rpt_obj_line_type = g_type_register_static (TYPE_RPT_OBJECT, "RptObjLine",
			                                            &rpt_obj_line_info, 0);
		}

	return rpt_obj_line_type;
}

static void
rpt_obj_line_class_init (RptObjLineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	RptObjectClass *rptobject_class = RPT_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjLinePrivate));

	object_class->set_property = rpt_obj_line_set_property;
	object_class->get_property = rpt_obj_line_get_property;

	rptobject_class->get_xml = rpt_obj_line_get_xml;

	g_object_class_install_property (object_class, PROP_SIZE,
	                                 g_param_spec_pointer ("size",
	                                                       "Size",
	                                                       "The object's size.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_ROTATION,
	                                 g_param_spec_pointer ("rotation",
	                                                       "Rotation",
	                                                       "The object's rotation.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_STROKE,
	                                 g_param_spec_pointer ("stroke",
	                                                       "Stroke",
	                                                       "The object's stroke.",
	                                                       G_PARAM_READWRITE));
}

static void
rpt_obj_line_init (RptObjLine *rpt_obj_line)
{
	RptObjLinePrivate *priv = RPT_OBJ_LINE_GET_PRIVATE (rpt_obj_line);

	priv->size = (RptSize *)g_malloc0 (sizeof (RptSize));
	priv->size->width = 0.0;
	priv->size->height = 0.0;

	priv->rotation = NULL;
	priv->stroke = NULL;
}

/**
 * rpt_obj_line_new:
 * @name: the #RptObjLine's name.
 * @position: an #RptPoint.
 *
 * Creates a new #RptObjLine object and sets its position to @position.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_line_new (const gchar *name, RptPoint position)
{
	RptObject *rpt_obj_line = NULL;

	gchar *name_ = g_strstrip (g_strdup (name));

	if (strcmp (name_, "") != 0)
		{
			rpt_obj_line = RPT_OBJECT (g_object_new (rpt_obj_line_get_type (), NULL));

			g_object_set (G_OBJECT (rpt_obj_line),
	                      "name", name_,
	                      "position", &position,
	                      NULL);
		}

	return rpt_obj_line;
}

/**
 * rpt_obj_line_new_from_xml:
 * @xnode:
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_line_new_from_xml (xmlNode *xnode)
{
	gchar *name;
	RptObject *rpt_obj_line = NULL;

	name = g_strdup ((gchar *)xmlGetProp (xnode, "name"));
	if (name != NULL && strcmp (g_strstrip (name), "") != 0)
		{
			RptPoint *position;
			RptObjLinePrivate *priv;

			position = rpt_common_get_position (xnode);

			rpt_obj_line = rpt_obj_line_new ((const gchar *)name, *position);

			if (rpt_obj_line != NULL)
				{
					priv = RPT_OBJ_LINE_GET_PRIVATE (rpt_obj_line);

					priv->size = rpt_common_get_size (xnode);
					priv->rotation = rpt_common_get_rotation (xnode);
					priv->stroke = rpt_common_get_stroke (xnode);
				}
		}

	return rpt_obj_line;
}

/**
 * rpt_obj_line_get_xml:
 * @rpt_objline:
 * @xnode:
 *
 */
void
rpt_obj_line_get_xml (RptObject *rpt_objline, xmlNode *xnode)
{
	RptObjLinePrivate *priv = RPT_OBJ_LINE_GET_PRIVATE (rpt_objline);

	xmlNodeSetName (xnode, "line");

	rpt_common_set_size (xnode, priv->size);
	rpt_common_set_rotation (xnode, priv->rotation);
	rpt_common_set_stroke (xnode, priv->stroke);
}

static void
rpt_obj_line_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObjLine *rpt_obj_line = RPT_OBJ_LINE (object);

	RptObjLinePrivate *priv = RPT_OBJ_LINE_GET_PRIVATE (rpt_obj_line);

	switch (property_id)
		{
			case PROP_SIZE:
				priv->size = g_memdup (g_value_get_pointer (value), sizeof (RptSize));
				break;

			case PROP_ROTATION:
				priv->rotation = g_memdup (g_value_get_pointer (value), sizeof (RptRotation));
				break;

			case PROP_STROKE:
				priv->stroke = g_memdup (g_value_get_pointer (value), sizeof (RptStroke));
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_obj_line_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptObjLine *rpt_obj_line = RPT_OBJ_LINE (object);

	RptObjLinePrivate *priv = RPT_OBJ_LINE_GET_PRIVATE (rpt_obj_line);

	switch (property_id)
		{
			case PROP_SIZE:
				g_value_set_pointer (value, g_memdup (priv->size, sizeof (RptSize)));
				break;

			case PROP_ROTATION:
				g_value_set_pointer (value, g_memdup (priv->rotation, sizeof (RptRotation)));
				break;

			case PROP_STROKE:
				if (priv->stroke == NULL)
					{
						RptStroke *stroke = rpt_common_rptstroke_new ();

						stroke->width = 1.0;
						stroke->color = rpt_common_rptcolor_new ();
						stroke->color->a = 1.0;

						g_value_set_pointer (value, g_memdup (stroke, sizeof (RptStroke)));
					}
				else
					{
						g_value_set_pointer (value, g_memdup (priv->stroke, sizeof (RptStroke)));
					}
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
