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

#include "rptobjectimage.h"
#include "rptcommon.h"

enum
{
	PROP_0,
	PROP_SIZE,
	PROP_ROTATION,
	PROP_BORDER,
	PROP_SOURCE,
	PROP_ADAPT
};

static void rpt_obj_image_class_init (RptObjImageClass *klass);
static void rpt_obj_image_init (RptObjImage *rpt_obj_image);

static void rpt_obj_image_set_property (GObject *object,
                                        guint property_id,
                                        const GValue *value,
                                        GParamSpec *pspec);
static void rpt_obj_image_get_property (GObject *object,
                                        guint property_id,
                                        GValue *value,
                                        GParamSpec *pspec);


#define RPT_OBJ_IMAGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_OBJ_IMAGE, RptObjImagePrivate))

typedef struct _RptObjImagePrivate RptObjImagePrivate;
struct _RptObjImagePrivate
	{
		RptSize *size;
		RptRotation *rotation;
		RptBorder *border;
		gchar *source;
		guint adapt;
	};

GType
rpt_obj_image_get_type (void)
{
	static GType rpt_obj_image_type = 0;

	if (!rpt_obj_image_type)
		{
			static const GTypeInfo rpt_obj_image_info =
			{
				sizeof (RptObjImageClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_obj_image_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptObjImage),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_obj_image_init,
				NULL
			};

			rpt_obj_image_type = g_type_register_static (TYPE_RPT_OBJECT, "RptObjImage",
			                                             &rpt_obj_image_info, 0);
		}

	return rpt_obj_image_type;
}

