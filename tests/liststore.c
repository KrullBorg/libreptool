/*
 * Copyright (C) 2011-2013 Andrea Zagli <azagli@libero.it>
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
                GtkTreeModel *treemodel,
                GtkTreeIter *iter,
                gpointer user_data)
{
	gchar *ret = NULL;

	if (g_strcmp0 (field_name, "nonexistent") == 0 &&
	    treemodel != NULL &&
	    iter != NULL)
		{
			gint id;
			gchar *name;

			gtk_tree_model_get (treemodel, iter,
			                    0, &id,
			                    1, &name,
			                    -1);

			ret = g_strdup_printf ("%d - %s", id, name);
		}

	return ret;
}

int
main (int argc, char **argv)
{
	RptReport *rptr;
	RptPrint *rptp;

	GtkListStore *model;
	GtkTreeIter iter;
	GHashTable *columns_names;

	rptr = rpt_report_new_from_file (argv[1]);

	model = gtk_list_store_new (2,
	                            G_TYPE_INT,
	                            G_TYPE_STRING);

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
	                    0, 1,
	                    1, "Mary Jane Red",
	                    -1);

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
	                    0, 2,
	                    1, "John Doe",
	                    -1);

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
	                    0, 3,
	                    1, "Elene McArty",
	                    -1);

	gtk_list_store_append (model, &iter);
	gtk_list_store_set (model, &iter,
	                    0, 4,
	                    1, "Raul Bread",
	                    -1);

	columns_names = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (columns_names, "id", "0");
	g_hash_table_insert (columns_names, "name", "1");

	rpt_report_set_database_as_gtktreemodel (rptr, GTK_TREE_MODEL (model), columns_names);

	g_signal_connect (rptr, "field-request", G_CALLBACK (field_request), NULL);

	if (rptr != NULL)
		{
			rpt_report_set_output_type (rptr, RPT_OUTPUT_PNG);
			rpt_report_set_output_filename (rptr, "test.png");

			xmlDoc *report = rpt_report_get_xml (rptr);
			xmlSaveFormatFile ("test_report.rpt", report, 2);

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFile ("test_report.rptr", rptprint, 2);
		
			rptp = rpt_print_new_from_xml (rptprint);
			if (rptp != NULL)
				{
					g_object_set (G_OBJECT (rptp), "path-relatives-to", "..", NULL);

					rpt_print_print (rptp, NULL);
				}
		}

	return 0;
}
