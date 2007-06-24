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
	PROP_ROTATION,
	PROP_BORDER,
	PROP_FONT,
	PROP_ALIGN,
	PROP_SOURCE,
	PROP_BACKGROUND_COLOR,
	PROP_PADDING_TOP,
	PROP_PADDING_RIGHT,
	PROP_PADDING_BOTTOM,
	PROP_PADDING_LEFT
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
		RptRotation *rotation;
		RptBorder *border;
		RptFont *font;
		RptAlign *align;
		gchar *source;
		RptColor *background_color;
		gdouble padding_top;
		gdouble padding_right;
		gdouble padding_bottom;
		gdouble padding_left;
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
	g_object_class_install_property (object_class, PROP_BACKGROUND_COLOR,
	                                 g_param_spec_pointer ("background-color",
	                                                       "Background Color",
	                                                       "The text's background color.",
	                                                       G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_PADDING_TOP,
	                                 g_param_spec_double ("padding-top",
	                                                      "Padding Top",
	                                                      "Padding Top",
	                                                      0, G_MAXDOUBLE, 0,
	                                                      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_PADDING_RIGHT,
	                                 g_param_spec_double ("padding-right",
	                                                      "Padding Right",
	                                                      "Padding Right",
	                                                      0, G_MAXDOUBLE, 0,
	                                                      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_PADDING_BOTTOM,
	                                 g_param_spec_double ("padding-bottom",
	                                                      "Padding Bottom",
	                                                      "Padding Bottom",
	                                                      0, G_MAXDOUBLE, 0,
	                                                      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_PADDING_LEFT,
	                                 g_param_spec_double ("padding-left",
	                                                      "Padding Left",
	                                                      "Padding Left",
	                                                      0, G_MAXDOUBLE, 0,
	                                                      G_PARAM_READWRITE));
}

static void
rpt_obj_text_init (RptObjText *rpt_obj_text)
{
	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

	RptFont *font;
	RptColor *color;

	priv->size = g_new0 (RptSize, 1);
	priv->size->width = 0.0;
	priv->size->height = 0.0;

	color = g_new0 (RptColor, 1);
	color->a = 1.0;

	font = g_new0 (RptFont, 1);
	font->name = g_strdup ("Sans");
	font->size = 12;
	font->color = color;

	priv->rotation = NULL;
	priv->border = NULL;
	priv->font = font;
	priv->align = NULL;
	priv->background_color = NULL;
}

/**
 * rpt_obj_text_new:
 * @name: the #RptObjText's name.
 * @position: an #RptPoint.
 *
 * Creates a new #RptObjText object and sets its position to @position.
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
 * @xnode: an #xmlNode.
 *
 * Returns: the newly created #RptObject object.
 */
RptObject
*rpt_obj_text_new_from_xml (xmlNode *xnode)
{
	gchar *name;
	RptObject *rpt_obj_text = NULL;

	name = (gchar *)xmlGetProp (xnode, "name");
	if (name != NULL && strcmp (g_strstrip (name), "") != 0)
		{
			RptPoint *position;
			RptObjTextPrivate *priv;

			position = rpt_common_get_position (xnode);

			rpt_obj_text = rpt_obj_text_new ((const gchar *)name, *position);

			if (rpt_obj_text != NULL)
				{
					gchar *prop;

					priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_obj_text);

					priv->size = rpt_common_get_size (xnode);
					priv->rotation = rpt_common_get_rotation (xnode);
					priv->border = rpt_common_get_border (xnode);
					priv->font = rpt_common_get_font (xnode);
					priv->align = rpt_common_get_align (xnode);

					g_object_set (rpt_obj_text, "source", xmlGetProp (xnode, "source"), NULL);

					prop = (gchar *)xmlGetProp (xnode, "background-color");
					if (prop != NULL)
						{
							RptColor *color;

							color = rpt_common_parse_color (g_strstrip (prop));
							g_object_set (rpt_obj_text, "background-color", color, NULL);
						}

					prop = (gchar *)xmlGetProp (xnode, "padding-top");
					if (prop != NULL)
						{
							g_object_set (rpt_obj_text, "padding-top", strtod (prop, NULL), NULL);
						}
					prop = (gchar *)xmlGetProp (xnode, "padding-right");
					if (prop != NULL)
						{
							g_object_set (rpt_obj_text, "padding-right", strtod (prop, NULL), NULL);
						}
					prop = (gchar *)xmlGetProp (xnode, "padding-bottom");
					if (prop != NULL)
						{
							g_object_set (rpt_obj_text, "padding-bottom", strtod (prop, NULL), NULL);
						}
					prop = (gchar *)xmlGetProp (xnode, "padding-left");
					if (prop != NULL)
						{
							g_object_set (rpt_obj_text, "padding-left", strtod (prop, NULL), NULL);
						}
				}
		}

	return rpt_obj_text;
}

/**
 * rpt_obj_text_get_xml:
 * @rpt_objtext: an #RptObjText object.
 * @xnode: an #xmlNode.
 *
 */
void
rpt_obj_text_get_xml (RptObject *rpt_objtext, xmlNode *xnode)
{
	RptObjTextPrivate *priv = RPT_OBJ_TEXT_GET_PRIVATE (rpt_objtext);

	xmlNodeSetName (xnode, "text");

	rpt_common_set_size (xnode, priv->size);
	rpt_common_set_rotation (xnode, priv->rotation);
	rpt_common_set_border (xnode, priv->border);
	rpt_common_set_font (xnode, priv->font);
	rpt_common_set_align (xnode, priv->align);

	xmlSetProp (xnode, "source", priv->source);

	if (priv->background_color != NULL)
		{
			xmlSetProp (xnode, "background-color", rpt_common_rptcolor_to_string (priv->background_color));
		}

	if (priv->padding_top != 0.0)
		{
			xmlSetProp (xnode, "padding-top", g_strdup_printf ("%f", priv->padding_top));
		}
	if (priv->padding_right != 0.0)
		{
			xmlSetProp (xnode, "padding-right", g_strdup_printf ("%f", priv->padding_right));
		}
	if (priv->padding_bottom != 0.0)
		{
			xmlSetProp (xnode, "padding-bottom", g_strdup_printf ("%f", priv->padding_bottom));
		}
	if (priv->padding_left != 0.0)
		{
			xmlSetProp (xnode, "padding-left", g_strdup_printf ("%f", priv->padding_left));
		}
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

			case PROP_ROTATION:
				priv->rotation = g_memdup (g_value_get_pointer (value), sizeof (RptRotation));
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

			case PROP_BACKGROUND_COLOR:
				priv->background_color = g_memdup (g_value_get_pointer (value), sizeof (RptColor));
				break;

			case PROP_PADDING_TOP:
				priv->padding_top = g_value_get_double (value);
				break;

			case PROP_PADDING_RIGHT:
				priv->padding_right = g_value_get_double (value);
				break;

			case PROP_PADDING_BOTTOM:
				priv->padding_bottom = g_value_get_double (value);
				break;

			case PROP_PADDING_LEFT:
				priv->padding_left = g_value_get_double (value);
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

			case PROP_ROTATION:
				g_value_set_pointer (value, g_memdup (priv->rotation, sizeof (RptRotation)));
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

			case PROP_BACKGROUND_COLOR:
				g_value_set_pointer (value, g_memdup (priv->background_color, sizeof (RptColor)));
				break;

			case PROP_PADDING_TOP:
				g_value_set_double (value, priv->padding_top);
				break;

			case PROP_PADDING_RIGHT:
				g_value_set_double (value, priv->padding_right);
				break;

			case PROP_PADDING_BOTTOM:
				g_value_set_double (value, priv->padding_bottom);
				break;

			case PROP_PADDING_LEFT:
				g_value_set_double (value, priv->padding_left);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
