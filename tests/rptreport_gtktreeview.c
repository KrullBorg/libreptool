/*
 * Copyright (C) 2011 Andrea Zagli <azagli@libero.it>
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

enum
{
	TITLE_COLUMN,
	AUTHOR_COLUMN,
	CHECKED_COLUMN,
	N_COLUMNS
};

int
main (int argc, char **argv)
{
	RptReport *rptr;
	RptPrint *rptp;

	gtk_init (&argc, &argv);

	GtkListStore *store = gtk_list_store_new (N_COLUMNS,       /* Total number of columns */
	                                          G_TYPE_STRING,   /* Book title              */
	                                          G_TYPE_STRING,   /* Author                  */
	                                          G_TYPE_BOOLEAN); /* Is checked out?         */

	GtkTreeIter   iter;

	gtk_list_store_append (store, &iter);  /* Acquire an iterator */

	gtk_list_store_set (store, &iter,
	                    TITLE_COLUMN, "The Principle of Reason",
	                    AUTHOR_COLUMN, "Martin Heidegger",
	                    CHECKED_COLUMN, FALSE,
	                    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
	                    TITLE_COLUMN, "The Art of Computer Programming",
	                    AUTHOR_COLUMN, "Donald E. Knuth",
	                    CHECKED_COLUMN, FALSE,
	                    -1);

	GtkWidget *tree;

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Book's title",
	                                                   renderer,
	                                                   "text", TITLE_COLUMN,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Author",
	                                                   renderer,
	                                                   "text", AUTHOR_COLUMN,
	                                                   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	GtkWidget *w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (w), 500, 400);

	gtk_container_add (GTK_CONTAINER (w), tree);

	gtk_widget_show_all (w);

	rptr = rpt_report_new_from_gtktreeview (GTK_TREE_VIEW (tree), "\"Report's Title\"");

	if (rptr != NULL)
		{
			xmlDoc *report = rpt_report_get_xml (rptr);
			xmlSaveFormatFile ("test_report.rpt", report, 2);

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFile ("test_report.rptr", rptprint, 2);
		
			rptp = rpt_print_new_from_xml (rptprint);
			if (rptp != NULL)
				{
					g_object_set (G_OBJECT (rptp), "path-relatives-to", "..", NULL);
					rpt_print_set_output_type (rptp, RPT_OUTPUT_PDF);
					rpt_print_set_output_filename (rptp, "rptreport.pdf");
					rpt_print_print (rptp, NULL);
				}
		}

	gtk_main ();

	return 0;
}
