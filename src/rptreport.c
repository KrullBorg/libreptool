/*
 * Copyright (C) 2007-2011 Andrea Zagli <azagli@libero.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libxml/xpath.h>

#include <sql-parser/gda-sql-parser.h>

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "rptreport.h"
#include "rptreport_priv.h"
#include "rptcommon.h"
#include "rptobjecttext.h"
#include "rptobjectline.h"
#include "rptobjectrect.h"
#include "rptobjectellipse.h"
#include "rptobjectimage.h"

#include "rptmarshal.h"

#include "parser.tab.h"

typedef struct
{
	gchar *provider_id;
	gchar *connection_string;
	gchar *sql;

	GdaConnection *gda_conn;
	GdaDataModel *gda_datamodel;

	GtkTreeModel *treemodel;
	GHashTable *columns_names;
} Database;

typedef struct
{
	RptSize *size;
	RptMargin *margin;
} Page;

typedef struct
{
	gdouble height;
	GList *objects;
	gboolean new_page_after;
} ReportHeader;

typedef struct
{
	gdouble height;
	GList *objects;
	gboolean new_page_before;
} ReportFooter;

typedef struct
{
	gdouble height;
	GList *objects;
	gboolean first_page;
	gboolean last_page;
} PageHeader;

typedef struct
{
	gdouble height;
	GList *objects;
	gboolean first_page;
	gboolean last_page;
} PageFooter;

typedef struct
{
	gdouble height;
	GList *objects;
	gboolean new_page_after;
} Body;

enum
{
	PROP_0,
	PROP_UNIT_LENGTH
};

static void rpt_report_class_init (RptReportClass *klass);
static void rpt_report_init (RptReport *rpt_report);

static void rpt_report_set_property (GObject *object,
                                     guint property_id,
                                     const GValue *value,
                                     GParamSpec *pspec);
static void rpt_report_get_property (GObject *object,
                                     guint property_id,
                                     GValue *value,
                                     GParamSpec *pspec);

static void rpt_report_xml_parse_section (RptReport *rpt_report, xmlNode *xnode, RptReportSection section);

static RptObject *rpt_report_get_object_from_name_in_list (GList *list, const gchar *name);

static RptReportSection rpt_report_object_get_section (RptReport *rpt_report, RptObject *rpt_object);
static gboolean rpt_report_object_is_in_section (RptReport *rpt_report, RptObject *rpt_object, RptReportSection section);

static void rpt_report_section_create (RptReport *rpt_report, RptReportSection section);
static xmlNode *rpt_report_section_get_xml (RptReport *rpt_report, RptReportSection section);

static xmlNode *rpt_report_rptprint_get_properties_node (xmlDoc *xdoc);

static xmlNode *rpt_report_rptprint_new_page (RptReport *rpt_report,
                                              xmlDoc *xdoc);
static void rpt_report_rptprint_section (RptReport *rpt_report,
                                         xmlNode *xpage,
                                         gdouble *cur_y,
                                         RptReportSection section);

static void rpt_report_rptprint_parse_text_source (RptReport *rpt_report,
                                                   RptObject *rptobj,
                                                   xmlNode *xnode);

static void rpt_report_change_specials (RptReport *rpt_report, xmlDoc *xdoc);


#define RPT_REPORT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_REPORT, RptReportPrivate))

typedef struct _RptReportPrivate RptReportPrivate;
struct _RptReportPrivate
	{
		eRptUnitLength unit;

		eRptOutputType output_type;
		gchar *output_filename;

		guint copies;

		Database *db;

		Page *page;
		ReportHeader *report_header;
		ReportFooter *report_footer;
		PageHeader *page_header;
		PageFooter *page_footer;
		Body *body;

		guint cur_page;
		gint cur_row;
		GtkTreeIter *cur_iter;
	};

G_DEFINE_TYPE (RptReport, rpt_report, G_TYPE_OBJECT)

static void
rpt_report_class_init (RptReportClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptReportPrivate));

	object_class->set_property = rpt_report_set_property;
	object_class->get_property = rpt_report_get_property;

	g_object_class_install_property (object_class, PROP_UNIT_LENGTH,
	                                 g_param_spec_int ("unit-length",
	                                                   "Unit length",
	                                                   "The unit length.",
	                                                   RPT_UNIT_POINTS, RPT_UNIT_MILLIMETRE,
	                                                   RPT_UNIT_POINTS,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	/**
	 * RptReport::field-request:
	 * @rpt_report: an #RptReport object that recieved the signal.
	 * @field_name: the name of the field requested.
	 * @data_model: a #GdaDataModel; or NULL if there's no database source.
	 * @row: the current @data_model's row; -1 if @data_model is NULL.
	 * @treemodel: a #GtkTreeModel; or NULL if there's no #GtkTreeModel source.
	 * @iter: a #GtkTreeIter; or NULL if @treemodel is NULL.
	 *
	 * The signal is emitted each time there's into text's attribute source
	 * a field that doesn't exists.
	 */
	klass->field_request_signal_id = g_signal_new ("field-request",
	                                               G_TYPE_FROM_CLASS (object_class),
	                                               G_SIGNAL_RUN_LAST,
	                                               0,
	                                               NULL,
	                                               NULL,
	                                               _rpt_marshal_STRING__STRING_POINTER_INT_POINTER_POINTER,
	                                               G_TYPE_STRING,
	                                               5,
	                                               G_TYPE_STRING,
	                                               G_TYPE_POINTER,
	                                               G_TYPE_INT,
	                                               G_TYPE_POINTER,
	                                               G_TYPE_POINTER);
}

static void
rpt_report_init (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->db = NULL;

	priv->page = (Page *)g_malloc0 (sizeof (Page));

	priv->page->size = rpt_common_rptsize_new ();
	priv->page->margin = rpt_common_rptmargin_new ();

	priv->report_header = NULL;
	priv->report_footer = NULL;
	priv->page_header = NULL;
	priv->page_footer = NULL;

	priv->body = (Body *)g_malloc0 (sizeof (Body));
	priv->body->height = 0.0;
	priv->body->objects = NULL;
	priv->body->new_page_after = FALSE;

	priv->output_type = RPT_OUTPUT_PDF;
	priv->output_filename = g_strdup ("rptreport.pdf");
	priv->copies = 1;

	priv->cur_row = -1;
	priv->cur_iter = NULL;
}

/**
 * rpt_report_new:
 *
 * Creates a new #RptReport object.
 *
 * Returns: the newly created #RptReport object.
 */
RptReport
*rpt_report_new ()
{
	RptReport *rpt_report;

	rpt_report = RPT_REPORT (g_object_new (rpt_report_get_type (), NULL));

	return rpt_report;
}

/**
 * rpt_report_new_from_file:
 * @filename: the path of the xml file to load.
 *
 * Returns: the newly created #RptReport object.
 */
RptReport
*rpt_report_new_from_file (const gchar *filename)
{
	RptReport *rpt_report = NULL;

	xmlDoc *xdoc = xmlParseFile (filename);
	if (xdoc != NULL)
		{
			rpt_report = rpt_report_new_from_xml (xdoc);
		}

	return rpt_report;
}

/**
 * rpt_report_new_from_xml:
 * @xdoc: an #xmlDoc.
 *
 * Returns: the newly created #RptReport object.
 */
