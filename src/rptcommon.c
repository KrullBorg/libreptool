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

#include <stdlib.h>
#include <string.h>

#include "rptcommon.h"

static GArray *rpt_common_parse_style (const gchar *style);
static gchar *rpt_common_style_to_string (const GArray *style);


/**
 * rpt_common_value_to_points:
 * @unit: unit of length.
 * @value: the value to convert.
 *
 * Returns: the @value converted in points.
 */
gdouble
rpt_common_value_to_points (eRptUnitLength unit, gdouble value)
{
	gdouble ret;

	switch (unit)
		{
			case RPT_UNIT_POINTS:
				ret = value;
				break;

			case RPT_UNIT_INCHES:
				ret = value * 72;
				break;

			case RPT_UNIT_CENTIMETRE:
				ret = value / 2.54 * 72;
				break;

			case RPT_UNIT_MILLIMETRE:
				ret = value / 25.4 * 72;
				break;

			default:
				g_warning ("Unit length «%d» not available.", unit);
				ret = value;
				break;
		}

	return ret;
}

/**
 * rpt_common_points_to_value:
 * @unit: unit of length.
 * @value: the value to convert to.
 *
 * Returns: the points from the @value.
 */
gdouble
rpt_common_points_to_value (eRptUnitLength unit, gdouble value)
{
	gdouble ret;

	switch (unit)
		{
			case RPT_UNIT_POINTS:
				ret = value;
				break;

			case RPT_UNIT_INCHES:
				ret = value / 72;
				break;

			case RPT_UNIT_CENTIMETRE:
				ret = value / 72 * 2.54;
				break;

			case RPT_UNIT_MILLIMETRE:
				ret = value / 72 * 25.4;
				break;

			default:
				g_warning ("Unit length «%d» not available.", unit);
				ret = value;
				break;
		}

	return ret;
}

/**
 * rpt_common_strunit_to_enum:
 * @unit:
 *
 * Returns: the enum value that match the string @unit.
 */
eRptUnitLength
rpt_common_strunit_to_enum (const gchar *unit)
{
	eRptUnitLength ret;

	gchar *real_unit;

	ret = RPT_UNIT_POINTS;

	if (unit != NULL)
		{
			real_unit = g_strstrip (g_strdup (unit));
			if (g_ascii_strcasecmp (real_unit, "pt") == 0)
				{
					/* already setted */
				}
			else if (g_ascii_strcasecmp (real_unit, "in") == 0)
				{
					ret = RPT_UNIT_INCHES;
				}
			else if (g_ascii_strcasecmp (real_unit, "cm") == 0)
				{
					ret = RPT_UNIT_CENTIMETRE;
				}
			else if (g_ascii_strcasecmp (real_unit, "mm") == 0)
				{
					ret = RPT_UNIT_MILLIMETRE;
				}
			else
				{
					g_warning ("Unit length «%s» not available.", real_unit);
				}
		}

	return ret;
}

/**
 * rpt_common_enum_to_strunit:
 * @unit:
 *
 * Returns: the string value that represents then enum value @unit.
 */
const gchar
*rpt_common_enum_to_strunit (eRptUnitLength unit)
{
	gchar *ret;

	switch (unit)
		{
			case RPT_UNIT_POINTS:
				ret = g_strdup ("pt");
				break;

			case RPT_UNIT_INCHES:
				ret = g_strdup ("in");
				break;

			case RPT_UNIT_CENTIMETRE:
				ret = g_strdup ("cm");
				break;

			case RPT_UNIT_MILLIMETRE:
				ret = g_strdup ("mm");
				break;

			default:
				g_warning ("Unit length «%d» not available.", unit);
				ret = g_strdup ("pt");
				break;
		}

	return ret;
}

/**
 * rpt_common_strellipsize_to_enum:
 * @ellipsize:
 *
 * Returns: the enum value that match the string @ellipsize.
 */
eRptEllipsize
rpt_common_strellipsize_to_enum (const gchar *ellipsize)
{
	eRptEllipsize ret;

	gchar *real_ellipsize;

	ret = RPT_ELLIPSIZE_NONE;

	if (ellipsize != NULL)
		{
			real_ellipsize = g_strstrip (g_strdup (ellipsize));
			if (g_ascii_strcasecmp (real_ellipsize, "none") == 0)
				{
					/* already setted */
				}
			else if (g_ascii_strcasecmp (real_ellipsize, "start") == 0)
				{
					ret = RPT_ELLIPSIZE_START;
				}
			else if (g_ascii_strcasecmp (real_ellipsize, "middle") == 0)
				{
					ret = RPT_ELLIPSIZE_MIDDLE;
				}
			else if (g_ascii_strcasecmp (real_ellipsize, "end") == 0)
				{
					ret = RPT_ELLIPSIZE_END;
				}
			else
				{
					g_warning ("Ellipsize type «%s» not available.", real_ellipsize);
				}
		}

	return ret;
}

