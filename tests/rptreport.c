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

#include <rptreport.h>

int
main (int argc, char **argv)
{
	RptReport *rptr;

	g_type_init ();

	rptr = rpt_report_new_from_file (argv[1]);

	if (rptr != NULL)
		{
			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFile ("test_report.rptr", rptprint, 2);
		}

	return 0;
}
