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

gchar
*field_request (RptReport *rpt_report,
                gchar *field_name,
                GdaDataModel *data_model,
                gint row,
                gpointer user_data)
{
	gchar *ret = NULL;

	if (strcmp (field_name, "field_to_request") == 0)
		{
			ret = g_strdup ("the field requested");
		}
	else if (strcmp (field_name, "nonexistent") == 0 &&
	         data_model != NULL &&
	         row > -1)
		{
			ret = g_strdup_printf ("%s - %s",
			                       gda_value_stringify (gda_data_model_get_value_at (data_model, 0, row, NULL)),
			                       gda_value_stringify (gda_data_model_get_value_at (data_model, 1, row, NULL)));
		}

	return ret;
}

int
main (int argc, char **argv)
{
	RptReport *rptr;
	RptPrint *rptp;

	g_type_init ();

	rptr = rpt_report_new_from_file (argv[1]);

	g_signal_connect (rptr, "field-request", G_CALLBACK (field_request), NULL);

	if (rptr != NULL)
		{
			xmlDoc *report = rpt_report_get_xml (rptr);
			xmlSaveFormatFile ("test_report.rpt", report, 2);

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFile ("test_report.rptr", rptprint, 2);
		
			rptp = rpt_print_new_from_xml (rptprint);
			if (rptp != NULL)
				{
					rpt_print_set_output_type (rptp, RPT_OUTPUT_PDF);
					rpt_print_set_output_filename (rptp, "test.pdf");
					rpt_print_print (rptp);
				}
		}

	return 0;
}