/**
 * rpt_common_enum_to_strellipsize:
 * @unit:
 *
 * Returns: the string value that represents then enum value @unit.
 */
const gchar
*rpt_common_enum_to_strellipsize (eRptEllipsize ellipsize)
{
	gchar *ret;

	switch (ellipsize)
		{
			case RPT_ELLIPSIZE_NONE:
				ret = g_strdup ("none");
				break;

			case RPT_ELLIPSIZE_START:
				ret = g_strdup ("start");
				break;

			case RPT_ELLIPSIZE_MIDDLE:
				ret = g_strdup ("middle");
				break;

			case RPT_ELLIPSIZE_END:
				ret = g_strdup ("end");
				break;

			default:
				g_warning ("Ellipsize type «%d» not available.", ellipsize);
				ret = g_strdup ("none");
				break;
		}

	return ret;
}

/**
 * rpt_common_stroutputtype_to_enum:
 * @output_type:
 *
 * Returns: the enum value that match the string @output_type.
 */
eRptOutputType
rpt_common_stroutputtype_to_enum (const gchar *output_type)
{
	eRptOutputType ret;

	gchar *real_outputtype;

	ret = RPT_OUTPUT_PDF;

	if (output_type != NULL)
		{
			real_outputtype = g_strstrip (g_strdup (output_type));
			if (g_ascii_strcasecmp (real_outputtype, "pdf") == 0)
				{
					/* already setted */
				}
			else if (g_ascii_strcasecmp (real_outputtype, "png") == 0)
				{
					ret = RPT_OUTPUT_PNG;
				}
			else if (g_ascii_strcasecmp (real_outputtype, "ps") == 0)
				{
					ret = RPT_OUTPUT_PS;
				}
			else if (g_ascii_strcasecmp (real_outputtype, "svg") == 0)
				{
					ret = RPT_OUTPUT_SVG;
				}
			else if (g_ascii_strcasecmp (real_outputtype, "gtk") == 0)
				{
					ret = RPT_OUTPUT_GTK;
				}
			else if (g_ascii_strcasecmp (real_outputtype, "gtk-default") == 0)
				{
					ret = RPT_OUTPUT_GTK_DEFAULT_PRINTER;
				}
			else
				{
					g_warning ("Output type «%s» not available.", real_outputtype);
				}
		}

	return ret;
}

/**
 * rpt_common_enum_to_stroutputtype:
 * @output_type:
 *
 * Returns: the string value that represents then enum value @output_type.
 */
const gchar
*rpt_common_enum_to_stroutputtype (eRptOutputType output_type)
{
	gchar *ret;

	switch (output_type)
		{
			case RPT_OUTPUT_PDF:
				ret = g_strdup ("pdf");
				break;

			case RPT_OUTPUT_PNG:
				ret = g_strdup ("png");
				break;

			case RPT_OUTPUT_PS:
				ret = g_strdup ("ps");
				break;

			case RPT_OUTPUT_SVG:
				ret = g_strdup ("svg");
				break;

			case RPT_OUTPUT_GTK:
				ret = g_strdup ("gtk");
				break;

			case RPT_OUTPUT_GTK_DEFAULT_PRINTER:
				ret = g_strdup ("gtk-default");
				break;

			default:
				g_warning ("Output type «%d» not available.", output_type);
				ret = g_strdup ("pdf");
				break;
		}

	return ret;
}

/**
 * rpt_common_rptpoint_new:
 *
 * Returns: an new allocated #RptPoint struct.
 */
RptPoint
*rpt_common_rptpoint_new (void)
{
	RptPoint *point;

	point = (RptPoint *)g_malloc0 (sizeof (RptPoint));
	point->x = 0.0;
	point->y = 0.0;

	return point;
}

/**
 * rpt_common_rptpoint_new_with_values:
 * @x:
 * @y:
 *
 * Returns: an new allocated #RptPoint struct.
 */
RptPoint
*rpt_common_rptpoint_new_with_values (gdouble x, gdouble y)
{
	RptPoint *point;

	point = rpt_common_rptpoint_new ();
	point->x = x;
	point->y = y;

	return point;
}