RptReport
*rpt_report_new_from_xml (xmlDoc *xdoc)
{
	RptReport *rpt_report = NULL;

	xmlNode *cur = xmlDocGetRootElement (xdoc);
	if (cur != NULL)
		{
			if (g_strcmp0 (cur->name, "reptool") == 0)
				{
					xmlXPathContextPtr xpcontext;
					xmlXPathObjectPtr xpresult;
					xmlNodeSetPtr xnodeset;
					RptReportPrivate *priv;

					rpt_report = rpt_report_new ();

					priv = RPT_REPORT_GET_PRIVATE (rpt_report);

					/* search for node "properties" */
					xmlNode *xnodeprop = rpt_report_rptprint_get_properties_node (xdoc);
					if (xnodeprop != NULL)
						{
							xmlNode *cur_property = xnodeprop->children;
							while (cur_property != NULL)
								{
									if (g_strcmp0 (cur_property->name, "unit-length") == 0)
										{
											g_object_set (G_OBJECT (rpt_report), "unit-length", rpt_common_strunit_to_enum ((const gchar *)xmlNodeGetContent (cur_property)), NULL);
										}
									else if (g_strcmp0 (cur_property->name, "output-type") == 0)
										{
											rpt_report_set_output_type (rpt_report, rpt_common_stroutputtype_to_enum ((const gchar *)xmlNodeGetContent (cur_property)));
										}
									else if (g_strcmp0 (cur_property->name, "output-filename") == 0)
										{
											rpt_report_set_output_filename (rpt_report, (const gchar *)xmlNodeGetContent (cur_property));
										}
									else if (g_strcmp0 (cur_property->name, "copies") == 0)
										{
											rpt_report_set_copies (rpt_report, strtol ((const gchar *)xmlNodeGetContent (cur_property), NULL, 10));
										}

									cur_property = cur_property->next;
								}
						}

					/* search for node "database" */
					xpcontext = xmlXPathNewContext (xdoc);
					xpcontext->node = cur;
					xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::database", xpcontext);
					if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
						{
							xnodeset = xpresult->nodesetval;
							if (xnodeset->nodeNr == 1)
								{
									gchar *provider_id;
									gchar *connection_string;
									gchar *sql;

									xmlNode *cur_database = xnodeset->nodeTab[0]->children;
									while (cur_database != NULL)
										{
											if (g_strcmp0 (cur_database->name, "provider") == 0)
												{
													provider_id = g_strstrip ((gchar *)xmlNodeGetContent (cur_database));
												}
											else if (g_strcmp0 (cur_database->name, "connection-string") == 0)
												{
													connection_string = g_strstrip ((gchar *)xmlNodeGetContent (cur_database));
												}
											else if (g_strcmp0 (cur_database->name, "sql") == 0)
												{
													sql = g_strstrip ((gchar *)xmlNodeGetContent (cur_database));
												}
											
											cur_database = cur_database->next;
										}

									if (g_strcmp0 (provider_id, "") == 0 ||
										g_strcmp0 (connection_string, "") == 0 ||
										g_strcmp0 (sql, "") == 0)
										{
											/* TO DO */
										}
									else
										{
											rpt_report_set_database (rpt_report, provider_id, connection_string, sql);
										}

									g_free (provider_id);
									g_free (connection_string);
									g_free (sql);
								}
						}

					/* search for node "page" */
					xpcontext->node = cur;
					xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page", xpcontext);
					if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
						{
							xnodeset = xpresult->nodesetval;
							if (xnodeset->nodeNr == 1)
								{
									gchar *prop;
									gdouble margin_top = 0.0;
									gdouble margin_right = 0.0;
									gdouble margin_bottom = 0.0;
									gdouble margin_left = 0.0;
									RptSize *size;

									size = rpt_common_get_size (xnodeset->nodeTab[0]);
									rpt_report_set_page_size (rpt_report, *size);

									prop = xmlGetProp (xnodeset->nodeTab[0], "margin-top");
									if (prop != NULL)
										{
											margin_top = g_strtod (prop, NULL);
										}
									prop = xmlGetProp (xnodeset->nodeTab[0], "margin-right");
									if (prop != NULL)
										{
											margin_right = g_strtod (prop, NULL);
										}
									prop = xmlGetProp (xnodeset->nodeTab[0], "margin-bottom");
									if (prop != NULL)
										{
											margin_bottom = g_strtod (prop, NULL);
										}
									prop = xmlGetProp (xnodeset->nodeTab[0], "margin-left");
									if (prop != NULL)
										{
											margin_left = g_strtod (prop, NULL);
										}
									rpt_report_set_page_margins (rpt_report, margin_top, margin_right, margin_bottom, margin_left);
								}
							else
								{
									/* TO DO */
									/* return */
									g_error ("Node «page» is missing");
								}
						}
					else
						{
							/* TO DO */
							/* return */
							g_error ("Node «page» is missing");
						}

					/* search for node "report" */
					xpcontext->node = cur;
					xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::report", xpcontext);
					if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
						{
							xnodeset = xpresult->nodesetval;
							if (xnodeset->nodeNr == 1)
								{
									/* search for node "report-header" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::report-header", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
											rpt_report_section_create (rpt_report, RPTREPORT_SECTION_REPORT_HEADER);
											rpt_report_xml_parse_section (rpt_report, xpresult->nodesetval->nodeTab[0], RPTREPORT_SECTION_REPORT_HEADER);
										}

									/* search for node "report-footer" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::report-footer", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
											rpt_report_section_create (rpt_report, RPTREPORT_SECTION_REPORT_FOOTER);
											rpt_report_xml_parse_section (rpt_report, xpresult->nodesetval->nodeTab[0], RPTREPORT_SECTION_REPORT_FOOTER);
										}

									/* search for node "page-header" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page-header", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
											rpt_report_section_create (rpt_report, RPTREPORT_SECTION_PAGE_HEADER);
											rpt_report_xml_parse_section (rpt_report, xpresult->nodesetval->nodeTab[0], RPTREPORT_SECTION_PAGE_HEADER);
										}

									/* search for node "page-footer" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page-footer", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
											rpt_report_section_create (rpt_report, RPTREPORT_SECTION_PAGE_FOOTER);
											rpt_report_xml_parse_section (rpt_report, xpresult->nodesetval->nodeTab[0], RPTREPORT_SECTION_PAGE_FOOTER);
										}

									/* search for node "body" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::body", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
											rpt_report_xml_parse_section (rpt_report, xpresult->nodesetval->nodeTab[0], RPTREPORT_SECTION_BODY);
										}
									else
										{
											/* TO DO */
											/* return */
											g_error ("Node «body» is missing");
										}
								}
							else
								{
									/* TO DO */
									/* return */
									g_error ("Only one node «report» is allowed");
								}
						}
					else
						{
							/* TO DO */
							/* return */
							g_error ("Node «report» is missing");
						}
				}
			else
				{
					/* TO DO */
					g_warning ("The file is not a valid reptool report definition file.");
				}
		}

	return rpt_report;
}

/**
 * rpt_report_set_output_type:
 * @rpt_report:
 * @output_type:
 *
 */
void
rpt_report_set_output_type (RptReport *rpt_report, eRptOutputType output_type)
{
	g_return_if_fail (IS_RPT_REPORT (rpt_report));

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->output_type = output_type;
}

/**
 * rpt_report_set_output_filename:
 * @rpt_report:
 * @output_filename:
 *
 */
void
rpt_report_set_output_filename (RptReport *rpt_report, const gchar *output_filename)
{
	g_return_if_fail (IS_RPT_REPORT (rpt_report));

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->output_filename = g_strstrip (g_strdup (output_filename));
	if (g_strcmp0 (priv->output_filename, "") == 0)
		{
			g_warning ("It's not possible to set an empty output filename.");
			priv->output_filename = g_strdup ("rptreport.pdf");
		}
}

/**
 * rpt_report_set_copies:
 * @rpt_report:
 * @copies:
 *
 */
void
rpt_report_set_copies (RptReport *rpt_report, guint copies)
{
	g_return_if_fail (IS_RPT_REPORT (rpt_report));

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->copies = copies;
}

/**
 * rpt_report_database_get_provider:
 * @rpt_report:
 *
 */
const gchar
*rpt_report_database_get_provider (RptReport *rpt_report)
{
	gchar *ret = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			ret = g_strdup (priv->db->provider_id);
		}

	return (const gchar *)ret;
}

/**
 * rpt_report_database_get_connection_string:
 * @rpt_report:
 *
 */
const gchar
*rpt_report_database_get_connection_string (RptReport *rpt_report)
{
	gchar *ret = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			ret = g_strdup (priv->db->connection_string);
		}

	return (const gchar *)ret;
}

/**
 * rpt_report_database_get_sql:
 * @rpt_report:
 *
 */
const gchar
*rpt_report_database_get_sql (RptReport *rpt_report)
{
	gchar *ret = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			ret = g_strdup (priv->db->sql);
		}

	return (const gchar *)ret;
}

/**
 * rpt_report_set_database:
 * @rpt_report: an #RptReport object.
 * @provider_id: a libgda's provider_id.
 * @connection_string: a libgda's connection string.
 * @sql: a valid SQL statement.
 *
 */
void
rpt_report_set_database (RptReport *rpt_report,
                         const gchar *provider_id,
                         const gchar *connection_string,
                         const gchar *sql)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			g_free (priv->db);
		}
	priv->db = (Database *)g_new0 (Database, 1);

	priv->db->provider_id = g_strstrip (g_strdup (provider_id));
	priv->db->connection_string = g_strstrip (g_strdup (connection_string));
	priv->db->sql = g_strstrip (g_strdup (sql));
	priv->db->gda_conn = NULL;
	priv->db->gda_datamodel = NULL;
	priv->db->treemodel = NULL;
	priv->db->columns_names = NULL;
}

/**
 * rpt_report_set_database_from_datamodel:
 * @rpt_report: an #RptReport object.
 * @data_model: a #GdaDataModel.
 *
 */
