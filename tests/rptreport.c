/*
 * Copyright (C) 2007-2011 Andrea Zagli <azagli@libero.it>
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

static gchar *rpt_file_name = NULL;
static gchar *xml_rpt_file_name = NULL;
static gchar *xml_rptr_file_name = NULL;
static gchar *path_relatives_to  = NULL;
static gchar *output_type = NULL;
static gchar *output_file_name = NULL;

static GOptionEntry entries[] =
{
	{ "rpt-file-name", 'r', 0, G_OPTION_ARG_STRING, &rpt_file_name, "RptReport definition file name", "RPT_FILE_NAME" },
	{ "xml-report-file-name", 'x', 0, G_OPTION_ARG_FILENAME, &xml_rpt_file_name, "RptReport xml output file name", "FILE-NAME" },
	{ "xml-print-file-name", 'p', 0, G_OPTION_ARG_FILENAME, &xml_rptr_file_name, "RptPrint xml output file name", "FILE-NAME" },
	{ "path-relatives-to", 't', 0, G_OPTION_ARG_FILENAME, &path_relatives_to, "Path relatives to", "FILE-NAME" },
	{ "output-type", 'o', 0, G_OPTION_ARG_STRING, &output_type, "Output type (png | pdf | ps | svg | gtk | gtk-default)", "OUTPUT-TYPE" },
	{ "output-file-name", 'f', 0, G_OPTION_ARG_FILENAME, &output_file_name, "Output file name", "FILE-NAME" },
	{ NULL }
};

gchar
*field_request (RptReport *rpt_report,
                gchar *field_name,
                GdaDataModel *data_model,
                gint row,
                GtkTreeModel *treemodel,
                GtkTreeIter *iter,
                gpointer user_data)
{
	gchar *ret = NULL;

	if (g_strcmp0 (field_name, "field_to_request") == 0)
		{
			ret = g_strdup ("the field requested");
		}
	else if (g_strcmp0 (field_name, "nonexistent") == 0 &&
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
	GError *error;
	GOptionContext *context;

	RptReport *rptr;
	RptPrint *rptp;

	g_type_init ();

	context = g_option_context_new ("- test rptprint");
	g_option_context_add_main_entries (context, entries, NULL);

	error = NULL;
	if (!g_option_context_parse (context, &argc, &argv, &error)
	    || error != NULL)
		{
			g_error ("Option parsing failed: %s.", error != NULL && error->message != NULL ? error->message : "no details");
			return 0;
		}

	rptr = rpt_report_new_from_file (rpt_file_name);
	if (rptr == NULL)
		{
			g_error ("Error on creating RptReport object.");
			return 0;
		}

	g_signal_connect (rptr, "field-request", G_CALLBACK (field_request), NULL);

	if (rptr != NULL)
		{
			xmlDoc *report = rpt_report_get_xml (rptr);
			if (xml_rpt_file_name != NULL)
				{
					xmlSaveFormatFileEnc (xml_rpt_file_name, report, "UTF-8", 2);
				}

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			if (xml_rptr_file_name != NULL)
				{
					xmlSaveFormatFileEnc (xml_rptr_file_name, rptprint, "UTF-8", 2);
				}

			rptp = rpt_print_new_from_xml (rptprint);
			if (rptp != NULL)
				{
					if (path_relatives_to != NULL)
						{
							g_object_set (G_OBJECT (rptp), "path-relatives-to", path_relatives_to, NULL);
						}

					rpt_print_set_output_type (rptp, rpt_common_stroutputtype_to_enum (output_type));
					if (g_strcmp0 (output_type, "png") == 0
					    || g_strcmp0 (output_type, "pdf") == 0
					    || g_strcmp0 (output_type, "ps") == 0
					    || g_strcmp0 (output_type, "svg") == 0)
						{
							rpt_print_set_output_filename (rptp, output_file_name == NULL ? g_strdup_printf ("test.%s", output_type) : output_file_name);
						}
					rpt_print_print (rptp, NULL);
				}
			else
				{
					g_error ("Error on creating RptPrint object.");
					return 0;
				}
		}

	return 0;
}
