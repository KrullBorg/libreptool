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

static gchar *convert_to_str_color (RptColor color);

/**
 * rpt_common_get_position:
 * @xnode:
 * @position:
 *
 */
void
rpt_common_get_position (xmlNode *xnode, RptPoint *position)
{
	gchar *prop;

	position->x = 0.0;
	position->y = 0.0;

	prop = xmlGetProp (xnode, (const xmlChar *)"x");
	if (prop != NULL)
		{
			position->x = strtod (prop, NULL);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"y");
	if (prop != NULL)
		{
			position->y = strtod (prop, NULL);
		}
}

/**
 * rpt_common_get_size:
 * @xnode:
 * @size:
 *
 */
void
rpt_common_get_size (xmlNode *xnode, RptSize *size)
{
	gchar *prop;

	size->width = 0.0;
	size->height = 0.0;

	prop = xmlGetProp (xnode, (const xmlChar *)"width");
	if (prop != NULL)
		{
			size->width = strtod (prop, NULL);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"height");
	if (prop != NULL)
		{
			size->height = strtod (prop, NULL);
		}
}

/**
 * rpt_common_get_font:
 * @xnode:
 * @font:
 *
 */
void
rpt_common_get_font (xmlNode *xnode, RptFont *font)
{
	gchar *prop;

	font->name = g_strdup ("sans");
	font->size = 12.0;
	font->bold = FALSE;
	font->italic = FALSE;
	font->underline = FALSE;
	font->strike = FALSE;
	font->color.r = 0.0;
	font->color.g = 0.0;
	font->color.b = 0.0;
	font->color.a = 1.0;

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
			font->underline = (strcmp (g_strstrip (prop), "y") == 0);
		}

	prop = xmlGetProp (xnode, "font-strike");
	if (prop != NULL)
		{
			font->strike = (strcmp (g_strstrip (prop), "y") == 0);
		}

	prop = xmlGetProp (xnode, "font-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &font->color);
		}
}

/**
 * rpt_common_set_font:
 * @xnode:
 * @font:
 *
 */
void
rpt_common_set_font (xmlNode *xnode, RptFont font)
{
	xmlSetProp (xnode, "font-name", font.name);
	xmlSetProp (xnode, "font-size", g_strdup_printf ("%f", font.size));
	if (font.bold)
		{
			xmlSetProp (xnode, "font-bold", "y");
		}
	if (font.italic)
		{
			xmlSetProp (xnode, "font-italic", "y");
		}
	if (font.underline)
		{
			xmlSetProp (xnode, "font-underline", "y");
		}
	if (font.strike)
		{
			xmlSetProp (xnode, "font-strike", "y");
		}
	xmlSetProp (xnode, "font-color", convert_to_str_color (font.color));
}

/**
 * rpt_common_get_border:
 * @xnode:
 * @border:
 *
 */
void
rpt_common_get_border (xmlNode *xnode, RptBorder *border)
{
	gchar *prop;

	border->top_width = 0.0;
	border->right_width = 0.0;
	border->bottom_width = 0.0;
	border->left_width = 0.0;
	border->top_color.r = 0.0;
	border->top_color.g = 0.0;
	border->top_color.b = 0.0;
	border->top_color.a = 1.0;
	border->right_color.r = 0.0;
	border->right_color.g = 0.0;
	border->right_color.b = 0.0;
	border->right_color.a = 1.0;
	border->bottom_color.r = 0.0;
	border->bottom_color.g = 0.0;
	border->bottom_color.b = 0.0;
	border->bottom_color.a = 1.0;
	border->left_color.r = 0.0;
	border->left_color.g = 0.0;
	border->left_color.b = 0.0;
	border->left_color.a = 1.0;

	prop = xmlGetProp (xnode, "border-top-width");
	if (prop != NULL)
		{
			border->top_width = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "border-right-width");
	if (prop != NULL)
		{
			border->right_width = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "border-bottom-width");
	if (prop != NULL)
		{
			border->bottom_width = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "border-left-width");
	if (prop != NULL)
		{
			border->left_width = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "border-top-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &border->top_color);
		}

	prop = xmlGetProp (xnode, "border-right-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &border->right_color);
		}

	prop = xmlGetProp (xnode, "border-bottom-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &border->bottom_color);
		}

	prop = xmlGetProp (xnode, "border-left-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &border->left_color);
		}
}

/**
 * rpt_common_set_border:
 * @xnode:
 * @border:
 *
 */
void
rpt_common_set_border (xmlNode *xnode, RptBorder border)
{
	if (border.top_width > 0.0)
		{
			xmlSetProp (xnode, "border-top-width", g_strdup_printf ("%f", border.top_width));
			xmlSetProp (xnode, "border-top-color", convert_to_str_color (border.top_color));
		}
	if (border.right_width > 0.0)
		{
			xmlSetProp (xnode, "border-right-width", g_strdup_printf ("%f", border.right_width));
			xmlSetProp (xnode, "border-right-color", convert_to_str_color (border.right_color));
		}
	if (border.bottom_width > 0.0)
		{
			xmlSetProp (xnode, "border-bottom-width", g_strdup_printf ("%f", border.bottom_width));
			xmlSetProp (xnode, "border-bottom-color", convert_to_str_color (border.bottom_color));
		}
	if (border.left_width > 0.0)
		{
			xmlSetProp (xnode, "border-left-width", g_strdup_printf ("%f", border.left_width));
			xmlSetProp (xnode, "border-left-color", convert_to_str_color (border.left_color));
		}
}

/**
 * rpt_common_get_align:
 * @xnode:
 * @align:
 *
 */
void
rpt_common_get_align (xmlNode *xnode, RptAlign *align)
{
	gchar *prop;

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
}

/**
 * rpt_common_set_align:
 * @xnode:
 * @align:
 *
 */
void
rpt_common_set_align (xmlNode *xnode, RptAlign align)
{
	if (align.h_align != RPT_HALIGN_LEFT)
		{
			switch (align.h_align)
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
	if (align.v_align != RPT_VALIGN_TOP)
		{
			switch (align.v_align)
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

/**
 * rpt_common_get_stroke:
 * @xnode:
 * @stroke:
 *
 */
void
rpt_common_get_stroke (xmlNode *xnode, RptStroke *stroke)
{
	gchar *prop;

	stroke->width = 1.0;
	stroke->color.r = 0.0;
	stroke->color.g = 0.0;
	stroke->color.b = 0.0;
	stroke->color.a = 1.0;

	prop = xmlGetProp (xnode, "stroke-width");
	if (prop != NULL)
		{
			stroke->width = strtod (prop, NULL);
		}

	prop = xmlGetProp (xnode, "stroke-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &stroke->color);
		}
}

/**
 * rpt_common_parse_color:
 * @str_color:
 * @color:
 *
 */
void
rpt_common_parse_color (const gchar *str_color, RptColor *color)
{
	gchar *c = g_strstrip (g_strdup (str_color));

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
					if (strlen (c) == 9)
						{
							color->a = strtol (g_strndup (&c[7], 2), NULL, 16) / 255.0;
						}
				}
		}
}

static gchar *
convert_to_str_color (RptColor color)
{
	gchar *ret = "#";

	ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)color.r * 255), NULL);
	ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)color.g * 255), NULL);
	ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)color.b * 255), NULL);
	ret = g_strconcat (ret, g_strdup_printf ("%.2X", (gint)color.a * 255), NULL);

	return ret;
}