/**
 * rpt_common_get_position:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptPoint struct that represent the object's position specified
 * on @xnode.
 */
RptPoint
*rpt_common_get_position (xmlNode *xnode)
{
	RptPoint *position = NULL;
	gchar *x;
	gchar *y;

	x = xmlGetProp (xnode, (const xmlChar *)"x");
	y = xmlGetProp (xnode, (const xmlChar *)"y");

	if (x != NULL || y != NULL)
		{
			position = rpt_common_rptpoint_new ();
			position->x = (x == NULL ? 0.0 : strtod (x, NULL));
			position->y = (y == NULL ? 0.0 : strtod (y, NULL));
		}

	return position;
}

/**
 * rpt_common_set_position:
 * @xnode: an #xmlNode.
 * @position:
 *
 */
void
rpt_common_set_position (xmlNode *xnode, const RptPoint *position)
{
	if (position != NULL)
		{
			xmlSetProp (xnode, "x", g_strdup_printf ("%f", position->x));
			xmlSetProp (xnode, "y", g_strdup_printf ("%f", position->y));
		}
}

/**
 * rpt_common_rptsize_new:
 *
 * Returns: an new allocated #RptSize struct.
 */
RptSize
*rpt_common_rptsize_new (void)
{
	RptSize *size;

	size = (RptSize *)g_malloc0 (sizeof (RptSize));
	size->width = 0.0;
	size->height = 0.0;

	return size;
}

/**
 * rpt_common_rptsize_new_with_values:
 * @width:
 * @height:
 *
 * Returns: an new allocated #RptSize struct.
 */
RptSize
*rpt_common_rptsize_new_with_values (gdouble width, gdouble height)
{
	RptSize *size;

	size = rpt_common_rptsize_new ();
	size->width = width;
	size->height = height;

	return size;
}

/**
 * rpt_common_get_size:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptSize struct that represent the object's size specified
 * on @xnode.
 */
RptSize
*rpt_common_get_size (xmlNode *xnode)
{
	RptSize *size = NULL;
	gchar *width;
	gchar *height;

	width = xmlGetProp (xnode, (const xmlChar *)"width");
	height = xmlGetProp (xnode, (const xmlChar *)"height");
	if (width != NULL && height != NULL)
		{
			size = rpt_common_rptsize_new_with_values (g_strtod (width, NULL),
			                                           g_strtod (height, NULL));
		}

	return size;
}

/**
 * rpt_common_set_size:
 * @xnode: an #xmlNode.
 * @size:
 *
 */
void
rpt_common_set_size (xmlNode *xnode, const RptSize *size)
{
	if (size != NULL)
		{
			xmlSetProp (xnode, "width", g_strdup_printf ("%f", size->width));
			xmlSetProp (xnode, "height", g_strdup_printf ("%f", size->height));
		}
}

/**
 * rpt_common_rptrotation_new:
 *
 * Returns: an new allocated #RptRotation struct.
 */
RptRotation
*rpt_common_rptrotation_new (void)
{
	RptRotation *rotation;

	rotation = (RptRotation *)g_malloc0 (sizeof (RptRotation));
	rotation->angle = 0.0;

	return rotation;
}

/**
 * rpt_common_rptrotation_new_with_values:
 * @angle:
 *
 * Returns: an new allocated #RptRotation struct.
 */
RptRotation
*rpt_common_rptrotation_new_with_values (gdouble angle)
{
	RptRotation *rotation;

	rotation = rpt_common_rptrotation_new ();
	rotation->angle = angle;

	return rotation;
}

/**
 * rpt_common_get_rotation:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptRotation struct that represent the object's rotation
 * specified on @xnode.
 */
RptRotation
*rpt_common_get_rotation (xmlNode *xnode)
{
	gchar *prop;
	RptRotation *rotation = NULL;

	prop = xmlGetProp (xnode, "rotation");
	if (prop != NULL)
		{
			rotation = rpt_common_rptrotation_new ();
			rotation->angle = strtod (prop, NULL);
		}

	return rotation;
}

/**
 * rpt_common_set_rotation:
 * @xnode: an #xmlNode.
 * @rotation:
 *
 */
void
rpt_common_set_rotation (xmlNode *xnode, const RptRotation *rotation)
{
	if (rotation != NULL)
		{
			xmlSetProp (xnode, "rotation", g_strdup_printf ("%f", rotation->angle));
		}
}

