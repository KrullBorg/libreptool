/*
 * Copyright (C) 2007 Andrea Zagli <azagli@inwind.it>
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

#include <libgda/libgda.h>
#include <libxml/xpath.h>

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "rptreport.h"
#include "rptcommon.h"
#include "rptobject.h"
#include "rptobjecttext.h"
#include "rptobjectline.h"
#include "rptobjectrect.h"
#include "rptobjectimage.h"

typedef enum
{
	RPTREPORT_SECTION_PAGE_HEADER,
	RPTREPORT_SECTION_PAGE_FOOTER,
	RPTREPORT_SECTION_BODY
} RptReportSection;

typedef struct
{
	RptSize *size;
} Page;

typedef struct
{
	gdouble height;
	GList *objects;
} PageHeader;

typedef struct
{
	gdouble height;
	GList *objects;
} PageFooter;

typedef struct
{
	gdouble height;
	GList *objects;
} Body;

enum
{
	PROP_0
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

static RptObject *rpt_report_get_object_from_name (RptReport *rpt_report, const gchar *name);
static RptObject *rpt_report_get_object_from_name_in_list (GList *list, const gchar *name);

static xmlNode *rpt_report_rptprint_new_page (RptReport *rpt_report,
                                              xmlNode *xroot);
static void rpt_report_rptprint_section (RptReport *rpt_report,
                                         xmlNode *xpage,
                                         gdouble *cur_y,
                                         RptReportSection section,
                                         gint row);
									  
static void rpt_report_rptprint_parse_text_source (RptReport *rpt_report,
                                                   RptObject *rptobj,
                                                   xmlNode *xnode,
                                                   gint row);


#define RPT_REPORT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_REPORT, RptReportPrivate))

typedef struct _RptReportPrivate RptReportPrivate;
struct _RptReportPrivate
	{
		GdaClient *gda_client;
		GdaConnection *gda_conn;
		GdaDataModel *gda_datamodel;

		Page *page;
		PageHeader *page_header;
		PageFooter *page_footer;
		Body *body;
	};

GType
rpt_report_get_type (void)
{
	static GType rpt_report_type = 0;

	if (!rpt_report_type)
		{
			static const GTypeInfo rpt_report_info =
			{
				sizeof (RptReportClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_report_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptReport),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_report_init,
				NULL
			};

			rpt_report_type = g_type_register_static (G_TYPE_OBJECT, "RptReport",
			                                          &rpt_report_info, 0);
		}

	return rpt_report_type;
}

static void
rpt_report_class_init (RptReportClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptReportPrivate));

	object_class->set_property = rpt_report_set_property;
	object_class->get_property = rpt_report_get_property;
}

static void
rpt_report_init (RptReport *rpt_report)
{
	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	priv->gda_client = NULL;
	priv->gda_conn = NULL;
	priv->gda_datamodel = NULL;

	priv->page = (Page *)g_malloc0 (sizeof (Page));
	priv->page->size = (RptSize *)g_malloc0 (sizeof (RptSize));
	priv->page->size->width = 0.0;
	priv->page->size->height = 0.0;

	priv->page_header = (PageHeader *)g_malloc0 (sizeof (PageHeader));
	priv->page_header->height = 0.0;
	priv->page_header->objects = NULL;

	priv->page_footer = (PageFooter *)g_malloc0 (sizeof (PageFooter));
	priv->page_footer->height = 0.0;
	priv->page_footer->objects = NULL;

	priv->body = (Body *)g_malloc0 (sizeof (Body));
	priv->body->height = 0.0;
	priv->body->objects = NULL;
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
			if (strcmp (cur->name, "reptool") == 0)
				{
					xmlXPathContextPtr xpcontext;
					xmlXPathObjectPtr xpresult;
					xmlNodeSetPtr xnodeset;
					RptReportPrivate *priv;

					rpt_report = RPT_REPORT (g_object_new (rpt_report_get_type (), NULL));

					priv = RPT_REPORT_GET_PRIVATE (rpt_report);
					xpcontext = xmlXPathNewContext (xdoc);

					/* search for node "page" */
					xpcontext->node = cur;
					xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page", xpcontext);
					if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
						{
							xnodeset = xpresult->nodesetval;
							if (xnodeset->nodeNr == 1)
								{
									RptSize size;

									rpt_common_get_size (xnodeset->nodeTab[0], &size);
									priv->page->size->width = size.width;
									priv->page->size->height = size.height;
								}
							else
								{
									/* TO DO */
									/* return */
								}
						}
					else
						{
							/* TO DO */
							/* return */
						}

					/* search for node "database" */
					xpcontext->node = cur;
					xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::database", xpcontext);
					if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
						{
							xnodeset = xpresult->nodesetval;
							if (xnodeset->nodeNr == 1)
								{
									gchar *provider;
									gchar *connection_string;
									gchar *sql;
								
									xmlNode *cur_database = xnodeset->nodeTab[0]->children;
									while (cur_database != NULL)
										{
											if (strcmp (cur_database->name, "provider") == 0)
												{
													provider = g_strstrip ((gchar *)xmlNodeGetContent (cur_database));
												}
											else if (strcmp (cur_database->name, "connection_string") == 0)
												{
													connection_string = g_strstrip ((gchar *)xmlNodeGetContent (cur_database));
												}
											else if (strcmp (cur_database->name, "sql") == 0)
												{
													sql = g_strstrip ((gchar *)xmlNodeGetContent (cur_database));
												}
											
											cur_database = cur_database->next;
										}

									if (strcmp (provider, "") == 0 ||
										strcmp (connection_string, "") == 0 ||
										strcmp (sql, "") == 0)
										{
											/* TO DO */
										}
									else
										{
											/* database connection */
											gda_init (PACKAGE_NAME, PACKAGE_VERSION, 0, NULL);
											priv->gda_client = gda_client_new ();
											priv->gda_conn = gda_client_open_connection_from_string (priv->gda_client,
																									 provider,
																									 connection_string,
																									 0);
											if (priv->gda_conn == NULL)
												{
													/* TO DO */
													g_warning ("Unable to establish the connection.");
												}
											else
												{
													GdaCommand *command = gda_command_new (sql, GDA_COMMAND_TYPE_SQL, GDA_COMMAND_OPTION_STOP_ON_ERRORS);

													priv->gda_datamodel = gda_connection_execute_single_command (priv->gda_conn, command, NULL);

													gda_command_free (command);
												}
										}
								}
						}

					/* search for node "report" */
					xpcontext->node = cur;
					xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::report", xpcontext);
					if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
						{
							xnodeset = xpresult->nodesetval;
							if (xnodeset->nodeNr == 1)
								{
									/* search for node "page-header" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page-header", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
											rpt_report_xml_parse_section (rpt_report, xpresult->nodesetval->nodeTab[0], RPTREPORT_SECTION_PAGE_HEADER);
										}

									/* search for node "page-footer" */
									xpcontext->node = xnodeset->nodeTab[0];
									xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page-footer", xpcontext);
									if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval) && xpresult->nodesetval->nodeNr == 1)
										{
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
										}
								}
							else
								{
									/* TO DO */
									/* return */
								}
						}
					else
						{
							/* TO DO */
							/* return */
						}
				}
			else
				{
					/* TO DO */
				}
		}

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
 * rpt_report_get_xml:
 * @rpt_report: an #RptReport object.
 *
 */
