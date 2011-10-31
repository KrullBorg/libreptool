/*
 * Copyright (C) 2007-2011 Andrea Zagli <azagli@libero.it>
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
	PROP_POSITION,
	PROP_VISIBLE
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
		gboolean visibile;
	};

G_DEFINE_TYPE (RptObject, rpt_object, G_TYPE_OBJECT)

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
	g_object_class_install_property (object_class, PROP_VISIBLE,
	                                 g_param_spec_boolean ("visible",
	                                                       "Visibility",
	                                                       "The object's visibility.",
	                                                       TRUE,
	                                                       G_PARAM_READWRITE));
}

static void
rpt_object_init (RptObject *rpt_object)
{
	RptObjectPrivate *priv = RPT_OBJECT_GET_PRIVATE (rpt_object);

	priv->name = NULL;
	priv->position = NULL;
	priv->visibile = TRUE;
}

/**
 * rpt_object_new:
 * @name: the #RptObject's name.
 * @position: an #RptPoint that represents the object's position.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_object_new (const gchar *name, RptPoint position)
{
	RptObject *rpt_object = RPT_OBJECT (g_object_new (rpt_object_get_type (), NULL));

	g_object_set (rpt_object,
	              "name", name,
	              "poisition", &position,
	              NULL);

	return rpt_object;
}

/**
 * rpt_object_get_xml:
 * @rpt_object:
 * @xnode:
 *
 */
void
rpt_object_get_xml (RptObject *rpt_object, xmlNode *xnode)
{
	if (IS_RPT_OBJECT (rpt_object) && RPT_OBJECT_GET_CLASS (rpt_object)->get_xml != NULL)
		{
			RptObjectPrivate *priv = RPT_OBJECT_GET_PRIVATE (rpt_object);
		
			xmlSetProp (xnode, "name", priv->name);

			if (priv->position != NULL)
				{
					rpt_common_set_position (xnode, priv->position);
				}

			xmlSetProp (xnode, "visible", priv->visibile ? "y" : "n");

			RPT_OBJECT_GET_CLASS (rpt_object)->get_xml (rpt_object, xnode);
		}
}

/**
 * rpt_object_set_from_xml:
 * @rpt_object:
 * @xnode:
 *
 */
void
rpt_object_set_from_xml (RptObject *rpt_object, xmlNode *xnode)
{
	gchar *prop;

	g_return_if_fail (IS_RPT_OBJECT (rpt_object));

	prop = (gchar *)xmlGetProp (xnode, "visible");
	g_object_set (rpt_object, "visible", g_strcmp0 (prop, "y") == 0, NULL);
	if (prop != NULL) xmlFree (prop);
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

			case PROP_VISIBLE:
				priv->visibile = g_value_get_boolean (value);
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

			case PROP_VISIBLE:
				g_value_set_boolean (value, priv->visibile);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