/**
 * rpt_common_rptmargin_new:
 *
 * Returns: an new allocated #RptMargin struct.
 */
RptMargin
*rpt_common_rptmargin_new (void)
{
	RptMargin *margin;

	margin = (RptMargin *)g_malloc0 (sizeof (RptMargin));
	margin->top = 0.0;
	margin->right = 0.0;
	margin->bottom = 0.0;
	margin->left = 0.0;

	return margin;
}

/**
 * rpt_common_rptmargin_new_with_values:
 * @top:
 * @right:
 * @bottom:
 * @left:
 *
 * Returns: an new allocated #RptMargin struct.
 */
RptMargin
*rpt_common_rptmargin_new_with_values (gdouble top,
                                       gdouble right,
                                       gdouble bottom,
                                       gdouble left)
{
	RptMargin *margin;

	margin = rpt_common_rptmargin_new ();
	margin->top = top;
	margin->right = right;
	margin->bottom = bottom;
	margin->left = left;

	return margin;
}

/**
 * rpt_common_get_margin:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptMargin struct that represent the page's margin specified
 * on @xnode.
 */
RptMargin
*rpt_common_get_margin (xmlNode *xnode)
{
	RptMargin *margin = NULL;
	gchar *prop;

	margin = rpt_common_rptmargin_new ();

	prop = xmlGetProp (xnode, (const xmlChar *)"top");
	if (prop != NULL)
		{
			margin->top = g_strtod (prop, NULL);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"right");
	if (prop != NULL)
		{
			margin->right = g_strtod (prop, NULL);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"bottom");
	if (prop != NULL)
		{
			margin->bottom = g_strtod (prop, NULL);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"left");
	if (prop != NULL)
		{
			margin->left = g_strtod (prop, NULL);
		}

	return margin;
}

/**
 * rpt_common_set_margin:
 * @xnode: an #xmlNode.
 * @margin:
 *
 */
void
rpt_common_set_margin (xmlNode *xnode, const RptMargin *margin)
{
	if (margin != NULL)
		{
			xmlSetProp (xnode, "top", g_strdup_printf ("%f", margin->top));
			xmlSetProp (xnode, "right", g_strdup_printf ("%f", margin->right));
			xmlSetProp (xnode, "bottom", g_strdup_printf ("%f", margin->bottom));
			xmlSetProp (xnode, "left", g_strdup_printf ("%f", margin->left));
		}
}

/**
 * rpt_common_rptfont_new:
 *
 * Returns: an new allocated #RptFont struct.
 */
RptFont
*rpt_common_rptfont_new (void)
{
	RptFont *font;

	font = (RptFont *)g_malloc0 (sizeof (RptFont));

	return font;
}

/**
 * rpt_common_get_font:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptFont struct that represent the object's font
 * specified on @xnode.
 */
RptFont
*rpt_common_get_font (xmlNode *xnode)
{
	RptFont *font = NULL;
	gchar *prop;

	font = rpt_common_rptfont_new ();

	font->name = g_strdup ("Sans");
	font->size = 12.0;
	font->bold = FALSE;
	font->italic = FALSE;
	font->underline = PANGO_UNDERLINE_NONE;
	font->strike = FALSE;

	prop = xmlGetProp (xnode, "font-name");
	if (prop != NULL)
		{
			font->name = g_strdup (prop);
		}

	prop = xmlGetProp (xnode, "font-size");
	if (prop != NULL)
		{
			font->size = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "font-bold");
	if (prop != NULL)
		{
			font->bold = (strcmp (g_strstrip (prop), "y") == 0);
		}

	prop = xmlGetProp (xnode, "font-italic");
	if (prop != NULL)
		{
			font->italic = (strcmp (g_strstrip (prop), "y") == 0);
		}

	prop = xmlGetProp (xnode, "font-underline");
	if (prop != NULL)
		{
			g_strstrip (prop);

			if (strcmp (prop, "single") == 0)
				{
					font->underline = PANGO_UNDERLINE_SINGLE;
				}
			else if (strcmp (prop, "double") == 0)
				{
					font->underline = PANGO_UNDERLINE_DOUBLE;
				}
			else if (strcmp (prop, "low") == 0)
				{
					font->underline = PANGO_UNDERLINE_LOW;
				}
			else if (strcmp (prop, "error") == 0)
				{
					font->underline = PANGO_UNDERLINE_ERROR;
				}
		}

	prop = xmlGetProp (xnode, "font-strike");
	if (prop != NULL)
		{
			font->strike = (strcmp (g_strstrip (prop), "y") == 0);
		}

	prop = xmlGetProp (xnode, "font-color");
	if (prop != NULL)
		{
			font->color = rpt_common_parse_color (prop);
		}

	return font;
}