xmlDoc
*rpt_report_get_xml (RptReport *rpt_report)
{
	xmlDoc *xdoc = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	/* TO DO */

	return xdoc;
}

/**
 * rpt_report_get_xml_rptprint:
 * @rpt_report: an #RptReport object.
 *
 */
xmlDoc
*rpt_report_get_xml_rptprint (RptReport *rpt_report)
{
	xmlDoc *xdoc;
	xmlNode *xroot;
	xmlNode *xpage;
	gint pages = 0;
	gdouble cur_y = 0.0;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	xdoc = xmlNewDoc ("1.0");
	xroot = xmlNewNode (NULL, "reptool_report");
	xmlDocSetRootElement (xdoc, xroot);

	if (priv->gda_datamodel != NULL)
		{
			gint row;
			gint rows;

			rows = gda_data_model_get_n_rows (priv->gda_datamodel);

			for (row = 0; row < rows; row++)
				{
					if (row == 0 ||
					    (priv->page_footer != NULL && (cur_y > priv->page->size->height - priv->page_footer->height)) ||
					    cur_y > priv->page->size->height)
						{
							if (pages > 0 && priv->page_footer != NULL)
								{
									cur_y = priv->page->size->height - priv->page_footer->height;
									rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER, row - 1);
								}

							cur_y = 0.0;
							pages++;
							xpage = rpt_report_rptprint_new_page (rpt_report, xroot);

							if (priv->page_header != NULL)
								{
									rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER, row);
								}
						}

					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_BODY, row);
				}

			if (pages > 0 && priv->page_footer != NULL)
				{
					cur_y = priv->page->size->height - priv->page_footer->height;
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER, row - 1);
				}
		}
	else
		{
			if (priv->page_header != NULL)
				{
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_HEADER, -1);
				}

			pages++;
			xpage = rpt_report_rptprint_new_page (rpt_report, xroot);
			rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_BODY, -1);

			if (priv->page_footer != NULL)
				{
					cur_y = priv->page->size->height - priv->page_footer->height;
					rpt_report_rptprint_section (rpt_report, xpage, &cur_y, RPTREPORT_SECTION_PAGE_FOOTER, -1);
				}
		}

	return xdoc;
}