void
rpt_report_set_database_from_datamodel (RptReport *rpt_report, GdaDataModel *data_model)
{
	g_return_if_fail (IS_RPT_REPORT (rpt_report));
	g_return_if_fail (GDA_IS_DATA_MODEL (data_model));

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			if (priv->db->columns_names != NULL)
				{
					g_hash_table_destroy (priv->db->columns_names);
				}

			g_free (priv->db);
		}
	priv->db = (Database *)g_new0 (Database, 1);

	priv->db->provider_id = NULL;
	priv->db->connection_string = NULL;
	priv->db->sql = NULL;
	priv->db->gda_conn = NULL;
	priv->db->gda_datamodel = data_model;
	priv->db->treemodel = NULL;
	priv->db->columns_names = NULL;
}

/**
 * rpt_report_set_database_as_gtktreemodel:
 * @rpt_report: an #RptReport object.
 * @model: a #GtkTreeModel (for now only #GtkListStore is supported).
 * @columns_names:
 *
 */
void
rpt_report_set_database_as_gtktreemodel (RptReport *rpt_report,
                                         GtkTreeModel *model,
                                         GHashTable *columns_names)
{
	g_return_if_fail (IS_RPT_REPORT (rpt_report));
	g_return_if_fail (GTK_IS_TREE_MODEL (model));
	g_return_if_fail (columns_names != NULL);

	g_return_if_fail (GTK_IS_LIST_STORE (model));

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			if (priv->db->gda_datamodel != NULL)
				{
					g_object_unref (priv->db->gda_datamodel);
				}
			if (priv->db->gda_conn != NULL)
				{
					gda_connection_close_no_warning (priv->db->gda_conn);
				}

			g_free (priv->db);
		}
	priv->db = (Database *)g_new0 (Database, 1);

	priv->db->provider_id = NULL;
	priv->db->connection_string = NULL;
	priv->db->sql = NULL;
	priv->db->gda_conn = NULL;
	priv->db->gda_datamodel = NULL;
	priv->db->treemodel = model;
	priv->db->columns_names = g_hash_table_ref (columns_names);
}

/**
 * rpt_report_set_page_size:
 * @rpt_report: an #RptReport object.
 * @size: an #RptSize.
 *
 * Sets page's size.
 */
void
rpt_report_set_page_size (RptReport *rpt_report,
                          RptSize size)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (size.width <= 0.0 ||
	    size.height <= 0.0)
		{
			/* TO DO */
			g_warning ("Page's width and height must be greater than zero.");
		}
	else
		{
			priv->page->size->width = size.width;
			priv->page->size->height = size.height;
		}
}

/**
 * rpt_report_get_page_size:
 * @rpt_report:
 *
 */
RptSize
*rpt_report_get_page_size (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	RptSize *size = (RptSize *)g_malloc0 (sizeof (RptSize));

	size->width = priv->page->size->width;
	size->height = priv->page->size->height;

	return size;
}

/**
 * rpt_report_set_page_margins:
 * @rpt_report:
 * @top:
 * @right:
 * @bottom:
 * @left:
 *
 * Sets page's margins.
 */
void
rpt_report_set_page_margins (RptReport *rpt_report,
                             gdouble top,
                             gdouble right,
                             gdouble bottom,
                             gdouble left)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->page->margin->top = top;
	priv->page->margin->right = right;
	priv->page->margin->bottom = bottom;
	priv->page->margin->left = left;
}

/**
 * rpt_report_get_page_margins:
 * @rpt_report:
 * @top:
 * @right:
 * @bottom:
 * @left:
 *
 */
void
rpt_report_get_page_margins (RptReport *rpt_report,
                             gdouble *top,
                             gdouble *right,
                             gdouble *bottom,
                             gdouble *left)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	*top = priv->page->margin->top;
	*right = priv->page->margin->right;
	*bottom = priv->page->margin->bottom;
	*left = priv->page->margin->left;
}

RptMargin
*rpt_report_get_page_margins_struct (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return g_memdup (priv->page->margin, sizeof (RptMargin));
}

void
rpt_report_set_page_margins_struct (RptReport *rpt_report, RptMargin margin)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	g_free (priv->page->margin);
	priv->page->margin = g_memdup (&margin, sizeof (RptMargin));
}

/**
 * rpt_report_get_section_height:
 * @rpt_report:
 * @section:
 *
 */
gdouble
rpt_report_get_section_height (RptReport *rpt_report,
                               RptReportSection section)
{
	gdouble ret = -1;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				if (priv->report_header != NULL)
					{
						ret = priv->report_header->height;
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				if (priv->report_footer != NULL)
					{
						ret = priv->report_footer->height;
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				if (priv->page_header != NULL)
					{
						ret = priv->page_header->height;
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				if (priv->page_footer != NULL)
					{
						ret = priv->page_footer->height;
					}
				break;

			case RPTREPORT_SECTION_BODY:
				ret = priv->body->height;
				break;
		}

	return ret;
}

/**
 * rpt_report_set_section_height: 
 * @rpt_report: an #RptReport object.
 * @section:
 * @height: the section's height.
 *
 * Sets @section's height.
 */
void
rpt_report_set_section_height (RptReport *rpt_report,
                               RptReportSection section,
                               gdouble height)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	rpt_report_section_create (rpt_report, section);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				priv->report_header->height = height;
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				priv->report_footer->height = height;
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				priv->page_header->height = height;
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				priv->page_footer->height = height;
				break;

			case RPTREPORT_SECTION_BODY:
				priv->body->height = height;
				break;
		}
}

/**
 * rpt_report_section_get_objects:
 * @rpt_report:
 * @section:
 *
 */
GList
*rpt_report_section_get_objects (RptReport *rpt_report,
                                 RptReportSection section)
{
	GList *objects = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				if (priv->report_header != NULL)
					{
						objects = g_list_copy (priv->report_header->objects);
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				if (priv->report_footer != NULL)
					{
						objects = g_list_copy (priv->report_footer->objects);
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				if (priv->page_header != NULL)
					{
						objects = g_list_copy (priv->page_header->objects);
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				if (priv->page_footer != NULL)
					{
						objects = g_list_copy (priv->page_footer->objects);
					}
				break;

			case RPTREPORT_SECTION_BODY:
				objects = g_list_copy (priv->body->objects);
				break;
		}

	return objects;
}

/**
 * rpt_report_section_remove:
 * @rpt_report:
 * @section:
 *
 */
void
rpt_report_section_remove (RptReport *rpt_report, RptReportSection section)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				if (priv->report_header != NULL)
					{
						g_free (priv->report_header);
						priv->report_header = NULL;
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				if (priv->report_footer != NULL)
					{
						g_free (priv->report_footer);
						priv->report_footer = NULL;
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				if (priv->page_header != NULL)
					{
						g_free (priv->page_header);
						priv->page_header = NULL;
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				if (priv->page_footer != NULL)
					{
						g_free (priv->page_footer);
						priv->page_footer = NULL;
					}
				break;

			case RPTREPORT_SECTION_BODY:
				/* TO DO */
				g_warning ("Body cannot be removed.");
				break;
		}
}

/**
 * rpt_report_get_report_header_new_page_after:
 * @rpt_report:
 *
 */
gboolean
rpt_report_get_report_header_new_page_after (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return (priv->report_header == NULL ? FALSE : priv->report_header->new_page_after);
}

/**
 * rpt_report_set_report_header_new_page_after:
 * @rpt_report: an #RptReport object.
 * @new_page_after:
 *
 */
void
rpt_report_set_report_header_new_page_after (RptReport *rpt_report, gboolean new_page_after)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->report_header != NULL)
		{
			priv->report_header->new_page_after = new_page_after;
		}
}

/**
 * rpt_report_get_report_footer_new_page_before:
 * @rpt_report:
 *
 */
gboolean
rpt_report_get_report_footer_new_page_before (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return (priv->report_footer == NULL ? FALSE : priv->report_footer->new_page_before);
}

/**
 * rpt_report_set_report_footer_new_page_before:
 * @rpt_report: an #RptReport object.
 * @new_page_before:
 *
 */
void
rpt_report_set_report_footer_new_page_before (RptReport *rpt_report, gboolean new_page_before)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->report_footer != NULL)
		{
			priv->report_footer->new_page_before = new_page_before;
		}
}

/**
 * rpt_report_get_page_header_first_page:
 * @rpt_report: an #RptReport object.
 *
 */
gboolean
rpt_report_get_page_header_first_page (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return (priv->page_header == NULL ? FALSE : priv->page_header->first_page);
}

/**
 * rpt_report_get_page_header_last_page:
 * @rpt_report: an #RptReport object.
 *
 */
gboolean
rpt_report_get_page_header_last_page (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return (priv->page_header == NULL ? FALSE : priv->page_header->last_page);
}

/**
 * rpt_report_set_page_header_first_last_page:
 * @rpt_report: an #RptReport object.
 * @first_page:
 * @last_page:
 *
 */
