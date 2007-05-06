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
 * 
 */

#ifndef __RPT_COMMON_H__
#define __RPT_COMMON_H__

#include <glib.h>
#include <libxml/tree.h>

#include <pango/pango-attributes.h>

G_BEGIN_DECLS


/**
 * RptColor:
 * @r: the red channel.
 * @g: the green channel.
 * @b: the blue channel.
 * @a: the alpha channel.
 */
struct _RptColor
{
	gdouble r;
	gdouble g;
	gdouble b;
	gdouble a;
};
typedef struct _RptColor RptColor;

struct _RptPoint
{
	gdouble x;
	gdouble y;
};
typedef struct _RptPoint RptPoint;

struct _RptSize
{
	gdouble width;
	gdouble height;
};
typedef struct _RptSize RptSize;

struct _RptRotation
{
	gdouble angle;
};
typedef struct _RptRotation RptRotation;

/**
 * RptFont:
 * @name: the font's family name.
 * @size: the font's size in pixel.
 * @bold: if the font is bold.
 * @italic: if the font is italic.
 * @underline: if the font is underline.
 * @strike: if the font is striked.
 * @color: an #RptColor.
 */
struct _RptFont
{
	gchar *name;
	gdouble size;
	gboolean bold;
	gboolean italic;
	PangoUnderline underline;
	gboolean strike;
	RptColor *color;
};
typedef struct _RptFont RptFont;

struct _RptBorder
{
	gdouble top_width;
	gdouble right_width;
	gdouble bottom_width;
	gdouble left_width;
	RptColor *top_color;
	RptColor *right_color;
	RptColor *bottom_color;
	RptColor *left_color;
	GArray *top_style;
	GArray *right_style;
	GArray *bottom_style;
	GArray *left_style;
};
typedef struct _RptBorder RptBorder;

typedef enum
{
	RPT_HALIGN_LEFT,
	RPT_HALIGN_CENTER,
	RPT_HALIGN_RIGHT,
	RPT_HALIGN_JUSTIFIED
} eRptHAlign;

typedef enum
{
	RPT_VALIGN_TOP,
	RPT_VALIGN_CENTER,
	RPT_VALIGN_BOTTOM
} eRptVAlign;

struct _RptAlign
{
	eRptHAlign h_align;
	eRptVAlign v_align;
};
typedef struct _RptAlign RptAlign;

struct _RptStroke
{
	gdouble width;
	RptColor *color;
	GArray *style;
};
typedef struct _RptStroke RptStroke;


RptPoint *rpt_common_get_position (xmlNode *xnode);
void rpt_common_set_position (xmlNode *xnode,
                              const RptPoint *position);

RptSize *rpt_common_get_size (xmlNode *xnode);
void rpt_common_set_size (xmlNode *xnode,
                          const RptSize *size);

RptRotation *rpt_common_get_rotation (xmlNode *xnode);
void rpt_common_set_rotation (xmlNode *xnode,
                              const RptRotation *rotation);

RptFont *rpt_common_get_font (xmlNode *xnode);
void rpt_common_set_font (xmlNode *xnode,
                          const RptFont *font);

RptBorder *rpt_common_get_border (xmlNode *xnode);
void rpt_common_set_border (xmlNode *xnode,
                            const RptBorder *border);

RptAlign *rpt_common_get_align (xmlNode *xnode);
void rpt_common_set_align (xmlNode *xnode,
                           const RptAlign *align);

RptStroke *rpt_common_get_stroke (xmlNode *xnode);
void rpt_common_set_stroke (xmlNode *xnode,
                            const RptStroke *stroke);

RptColor *rpt_common_parse_color (const gchar *str_color);
gchar *rpt_common_rptcolor_to_string (const RptColor *color);

gdouble *rpt_common_style_to_array (const GArray *style);


G_END_DECLS

#endif /* __RPT_COMMON_H__ */