static void
rpt_report_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptReport *rpt_report = RPT_REPORT (object);

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (property_id)
		{
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
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
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
			height = strtod (g_strstrip (g_strdup (prop)), NULL);
		}

	cur = xnode->children;
	while (cur != NULL)
		{
			if (strcmp (cur->name, "text") == 0)
				{
					rptobj = rpt_obj_text_new_from_xml (cur);
				}
			else if (strcmp (cur->name, "line") == 0)
				{
					rptobj = rpt_obj_line_new_from_xml (cur);
				}
			else if (strcmp (cur->name, "rect") == 0)
				{
					rptobj = rpt_obj_rect_new_from_xml (cur);
				}
			else if (strcmp (cur->name, "image") == 0)
				{
					rptobj = rpt_obj_image_new_from_xml (cur);
				}

			if (rptobj != NULL)
				{
					g_object_get (rptobj, "name", &objname, NULL);

					if (rpt_report_get_object_from_name (rpt_report, objname) == NULL)
						{
							objects = g_list_append (objects, rptobj);
						}
					else
						{
							/* TO DO */
							g_warning ("An object with name \"%s\" already exists.", objname);
						}
				}

			cur = cur->next;
		}

	switch (section)
		{
			case RPTREPORT_SECTION_PAGE_HEADER:
				priv->page_header->height = height;
				priv->page_header->objects = objects;
				break;

			case RPTREPORT_SECTION_PAGE_FOOTER:
				priv->page_footer->height = height;
				priv->page_footer->objects = objects;
				break;

			case RPTREPORT_SECTION_BODY:
				priv->body->height = height;
				priv->body->objects = objects;
				break;
		}
}

static RptObject
*rpt_report_get_object_from_name (RptReport *rpt_report, const gchar *name)
{
	RptObject *obj = NULL;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	if ((obj = rpt_report_get_object_from_name_in_list (priv->page_header->objects, name)) != NULL)
		{
		}
	else if ((obj = rpt_report_get_object_from_name_in_list (priv->page_footer->objects, name)) != NULL)
		{
		}
	else if ((obj = rpt_report_get_object_from_name_in_list (priv->body->objects, name)) != NULL)
		{
		}

	return obj;
}

static RptObject
*rpt_report_get_object_from_name_in_list (GList *list, const gchar *name)
{
	gchar *objname;
	RptObject *obj = NULL;

	list = g_list_first (list);
	while (list != NULL)
		{
			g_object_get ((RptObject *)list->data, "name", &objname, NULL);
			if (strcmp (name, objname) == 0)
				{
					obj = (RptObject *)list->data;
					break;
				}
		
			list = g_list_next (list);
		}

	return obj;
}

static xmlNode
*rpt_report_rptprint_new_page (RptReport *rpt_report, xmlNode *xroot)
{
	xmlNode *xnode;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	xnode = xmlNewNode (NULL, "page");
	xmlSetProp (xnode, "width", g_strdup_printf ("%f", priv->page->size->width));
	xmlSetProp (xnode, "height", g_strdup_printf ("%f", priv->page->size->height));
	xmlAddChild (xroot, xnode);

	return xnode;
}

static void
rpt_report_rptprint_section (RptReport *rpt_report, xmlNode *xpage, gdouble *cur_y, RptReportSection section, gint row)
{
	GList *objects;
	xmlAttrPtr attr;
	xmlNode *xnode;
	gchar *prop;

	RptObject *rptobj;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	switch (section)
		{
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

			prop = (gchar *)xmlGetProp (xnode, "y");
			if (prop == NULL)
				{
					prop = g_strdup ("0.0");
				}
			xmlSetProp (xnode, "y", g_strdup_printf ("%f", strtod (prop, NULL) + *cur_y));

			if (IS_RPT_OBJ_TEXT (rptobj))
				{
					rpt_report_rptprint_parse_text_source (rpt_report, rptobj, xnode, row);
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
rpt_report_rptprint_parse_text_source (RptReport *rpt_report, RptObject *rptobj, xmlNode *xnode, gint row)
{
	/* TO DO */
	gchar *source;

	RptReportPrivate *priv = RPT_REPORT_GET_PRIVATE (rpt_report);

	g_object_get (G_OBJECT (rptobj), "source", &source, NULL);

	if (row > -1 && source[0] == '[' && source[strlen (source) - 1] == ']')
		{
			gint col;
			gchar *field;

			field = g_strstrip (g_strndup (source + 1, strlen (source) - 2));
			col = gda_data_model_get_column_position (priv->gda_datamodel, field);

			if (col > -1)
				{
					source = gda_value_stringify ((GdaValue *)gda_data_model_get_value_at (priv->gda_datamodel, col, row));
				}
		}

	xmlNodeSetContent (xnode, source);
}