void
rpt_report_set_page_header_first_last_page (RptReport *rpt_report, gboolean first_page, gboolean last_page)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->page_header != NULL)
		{
			priv->page_header->first_page = first_page;
			priv->page_header->last_page = last_page;
		}
}

/**
 * rpt_report_get_page_footer_first_page:
 * @rpt_report: an #RptReport object.
 *
 */
gboolean
rpt_report_get_page_footer_first_page (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return (priv->page_footer ? FALSE : priv->page_footer->first_page);
}

/**
 * rpt_report_get_page_footer_last_page:
 * @rpt_report: an #RptReport object.
 *
 */
gboolean
rpt_report_get_page_footer_last_page (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return (priv->page_footer == NULL ? FALSE : priv->page_footer->last_page);
}

/**
 * rpt_report_set_page_footer_first_last_page:
 * @rpt_report: an #RptReport object.
 * @first_page:
 * @last_page:
 *
 */
void
rpt_report_set_page_footer_first_last_page (RptReport *rpt_report, gboolean first_page, gboolean last_page)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->page_footer != NULL)
		{
			priv->page_footer->first_page = first_page;
			priv->page_footer->last_page = last_page;
		}
}

/**
 * rpt_report_body_get_new_page_after:
 * @rpt_report:
 *
 */
gboolean
rpt_report_body_get_new_page_after (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	return priv->body->new_page_after;
}

/**
 * rpt_report_body_set_new_page_after:
 * @rpt_report:
 * @new_page_after:
 *
 */
void
rpt_report_body_set_new_page_after (RptReport *rpt_report, gboolean new_page_after)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->body->new_page_after = new_page_after;
}

/**
 * rpt_report_get_xml:
 * @rpt_report: an #RptReport object.
 *
 */
xmlDoc
*rpt_report_get_xml (RptReport *rpt_report)
{
	xmlDoc *xdoc = NULL;
	xmlNode *xroot;
	xmlNode *xreport;
	xmlNode *xnode;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	xdoc = xmlNewDoc ("1.0");

	xroot = xmlNewNode (NULL, "reptool");
	xmlDocSetRootElement (xdoc, xroot);

	xmlNode *xnodeprop = xmlNewNode (NULL, "properties");
	xmlAddChild (xroot, xnodeprop);

	xnode = xmlNewNode (NULL, "unit-length");
	xmlNodeSetContent (xnode, rpt_common_enum_to_strunit (priv->unit));
	xmlAddChild (xnodeprop, xnode);

	xnode = xmlNewNode (NULL, "output-type");
	xmlNodeSetContent (xnode, rpt_common_enum_to_stroutputtype (priv->output_type));
	xmlAddChild (xnodeprop, xnode);

	xnode = xmlNewNode (NULL, "output-filename");
	xmlNodeSetContent (xnode, priv->output_filename);
	xmlAddChild (xnodeprop, xnode);

	xnode = xmlNewNode (NULL, "copies");
	xmlNodeSetContent (xnode, g_strdup_printf ("%d", priv->copies));
	xmlAddChild (xnodeprop, xnode);

	if (priv->db != NULL)
		{
			xmlNode *xnodedb = xmlNewNode (NULL, "database");
			xmlAddChild (xroot, xnodedb);

			xnode = xmlNewNode (NULL, "provider");
			xmlNodeSetContent (xnode, priv->db->provider_id);
			xmlAddChild (xnodedb, xnode);

			xnode = xmlNewNode (NULL, "connection-string");
			xmlNodeSetContent (xnode, priv->db->connection_string);
			xmlAddChild (xnodedb, xnode);

			xnode = xmlNewNode (NULL, "sql");
			xmlNodeSetContent (xnode, priv->db->sql);
			xmlAddChild (xnodedb, xnode);
		};

	xnode = xmlNewNode (NULL, "page");
	rpt_common_set_size (xnode, priv->page->size);
	if (priv->page->margin->top != 0.0)
		{
			xmlSetProp (xnode, "margin-top", g_strdup_printf ("%f", priv->page->margin->top));
		}
	if (priv->page->margin->right != 0.0)
		{
			xmlSetProp (xnode, "margin-right", g_strdup_printf ("%f", priv->page->margin->right));
		}
	if (priv->page->margin->bottom != 0.0)
		{
			xmlSetProp (xnode, "margin-bottom", g_strdup_printf ("%f", priv->page->margin->bottom));
		}
	if (priv->page->margin->left != 0.0)
		{
			xmlSetProp (xnode, "margin-left", g_strdup_printf ("%f", priv->page->margin->left));
		}
	xmlAddChild (xroot, xnode);

	xreport = xmlNewNode (NULL, "report");
	xmlAddChild (xroot, xreport);

	if (priv->report_header != NULL)
		{
			xnode = rpt_report_section_get_xml (rpt_report, RPTREPORT_SECTION_REPORT_HEADER);
			xmlAddChild (xreport, xnode);
		}
	if (priv->page_header != NULL)
		{
			xnode = rpt_report_section_get_xml (rpt_report, RPTREPORT_SECTION_PAGE_HEADER);
			xmlAddChild (xreport, xnode);
		}

	xnode = rpt_report_section_get_xml (rpt_report, RPTREPORT_SECTION_BODY);
	xmlAddChild (xreport, xnode);

	if (priv->report_footer != NULL)
		{
			xnode = rpt_report_section_get_xml (rpt_report, RPTREPORT_SECTION_REPORT_FOOTER);
			xmlAddChild (xreport, xnode);
		}
	if (priv->page_footer != NULL)
		{
			xnode = rpt_report_section_get_xml (rpt_report, RPTREPORT_SECTION_PAGE_FOOTER);
			xmlAddChild (xreport, xnode);
		}

	return xdoc;
}

/**
 * rpt_report_get_xml_rptprint:
 * @rpt_report: an #RptReport object.
 *
 * Returns: an #xmlDoc, that represents the generated report, to pass to 
 * function rpt_print_new_from_xml().
 */
