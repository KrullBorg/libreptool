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

#include "rptobject.h"

enum
{
	PROP_0,
	PROP_NAME,
	PROP_POSITION
};

static void rpt_object_class_init (RptObjectClass *klass);
static void rpt_object_init (RptObject *rpt_object);

static void rpt_object_set_property (GObject *object,
                                    guint property_id,
                                    const GValue *value,
                                    GParamSpec *pspec);
static void rpt_object_get_property (GObject *object,
                                    guint property_id,
                                    GValue *value,
                                    GParamSpec *pspec);


#define RPT_OBJECT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_OBJECT, RptObjectPrivate))

typedef struct _RptObjectPrivate RptObjectPrivate;
struct _RptObjectPrivate
	{
		gchar *name;
		RptPoint *position;
	};

GType
rpt_object_get_type (void)
{
	static GType rpt_object_type = 0;

	if (!rpt_object_type)
		{
			static const GTypeInfo rpt_object_info =
			{
				sizeof (RptObjectClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_object_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptObject),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_object_init,
				NULL
			};

			rpt_object_type = g_type_register_static (G_TYPE_OBJECT, "RptObject",
			                                          &rpt_object_info, 0);
		}

	return rpt_object_type;
}

static void
rpt_object_class_init (RptObjectClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjectPrivate));

	klass->get_xml = NULL;

	object_class->set_property = rpt_object_set_property;
	object_class->get_property = rpt_object_get_property;

	g_object_class_install_property (object_class, PROP_NAME,
	                                 g_param_spec_string ("name",
	                                                      "Name",
	                                                      "The object's name.",
	                                                      "",
	                                                      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_POSITION,
	                                 g_param_spec_pointer ("position",
	                                                       "Position",
	                                                       "The object's position.",
	                                                       G_PARAM_READWRITE));
}

static void
rpt_object_init (RptObject *rpt_object)
{
}

/**
 * rpt_object_new:
 * @name: the #RptObject's name.
 * @position: an #RptPoint that represents the object's position.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_object_new (const gchar *name, RptPoint *position)
{
	RptObject *rpt_object = RPT_OBJECT (g_object_new (rpt_object_get_type (), NULL));;

	g_object_set (rpt_object,
	              "name", name,
	              "poisition", position,
	              NULL);

	return rpt_object;
}

void
rpt_object_get_xml (RptObject *rpt_object, xmlNode *xnode)
{
	if (IS_RPT_OBJECT (rpt_object) && RPT_OBJECT_GET_CLASS (rpt_object)->get_xml != NULL)
		{
			RptObjectPrivate *priv = RPT_OBJECT_GET_PRIVATE (rpt_object);
		
			xmlSetProp (xnode, "name", priv->name);
			xmlSetProp (xnode, "x", g_strdup_printf ("%f", priv->position->x));
			xmlSetProp (xnode, "y", g_strdup_printf ("%f", priv->position->y));

			RPT_OBJECT_GET_CLASS (rpt_object)->get_xml (rpt_object, xnode);
		}
}

static void
rpt_object_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObject *rpt_object = RPT_OBJECT (object);

	RptObjectPrivate *priv = RPT_OBJECT_GET_PRIVATE (rpt_object);

	switch (property_id)
		{
			case PROP_NAME:
				priv->name = g_strstrip (g_strdup (g_value_get_string (value)));
				break;

			case PROP_POSITION:
				priv->position = g_memdup (g_value_get_pointer (value), sizeof (RptPoint));
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_object_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptObject *rpt_object = RPT_OBJECT (object);

	RptObjectPrivate *priv = RPT_OBJECT_GET_PRIVATE (rpt_object);

	switch (property_id)
		{
			case PROP_NAME:
				g_value_set_string (value, priv->name);
				break;

			case PROP_POSITION:
				g_value_set_pointer (value, g_memdup (priv->position, sizeof (RptPoint)));
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
