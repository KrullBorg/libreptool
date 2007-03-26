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

G_BEGIN_DECLS


typedef struct
{
	gdouble r;
	gdouble g;
	gdouble b;
	gdouble a;
} RptColor;

typedef struct
{
	gdouble x;
	gdouble y;
} RptPoint;

typedef struct
{
	gdouble width;
	gdouble height;
} RptSize;

typedef struct
{
	gchar *name;
	gdouble size;
	gboolean bold;
	gboolean italic;
	gboolean underline;
	gboolean strike;
	RptColor color;
} RptFont;

typedef struct
{
	gdouble top_width;
	gdouble right_width;
	gdouble bottom_width;
	gdouble left_width;
	RptColor top_color;
	RptColor right_color;
	RptColor bottom_color;
	RptColor left_color;
} RptBorder;

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

typedef struct
{
	eRptHAlign h_align;
	eRptVAlign v_align;
} RptAlign;

typedef struct
{
	gdouble width;
	RptColor color;
} RptStroke;


void rpt_common_get_position (xmlNode *xnode,
                              RptPoint *position);
void rpt_common_get_size (xmlNode *xnode,
                          RptSize *size);
void rpt_common_get_font (xmlNode *xnode,
                          RptFont *font);
void rpt_common_set_font (xmlNode *xnode,
                          RptFont font);
void rpt_common_get_border (xmlNode *xnode,
                            RptBorder *border);
void rpt_common_set_border (xmlNode *xnode,
                            RptBorder border);
void rpt_common_get_align (xmlNode *xnode,
                           RptAlign *align);
void rpt_common_set_align (xmlNode *xnode,
                           RptAlign align);
void rpt_common_get_stroke (xmlNode *xnode,
                            RptStroke *stroke);
void rpt_common_parse_color (const gchar *str_color,
                             RptColor *color);


G_END_DECLS

#endif /* __RPT_COMMON_H__ */