xmlDoc
*rpt_report_get_xml_rptprint (RptReport *rpt_report)
{
	GError *error;

	xmlDoc *xdoc;
	xmlNode *xroot;
	xmlNode *xpage;

	gdouble cur_y = 0.0;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	xdoc = rpt_report_rptprint_new ();
	xroot = xmlDocGetRootElement (xdoc);

	priv->cur_page = 0;

	/* properties */
	rpt_report_rptprint_set_unit_length (xdoc, priv->unit);
	rpt_report_rptprint_set_output_type (xdoc, priv->output_type);
	rpt_report_rptprint_set_output_filename (xdoc, priv->output_filename);
	rpt_report_rptprint_set_copies (xdoc, priv->copies);

	if (priv->db != NULL)
		{
			gint row;

			if (priv->db->treemodel != NULL)
				{
					GtkTreeIter *iter_prec;
					GtkTreeIter iter;

					if (!gtk_tree_model_get_iter_first (priv->db->treemodel, &iter))
						{
							g_warning ("GtkTreeModel is empty.");
							return NULL;
						}

					row = 0;
					do
						{
							priv->cur_iter = &iter;
							if (row == 0 ||
							    priv->body->new_page_after ||
							    (priv->page_footer != NULL && (cur_y + priv->body->height > priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height)) ||
							    cur_y > (priv->page->size->height - priv->page->margin->bottom))
								{
									if (priv->cur_page > 0 && priv->page_footer != NULL)
										{
											if ((priv->cur_page == 1 && priv->page_footer->first_page) ||
											    priv->cur_page > 1)
												{
													cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
													priv->cur_iter = iter_prec;
													rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
													priv->cur_iter = &iter;
												}
										}

									cur_y = priv->page->margin->top;
									xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);

									if (priv->page_header != NULL)
										{
											if ((priv->cur_page == 1 && priv->page_header->first_page) ||
											    priv->cur_page > 1)
												{
													rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER);
												}
										}
									if (priv->cur_page == 1 && priv->report_header != NULL)
										{
											rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_REPORT_HEADER);
											if (priv->report_header->new_page_after)
												{
													cur_y = 0.0;
													xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);
												}
										}
								}

							rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_BODY);

							iter_prec = g_memdup (&iter, sizeof (GtkTreeIter));
							row++;
						} while (gtk_tree_model_iter_next (priv->db->treemodel, &iter));

					priv->cur_iter = iter_prec;
					if (priv->cur_page > 0 && priv->report_footer != NULL)
						{
							if ((cur_y + priv->report_footer->height > priv->page->size->height - priv->page->margin->bottom - (priv->page_footer != NULL ? priv->page_footer->height : 0.0)) ||
							    priv->report_footer->new_page_before)
								{
									if (priv->page_header != NULL)
										{
											rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER);
										}

									cur_y = priv->page->margin->top;
									xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);

									if (priv->cur_page > 0 && priv->page_footer != NULL)
										{
											cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
											rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
										}
								}

							rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_REPORT_FOOTER);
						}

					if (priv->cur_page > 0 && priv->page_footer != NULL && priv->page_footer->last_page)
						{
							cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
							rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
						}
				}
			else
				{
					gint rows;

					/* database connection */
					gda_init ();
					if (priv->db->gda_datamodel == NULL)
						{
							error = NULL;
							priv->db->gda_conn = gda_connection_open_from_string (priv->db->provider_id,
							                                                      priv->db->connection_string,
							                                                      NULL,
							                                                      GDA_CONNECTION_OPTIONS_NONE,
							                                                      &error);
							if (priv->db->gda_conn == NULL || error != NULL)
								{
									/* TO DO */
									g_warning ("Unable to establish the connection: %s.",
									           error != NULL && error->message != NULL ? error->message : "no details");
									return NULL;
								}
							else
								{
									GdaSqlParser *parser = gda_sql_parser_new ();
									error = NULL;
									GdaStatement *stmt = gda_sql_parser_parse_string (parser, priv->db->sql, NULL, &error);

									error = NULL;
									priv->db->gda_datamodel = gda_connection_statement_execute_select (priv->db->gda_conn, stmt, NULL, &error);
									if (priv->db->gda_datamodel == NULL || error != NULL)
										{
											/* TO DO */
											g_warning ("Unable to create the datamodel: %s",
											           error != NULL && error->message != NULL ? error->message : "no details");
											return NULL;
										}
								}
						}

					rows = gda_data_model_get_n_rows (priv->db->gda_datamodel);
					for (row = 0; row < rows; row++)
						{
							priv->cur_row = row;
							if (row == 0 ||
							    priv->body->new_page_after ||
							    (priv->page_footer != NULL && (cur_y + priv->body->height > priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height)) ||
							    cur_y > (priv->page->size->height - priv->page->margin->bottom))
								{
									if (priv->cur_page > 0 && priv->page_footer != NULL)
										{
											if ((priv->cur_page == 1 && priv->page_footer->first_page) ||
											    priv->cur_page > 1)
												{
													cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
													priv->cur_row = row - 1;
													rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
													priv->cur_row = row;
												}
										}

									cur_y = priv->page->margin->top;
									xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);

									if (priv->page_header != NULL)
										{
											if ((priv->cur_page == 1 && priv->page_header->first_page) ||
											    priv->cur_page > 1)
												{
													rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER);
												}
										}
									if (priv->cur_page == 1 && priv->report_header != NULL)
										{
											rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_REPORT_HEADER);
											if (priv->report_header->new_page_after)
												{
													cur_y = 0.0;
													xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);
												}
										}
								}

							rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_BODY);
						}

					if (priv->cur_page > 0 && priv->report_footer != NULL)
						{
							if ((cur_y + priv->report_footer->height > priv->page->size->height - priv->page->margin->bottom - (priv->page_footer != NULL ? priv->page_footer->height : 0.0)) ||
							    priv->report_footer->new_page_before)
								{
									if (priv->page_header != NULL)
										{
											priv->cur_row = row - 1;
											rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER);
											priv->cur_row = row;
										}

									cur_y = priv->page->margin->top;
									xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);

									if (priv->cur_page > 0 && priv->page_footer != NULL)
										{
											cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
											priv->cur_row = row - 1;
											rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
											priv->cur_row = row;
										}
								}

							priv->cur_row = row - 1;
							rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_REPORT_FOOTER);
							priv->cur_row = row;
						}

					if (priv->cur_page > 0 && priv->page_footer != NULL && priv->page_footer->last_page)
						{
							cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
							priv->cur_row = row - 1;
							rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
							priv->cur_row = row;
						}
				}

			/* change @Pages */
			rpt_report_change_specials (rpt_report, xdoc);

			priv->cur_row = -1;
			priv->cur_iter = NULL;
		}
	else
		{
			cur_y = priv->page->margin->top;
			xpage = rpt_report_rptprint_new_page (rpt_report, xdoc);

			if (priv->page_header != NULL)
				{
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER);
				}
			if (priv->report_header != NULL)
				{
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_REPORT_HEADER);
				}

			rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_BODY);

			if (priv->report_footer != NULL)
				{
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_REPORT_FOOTER);
				}
			if (priv->page_footer != NULL)
				{
					cur_y = priv->page->size->height - priv->page->margin->bottom - priv->page_footer->height;
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER);
				}
		}

	return xdoc;
}

xmlDoc
*rpt_report_rptprint_new (void)
{
	xmlDoc *xdoc;
	xmlNode *xroot;

	xdoc = xmlNewDoc ("1.0");
	xroot = xmlNewNode (NULL, "reptool_report");
	xmlDocSetRootElement (xdoc, xroot);

	return xdoc;
}

void
rpt_report_rptprint_set_unit_length (xmlDoc *xdoc, eRptUnitLength unit)
{
	xmlNode *xnodeprop;
	xmlNode *xnode;
	xmlNode *xroot;

	xnodeprop = rpt_report_rptprint_get_properties_node (xdoc);
	if (xnodeprop == NULL)
		{
			xroot = xmlDocGetRootElement (xdoc);
			xnodeprop = xmlNewNode (NULL, "properties");
			xmlAddChild (xroot, xnodeprop);
		}

	/* TODO
	 * replace eventually already present node */
	xnode = xmlNewNode (NULL, "unit-length");
	xmlNodeSetContent (xnode, rpt_common_enum_to_strunit (unit));
	xmlAddChild (xnodeprop, xnode);
}

void
rpt_report_rptprint_set_output_type (xmlDoc *xdoc, eRptOutputType output_type)
{
	xmlNode *xnodeprop;
	xmlNode *xnode;
	xmlNode *xroot;

	xnodeprop = rpt_report_rptprint_get_properties_node (xdoc);
	if (xnodeprop == NULL)
		{
			xroot = xmlDocGetRootElement (xdoc);
			xnodeprop = xmlNewNode (NULL, "properties");
			xmlAddChild (xroot, xnodeprop);
		}

	/* TODO
	 * replace eventually already present node */
	xnode = xmlNewNode (NULL, "output-type");
	xmlNodeSetContent (xnode, rpt_common_enum_to_stroutputtype (output_type));
	xmlAddChild (xnodeprop, xnode);
}

void
rpt_report_rptprint_set_output_filename (xmlDoc *xdoc, const gchar *output_filename)
{
	xmlNode *xnodeprop;
	xmlNode *xnode;
	xmlNode *xroot;

	xnodeprop = rpt_report_rptprint_get_properties_node (xdoc);
	if (xnodeprop == NULL)
		{
			xroot = xmlDocGetRootElement (xdoc);
			xnodeprop = xmlNewNode (NULL, "properties");
			xmlAddChild (xroot, xnodeprop);
		}

	/* TODO
	 * replace eventually already present node */
	xnode = xmlNewNode (NULL, "output-filename");
	xmlNodeSetContent (xnode, output_filename);
	xmlAddChild (xnodeprop, xnode);
}

void
rpt_report_rptprint_set_copies (xmlDoc *xdoc, guint copies)
{
	xmlNode *xnodeprop;
	xmlNode *xnode;
	xmlNode *xroot;

	xnodeprop = rpt_report_rptprint_get_properties_node (xdoc);
	if (xnodeprop == NULL)
		{
			xroot = xmlDocGetRootElement (xdoc);
			xnodeprop = xmlNewNode (NULL, "properties");
			xmlAddChild (xroot, xnodeprop);
		}

	/* TODO
	 * replace eventually already present node */
	xnode = xmlNewNode (NULL, "copies");
	xmlNodeSetContent (xnode, g_strdup_printf ("%d", copies));
	xmlAddChild (xnodeprop, xnode);
}

xmlNode
*rpt_report_rptprint_page_new (xmlDoc *xdoc, RptSize *size, RptMargin *margin)
{
	xmlNode *xroot;
	xmlNode *xnode;

	xroot = xmlDocGetRootElement (xdoc);

	xnode = xmlNewNode (NULL, "page");
	xmlAddChild (xroot, xnode);

	rpt_common_set_size (xnode, size);
	rpt_common_set_margin (xnode, margin);

	return xnode;
}

