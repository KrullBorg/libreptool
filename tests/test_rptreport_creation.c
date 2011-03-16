/*
 * Copyright (C) 2007-2010 Andrea Zagli <azagli@inwind.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <rptreport.h>
#include <rptprint.h>
#include <rptobject.h>
#include <rptobjectellipse.h>
#include <rptobjectimage.h>
#include <rptobjectline.h>
#include <rptobjectrect.h>
#include <rptobjecttext.h>

int
main (int argc, char **argv)
{
	RptReport *rptr;
	RptPrint *rptp;
	RptObject *obj;
	RptPoint point;
	RptSize size;
	RptStroke stroke;
	RptColor *color;

	g_type_init ();

	rptr = rpt_report_new ();

	if (rptr != NULL)
		{
			g_object_set (G_OBJECT (rptr), "unit-length", 3, NULL);

			size.width = 210;
			size.height = 297;
			rpt_report_set_page_size (rptr, size);

			point.x = 10;
			point.y = 10;
			obj = rpt_obj_text_new ("text1", point);
			size.width = 210;
			size.height = 50;
			g_object_set (obj,
			              "source", "\"The first object inserted.\"",
			              "size", &size,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 10;
			point.y = 60;
			obj = rpt_obj_line_new ("line1", point);
			size.width = 210;
			size.height = 0;
			stroke.color = rpt_common_parse_color ("#FF0000");
			stroke.style = NULL;
			g_object_set (obj,
			              "size", &size,
			              "stroke", &stroke,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 0;
			point.y = 0;
			obj = rpt_obj_line_new ("line2", point);
			size.width = 210;
			size.height = 297;
			stroke.color = rpt_common_parse_color ("#000000AA");
			stroke.style = NULL;
			g_object_set (obj,
			              "size", &size,
			              "stroke", &stroke,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 210;
			point.y = 0;
			obj = rpt_obj_line_new ("line3", point);
			size.width = -210;
			size.height = 297;
			stroke.color = rpt_common_parse_color ("#000000AA");
			g_object_set (obj,
			              "size", &size,
			              "stroke", &stroke,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 105;
			point.y = 148.5;
			obj = rpt_obj_ellipse_new ("circle1", point);
			size.width = 50;
			size.height = 50;
			color = rpt_common_parse_color ("#00FF0099");
			stroke.color = rpt_common_parse_color ("#00FF00AA");
			stroke.style = NULL;
			g_object_set (obj,
			              "size", &size,
			              "stroke", &stroke,
			              "fill-color", color,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 50;
			point.y = 200;
			obj = rpt_obj_image_new ("image1", point);
			size.width = 100;
			size.height = 100;
			g_object_set (obj,
			              "size", &size,
			              "source", "gnome-globe.png",
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			xmlDoc *report = rpt_report_get_xml (rptr);
			xmlSaveFormatFileEnc ("test_report_created.rpt", report, "UTF-8", 2);

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFileEnc ("test_report_created.rptr", rptprint, "UTF-8", 2);
		
			rptp = rpt_print_new_from_xml (rptprint);
			if (rptp != NULL)
				{
					rpt_print_set_output_type (rptp, RPT_OUTPUT_PDF);
					rpt_print_set_output_filename (rptp, "test_report_created.pdf");
					rpt_print_print (rptp);
				}
		}

	return 0;
}
