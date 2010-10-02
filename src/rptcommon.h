/*
 * Copyright (C) 2007-2010 Andrea Zagli <azagli@libero.it>
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
#include <gdk/gdkcolor.h>
#include <libxml/tree.h>

#include <pango/pango-attributes.h>
#include <pango/pango-font.h>

G_BEGIN_DECLS


typedef enum
{
	RPT_UNIT_POINTS,
	RPT_UNIT_INCHES,
	RPT_UNIT_CENTIMETRE,
	RPT_UNIT_MILLIMETRE
} eRptUnitLength;

typedef enum
{
	RPT_OUTPUT_PNG,
	RPT_OUTPUT_PDF,
	RPT_OUTPUT_PS,
	RPT_OUTPUT_SVG,
	RPT_OUTPUT_GTK,
	RPT_OUTPUT_GTK_DEFAULT_PRINTER
} eRptOutputType;

/**
 * RptColor:
 * @r: the red channel; value from 0 to 1.
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

/**
 * RptBorder:
 * @top_width:
 * @right_width:
 * @bottom_width:
 * @left_width:
 * @top_color: an #RptColor
 * @right_color: an #RptColor
 * @bottom_color: an #RptColor
 * @left_color: an #RptColor
 * @top_style: a #GArray of #gdouble values representing dashes sequence
 * @right_style: a #GArray of #gdouble values representing dashes sequence
 * @bottom_style: a #GArray of #gdouble values representing dashes sequence
 * @left_style: a #GArray of #gdouble values representing dashes sequence
 */
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

/**
 * RptStroke:
 * @width:
 * @color: an #RptColor
 * @style: a #GArray of #gdouble values representing dashes sequence
 */
struct _RptStroke
{
	gdouble width;
	RptColor *color;
	GArray *style;
};
typedef struct _RptStroke RptStroke;


gdouble rpt_common_value_to_points (eRptUnitLength unit, gdouble value);
gdouble rpt_common_points_to_value (eRptUnitLength unit, gdouble value);

eRptUnitLength rpt_common_strunit_to_enum (const gchar *unit);
const gchar *rpt_common_enum_to_strunit (eRptUnitLength unit);

eRptOutputType rpt_common_stroutputtype_to_enum (const gchar *output_type);
const gchar *rpt_common_enum_to_stroutputtype (eRptOutputType output_type);

RptPoint *rpt_common_rptpoint_new (void);
RptPoint *rpt_common_get_position (xmlNode *xnode);
void rpt_common_set_position (xmlNode *xnode,
                              const RptPoint *position);

RptSize *rpt_common_rptsize_new (void);
RptSize *rpt_common_get_size (xmlNode *xnode);
void rpt_common_set_size (xmlNode *xnode,
                          const RptSize *size);

RptRotation *rpt_common_rptrotation_new (void);
RptRotation *rpt_common_get_rotation (xmlNode *xnode);
void rpt_common_set_rotation (xmlNode *xnode,
                              const RptRotation *rotation);

RptFont *rpt_common_rptfont_new (void);
RptFont *rpt_common_get_font (xmlNode *xnode);
void rpt_common_set_font (xmlNode *xnode,
                          const RptFont *font);
RptFont *rpt_common_rptfont_from_pango_description (const PangoFontDescription *description);

RptBorder *rpt_common_rptborder_new (void);
RptBorder *rpt_common_get_border (xmlNode *xnode);
void rpt_common_set_border (xmlNode *xnode,
                            const RptBorder *border);

RptAlign *rpt_common_rptalign_new (void);
RptAlign *rpt_common_get_align (xmlNode *xnode);
void rpt_common_set_align (xmlNode *xnode,
                           const RptAlign *align);

RptStroke *rpt_common_rptstroke_new (void);
RptStroke *rpt_common_get_stroke (xmlNode *xnode);
void rpt_common_set_stroke (xmlNode *xnode,
                            const RptStroke *stroke);

RptColor *rpt_common_rptcolor_new (void);
RptColor *rpt_common_parse_color (const gchar *str_color);
gchar *rpt_common_rptcolor_to_string (const RptColor *color);
GdkColor *rpt_common_rptcolor_to_gdkcolor (const RptColor *color);
RptColor *rpt_common_gdkcolor_to_rptcolor (const GdkColor *gdk_color, guint16 alpha);

gdouble *rpt_common_style_to_array (const GArray *style);


G_END_DECLS

#endif /* __RPT_COMMON_H__ */