void
rpt_report_rptprint_page_add_object (xmlNode *xnodepage, RptObject *rpt_object)
{
	xmlNode *xnodeobj;
	xmlAttrPtr attr;

	g_return_if_fail (IS_RPT_OBJECT (rpt_object));

	xnodeobj = xmlNewNode (NULL, "node");

	rpt_object_get_xml (rpt_object, xnodeobj);
	attr = xmlHasProp (xnodeobj, "name");
	if (attr != NULL)
		{
			xmlRemoveProp (attr);
		}

	if (IS_RPT_OBJ_TEXT (rpt_object))
		{
			rpt_report_rptprint_parse_text_source (NULL, rpt_object, xnodeobj);
			attr = xmlHasProp (xnodeobj, "source");
			if (attr != NULL)
				{
					xmlRemoveProp (attr);
				}
		}
	else if (IS_RPT_OBJ_IMAGE (rpt_object))
		{
			/* TO DO */
			/* rpt_report_rptprint_parse_image_source (rpt_report, rptobj, xnode); */
		}

	xmlAddChild (xnodepage, xnodeobj);
}

/**
 * rpt_report_add_object_to_section:
 * @rpt_report: an #RptReport object.
 * @rpt_object: an #RptObject object.
 * @section:
 *
 * Returns: FALSE if already exists an #RptObject with the same @rpt_object's
 * name; TRUE otherwise.
 */
gboolean
rpt_report_add_object_to_section (RptReport *rpt_report, RptObject *rpt_object, RptReportSection section)
{
	gboolean ret = FALSE;
	gchar *objname;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	g_object_get (rpt_object, "name", &objname, NULL);
	if (rpt_report_get_object_from_name (rpt_report, objname) == NULL)
		{
			switch (section)
				{
					case RPTREPORT_SECTION_REPORT_HEADER:
						priv->report_header->objects = g_list_append (priv->report_header->objects, rpt_object);;
						break;

					case RPTREPORT_SECTION_REPORT_FOOTER:
						priv->report_footer->objects = g_list_append (priv->report_footer->objects, rpt_object);;
						break;

					case RPTREPORT_SECTION_PAGE_HEADER:
						priv->page_header->objects = g_list_append (priv->page_header->objects, rpt_object);;
						break;

					case RPTREPORT_SECTION_PAGE_FOOTER:
						priv->page_footer->objects = g_list_append (priv->page_footer->objects, rpt_object);;
						break;

					case RPTREPORT_SECTION_BODY:
						priv->body->objects = g_list_append (priv->body->objects, rpt_object);;
						break;
				}

			ret = TRUE;
		}
	else
		{
			/* TO DO */
			g_warning ("An object with name «%s» already exists.", objname);
		}

	return ret;
}

/**
 * rpt_report_remove_object:
 * @rpt_report: an #RptReport object.
 * @rpt_object: an #RptObject object.
 *
 * Removes the @rpt_object from the @rpt_report.
 */
void
rpt_report_remove_object (RptReport *rpt_report, RptObject *rpt_object)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	RptReportSection section = rpt_report_object_get_section (rpt_report, rpt_object);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				priv->report_header->objects = g_list_remove (priv->report_header->objects, rpt_object);;
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				priv->report_footer->objects = g_list_remove (priv->report_footer->objects, rpt_object);;
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				priv->page_header->objects = g_list_remove (priv->page_header->objects, rpt_object);;
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				priv->page_footer->objects = g_list_remove (priv->page_footer->objects, rpt_object);;
				break;

			case RPTREPORT_SECTION_BODY:
				priv->body->objects = g_list_remove (priv->body->objects, rpt_object);;
				break;
		}
}

/**
 * rpt_report_get_object_from_name:
 * @rpt_report: an #RptReport object.
 * @name: the #RptObject's name.
 *
 * Returns: the #RptObject object represented by the name @name.
 */
RptObject
*rpt_report_get_object_from_name (RptReport *rpt_report, const gchar *name)
{
	RptObject *obj = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->report_header != NULL && (obj = rpt_report_get_object_from_name_in_list (priv->report_header->objects, name)) != NULL)
		{
		}
	else if (priv->report_footer != NULL && (obj = rpt_report_get_object_from_name_in_list (priv->report_footer->objects, name)) != NULL)
		{
		}
	else if (priv->page_header != NULL && (obj = rpt_report_get_object_from_name_in_list (priv->page_header->objects, name)) != NULL)
		{
		}
	else if (priv->page_footer != NULL && (obj = rpt_report_get_object_from_name_in_list (priv->page_footer->objects, name)) != NULL)
		{
		}
	else if (priv->body != NULL && (obj = rpt_report_get_object_from_name_in_list (priv->body->objects, name)) != NULL)
		{
		}

	return obj;
}