/**
 * rpt_common_set_font:
 * @xnode: an #xmlNode.
 * @font:
 *
 */
void
rpt_common_set_font (xmlNode *xnode, const RptFont *font)
{
	if (font != NULL)
		{
			xmlSetProp (xnode, "font-name", font->name);
			xmlSetProp (xnode, "font-size", g_strdup_printf ("%f", font->size));
			if (font->bold)
				{
					xmlSetProp (xnode, "font-bold", "y");
				}
			if (font->italic)
				{
					xmlSetProp (xnode, "font-italic", "y");
				}
			if (font->underline != PANGO_UNDERLINE_NONE)
				{
					switch (font->underline)
						{
							case PANGO_UNDERLINE_SINGLE:
								xmlSetProp (xnode, "font-underline", "single");
								break;
		
							case PANGO_UNDERLINE_DOUBLE:
								xmlSetProp (xnode, "font-underline", "double");
								break;
		
							case PANGO_UNDERLINE_LOW:
								xmlSetProp (xnode, "font-underline", "low");
								break;
		
							case PANGO_UNDERLINE_ERROR:
								xmlSetProp (xnode, "font-underline", "error");
								break;
						}
				}
			if (font->strike)
				{
					xmlSetProp (xnode, "font-strike", "y");
				}
			if (font->color != NULL)
				{
					xmlSetProp (xnode, "font-color", rpt_common_rptcolor_to_string (font->color));
				}
		}
}

/**
 * rpt_common_rptfont_from_pango_description:
 * @description: a #PangoFontDescription.
 *
 * Returns: a new allocated #RptFont from a #PangoFontDescription.
 */
RptFont
*rpt_common_rptfont_from_pango_description (const PangoFontDescription *description)
{
	RptFont *font;

	font = rpt_common_rptfont_new ();

	font->name = (gchar *)pango_font_description_get_family (description);
	font->size = pango_font_description_get_size (description) / PANGO_SCALE;
	font->bold = (pango_font_description_get_weight (description) == PANGO_WEIGHT_BOLD);
	font->italic = (pango_font_description_get_style (description) == PANGO_STYLE_ITALIC);
	font->underline = PANGO_UNDERLINE_NONE;
	font->strike = FALSE;

	font->color = rpt_common_rptcolor_new ();
	font->color->a = 1.0;

	return font;
}

/**
 * rpt_common_rptborder_new:
 *
 * Returns: a new allocated #RptBorder struct.
 */
RptBorder
*rpt_common_rptborder_new (void)
{
	RptBorder *border;

	border = (RptBorder *)g_malloc0 (sizeof (RptBorder));

	return border;
}

/**
 * rpt_common_get_border:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptBorder struct that represent the object's border
 * specified on @xnode.
 */
