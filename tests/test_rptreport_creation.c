/*
 * Copyright (C) 2007 Andrea Zagli <azagli@inwind.it>
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

#include <libreptool.h>

int
main (int argc, char **argv)
{
	RptReport *rptr;
	RptObject *obj;
	RptPoint point;
	RptSize size;
	RptStroke stroke;
	RptColor *color;

	g_type_init ();

	rptr = rpt_report_new ();

	if (rptr != NULL)
		{
			size.width = 500;
			size.height = 500;
			rpt_report_set_page_size (rptr, size);

			point.x = 10;
			point.y = 10;
			obj = rpt_obj_text_new ("text1", point);
			size.width = 480;
			size.height = 50;
			g_object_set (obj,
			              "source", "The first object inserted.",
			              "size", &size,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 10;
			point.y = 60;
			obj = rpt_obj_line_new ("line1", point);
			size.width = 480;
			size.height = 0;
			stroke.color = rpt_common_parse_color ("#FF0000");
			g_object_set (obj,
			              "size", &size,
						  "stroke", &stroke,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 0;
			point.y = 0;
			obj = rpt_obj_line_new ("line2", point);
			size.width = 500;
			size.height = 500;
			stroke.color = rpt_common_parse_color ("#000000AA");
			g_object_set (obj,
			              "size", &size,
						  "stroke", &stroke,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 500;
			point.y = 0;
			obj = rpt_obj_line_new ("line3", point);
			size.width = -500;
			size.height = 500;
			stroke.color = rpt_common_parse_color ("#000000AA");
			g_object_set (obj,
			              "size", &size,
						  "stroke", &stroke,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			point.x = 250;
			point.y = 250;
			obj = rpt_obj_ellipse_new ("circle1", point);
			size.width = 100;
			size.height = 100;
			color = rpt_common_parse_color ("#00FF0099");
			stroke.color = rpt_common_parse_color ("#00FF00AA");
			g_object_set (obj,
			              "size", &size,
						  "stroke", &stroke,
						  "fill-color", color,
			              NULL);
			rpt_report_add_object_to_section (rptr, obj, RPTREPORT_SECTION_BODY);

			xmlDoc *report = rpt_report_get_xml (rptr);
			xmlSaveFormatFile ("test_report.rpt", report, 2);

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFile ("test_report.rptr", rptprint, 2);
		
			rpt_print_new_from_xml (rptprint, RPTP_OUTPUT_PDF, "test.pdf");
		}

	return 0;
}
