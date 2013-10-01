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

enum
{
	TITLE_COLUMN,
	AUTHOR_COLUMN,
	CHECKED_COLUMN,
	N_COLUMNS
};

GtkWidget *w;
GtkWidget *tree;

gboolean
on_w_delete_event (GtkWidget *widget,
                   GdkEvent *event,
                   gpointer user_data)
{
	return FALSE;
}

void
on_btn_stampa_clicked (GtkButton *button, gpointer user_data)
{
	RptReport *rptr;
	RptPrint *rptp;

	rptr = rpt_report_new_from_gtktreeview (GTK_TREE_VIEW (tree), "\"Report's Title\"");

	if (rptr != NULL)
		{
			xmlDoc *report = rpt_report_get_xml (rptr);
			rpt_report_set_output_type (rptr, RPT_OUTPUT_GTK);
			xmlSaveFormatFile ("test_report.rpt", report, 2);

			xmlDoc *rptprint = rpt_report_get_xml_rptprint (rptr);
			xmlSaveFormatFile ("test_report.rptr", rptprint, 2);

			rptp = rpt_print_new_from_xml (rptprint);
			if (rptp != NULL)
				{
					g_object_set (G_OBJECT (rptp), "path-relatives-to", "..", NULL);
					rpt_print_set_output_type (rptp, RPT_OUTPUT_GTK);
					rpt_print_print (rptp, GTK_WINDOW (w));
				}
		}
}

int
main (int argc, char **argv)
{
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
	                    CHECKED_COLUMN, TRUE,
	                    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
	                    TITLE_COLUMN, "Pinocchio",
	                    AUTHOR_COLUMN, "Collodi",
	                    CHECKED_COLUMN, FALSE,
	                    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
	                    TITLE_COLUMN, "Hyperion",
	                    AUTHOR_COLUMN, "Dan Simmons",
	                    CHECKED_COLUMN, TRUE,
	                    -1);

	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_tree_view_set_headers_clickable (GTK_TREE_VIEW (tree), TRUE);

	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Book's title",
	                                                   renderer,
	                                                   "text", TITLE_COLUMN,
	                                                   NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, TITLE_COLUMN);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Author",
	                                                   renderer,
	                                                   "text", AUTHOR_COLUMN,
	                                                   NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, AUTHOR_COLUMN);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes ("Checked",
	                                                   renderer,
	                                                   "active", CHECKED_COLUMN,
	                                                   NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CHECKED_COLUMN);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (w), 500, 400);

	g_signal_connect (w, "delete-event", G_CALLBACK (on_w_delete_event), NULL);
	g_signal_connect (w, "destroy", gtk_main_quit, NULL);

	GtkWidget *box = gtk_vbox_new (FALSE, 5);

	gtk_container_add (GTK_CONTAINER (w), box);

	gtk_box_pack_start (GTK_BOX (box), tree, TRUE, TRUE, 0);

	GtkWidget *btn_stampa = gtk_button_new_from_stock ("gtk-print");

	gtk_box_pack_start (GTK_BOX (box), btn_stampa, FALSE, FALSE, 0);

	g_signal_connect (G_OBJECT (btn_stampa), "clicked",
	                  G_CALLBACK (on_btn_stampa_clicked), NULL);

	gtk_widget_show_all (w);

	gtk_main ();

	return 0;
}