RptBorder
*rpt_common_get_border (xmlNode *xnode)
{
	RptBorder *border = NULL;
	gchar *prop;

	border = rpt_common_rptborder_new ();

	border->top_width = 0.0;
	border->right_width = 0.0;
	border->bottom_width = 0.0;
	border->left_width = 0.0;
	border->top_color = rpt_common_rptcolor_new ();
	border->top_color->r = 0.0;
	border->top_color->g = 0.0;
	border->top_color->b = 0.0;
	border->top_color->a = 1.0;
	border->right_color = rpt_common_rptcolor_new ();
	border->right_color->r = 0.0;
	border->right_color->g = 0.0;
	border->right_color->b = 0.0;
	border->right_color->a = 1.0;
	border->bottom_color = rpt_common_rptcolor_new ();
	border->bottom_color->r = 0.0;
	border->bottom_color->g = 0.0;
	border->bottom_color->b = 0.0;
	border->bottom_color->a = 1.0;
	border->left_color = rpt_common_rptcolor_new ();
	border->left_color->r = 0.0;
	border->left_color->g = 0.0;
	border->left_color->b = 0.0;
	border->left_color->a = 1.0;
	border->top_style = NULL;
	border->right_style = NULL;
	border->bottom_style = NULL;
	border->left_style = NULL;

	prop = (gchar *)xmlGetProp (xnode, "border-top-width");
	if (prop != NULL)
		{
			border->top_width = strtod (prop, NULL);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-right-width");
	if (prop != NULL)
		{
			border->right_width = strtod (prop, NULL);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-bottom-width");
	if (prop != NULL)
		{
			border->bottom_width = strtod (prop, NULL);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-left-width");
	if (prop != NULL)
		{
			border->left_width = strtod (prop, NULL);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-top-color");
	if (prop != NULL)
		{
			border->top_color = rpt_common_parse_color (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-right-color");
	if (prop != NULL)
		{
			border->right_color = rpt_common_parse_color (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-bottom-color");
	if (prop != NULL)
		{
			border->bottom_color = rpt_common_parse_color (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-left-color");
	if (prop != NULL)
		{
			border->left_color = rpt_common_parse_color (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-top-style");
	if (prop != NULL)
		{
			border->top_style = rpt_common_parse_style (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-right-style");
	if (prop != NULL)
		{
			border->right_style = rpt_common_parse_style (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-bottom-style");
	if (prop != NULL)
		{
			border->bottom_style = rpt_common_parse_style (prop);
		}

	prop = (gchar *)xmlGetProp (xnode, "border-left-style");
	if (prop != NULL)
		{
			border->left_style = rpt_common_parse_style (prop);
		}

	return border;
}

/**
 * rpt_common_set_border:
 * @xnode: an #xmlNode.
 * @border:
 *
 */
void
rpt_common_set_border (xmlNode *xnode, const RptBorder *border)
{
	if (border != NULL)
		{
			if (border->top_width > 0.0 && border->top_color != NULL)
				{
					xmlSetProp (xnode, "border-top-width", g_strdup_printf ("%f", border->top_width));
					xmlSetProp (xnode, "border-top-color", rpt_common_rptcolor_to_string (border->top_color));
					if (border->top_style != NULL)
						{
							xmlSetProp (xnode, "border-top-style", rpt_common_style_to_string (border->top_style));
						}
				}
			if (border->right_width > 0.0 && border->right_color != NULL)
				{
					xmlSetProp (xnode, "border-right-width", g_strdup_printf ("%f", border->right_width));
					xmlSetProp (xnode, "border-right-color", rpt_common_rptcolor_to_string (border->right_color));
					if (border->right_style != NULL)
						{
							xmlSetProp (xnode, "border-right-style", rpt_common_style_to_string (border->right_style));
						}
				}
			if (border->bottom_width > 0.0 && border->bottom_color != NULL)
				{
					xmlSetProp (xnode, "border-bottom-width", g_strdup_printf ("%f", border->bottom_width));
					xmlSetProp (xnode, "border-bottom-color", rpt_common_rptcolor_to_string (border->bottom_color));
					if (border->bottom_style != NULL)
						{
							xmlSetProp (xnode, "border-bottom-style", rpt_common_style_to_string (border->bottom_style));
						}
				}
			if (border->left_width > 0.0 && border->left_color != NULL)
				{
					xmlSetProp (xnode, "border-left-width", g_strdup_printf ("%f", border->left_width));
					xmlSetProp (xnode, "border-left-color", rpt_common_rptcolor_to_string (border->left_color));
					if (border->left_style != NULL)
						{
							xmlSetProp (xnode, "border-left-style", rpt_common_style_to_string (border->left_style));
						}
				}
		}
}

/**
 * rpt_common_rptalign_new:
 *
 * Returns: an new allocated #RptAlign struct.
 */
RptAlign
*rpt_common_rptalign_new (void)
{
	RptAlign *align;

	align = (RptAlign *)g_malloc0 (sizeof (RptAlign));

	return align;
}

/**
 * rpt_common_get_align:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptAlign struct that represent the object's alignment
 * specified on @xnode.
 */
RptAlign
*rpt_common_get_align (xmlNode *xnode)
{
	RptAlign *align = NULL;
	gchar *prop;

	align = rpt_common_rptalign_new ();

	align->h_align = RPT_HALIGN_LEFT;
	align->v_align = RPT_VALIGN_TOP;

	prop = xmlGetProp (xnode, "horizontal-align");
	if (prop != NULL)
		{
			if (strcmp (prop, "center") == 0)
				{
					align->h_align = RPT_HALIGN_CENTER;
				}
			else if (strcmp (prop, "right") == 0)
				{
					align->h_align = RPT_HALIGN_RIGHT;
				}
			else if (strcmp (prop, "justified") == 0)
				{
					align->h_align = RPT_HALIGN_JUSTIFIED;
				}
		}

	prop = xmlGetProp (xnode, "vertical-align");
	if (prop != NULL)
		{
			if (strcmp (prop, "center") == 0)
				{
					align->v_align = RPT_VALIGN_CENTER;
				}
			else if (strcmp (prop, "bottom") == 0)
				{
					align->v_align = RPT_VALIGN_BOTTOM;
				}
		}

	return align;
}

/**
 * rpt_common_set_align:
 * @xnode: an #xmlNode.
 * @align:
 *
 */
void
rpt_common_set_align (xmlNode *xnode, const RptAlign *align)
{
	if (align != NULL)
		{
			if (align->h_align != RPT_HALIGN_LEFT)
				{
					switch (align->h_align)
						{
							case RPT_HALIGN_CENTER:
								xmlSetProp (xnode, "horizontal-align", "center");
								break;

							case RPT_HALIGN_RIGHT:
								xmlSetProp (xnode, "horizontal-align", "right");
								break;

							case RPT_HALIGN_JUSTIFIED:
								xmlSetProp (xnode, "horizontal-align", "justified");
								break;
						}
				}
			if (align->v_align != RPT_VALIGN_TOP)
				{
					switch (align->v_align)
						{
							case RPT_VALIGN_CENTER:
								xmlSetProp (xnode, "vertical-align", "center");
								break;

							case RPT_VALIGN_BOTTOM:
								xmlSetProp (xnode, "vertical-align", "bottom");
								break;
						}
				}
		}
}

/**
 * rpt_common_rptstroke_new:
 *
 * Returns: an new allocated #RptStroke struct.
 */
RptStroke
*rpt_common_rptstroke_new (void)
{
	RptStroke *stroke;

	stroke = (RptStroke *)g_malloc0 (sizeof (RptStroke));

	return stroke;
}

/**
 * rpt_common_get_stroke:
 * @xnode: an #xmlNode.
 *
 * Returns: an #RptStroke struct that represent the object's stroke
 * specified on @xnode.
 */
RptStroke
*rpt_common_get_stroke (xmlNode *xnode)
{
	RptStroke *stroke = NULL;
	gchar *prop;

	stroke = rpt_common_rptstroke_new ();
	stroke->width = 1.0;

	stroke->color = rpt_common_rptcolor_new ();
	stroke->color->r = 0.0;
	stroke->color->g = 0.0;
	stroke->color->b = 0.0;
	stroke->color->a = 1.0;

	stroke->style = NULL;

	prop = xmlGetProp (xnode, "stroke-width");
	if (prop != NULL)
		{
			stroke->width = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "stroke-color");
	if (prop != NULL)
		{
			stroke->color = rpt_common_parse_color (prop);
		}

	prop = xmlGetProp (xnode, "stroke-style");
	if (prop != NULL)
		{
			stroke->style = rpt_common_parse_style (prop);
		}

	return stroke;
}

/**
 * rpt_common_set_stroke:
 * @xnode: an #xmlNode.
 * @stroke:
 *
 */
void
rpt_common_set_stroke (xmlNode *xnode, const RptStroke *stroke)
{
	if (stroke != NULL)
		{
			if (stroke->width != 0.0)
				{
					xmlSetProp (xnode, "stroke-width", g_strdup_printf ("%f", stroke->width));
				}
			xmlSetProp (xnode, "stroke-color", rpt_common_rptcolor_to_string (stroke->color));
			if (stroke->style != NULL)
				{
					xmlSetProp (xnode, "stroke-style", rpt_common_style_to_string (stroke->style));
				}
		}
}

/**
 * rpt_common_rptcolor_new:
 *
 * Returns: an new allocated #RptColor struct.
 */
RptColor
*rpt_common_rptcolor_new (void)
{
	RptColor *color;

	color = (RptColor *)g_malloc0 (sizeof (RptColor));

	return color;
}

/**
 * rpt_common_parse_color:
 * @str_color: a color string.
 *
 * Returns: an #RptColor.
 */
RptColor
*rpt_common_parse_color (const gchar *str_color)
{
	RptColor *color = NULL;
	gchar *c = g_strstrip (g_strdup (str_color));

	color = rpt_common_rptcolor_new ();
	color->a = 1.0;

	if (c[0] == '#')
		{
			if (strlen (c) == 4 || strlen (c) == 5)
				{
					if (isxdigit (c[1]))
						{
							color->r = strtol (g_strdup_printf ("%c%c", c[1], c[1]), NULL, 16) / 255.0;
						}
					if (isxdigit (c[2]))
						{
							color->g = strtol (g_strdup_printf ("%c%c", c[2], c[2]), NULL, 16) / 255.0;
						}
					if (isxdigit (c[3]))
						{
							color->b = strtol (g_strdup_printf ("%c%c", c[3], c[3]), NULL, 16) / 255.0;
						}
					if (strlen (c) == 5 && isxdigit (c[4]))
						{
							color->a = strtol (g_strdup_printf ("%c%c", c[4], c[4]), NULL, 16) / 255.0;
						}
				}
			else if (strlen (c) == 7 || strlen (c) == 9)
				{
					if (isxdigit (c[1]) && isxdigit (c[2]))
						{
							color->r = strtol (g_strndup (&c[1], 2), NULL, 16) / 255.0;
						}
					if (isxdigit (c[3]) && isxdigit (c[4]))
						{
							color->g = strtol (g_strndup (&c[3], 2), NULL, 16) / 255.0;
						}
					if (isxdigit (c[5]) && isxdigit (c[6]))
						{
							color->b = strtol (g_strndup (&c[5], 2), NULL, 16) / 255.0;
						}
					if (strlen (c) == 9 && isxdigit (c[7]) && isxdigit (c[8]))
						{
							color->a = strtol (g_strndup (&c[7], 2), NULL, 16) / 255.0;
						}
				}
		}

	return color;
}

/**
 * rpt_common_rptcolor_to_string:
 * @color: an #RptColor value.
 *
 * Converts an #RptColor value to a string.
 *
 * Returns: the color string correspondent to @color.
 */
gchar
*rpt_common_rptcolor_to_string (const RptColor *color)
{
	gchar *ret = NULL;

	if (color != NULL)
		{
			ret = g_strdup  ("#");

			ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)(color->r * 255)), NULL);
			ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)(color->g * 255)), NULL);
			ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)(color->b * 255)), NULL);
			ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)(color->a * 255)), NULL);
		}

	return ret;
}

/**
 * rpt_common_rptcolor_to_gdkcolor:
 * @color: an #RptColor value.
 *
 * Converts an #RptColor value to a #GdkColor.
 *
 * Returns: the #GdkColor correspondent to @color.
 */
GdkColor
*rpt_common_rptcolor_to_gdkcolor (const RptColor *color)
{
	GdkColor *gdk_color;

	gdk_color = (GdkColor *)g_malloc0 (sizeof (GdkColor));
	
	gdk_color->red = color->r * 65535;
	gdk_color->green = color->g * 65535;
	gdk_color->blue = color->b * 65535;

	return gdk_color;
}

/**
 * rpt_common_gdkcolor_to_rptcolor:
 * @gdk_color: a #GdkColor.
 * @alpha: the alpha value.
 *
 * Converts an #GdkColor value to a #RptColor.
 *
 * Returns: the #GdkColor correspondent to @color.
 */
RptColor
*rpt_common_gdkcolor_to_rptcolor (const GdkColor *gdk_color, guint16 alpha)
{
	RptColor *color;

	color = rpt_common_rptcolor_new ();

	color->r = gdk_color->red / 65535.0;
	color->g = gdk_color->green / 65535.0;
	color->b = gdk_color->blue / 65535.0;
	color->a = alpha / 65535.0;

	return color;
}

static GArray
*rpt_common_parse_style (const gchar *style)
{
	gint i = 0;
	gdouble val;
	GArray *ret = NULL;

	gchar **values = g_strsplit (style, ";", 0);

	if (values != NULL)
		{
			ret = g_array_new (FALSE, FALSE, sizeof (gdouble));
			while (values[i] != NULL)
				{
					if (strtod (values[i], NULL) > 0.0)
						{
							val = strtod (values[i], NULL);
							g_array_append_val (ret, val);
						}
		
					i++;
				}
			g_strfreev (values);
		}

	return ret;
}

static gchar
*rpt_common_style_to_string (const GArray *style)
{
	gint i;
	gchar *ret = NULL;

	if (style != NULL)
		{
			ret = g_strdup ("");
			for (i = 0; i < style->len; i++)
				{
					ret = g_strconcat (ret, g_strdup_printf ("%f;", g_array_index (style, gdouble, i)), NULL);
				}
		}

	return ret;
}

/**
 * rpt_common_style_to_array:
 * @style:
 *
 */
gdouble
*rpt_common_style_to_array (const GArray *style)
{
	gint i;
	gdouble *ret = NULL;

	ret = (gdouble *)g_malloc (style->len * sizeof (gdouble));

	for (i = 0; i < style->len; i++)
		{
			ret[i] = g_array_index (style, gdouble, i);
		}

	return ret;
}
