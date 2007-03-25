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
