/*
 * Copyright (C) 2006-2007 Andrea Zagli <azagli@inwind.it>
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

#include <rptprint.h>

int
main (int argc, char **argv)
{
	RptPrint *rptp;

	g_type_init ();

	rptp = rpt_print_new_from_file (argv[1]);

	if (rptp != NULL)
		{
			rpt_print_set_output_type (rptp, RPTP_OUTPUT_PNG);
			rpt_print_set_output_filename (rptp, "test.png");
			rpt_print_print (rptp);
		}

	return 0;
}