static void
rpt_obj_image_class_init (RptObjImageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	RptObjectClass *rptobject_class = RPT_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjImagePrivate));

	object_class->set_property = rpt_obj_image_set_property;
	object_class->get_property = rpt_obj_image_get_property;

	rptobject_class->get_xml = rpt_obj_image_get_xml;

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
	g_object_class_install_property (object_class, PROP_BORDER,
	                                 g_param_spec_pointer ("border",
	                                                       "Border",
	                                                       "The object's border.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_SOURCE,
	                                 g_param_spec_string ("source",
	                                                      "Source",
	                                                      "The image's source.",
	                                                      "",
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (object_class, PROP_ADAPT,
	                                 g_param_spec_uint ("adapt",
	                                                    "Adapt",
	                                                    "Whether to adapt the image.",
	                                                    RPT_OBJ_IMAGE_ADAPT_NONE, RPT_OBJ_IMAGE_ADAPT_TO_IMAGE, 0,
	                                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
rpt_obj_image_init (RptObjImage *rpt_obj_image)
{
	RptObjImagePrivate *priv = RPT_OBJ_IMAGE_GET_PRIVATE (rpt_obj_image);

	priv->size = (RptSize *)g_malloc0 (sizeof (RptSize));
	priv->size->width = 0.0;
	priv->size->height = 0.0;

	priv->rotation = NULL;
	priv->border = NULL;
}

/**
 * rpt_obj_image_new:
 * @name: the #RptObjImage's name.
 * @position: an #RptPoint.
 *
 * Creates a new #RptObjImage object and sets its position to @position.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_image_new (const gchar *name, RptPoint position)
{
	RptObject *rpt_obj_image = NULL;

	gchar *name_ = g_strstrip (g_strdup (name));

	if (strcmp (name_, "") != 0)
		{
			rpt_obj_image = RPT_OBJECT (g_object_new (rpt_obj_image_get_type (), NULL));

			g_object_set (G_OBJECT (rpt_obj_image),
	                      "name", name_,
	                      "position", &position,
	                      NULL);
		}

	return rpt_obj_image;
}

/**
 * rpt_obj_image_new_from_xml:
 * @xnode:
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_image_new_from_xml (xmlNode *xnode)
{
	gchar *name;
	RptObject *rpt_obj_image = NULL;

	name = g_strdup ((gchar *)xmlGetProp (xnode, "name"));
	if (name != NULL && strcmp (g_strstrip (name), "") != 0)
		{
			RptPoint *position;
			RptObjImagePrivate *priv;

			position = rpt_common_get_position (xnode);

			rpt_obj_image = rpt_obj_image_new ((const gchar *)name, *position);

			if (rpt_obj_image != NULL)
				{
					priv = RPT_OBJ_IMAGE_GET_PRIVATE (rpt_obj_image);

					priv->size = rpt_common_get_size (xnode);
					priv->rotation = rpt_common_get_rotation (xnode);
					priv->border = rpt_common_get_border (xnode);

					priv->source = (gchar *)xmlGetProp (xnode, "source");

					if (xmlStrcasecmp (xmlGetProp (xnode, "adapt"), (const xmlChar *)"to-box") == 0)
						{
							priv->adapt = RPT_OBJ_IMAGE_ADAPT_TO_BOX;
						}
					else if (xmlStrcasecmp (xmlGetProp (xnode, "adapt"), (const xmlChar *)"to-image") == 0)
						{
							priv->adapt = RPT_OBJ_IMAGE_ADAPT_TO_IMAGE;
						}
				}
		}

	return rpt_obj_image;
}

/**
 * rpt_obj_image_get_xml:
 * @rpt_objimage:
 * @xnode:
 *
 */
void
rpt_obj_image_get_xml (RptObject *rpt_objimage, xmlNode *xnode)
{
	RptObjImagePrivate *priv = RPT_OBJ_IMAGE_GET_PRIVATE (rpt_objimage);

	xmlNodeSetName (xnode, "image");

	rpt_common_set_size (xnode, priv->size);
	rpt_common_set_rotation (xnode, priv->rotation);
	rpt_common_set_border (xnode, priv->border);

	xmlSetProp (xnode, "source", priv->source);

	switch (priv->adapt)
		{
			case RPT_OBJ_IMAGE_ADAPT_TO_BOX:
				xmlSetProp (xnode, "adapt", "to-box");
				break;

			case RPT_OBJ_IMAGE_ADAPT_TO_IMAGE:
				xmlSetProp (xnode, "adapt", "to-image");
				break;
		}
}

static void
rpt_obj_image_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObjImage *rpt_obj_image = RPT_OBJ_IMAGE (object);

	RptObjImagePrivate *priv = RPT_OBJ_IMAGE_GET_PRIVATE (rpt_obj_image);

	switch (property_id)
		{
			case PROP_SIZE:
				priv->size = g_memdup (g_value_get_pointer (value), sizeof (RptSize));
				break;

			case PROP_ROTATION:
				priv->rotation = g_memdup (g_value_get_pointer (value), sizeof (RptRotation));
				break;

			case PROP_BORDER:
				priv->border = g_memdup (g_value_get_pointer (value), sizeof (RptBorder));
				break;

			case PROP_SOURCE:
				priv->source = g_strstrip (g_strdup (g_value_get_string (value)));
				break;

			case PROP_ADAPT:
				priv->adapt = g_value_get_uint (value);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_obj_image_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptObjImage *rpt_obj_image = RPT_OBJ_IMAGE (object);

	RptObjImagePrivate *priv = RPT_OBJ_IMAGE_GET_PRIVATE (rpt_obj_image);

	switch (property_id)
		{
			case PROP_SIZE:
				g_value_set_pointer (value, g_memdup (priv->size, sizeof (RptSize)));
				break;

			case PROP_ROTATION:
				g_value_set_pointer (value, g_memdup (priv->rotation, sizeof (RptRotation)));
				break;

			case PROP_BORDER:
				g_value_set_pointer (value, g_memdup (priv->border, sizeof (RptBorder)));
				break;

			case PROP_SOURCE:
				g_value_set_string (value, priv->source);
				break;

			case PROP_ADAPT:
				g_value_set_uint (value, priv->adapt);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