static void
rpt_report_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptReport *rpt_report = RPT_REPORT (object);

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (property_id)
		{
			case PROP_UNIT_LENGTH:
				priv->unit = g_value_get_int (value);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_report_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptReport *rpt_report = RPT_REPORT (object);

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (property_id)
		{
			case PROP_UNIT_LENGTH:
				g_value_set_int (value, priv->unit);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

/**
 * rpt_report_get_object_from_name_in_list:
 * @list:
 * @name:
 *
 */
static RptObject
*rpt_report_get_object_from_name_in_list (GList *list, const gchar *name)
{
	gchar *objname;
	RptObject *obj = NULL;

	if (list != NULL)
		{
			list = g_list_first (list);
			while (list != NULL)
				{
					g_object_get ((RptObject *)list->data, "name", &objname, NULL);
					if (g_strcmp0 (name, objname) == 0)
						{
							obj = (RptObject *)list->data;
							break;
						}
				
					list = g_list_next (list);
				}
		}

	return obj;
}

/**
 * rpt_report_object_get_section:
 * @rpt_report: an #RptReport object.
 * @rpt_object: an #RptObject object.
 *
 * Returns: the #RptReportSection in which @rpt_object is contained.
 */
static RptReportSection
rpt_report_object_get_section (RptReport *rpt_report, RptObject *rpt_object)
{
	RptReportSection section = -1;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (rpt_report_object_is_in_section (rpt_report, rpt_object, RPTREPORT_SECTION_REPORT_HEADER))
		{
			section = RPTREPORT_SECTION_REPORT_HEADER;
		}
	else if (rpt_report_object_is_in_section (rpt_report, rpt_object, RPTREPORT_SECTION_REPORT_FOOTER))
		{
			section = RPTREPORT_SECTION_REPORT_FOOTER;
		}
	else if (rpt_report_object_is_in_section (rpt_report, rpt_object, RPTREPORT_SECTION_PAGE_HEADER))
		{
			section = RPTREPORT_SECTION_PAGE_HEADER;
		}
	else if (rpt_report_object_is_in_section (rpt_report, rpt_object, RPTREPORT_SECTION_PAGE_FOOTER))
		{
			section = RPTREPORT_SECTION_PAGE_FOOTER;
		}
	else if (rpt_report_object_is_in_section (rpt_report, rpt_object, RPTREPORT_SECTION_BODY))
		{
			section = RPTREPORT_SECTION_BODY;
		}

	return section;
}

/**
 * rpt_report_section_create:
 * @rpt_report:
 * @section:
 *
 */
static void
rpt_report_section_create (RptReport *rpt_report, RptReportSection section)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				if (priv->report_header == NULL)
					{
						priv->report_header = (ReportHeader *)g_malloc0 (sizeof (ReportHeader));
						priv->report_header->objects = NULL;
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				if (priv->report_footer == NULL)
					{
						priv->report_footer = (ReportFooter *)g_malloc0 (sizeof (ReportFooter));
						priv->report_footer->objects = NULL;
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				if (priv->page_header == NULL)
					{
						priv->page_header = (PageHeader *)g_malloc0 (sizeof (PageHeader));
						priv->page_header->objects = NULL;
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				if (priv->page_footer == NULL)
					{
						priv->page_footer = (PageFooter *)g_malloc0 (sizeof (PageFooter));
						priv->page_footer->objects = NULL;
					}
				break;

			case RPTREPORT_SECTION_BODY:
				/*g_warning ("Body cannot be created.");*/
				break;
		}
}

/** 
 * rpt_report_object_is_in_section:
 * @rpt_report: an #RptReport object.
 * @rpt_object: an #RptObject object.
 * @section:
 *
 * Returns: TRUE if rpt_object is contained into 
 */
static gboolean
rpt_report_object_is_in_section (RptReport *rpt_report, RptObject *rpt_object, RptReportSection section)
{
	gboolean ret = FALSE;
	GList *list = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				if (priv->report_header != NULL)
					{
						list = priv->report_header->objects;
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				if (priv->report_footer != NULL)
					{
						list = priv->report_footer->objects;
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				if (priv->page_header != NULL)
					{
						list = priv->page_header->objects;
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				if (priv->page_footer != NULL)
					{
						list = priv->page_footer->objects;
					}
				break;

			case RPTREPORT_SECTION_BODY:
				list = priv->body->objects;
				break;

			default:
				return FALSE;
		}

	if (list != NULL)
		{
			list = g_list_first (list);
			while (list != NULL)
				{
					if ((RptObject *)list->data == rpt_object)
						{
							ret = TRUE;
							break;
						}

					list = g_list_next (list);
				}
		}

	return ret;
}

/**
 * rpt_report_section_get_xml:
 * @rpt_report: an #RptReport object.
 * @section:
 *
 */
static xmlNode
*rpt_report_section_get_xml (RptReport *rpt_report , RptReportSection section)
{
	xmlNode *xnode = NULL;
	xmlNode *xnodechild;
	gdouble height;
	GList *objects;
	RptObject *rptobj;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				xnode = xmlNewNode (NULL, "report-header");
				height = priv->report_header->height;
				objects = priv->report_header->objects;
				if (priv->report_header->new_page_after)
					{
						xmlSetProp (xnode, "new-page-after", "y");
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				xnode = xmlNewNode (NULL, "report-footer");
				height = priv->report_footer->height;
				objects = priv->report_footer->objects;
				if (priv->report_footer->new_page_before)
					{
						xmlSetProp (xnode, "new-page-before", "y");
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				xnode = xmlNewNode (NULL, "page-header");
				height = priv->page_header->height;
				objects = priv->page_header->objects;
				if (priv->page_header->first_page)
					{
						xmlSetProp (xnode, "first-page", "y");
					}
				if (priv->page_header->last_page)
					{
						xmlSetProp (xnode, "last-page", "y");
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				xnode = xmlNewNode (NULL, "page-footer");
				height = priv->page_footer->height;
				objects = priv->page_footer->objects;
				if (priv->page_footer->first_page)
					{
						xmlSetProp (xnode, "first-page", "y");
					}
				if (priv->page_footer->last_page)
					{
						xmlSetProp (xnode, "last-page", "y");
					}
				break;

			case RPTREPORT_SECTION_BODY:
				xnode = xmlNewNode (NULL, "body");
				height = priv->body->height;
				objects = priv->body->objects;
				if (priv->body->new_page_after)
					{
						xmlSetProp (xnode, "new-page-after", "y");
					}
				break;
		}
	xmlSetProp (xnode, "height", g_strdup_printf ("%f", height));

	objects = g_list_first (objects);
	while (objects != NULL)
		{
			rptobj = (RptObject *)objects->data;

			xnodechild = xmlNewNode (NULL, "object");
			rpt_object_get_xml (rptobj, xnodechild);
			xmlAddChild (xnode, xnodechild);

			objects = g_list_next (objects);
		}

	return xnode;
}

static void
rpt_report_xml_parse_section (RptReport *rpt_report, xmlNode *xnode, RptReportSection section)
{
	RptObject *rptobj;
	GList *objects = NULL;
	gdouble height;
	gchar *prop;
	gchar *objname;
	xmlNode *cur;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	prop = (gchar *)xmlGetProp (xnode, "height");
	if (prop != NULL)
		{
			height = g_strtod (g_strstrip (g_strdup (prop)), NULL);
		}

	cur = xnode->children;
	while (cur != NULL)
		{
			if (g_strcmp0 (cur->name, "text") == 0)
				{
					rptobj = rpt_obj_text_new_from_xml (cur);
				}
			else if (g_strcmp0 (cur->name, "line") == 0)
				{
					rptobj = rpt_obj_line_new_from_xml (cur);
				}
			else if (g_strcmp0 (cur->name, "rect") == 0)
				{
					rptobj = rpt_obj_rect_new_from_xml (cur);
				}
			else if (g_strcmp0 (cur->name, "ellipse") == 0)
				{
					rptobj = rpt_obj_ellipse_new_from_xml (cur);
				}
			else if (g_strcmp0 (cur->name, "image") == 0)
				{
					rptobj = rpt_obj_image_new_from_xml (cur);
				}

			if (rptobj != NULL)
				{
					rpt_report_add_object_to_section (rpt_report, rptobj, section);
				}

			cur = cur->next;
		}

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				priv->report_header->height = height;

				prop = xmlGetProp (xnode, "new-page-after");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->report_header->new_page_after = TRUE;
							}
					}
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				priv->report_footer->height = height;

				prop = xmlGetProp (xnode, "new-page-before");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->report_footer->new_page_before = TRUE;
							}
					}
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				priv->page_header->height = height;

				prop = xmlGetProp (xnode, "first-page");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->page_header->first_page = TRUE;
							}
					}
				prop = xmlGetProp (xnode, "last-page");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->page_header->last_page = TRUE;
							}
					}
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				priv->page_footer->height = height;

				prop = xmlGetProp (xnode, "first-page");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->page_footer->first_page = TRUE;
							}
					}
				prop = xmlGetProp (xnode, "last-page");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->page_footer->last_page = TRUE;
							}
					}
				break;

			case RPTREPORT_SECTION_BODY:
				priv->body->height = height;
			
				prop = xmlGetProp (xnode, "new-page-after");
				if (prop != NULL)
					{
						if (strcasecmp (g_strstrip (prop), "y") == 0)
							{
								priv->body->new_page_after = TRUE;
							}
					}
				break;
		}
}

static xmlNode
*rpt_report_rptprint_get_properties_node (xmlDoc *xdoc)
{
	xmlNode *xnode;

	xmlXPathContextPtr xpcontext;
	xmlXPathObjectPtr xpresult;
	xmlNodeSetPtr xnodeset;

	xnode = NULL;

	xpcontext = xmlXPathNewContext (xdoc);

	/* search for node "properties" */
	xpcontext->node = xmlDocGetRootElement (xdoc);
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::properties", xpcontext);
	if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			if (xnodeset->nodeNr > 0)
				{
					xnode = xnodeset->nodeTab[0];
				}
		}

	return xnode;
}

static xmlNode
*rpt_report_rptprint_new_page (RptReport *rpt_report, xmlDoc *xdoc)
{
	xmlNode *xnode;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	xnode = rpt_report_rptprint_page_new (xdoc, priv->page->size, priv->page->margin);

	priv->cur_page++;

	return xnode;
}

static void
rpt_report_rptprint_section (RptReport *rpt_report,
                             xmlNode *xpage,
                             gdouble *cur_y,
                             RptReportSection section)
{
	GList *objects;
	xmlAttrPtr attr;
	xmlNode *xnode;
	gchar *prop;

	RptObject *rptobj;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				objects = g_list_first (priv->report_header->objects);
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				objects = g_list_first (priv->report_footer->objects);
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				objects = g_list_first (priv->page_header->objects);
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				objects = g_list_first (priv->page_footer->objects);
				break;

			case RPTREPORT_SECTION_BODY:
				objects = g_list_first (priv->body->objects);
				break;
		}

	while (objects != NULL)
		{
			xnode = xmlNewNode (NULL, "node");

			rptobj = (RptObject *)objects->data;
			rpt_object_get_xml (rptobj, xnode);
			attr = xmlHasProp (xnode, "name");
			if (attr != NULL)
				{
					xmlRemoveProp (attr);
				}

			if (priv->page->margin->left != 0.0)
				{
					prop = (gchar *)xmlGetProp (xnode, "x");
					if (prop == NULL)
						{
							prop = g_strdup ("0.0");
						}
					xmlSetProp (xnode, "x", g_strdup_printf ("%f", g_strtod (prop, NULL) + priv->page->margin->left));
				}
			
			prop = (gchar *)xmlGetProp (xnode, "y");
			if (prop == NULL)
				{
					prop = g_strdup ("0.0");
				}
			xmlSetProp (xnode, "y", g_strdup_printf ("%f", g_strtod (prop, NULL) + *cur_y));

			if (IS_RPT_OBJ_TEXT (rptobj))
				{
					rpt_report_rptprint_parse_text_source (rpt_report, rptobj, xnode);
					attr = xmlHasProp (xnode, "source");
					if (attr != NULL)
						{
							xmlRemoveProp (attr);
						}
				}
			else if (IS_RPT_OBJ_IMAGE (rptobj))
				{
					/* TO DO */
					/* rpt_report_rptprint_parse_image_source (rpt_report, rptobj, xnode); */
				}

			xmlAddChild (xpage, xnode);

			objects = g_list_next (objects);
		}

	switch (section)
		{
			case RPTREPORT_SECTION_REPORT_HEADER:
				*cur_y += priv->report_header->height;
				break;

			case RPTREPORT_SECTION_REPORT_FOOTER:
				*cur_y += priv->report_footer->height;
				break;

			case RPTREPORT_SECTION_PAGE_HEADER:
				*cur_y += priv->page_header->height;
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				*cur_y += priv->page_footer->height;
				break;

			case RPTREPORT_SECTION_BODY:
				*cur_y += priv->body->height;
				break;
		}
}

