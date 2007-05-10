/*
 * Copyright (C) 2007 Andrea Zagli <azagli@inwind.it>
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
			position = (RptPoint *)g_malloc0 (sizeof (RptPoint));
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
			size = (RptSize *)g_malloc0 (sizeof (RptSize));
			size->width = strtod (width, NULL);
			size->height = strtod (height, NULL);
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
			rotation = (RptRotation *)g_malloc0 (sizeof (RptRotation));
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

	font = (RptFont *)g_malloc0 (sizeof (RptFont));

	font->name = g_strdup ("sans");
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

	border = (RptBorder *)g_malloc0 (sizeof (RptBorder));

	border->top_width = 0.0;
	border->right_width = 0.0;
	border->bottom_width = 0.0;
	border->left_width = 0.0;
	border->top_color = (RptColor *)g_malloc0 (sizeof (RptColor));
	border->top_color->r = 0.0;
	border->top_color->g = 0.0;
	border->top_color->b = 0.0;
	border->top_color->a = 1.0;
	border->right_color = (RptColor *)g_malloc0 (sizeof (RptColor));
	border->right_color->r = 0.0;
	border->right_color->g = 0.0;
	border->right_color->b = 0.0;
	border->right_color->a = 1.0;
	border->bottom_color = (RptColor *)g_malloc0 (sizeof (RptColor));
	border->bottom_color->r = 0.0;
	border->bottom_color->g = 0.0;
	border->bottom_color->b = 0.0;
	border->bottom_color->a = 1.0;
	border->left_color = (RptColor *)g_malloc0 (sizeof (RptColor));
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

	align = (RptAlign *)g_malloc0 (sizeof (RptAlign));

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

	stroke = (RptStroke *)g_malloc0 (sizeof (RptStroke));
	stroke->width = 1.0;

	stroke->color = (RptColor *)g_malloc0 (sizeof (RptColor));
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

	color = (RptColor *)g_malloc0 (sizeof (RptColor));
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
