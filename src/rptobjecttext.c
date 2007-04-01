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
	PROP_SIZE,
	PROP_BORDER,
	PROP_FONT,
	PROP_ALIGN,
	PROP_SOURCE
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
		RptSize *size;
		RptBorder *border;
		RptFont *font;
		RptAlign *align;
		gchar *source;
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
	RptObjectClass *rptobject_class = RPT_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptObjTextPrivate));

	object_class->set_property = rpt_obj_text_set_property;
	object_class->get_property = rpt_obj_text_get_property;

	rptobject_class->get_xml = rpt_obj_text_get_xml;

	g_object_class_install_property (object_class, PROP_SIZE,
	                                 g_param_spec_pointer ("size",
	                                                       "Size",
	                                                       "The object's size.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_BORDER,
	                                 g_param_spec_pointer ("border",
	                                                       "Border",
	                                                       "The object's border.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_FONT,
	                                 g_param_spec_pointer ("font",
	                                                       "Font",
	                                                       "The object's font.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_ALIGN,
	                                 g_param_spec_pointer ("align",
	                                                       "Align",
	                                                       "The text's align.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_SOURCE,
	                                 g_param_spec_string ("source",
	                                                      "Source",
	                                                      "The source.",
	                                                      "",
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
rpt_obj_text_init (RptObjText *rpt_obj_text)
{
	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

	priv->size = (RptSize *)g_malloc0 (sizeof (RptSize));
	priv->size->width = 0.0;
	priv->size->height = 0.0;

	priv->border = (RptBorder *)g_malloc0 (sizeof (RptBorder));
	priv->font = (RptFont *)g_malloc0 (sizeof (RptFont));
	priv->align = (RptAlign *)g_malloc0 (sizeof (RptAlign));
}

/**
 * rpt_obj_text_new:
 * @name: the #RptObjText's name.
 * @position:
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_text_new (const gchar *name, RptPoint position)
{
	RptObject *rpt_obj_text = NULL;

	gchar *name_ = g_strstrip (g_strdup (name));

	if (strcmp (name_, "") != 0)
		{
			rpt_obj_text = RPT_OBJECT (g_object_new (rpt_obj_text_get_type (), NULL));

			g_object_set (G_OBJECT (rpt_obj_text),
	                      "name", name_,
	                      "position", &position,
	                      NULL);
		}

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

	name = g_strdup ((gchar *)xmlGetProp (xnode, "name"));
	if (name != NULL && strcmp (g_strstrip (name), "") != 0)
		{
			RptPoint position;
			RptObjTextPrivate *priv;

			rpt_common_get_position (xnode, &position);

			rpt_obj_text = rpt_obj_text_new ((const gchar *)name, position);

			if (rpt_obj_text != NULL)
				{
					priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

					rpt_common_get_size (xnode, priv->size);
					rpt_common_get_border (xnode, priv->border);
					rpt_common_get_font (xnode, priv->font);
					rpt_common_get_align (xnode, priv->align);

					g_object_set (rpt_obj_text, "source", xmlGetProp (xnode, "source"), NULL);
				}
		}

	return rpt_obj_text;
}

void
rpt_obj_text_get_xml (RptObject *rpt_objtext, xmlNode *xnode)
{
	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_objtext);

	xmlNodeSetName (xnode, "text");

	rpt_common_set_size (xnode, (RptSize)*priv->size);
	rpt_common_set_border (xnode, (RptBorder)*priv->border);
	rpt_common_set_font (xnode, (RptFont)*priv->font);
	rpt_common_set_align (xnode, (RptAlign)*priv->align);
}

static void
rpt_obj_text_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptObjText *rpt_obj_text = RPT_OBJ_TEXT (object);

	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

	switch (property_id)
		{
			case PROP_SIZE:
				priv->size = g_memdup (g_value_get_pointer (value), sizeof (RptSize));
				break;

			case PROP_BORDER:
				priv->border = g_memdup (g_value_get_pointer (value), sizeof (RptBorder));
				break;

			case PROP_FONT:
				priv->font = g_memdup (g_value_get_pointer (value), sizeof (RptFont));
				break;

			case PROP_ALIGN:
				priv->align = g_memdup (g_value_get_pointer (value), sizeof (RptAlign));
				break;

			case PROP_SOURCE:
				priv->source = g_strstrip (g_strdup (g_value_get_string (value)));
				break;

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
			case PROP_SIZE:
				g_value_set_pointer (value, g_memdup (priv->size, sizeof (RptSize)));
				break;

			case PROP_BORDER:
				g_value_set_pointer (value, g_memdup (priv->border, sizeof (RptBorder)));
				break;

			case PROP_FONT:
				g_value_set_pointer (value, g_memdup (priv->font, sizeof (RptFont)));
				break;

			case PROP_ALIGN:
				g_value_set_pointer (value, g_memdup (priv->align, sizeof (RptAlign)));
				break;

			case PROP_SOURCE:
				g_value_set_string (value, priv->source);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
