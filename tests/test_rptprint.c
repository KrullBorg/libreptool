/*
 * Copyright (C) 2006-2011 Andrea Zagli <azagli@libero.it>
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

static gchar *rptr_file_name = NULL;
static gchar *output_type = NULL;
static gchar *output_file_name = NULL;

static GOptionEntry entries[] =
{
	{ "rptr-file-name", 'r', 0, G_OPTION_ARG_STRING, &rptr_file_name, "RptPrint definition file name", "RPTR_FILE_NAME" },
	{ "output-type", 'o', 0, G_OPTION_ARG_STRING, &output_type, "Output type (png | pdf | ps | svg | gtk | gtk-default)", "OUTPUT-TYPE" },
	{ "output-file-name", 'f', 0, G_OPTION_ARG_FILENAME, &output_file_name, "Output file name", "FILE-NAME" },
	{ NULL }
};

int
main (int argc, char **argv)
{
	GError *error;
	GOptionContext *context;

	RptPrint *rptp;

	g_type_init ();

	context = g_option_context_new ("- test rptprint");
	g_option_context_add_main_entries (context, entries, NULL);

	error = NULL;
	if (!g_option_context_parse (context, &argc, &argv, &error))
		{
			g_error ("option parsing failed: %s\n", error->message);
			return 0;
		}

	rptp = rpt_print_new_from_file (rptr_file_name);

	if (rptp != NULL)
		{
			rpt_print_set_output_type (rptp, rpt_common_stroutputtype_to_enum (output_type));
			rpt_print_set_output_filename (rptp, output_file_name == NULL ? "test.out" : output_file_name);
			rpt_print_print (rptp, NULL);
		}

	return 0;
}
