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
 * 
 */

#ifndef __RPT_COMMON_H__
#define __RPT_COMMON_H__

#include <glib.h>
#include <libxml/tree.h>

G_BEGIN_DECLS


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


void rpt_common_get_position (xmlNode *xnode,
                              RptPoint *position);
void rpt_common_get_size (xmlNode *xnode,
                          RptSize *size);


G_END_DECLS

#endif /* __RPT_COMMON_H__ */