static void
rpt_report_rptprint_parse_text_source (RptReport *rpt_report,
                                       RptObject *rptobj,
                                       xmlNode *xnode)
{
	gchar *source;
	gchar *ret;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	g_object_get (G_OBJECT (rptobj), "source", &source, NULL);

	yy_scan_string (source);
	yyparse (rpt_report, &ret);

	if (g_strstr_len (ret, -1, "&#10;") != NULL)
		{
			ret = g_strjoinv ("\n", g_strsplit (ret, "&#10;", -1));
		}

	if (ret == NULL)
		{
			xmlNodeSetContent (xnode, "");
		}
	else
		{
			xmlNodeSetContent (xnode, ret);
		}
}

static void
rpt_report_change_specials (RptReport *rpt_report, xmlDoc *xdoc)
{
	xmlXPathContextPtr xpcontext;
	xmlXPathObjectPtr xpresult;
	xmlNodeSetPtr xnodeset;
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	xpcontext = xmlXPathNewContext (xdoc);

	xpcontext->node = xmlDocGetRootElement (xdoc);
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"//text[contains(node(), \"@Pages\")]", xpcontext);
	if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			gint i;
			xmlNode *cur;
			gchar *ref;
			gchar *cont;

			xnodeset = xpresult->nodesetval;

			for (i = 0; i < xnodeset->nodeNr; i++)
				{
					cur = xnodeset->nodeTab[i];
					cont = g_strdup (xmlNodeGetContent (cur));
					while ((ref = strstr (cont, "@Pages")) != NULL)
						{
							cont = g_strconcat ("",
							                    g_strndup (cont, strlen (cont) - strlen (ref)),
							                    g_strdup_printf ("%d", priv->cur_page),
							                    g_strndup (ref + 6, strlen (ref - 6)),
							                    NULL);
						}
					xmlNodeSetContent (cur, cont);
				}
		}
}

gchar
*rpt_report_get_field (RptReport *rpt_report,
                       const gchar *field_name)
{
	GError *error;

	gint col;
	GValue *gval;

	gchar *ret = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if (priv->db != NULL)
		{
			if (priv->db->treemodel != NULL
			    && priv->db->columns_names != NULL)
				{
					gchar *str_col;

					str_col = (gchar *)g_hash_table_lookup (priv->db->columns_names, field_name);
					if (str_col != NULL)
						{
							str_col = g_strstrip (g_strdup (str_col));
							if (g_strcmp0 (str_col, "") != 0)
								{
									gval = g_new0 (GValue, 1);

									col = strtol (str_col, NULL, 10);
									gtk_tree_model_get_value (priv->db->treemodel, priv->cur_iter,
									                          col, gval);

									ret = (gchar *)gda_value_stringify (gval);

									g_value_unset (gval);
									g_free (str_col);
								}
						}
				}
			else if (priv->db->gda_datamodel != NULL)
				{
					col = gda_data_model_get_column_index (priv->db->gda_datamodel, field_name);

					if (col > -1)
						{
							error = NULL;
							gval = (GValue *)gda_data_model_get_value_at (priv->db->gda_datamodel, col, priv->cur_row, &error);
							if (error != NULL)
								{
									g_warning ("Error on retrieving field «%s» value: %s.",
									           field_name,
									           error->message != NULL ? error->message : "no details");
								}
							else
								{
									if (gda_value_is_null (gval))
										{
											ret = g_strdup ("");
										}
									else
										{
											ret = (gchar *)gda_value_stringify (gval);
											if (ret == NULL)
												{
													ret = g_strdup ("");
												}
										}
								}
						}
				}
		}

	if (ret == NULL)
		{
			ret = rpt_report_ask_field (rpt_report, field_name);
		}
	if (ret == NULL)
		{
			ret = g_strdup ("{ERROR}");
		}
	else
		{
			gchar **strv;

			strv = g_strsplit (ret, "&", -1);
			g_free (ret);

			ret = g_strjoinv ("&amp;", strv);
			g_strfreev (strv);
		}

	return ret;
}

gchar
*rpt_report_ask_field (RptReport *rpt_report,
                       const gchar *field)
{
	gchar *ret = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	RptReportClass *klass = RPT_REPORT_GET_CLASS (rpt_report);

	if (priv->db != NULL)
		{
			if (priv->db->gda_datamodel != NULL)
				{
					g_signal_emit (rpt_report, klass->field_request_signal_id,
					               0, field,
					               priv->db->gda_datamodel, priv->cur_row,
					               NULL, NULL,
					               &ret);
				}
			else if (priv->db->treemodel != NULL)
				{
					g_signal_emit (rpt_report, klass->field_request_signal_id,
					               0, field,
					               NULL, NULL,
					               priv->db->treemodel, priv->cur_iter,
					               &ret);
				}
		}
	else
		{
			g_signal_emit (rpt_report, klass->field_request_signal_id,
			               0, field, NULL, -1, &ret);
		}
	if (ret != NULL)
		{
			ret = g_strdup (ret);
		}
	else
		{
			ret = g_strdup ("{ERROR}");
		}

	return ret;
}

/**
 * rpt_report_replace_str:
 * @string: the string where make the replace.
 * @origin: the string to replace.
 * @replace: the string to insert.
 *
 * Returns: a string with replaced string. Must be freed.
 */
static gchar
*rpt_report_str_replace (const gchar *string,
                         const gchar *origin,
                         const gchar *replace)
{
	gchar *ret;
	gchar *p;

	p = g_strstr_len (string, -1, origin);

	if (p == NULL)
		{
			return g_strdup (string);
		}

	ret = g_strndup (string, p - string);

	ret = g_strdup_printf ("%s%s%s", ret, replace, p + strlen (origin));

	return ret;
}

/**
 * rpt_report_get_str_from_tm:
 * @datetime: a tm struct.
 * @format:
 *
 * Returns: a string representation of @datetime based on the format in @format.
 * It interprets a very little subset of format identifiers from strftime.
 * %Y: the year with 4 digits.
 * %m: the month with 2 digits.
 * %d: the day with 2 digits.
 * %H: the hours with 2 digits 00-24.
 * %M: the minutes with 2 digits 00-59.
 * %S: the seconds with 2 digits 00-59.
 */
gchar
*rpt_report_get_str_from_tm (struct tm *datetime,
                             const gchar *format)
{
	gchar *ret;

	ret = g_strdup ("");

	g_return_val_if_fail (datetime != NULL, ret);

	ret = rpt_report_str_replace (format, "%Y",
	                              g_strdup_printf ("%04u", datetime->tm_year + 1900));
	ret = rpt_report_str_replace (ret, "%m",
	                              g_strdup_printf ("%02u", datetime->tm_mon + 1));
	ret = rpt_report_str_replace (ret, "%d",
	                              g_strdup_printf ("%02u", datetime->tm_mday));
	ret = rpt_report_str_replace (ret, "%H",
	                              g_strdup_printf ("%02u", datetime->tm_hour));
	ret = rpt_report_str_replace (ret, "%M",
	                              g_strdup_printf ("%02u", datetime->tm_min));
	ret = rpt_report_str_replace (ret, "%S",
	                              g_strdup_printf ("%02u", datetime->tm_sec));

	return ret;
}

gchar
*rpt_report_get_special (RptReport *rpt_report, const gchar *special)
{
	gchar *ret;
	gchar *real_special;

	if (special == NULL) return "";

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	ret = g_strdup ("");
	real_special = g_strstrip (g_strdup (special));

	if (g_strcmp0 (real_special, "@Page") == 0)
		{
			ret = g_strdup_printf ("%d", priv->cur_page);
		}
	else if (g_strcmp0 (real_special, "@Pages") == 0)
		{
			ret = g_strdup ("@Pages");
		}
	else if (strncmp (real_special, "@Date", 5) == 0)
		{
			gchar *format;
			time_t now = time (NULL);
			struct tm *tm = localtime (&now);

			if (strlen (real_special) > 5
			    && real_special[5] == '{'
			    && real_special[strlen (real_special) - 1] == '}')
				{
					format = g_strndup (real_special + 6, strlen (real_special + 6) - 1);
				}
			else
				{
					/* TODO get from locale */
					format = g_strdup ("%Y-%m-%d");
				}

			ret = g_strdup_printf ("%s", rpt_report_get_str_from_tm (tm, format));
		}
	else if (strncmp (real_special, "@Time", 5) == 0)
		{
			gchar *format;
			time_t now = time (NULL);
			struct tm *tm = localtime (&now);

			if (strlen (real_special) > 5
			    && real_special[5] == '{'
			    && real_special[strlen (real_special) - 1] == '}')
				{
					format = g_strndup (real_special + 6, strlen (real_special + 6) - 1);
				}
			else
				{
					/* TODO get from locale */
					format = g_strdup ("%H:%M:%S");
				}

			ret = g_strdup_printf ("%s", rpt_report_get_str_from_tm (tm, format));
		}

	return ret;
}
