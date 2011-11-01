/*
 * Copyright (C) 2006-2011 Andrea Zagli <azagli@libero.it>
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
#include <math.h>
#include <locale.h>

#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>
#include <pango/pangocairo.h>
#include <pango/pango-attributes.h>
#include <libxml/xpath.h>

#include "rptprint.h"
#include "rptcommon.h"

enum
{
	PROP_0,
	PROP_UNIT_LENGTH,
	PROP_OUTPUT_TYPE,
	PROP_OUTPUT_FILENAME,
	PROP_COPIES,
	PROP_PATH_RELATIVES_TO
};

static void rpt_print_class_init (RptPrintClass *klass);
static void rpt_print_init (RptPrint *rpt_print);

static void rpt_print_set_property (GObject *object,
                                    guint property_id,
                                    const GValue *value,
                                    GParamSpec *pspec);
static void rpt_print_get_property (GObject *object,
                                    guint property_id,
                                    GValue *value,
                                    GParamSpec *pspec);

static void rpt_print_get_xml_page_attributes (RptPrint *rpt_print,
                                               xmlNode *xml_page);

static void rpt_print_page (RptPrint *rpt_print,
                            xmlNode *xnode);
static void rpt_print_text_xml (RptPrint *rpt_print,
                                xmlNode *xnode);
static void rpt_print_line_xml (RptPrint *rpt_print,
                                xmlNode *xnode);
static void rpt_print_rect_xml (RptPrint *rpt_print,
                                xmlNode *xnode);
static void rpt_print_ellipse_xml (RptPrint *rpt_print,
                                   xmlNode *xnode);
static void rpt_print_image_xml (RptPrint *rpt_print,
                                 xmlNode *xnode);
static void rpt_print_line (RptPrint *rpt_print,
                            const RptPoint *from_p,
                            const RptPoint *to_p,
                            const RptStroke *stroke,
                            const RptRotation *rotation);
static void rpt_print_border (RptPrint *rpt_print,
                              const RptPoint *position,
                              const RptSize *size,
                              const RptBorder *border,
                              const RptRotation *rotation);


static gchar *rpt_print_new_numbered_filename (const gchar *filename,
                                               int number);
static void rpt_print_rotate (RptPrint *rpt_print,
                              const RptPoint *position,
                              const RptSize *size,
                              gdouble angle);


static void rpt_print_gtk_begin_print (GtkPrintOperation *operation, 
                                       GtkPrintContext *context,
                                       gpointer user_data);
static void rpt_print_gtk_request_page_setup (GtkPrintOperation *operation,
                                              GtkPrintContext *context,
                                              gint page_nr,
                                              GtkPageSetup *setup,
                                              gpointer user_data);
static void rpt_print_gtk_draw_page (GtkPrintOperation *operation,
                                     GtkPrintContext *context,
                                     gint page_nr,
                                     gpointer user_data);


#define RPT_PRINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_PRINT, RptPrintPrivate))

typedef struct _RptPrintPrivate RptPrintPrivate;
struct _RptPrintPrivate
	{
		eRptUnitLength unit;

		eRptOutputType output_type;
		gchar *output_filename;

		guint copies;

		gdouble width;
		gdouble height;
		gdouble margin_top;
		gdouble margin_right;
		gdouble margin_bottom;
		gdouble margin_left;

		xmlDoc *xdoc;

		gchar *path_relatives_to;

		xmlNodeSet *pages;

		cairo_surface_t *surface;
		cairo_t *cr;
		GtkPrintContext *gtk_print_context;
	};

G_DEFINE_TYPE (RptPrint, rpt_print, G_TYPE_OBJECT)

static void
rpt_print_class_init (RptPrintClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptPrintPrivate));

	object_class->set_property = rpt_print_set_property;
	object_class->get_property = rpt_print_get_property;

	g_object_class_install_property (object_class, PROP_UNIT_LENGTH,
	                                 g_param_spec_int ("unit-length",
	                                                   "Unit length",
	                                                   "The unit length.",
	                                                   RPT_UNIT_POINTS, RPT_UNIT_MILLIMETRE,
	                                                   RPT_UNIT_POINTS,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class, PROP_OUTPUT_TYPE,
	                                 g_param_spec_int ("output-type",
	                                                   "Output Type",
	                                                   "The output type.",
	                                                   RPT_OUTPUT_PNG, RPT_OUTPUT_GTK,
	                                                   RPT_OUTPUT_PDF,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class, PROP_OUTPUT_FILENAME,
	                                 g_param_spec_string ("output-filename",
	                                                      "Output File Name",
	                                                      "The output file's name.",
	                                                      "rptreport.pdf",
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class, PROP_COPIES,
	                                 g_param_spec_uint ("copies",
	                                                    "Copies",
	                                                    "The number of copies to print.",
	                                                    1, G_MAXUINT,
	                                                    1,
	                                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class, PROP_PATH_RELATIVES_TO,
	                                 g_param_spec_string ("path-relatives-to",
	                                                      "Path are relatives to",
	                                                      "Path are relatives to this property's content.",
	                                                      "",
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
rpt_print_init (RptPrint *rpt_print)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->surface = NULL;
	priv->cr = NULL;
	priv->gtk_print_context = NULL;
}

/**
 * rpt_print_new_from_xml:
 * @xdoc: an #xmlDoc.
 *
 * Creates a new #RptPrint object.
 *
 * Returns: the newly created #RptPrint object.
 */
RptPrint
*rpt_print_new_from_xml (xmlDoc *xdoc)
{
	RptPrint *rpt_print;

	rpt_print = NULL;

	xmlNode *cur = xmlDocGetRootElement (xdoc);
	if (cur != NULL)
		{
			if (strcmp (cur->name, "reptool_report") == 0)
				{
					RptPrintPrivate *priv;

					rpt_print = RPT_PRINT (g_object_new (rpt_print_get_type (), NULL));

					priv = RPT_PRINT_GET_PRIVATE (rpt_print);

					priv->xdoc = xdoc;
				}
			else
				{
					/* TODO */
					g_warning ("Not a valid RepTool print report format.");
				}
		}
	else
		{
			g_warning ("No root element on xmlDoc.");
		}

	return rpt_print;
}

/**
 * rpt_print_new_from_file:
 * @filename: the path of the xml file to load.
 *
 * Creates a new #RptPrint object.
 *
 * Returns: the newly created #RptPrint object.
 */
RptPrint
*rpt_print_new_from_file (const gchar *filename)
{
	RptPrint *rpt_print;

	rpt_print = NULL;

	xmlDoc *xdoc = xmlParseFile (filename);
	if (xdoc != NULL)
		{
			rpt_print = rpt_print_new_from_xml (xdoc);
		}

	return rpt_print;
}

/**
 * rpt_print_set_output_type:
 * @rpt_print: an #RptPrint object.
 * @output_type:
 *
 */
void
rpt_print_set_output_type (RptPrint *rpt_print, eRptOutputType output_type)
{
	g_return_if_fail (IS_RPT_PRINT (rpt_print));

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->output_type = output_type;
}

/**
 * rpt_print_set_output_filename:
 * @rpt_print: an #RptPrint object.
 * @out_filename:
 *
 */
void
rpt_print_set_output_filename (RptPrint *rpt_print, const gchar *output_filename)
{
	g_return_if_fail (IS_RPT_PRINT (rpt_print));

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->output_filename = g_strdup (output_filename);
	if (g_strcmp0 (priv->output_filename, "") == 0)
		{
			g_warning ("It's not possible to set an empty output filename; default to rptreport.pdf.");
			priv->output_filename = g_strdup ("rptreport.pdf");
		}
}

/**
 * rpt_print_set_copies:
 * @rpt_print: an #RptPrint object.
 * @copies: number of copies.
 *
 */
void
rpt_print_set_copies (RptPrint *rpt_print, guint copies)
{
	g_return_if_fail (IS_RPT_PRINT (rpt_print));

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->copies = copies;
}

/**
 * rpt_print_print:
 * @rpt_print: an #RptPrint object.
 * @transient:
 *
 */
void
rpt_print_print (RptPrint *rpt_print, GtkWindow *transient)
{
	xmlXPathContextPtr xpcontext;
	xmlXPathObjectPtr xpresult;
	xmlNodeSetPtr xnodeset;

	FILE *fout;
	gchar *prop;

	gdouble width;
	gdouble height;

	gint npage = 0;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	xmlNode *cur = xmlDocGetRootElement (priv->xdoc);
	if (cur == NULL)
		{
			/* TODO */
			g_warning ("Xml isn't a valid reptool print definition.");
			return;
		}
	else
		{
			if (xmlStrcmp (cur->name, (const xmlChar *)"reptool_report") != 0)
				{
					/* TODO */
					g_warning ("Xml isn't a valid reptool print definition.");
					return;
				}
		}

	xpcontext = xmlXPathNewContext (priv->xdoc);

	/* search for node "properties" */
	xpcontext->node = cur;
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::properties", xpcontext);
	if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			if (xnodeset->nodeNr == 1)
				{
					xmlNode *cur_property = xnodeset->nodeTab[0]->children;
					while (cur_property != NULL)
						{
							if (strcmp (cur_property->name, "unit-length") == 0)
								{
									g_object_set (G_OBJECT (rpt_print), "unit-length", rpt_common_strunit_to_enum ((const gchar *)xmlNodeGetContent (cur_property)), NULL);
								}
							else if (strcmp (cur_property->name, "output-type") == 0)
								{
									rpt_print_set_output_type (rpt_print, rpt_common_stroutputtype_to_enum ((const gchar *)xmlNodeGetContent (cur_property)));
								}
							else if (strcmp (cur_property->name, "output-filename") == 0)
								{
									rpt_print_set_output_filename (rpt_print, (const gchar *)xmlNodeGetContent (cur_property));
								}
							else if (strcmp (cur_property->name, "copies") == 0)
								{
									rpt_print_set_copies (rpt_print, strtol ((const gchar *)xmlNodeGetContent (cur_property), NULL, 10));
								}

							cur_property = cur_property->next;
						}
				}
		}

	/* find number of pages */
	xpcontext->node = cur;
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page", xpcontext);
	if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			priv->pages = xnodeset;
		}
	else
		{
			/* TODO */
			g_warning ("No pages found in xml.");
			return;
		}

	if (priv->output_type == RPT_OUTPUT_GTK
	    || priv->output_type == RPT_OUTPUT_GTK_DEFAULT_PRINTER)
		{
			gchar *locale_old;
			gchar *locale_num;
			GtkPrintOperation *operation;
			GError *error;
			GtkPrintOperationResult res;

			locale_old = setlocale (LC_ALL, NULL);
			gtk_init (0, NULL);

			operation = gtk_print_operation_new ();
			g_signal_connect (G_OBJECT (operation), "begin-print",
			                  G_CALLBACK (rpt_print_gtk_begin_print), (gpointer)rpt_print);
			g_signal_connect (G_OBJECT (operation), "request-page-setup",
			                  G_CALLBACK (rpt_print_gtk_request_page_setup), (gpointer)rpt_print);
			g_signal_connect (G_OBJECT (operation), "draw-page",
			                  G_CALLBACK (rpt_print_gtk_draw_page), (gpointer)rpt_print);

			error = NULL;
			locale_num = setlocale (LC_NUMERIC, "C");
			res = gtk_print_operation_run (operation,
			                               (priv->output_type == RPT_OUTPUT_GTK ? GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG : GTK_PRINT_OPERATION_ACTION_PRINT),
			                               transient, &error);
			setlocale (LC_NUMERIC, locale_num);
			setlocale (LC_ALL, locale_old);

			g_free (locale_old);
			g_free (locale_num);

			if (priv->output_type == RPT_OUTPUT_GTK
			    && res == GTK_PRINT_OPERATION_RESULT_CANCEL)
				{
					return;
				}
			if (error != NULL && error->message != NULL)
				{
					g_warning ("Error on starting print operation: %s.\n", error->message);
				}
		}
	else
		{
			if (strcmp (g_strstrip (priv->output_filename), "") == 0)
				{
					switch (priv->output_type)
						{
							case RPT_OUTPUT_PNG:
								priv->output_filename = g_strdup ("reptool.png");
								break;
							case RPT_OUTPUT_PDF:
								priv->output_filename = g_strdup ("reptool.pdf");
								break;
							case RPT_OUTPUT_PS:
								priv->output_filename = g_strdup ("reptool.ps");
								break;
							case RPT_OUTPUT_SVG:
								priv->output_filename = g_strdup ("reptool.svg");
								break;
						}
				}
			if (priv->output_type != RPT_OUTPUT_PNG && priv->output_type != RPT_OUTPUT_SVG)
				{
					fout = fopen (priv->output_filename, "w");
					if (fout == NULL)
						{
							/* TODO */
							g_warning ("Unable to write to the output file.");
							return;
						}
				}

			for (npage = 0; npage < priv->pages->nodeNr; npage++)
				{
					cur = priv->pages->nodeTab[npage];
					if (strcmp (cur->name, "page") == 0)
						{
							rpt_print_get_xml_page_attributes (rpt_print, cur);
							if (priv->width != 0 && priv->height != 0)
								{
									width = rpt_common_value_to_points (priv->unit, priv->width);
									height = rpt_common_value_to_points (priv->unit, priv->height);

									if (priv->output_type == RPT_OUTPUT_PNG)
										{
											priv->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, (int)width, (int)height);
										}
									else if (priv->output_type == RPT_OUTPUT_PDF && npage == 0)
										{
											priv->surface = cairo_pdf_surface_create (priv->output_filename, width, height);
										}
									else if (priv->output_type == RPT_OUTPUT_PS && npage == 0)
										{
											priv->surface = cairo_ps_surface_create (priv->output_filename, width, height);
										}
									else if (priv->output_type == RPT_OUTPUT_SVG)
										{
											gchar *new_out_filename = rpt_print_new_numbered_filename (priv->output_filename, npage + 1);
											fout = fopen (new_out_filename, "w");
											if (fout == NULL)
												{
													/* TODO */
													g_warning ("Unable to write to the output file.");
													return;
												}
											g_free (new_out_filename);

											priv->surface = cairo_svg_surface_create (new_out_filename, width, height);
										}

									if (cairo_surface_status (priv->surface) == CAIRO_STATUS_SUCCESS)
										{
											if (priv->output_type == RPT_OUTPUT_PNG || priv->output_type == RPT_OUTPUT_SVG)
												{
													priv->cr = cairo_create (priv->surface);
												}
											else if (npage == 0)
												{
													priv->cr = cairo_create (priv->surface);
												}

											if (priv->output_type != RPT_OUTPUT_PNG && priv->output_type != RPT_OUTPUT_SVG && npage == 0)
												{
													cairo_surface_destroy (priv->surface);
												}

											if (cairo_status (priv->cr) == CAIRO_STATUS_SUCCESS)
												{
													rpt_print_page (rpt_print, cur);

													if (priv->output_type == RPT_OUTPUT_PNG)
														{
															gchar *new_out_filename = rpt_print_new_numbered_filename (priv->output_filename, npage + 1);
														
															cairo_surface_write_to_png (priv->surface,
															                            new_out_filename);
															cairo_surface_destroy (priv->surface);
															cairo_destroy (priv->cr);
															g_free (new_out_filename);
														}
													else
														{
															cairo_show_page (priv->cr);
														}

													if (priv->output_type == RPT_OUTPUT_SVG)
														{
															cairo_surface_destroy (priv->surface);
															cairo_destroy (priv->cr);
															fclose (fout);
														}
												}
											else
												{
													/* TODO */
													g_warning ("Cairo status not sucess: %d", cairo_status (priv->cr));
												}
										}
									else
										{
											/* TODO */
											g_warning ("Cairo surface status not sucess.");
										}
								}
							else
								{
									/* TODO */
									g_warning ("Page width or height cannot be zero.");
								}
						}
					else
						{
							/* TODO */
						}

					cur = cur->next;
				}

			if (priv->cr != NULL)
				{
					cairo_destroy (priv->cr);
				}
			if (priv->output_type != RPT_OUTPUT_PNG && priv->output_type != RPT_OUTPUT_SVG)
				{
					fclose (fout);
				}
		}
}

static void
rpt_print_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptPrint *rpt_print = RPT_PRINT (object);
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	switch (property_id)
		{
			case PROP_UNIT_LENGTH:
				priv->unit = g_value_get_int (value);
				break;

			case PROP_OUTPUT_TYPE:
				rpt_print_set_output_type (rpt_print, g_value_get_int (value));
				break;

			case PROP_OUTPUT_FILENAME:
				rpt_print_set_output_filename (rpt_print, g_value_get_string (value));
				break;

			case PROP_COPIES:
				rpt_print_set_copies (rpt_print, g_value_get_uint (value));
				break;

			case PROP_PATH_RELATIVES_TO:
				priv->path_relatives_to = g_strstrip (g_strdup (g_value_get_string (value)));
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_print_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	RptPrint *rpt_print = RPT_PRINT (object);
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	switch (property_id)
		{
			case PROP_UNIT_LENGTH:
				g_value_set_int (value, priv->unit);
				break;

			case PROP_OUTPUT_TYPE:
				g_value_set_int (value, priv->output_type);
				break;

			case PROP_OUTPUT_FILENAME:
				g_value_set_string (value, priv->output_filename);
				break;

			case PROP_COPIES:
				g_value_set_uint (value, priv->copies);
				break;

			case PROP_PATH_RELATIVES_TO:
				g_value_set_string (value, priv->path_relatives_to);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_print_get_xml_page_attributes (RptPrint *rpt_print, xmlNode *xml_page)
{
	gchar *prop;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	prop = xmlGetProp (xml_page, (const xmlChar *)"width");
	if (prop != NULL)
		{
			priv->width = g_strtod (prop, NULL);
			xmlFree (prop);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"height");
	if (prop != NULL)
		{
			priv->height = g_strtod (prop, NULL);
			xmlFree (prop);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-top");
	if (prop != NULL)
		{
			priv->margin_top = g_strtod (prop, NULL);
			xmlFree (prop);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-right");
	if (prop != NULL)
		{
			priv->margin_right = g_strtod (prop, NULL);
			xmlFree (prop);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-bottom");
	if (prop != NULL)
		{
			priv->margin_bottom = g_strtod (prop, NULL);
			xmlFree (prop);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-left");
	if (prop != NULL)
		{
			priv->margin_left = g_strtod (prop, NULL);
			xmlFree (prop);
		}
}

static void
rpt_print_page (RptPrint *rpt_print, xmlNode *xnode)
{
	gchar *prop;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	xmlNode *cur = xnode->children;

	gdouble width = rpt_common_value_to_points (priv->unit, priv->width);
	gdouble height = rpt_common_value_to_points (priv->unit, priv->height);
	gdouble margin_left = rpt_common_value_to_points (priv->unit, priv->margin_left);
	gdouble margin_right = rpt_common_value_to_points (priv->unit, priv->margin_right);
	gdouble margin_top = rpt_common_value_to_points (priv->unit, priv->margin_top);
	gdouble margin_bottom = rpt_common_value_to_points (priv->unit, priv->margin_bottom);

	/* clipping region for page's margins */
	cairo_rectangle (priv->cr,
	                 margin_left,
	                 margin_top,
	                 width - margin_left - margin_right,
	                 height - margin_top - margin_bottom);
	cairo_clip (priv->cr);

	while (cur != NULL)
		{
			if (!xmlNodeIsText (cur))
				{
					prop = (gchar *)xmlGetProp (cur, "visible");
					if (prop != NULL
					    && strcmp (g_strstrip (prop), "y") == 0)
						{
							cairo_save (priv->cr);
							if (g_strcmp0 (cur->name, "text") == 0)
								{
									rpt_print_text_xml (rpt_print, cur);
								}
							else if (g_strcmp0 (cur->name, "line") == 0)
								{
									rpt_print_line_xml (rpt_print, cur);
								}
							else if (g_strcmp0 (cur->name, "rect") == 0)
								{
									rpt_print_rect_xml (rpt_print, cur);
								}
							else if (g_strcmp0 (cur->name, "ellipse") == 0)
								{
									rpt_print_ellipse_xml (rpt_print, cur);
								}
							else if (g_strcmp0 (cur->name, "image") == 0)
								{
									rpt_print_image_xml (rpt_print, cur);
								}
							cairo_restore (priv->cr);
						}

					if (prop != NULL)
						{
							xmlFree (prop);
						}
				}

			cur = cur->next;
		}
}

static void
rpt_print_text_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	RptPoint *position;
	RptSize *size;
	RptRotation *rotation;
	RptAlign *align;
	RptBorder *border;
	RptFont *font;
	RptColor *color;

	PangoLayout *playout;
	PangoFontDescription *pfdesc;
	PangoAttribute *pattr;
	PangoAttrList *lpattr = NULL;

	GString *text;
	gchar *prop;

	gdouble padding_top = 0.0;
	gdouble padding_right = 0.0;
	gdouble padding_bottom = 0.0;
	gdouble padding_left = 0.0;

	gdouble layout_width;

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	rotation = rpt_common_get_rotation (xnode);
	align = rpt_common_get_align (xnode);
	border = rpt_common_get_border (xnode);
	font = rpt_common_get_font (xnode);

	if (position == NULL)
		{
			g_warning ("Text node position is mandatory.");
			return;
		}

	text = g_string_new ((gchar *)xmlNodeGetContent (xnode));

	/* padding */
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-top");
	if (prop != NULL)
		{
			padding_top = rpt_common_value_to_points (priv->unit, g_strtod (prop, NULL));
			xmlFree (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-right");
	if (prop != NULL)
		{
			padding_right = rpt_common_value_to_points (priv->unit, g_strtod (prop, NULL));
			xmlFree (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-bottom");
	if (prop != NULL)
		{
			padding_bottom = rpt_common_value_to_points (priv->unit, g_strtod (prop, NULL));
			xmlFree (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-left");
	if (prop != NULL)
		{
			padding_left = rpt_common_value_to_points (priv->unit, g_strtod (prop, NULL));
			xmlFree (prop);
		}

	layout_width = rpt_common_value_to_points (priv->unit, size->width) - padding_left - padding_right;

	/* creating pango layout */
	/*if (priv->output_type == RPTP_OUTPUT_GTK)
		{
			playout = gtk_print_context_create_pango_layout (priv->gtk_print_context);
		}
	else
		{*/
			playout = pango_cairo_create_layout (priv->cr);
		/*}*/
	if (size != NULL)
		{
			pango_layout_set_width (playout, layout_width * PANGO_SCALE);
		}

	/* creating pango font description */
	pfdesc = pango_font_description_new ();

	pango_font_description_set_family (pfdesc, font->name);
	if (font->bold)
		{
			pango_font_description_set_weight (pfdesc, PANGO_WEIGHT_BOLD);
		}
	if (font->italic)
		{
			pango_font_description_set_style (pfdesc, PANGO_STYLE_ITALIC);
		}
	if (font->size > 0.0f)
		{
			pango_font_description_set_size (pfdesc, (int)font->size * PANGO_SCALE);
		}
	else
		{
			pango_font_description_set_size (pfdesc, 12 * PANGO_SCALE);
		}

	pango_layout_set_font_description (playout, pfdesc);
	pango_font_description_free (pfdesc);

	/* setting layout attributes */
	if (font->underline != PANGO_UNDERLINE_NONE)
		{
			PangoAttribute *pattr;

			pattr = pango_attr_underline_new (font->underline);
			pattr->start_index = 0;
			pattr->end_index = text->len + 1;

			if (lpattr == NULL)
				{
					lpattr = pango_attr_list_new ();
				}
			pango_attr_list_insert (lpattr, pattr);
		}
	if (font->strike)
		{
			PangoAttribute *pattr;
		
			pattr = pango_attr_strikethrough_new (TRUE);
			pattr->start_index = 0;
			pattr->end_index = text->len + 1;

			if (lpattr == NULL)
				{
					lpattr = pango_attr_list_new ();
				}
			pango_attr_list_insert (lpattr, pattr);
		}

	/* letter spacing */
	prop = xmlGetProp (xnode, (const xmlChar *)"letter-spacing");
	if (prop != NULL)
		{
			guint spacing;

			spacing = strtol (prop, NULL, 10);

			if (spacing > 0)
				{
					PangoAttribute *pattr;

					pattr = pango_attr_letter_spacing_new (spacing * PANGO_SCALE);
					pattr->start_index = 0;
					pattr->end_index = text->len + 1;

					if (lpattr == NULL)
						{
							lpattr = pango_attr_list_new ();
						}
					pango_attr_list_insert (lpattr, pattr);
				}

			xmlFree (prop);
		}

	if (lpattr != NULL)
		{
			pango_layout_set_attributes (playout, lpattr);
		}

	if (rotation != NULL)
		{
			rpt_print_rotate (rpt_print, position, size, rotation->angle);
		}

	/* background */
	prop = xmlGetProp (xnode, (const xmlChar *)"background-color");
	if (prop != NULL && position != NULL && size != NULL)
		{
			color = rpt_common_parse_color (prop);

			cairo_rectangle (priv->cr, rpt_common_value_to_points (priv->unit, position->x),
			                 rpt_common_value_to_points (priv->unit, position->y),
			                 rpt_common_value_to_points (priv->unit, size->width),
			                 rpt_common_value_to_points (priv->unit, size->height));
			cairo_set_source_rgba (priv->cr, color->r, color->g, color->b, color->a);
			cairo_fill_preserve (priv->cr);
		}
	if (prop != NULL) xmlFree (prop);

	/* drawing border */
	rpt_print_border (rpt_print, position, size, border, rotation);

	/* setting horizontal alignment */
	switch (align->h_align)
		{
			case RPT_HALIGN_LEFT:
				break;

			case RPT_HALIGN_CENTER:
				pango_layout_set_alignment (playout, PANGO_ALIGN_CENTER);
				break;

			case RPT_HALIGN_RIGHT:
				pango_layout_set_alignment (playout, PANGO_ALIGN_RIGHT);
				break;

			case RPT_HALIGN_JUSTIFIED:
				pango_layout_set_justify (playout, TRUE);
				break;
		}

	/* TODO */
	/* setting vertical alignment */
	switch (align->v_align)
		{
	 		case RPT_VALIGN_TOP:
	 			break;

	 		case RPT_VALIGN_CENTER:
	 			break;

	 		case RPT_VALIGN_BOTTOM:
	 			break;
		}

	/* setting clipping region */
	if (size != NULL)
		{
			cairo_rectangle (priv->cr,
			                 rpt_common_value_to_points (priv->unit, position->x) + padding_left,
			                 rpt_common_value_to_points (priv->unit, position->y) + padding_top,
			                 rpt_common_value_to_points (priv->unit, size->width) - padding_left - padding_right,
			                 rpt_common_value_to_points (priv->unit, size->height) - padding_top - padding_bottom);
			cairo_clip (priv->cr);
		}

	/* ellipsize */
	prop = xmlGetProp (xnode, (const xmlChar *)"ellipsize");
	if (prop != NULL)
		{
			eRptEllipsize ellipsize = rpt_common_strellipsize_to_enum (prop);
			if (ellipsize > RPT_ELLIPSIZE_NONE)
				{
					switch (ellipsize)
						{
							case RPT_ELLIPSIZE_START:
								pango_layout_set_ellipsize (playout, PANGO_ELLIPSIZE_START);
								break;

							case RPT_ELLIPSIZE_MIDDLE:
								pango_layout_set_ellipsize (playout, PANGO_ELLIPSIZE_MIDDLE);
								break;

							case RPT_ELLIPSIZE_END:
								pango_layout_set_ellipsize (playout, PANGO_ELLIPSIZE_END);
								break;

							default:
								break;
						}
				}
			xmlFree (prop);
		}

	/* drawing text */
	if (font != NULL)
		{
			if (font->color != NULL)
				{
					cairo_set_source_rgba (priv->cr, font->color->r, font->color->g, font->color->b, font->color->a);
				}
			else
				{
					cairo_set_source_rgba (priv->cr, 0.0, 0.0, 0.0, 1.0);
				}
		}

	cairo_move_to (priv->cr, rpt_common_value_to_points (priv->unit, position->x) + padding_left,
	               rpt_common_value_to_points (priv->unit, position->y) + padding_top);

	pango_layout_set_text (playout, text->str, -1);

	/* fill-with */
	prop = xmlGetProp (xnode, (const xmlChar *)"fill-with");
	if (prop != NULL
	    && g_strcmp0 (g_strstrip (prop), "") != 0)
		{
			PangoLayoutLine *line;
			PangoRectangle rect;
			gint lines;

			GString *text_tmp;

			lines = pango_layout_get_line_count (playout);

			text_tmp = g_string_new (text->str);

			g_string_append (text_tmp, prop);
			pango_layout_set_text (playout, text_tmp->str, -1);
			line = pango_layout_get_line (playout, pango_layout_get_line_count (playout) - 1);
			pango_layout_line_get_pixel_extents (line, NULL, &rect);
			while (lines == pango_layout_get_line_count (playout)
			       && rect.width < layout_width)
				{
					g_string_append (text, prop);

					g_string_append (text_tmp, prop);
					pango_layout_set_text (playout, text_tmp->str, -1);
					line = pango_layout_get_line (playout, pango_layout_get_line_count (playout) - 1);
					pango_layout_line_get_pixel_extents (line, NULL, &rect);
				}

			g_string_free (text_tmp, TRUE);
		}

	pango_layout_set_text (playout, text->str, -1);

	pango_cairo_show_layout (priv->cr, playout);

	if (position != NULL && size != NULL)
		{
			cairo_reset_clip (priv->cr);
		}

	g_free (position);
	g_free (size);
	g_free (rotation);
	g_free (align);
	g_free (border);
	g_free (font);
	g_string_free (text, TRUE);
}

static void
rpt_print_line_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint *position;
	RptPoint *from_p;
	RptPoint *to_p;
	RptSize *size;
	RptRotation *rotation;
	RptStroke *stroke;

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	rotation = rpt_common_get_rotation (xnode);
	stroke = rpt_common_get_stroke (xnode);

	from_p = rpt_common_rptpoint_new_with_values (position->x,
	                                              position->y);
	to_p = rpt_common_rptpoint_new_with_values (position->x + size->width,
	                                            position->y + size->height);

	rpt_print_line (rpt_print, from_p, to_p, stroke, rotation);

	g_free (position);
	g_free (from_p);
	g_free (to_p);
	g_free (size);
	g_free (rotation);
	g_free (stroke);
}

static void
rpt_print_rect_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint *position;
	RptSize *size;
	RptRotation *rotation;
	RptStroke *stroke;
	RptColor *fill_color;
	gchar *prop;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	rotation = rpt_common_get_rotation (xnode);
	stroke = rpt_common_get_stroke (xnode);

	if (position == NULL || size == NULL)
		{
			g_warning ("Rect node position and size are mandatories.");
			return;
		}
	if (stroke == NULL)
		{
			stroke = rpt_common_rptstroke_new ();
			stroke->width = rpt_common_points_to_value (priv->unit, 1.0);
			stroke->color = rpt_common_rptcolor_new ();
			stroke->color->a = 1.0;
			stroke->style = NULL;
		}

	prop = xmlGetProp (xnode, (const xmlChar *)"fill-color");
	if (prop != NULL)
		{
			fill_color = rpt_common_parse_color (prop);
			xmlFree (prop);
		}

	if (rotation != NULL)
		{
			rpt_print_rotate (rpt_print, position, size, rotation->angle);
		}

	/* TODO */
	/*cairo_set_line_width (priv->cr, stroke.width);*/
	cairo_rectangle (priv->cr,
	                 rpt_common_value_to_points (priv->unit, position->x),
	                 rpt_common_value_to_points (priv->unit, position->y),
	                 rpt_common_value_to_points (priv->unit, size->width),
	                 rpt_common_value_to_points (priv->unit, size->height));

	if (prop != NULL && fill_color != NULL)
		{
			cairo_set_source_rgba (priv->cr, fill_color->r, fill_color->g, fill_color->b, fill_color->a);
			cairo_fill_preserve (priv->cr);
		}

	if (stroke->style != NULL)
		{
			gdouble *dash = rpt_common_style_to_array (stroke->style);
			cairo_set_dash (priv->cr, dash, stroke->style->len, 0.0);
		}

	cairo_set_source_rgba (priv->cr, stroke->color->r, stroke->color->g, stroke->color->b, stroke->color->a);
	cairo_stroke (priv->cr);

	g_free (position);
	g_free (size);
	g_free (rotation);
	g_free (stroke);
}

static void
rpt_print_ellipse_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint *position;
	RptSize *size;
	RptStroke *stroke;
	RptColor *fill_color;
	gchar *prop;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	stroke = rpt_common_get_stroke (xnode);

	/* TODO */
	/* rotation */

	if (position == NULL || size == NULL)
		{
			g_warning ("Ellipse node position and size are mandatories.");
			return;
		}
	if (stroke == NULL)
		{
			stroke = rpt_common_rptstroke_new ();
			stroke->width = rpt_common_points_to_value (priv->unit, 1.0);
			stroke->color = rpt_common_rptcolor_new ();
			stroke->color->a = 1.0;
			stroke->style = NULL;
		}

	prop = xmlGetProp (xnode, (const xmlChar *)"fill-color");
	if (prop != NULL)
		{
			fill_color = rpt_common_parse_color (prop);
		}

	cairo_new_path (priv->cr);

	cairo_save (priv->cr);
	cairo_translate (priv->cr, rpt_common_value_to_points (priv->unit, position->x),
	                 rpt_common_value_to_points (priv->unit, position->y));
	cairo_scale (priv->cr, rpt_common_value_to_points (priv->unit, size->width),
	             rpt_common_value_to_points (priv->unit, size->height));
	cairo_arc (priv->cr, 0., 0., 1., 0., 2. * M_PI);
	cairo_restore (priv->cr);
	
	if (prop != NULL && fill_color != NULL)
		{
			cairo_set_source_rgba (priv->cr, fill_color->r, fill_color->g, fill_color->b, fill_color->a);
			cairo_fill_preserve (priv->cr);
		}

	cairo_set_source_rgba (priv->cr, stroke->color->r, stroke->color->g, stroke->color->b, stroke->color->a);
	cairo_stroke (priv->cr);

	g_free (position);
	g_free (size);
	g_free (stroke);
}

static void
rpt_print_image_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint *position;
	RptSize *size;
	RptRotation *rotation;
	RptBorder *border;

	cairo_surface_t *image;
	cairo_pattern_t *pattern;
	cairo_matrix_t matrix;

	gint w, h;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	gchar *adapt;
	gchar *filename= xmlGetProp (xnode, (const xmlChar *)"source");
	if (filename == NULL)
		{
			g_warning ("Image node source is mandatory.");
			return;
		}

	filename = g_build_filename (priv->path_relatives_to, filename, NULL);

	adapt = xmlGetProp (xnode, (const xmlChar *)"adapt");
	if (adapt == NULL)
		{
			adapt = g_strdup ("none");
		}
	else
		{
			g_strstrip (adapt);
		}

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	rotation = rpt_common_get_rotation (xnode);
	border = rpt_common_get_border (xnode);

	image = cairo_image_surface_create_from_png (filename);
	if (cairo_surface_status (image) != CAIRO_STATUS_SUCCESS)
		{
			g_warning ("Unable to create the cairo surface from the image «%s».", filename);
			return;
		}

	pattern = cairo_pattern_create_for_surface (image);

	if (rotation != NULL)
		{
			rpt_print_rotate (rpt_print, position, size, rotation->angle);
		}

	cairo_matrix_init_identity (&matrix);
	if (strcmp (adapt, "none") != 0)
		{
			w = cairo_image_surface_get_width (image);
			h = cairo_image_surface_get_height (image);

			if (strcmp (adapt, "to-box") == 0)
				{
					cairo_matrix_scale (&matrix, w / rpt_common_value_to_points (priv->unit, size->width), h / rpt_common_value_to_points (priv->unit, size->height));
				}
			else if (strcmp (adapt, "to-image") == 0)
				{
					size->width = rpt_common_points_to_value (priv->unit, (gdouble)w);
					size->height = rpt_common_points_to_value (priv->unit, (gdouble)h);
				}
		}
	cairo_matrix_translate (&matrix, rpt_common_value_to_points (priv->unit, -position->x), rpt_common_value_to_points (priv->unit, -position->y));
	
	cairo_pattern_set_matrix (pattern, &matrix);
	cairo_set_source (priv->cr, pattern);

	cairo_rectangle (priv->cr, rpt_common_value_to_points (priv->unit, position->x),
	                 rpt_common_value_to_points (priv->unit, position->y),
	                 rpt_common_value_to_points (priv->unit, size->width),
	                 rpt_common_value_to_points (priv->unit, size->height));
	cairo_fill (priv->cr);

	rpt_print_border (rpt_print, position, size, border, rotation);
	
	cairo_pattern_destroy (pattern);
	cairo_surface_destroy (image);

	g_free (position);
	g_free (size);
	g_free (rotation);
	g_free (border);
	g_free (adapt);
}

static void
rpt_print_line (RptPrint *rpt_print,
                const RptPoint *from_p,
                const RptPoint *to_p,
                const RptStroke *stroke,
                const RptRotation *rotation)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	gdouble to_add;

	if (from_p == NULL || to_p == NULL)
		{
			g_warning ("Line node from point and to point are mandatories.");
			return;
		}

	to_add = 0;
	if (stroke != NULL)
		{
			cairo_set_line_width (priv->cr, stroke->width);
			if ((gint)stroke->width % 2 != 0)
				{
					to_add = 0.5;
				}

			if (stroke->color != NULL)
				{
					cairo_set_source_rgba (priv->cr, stroke->color->r, stroke->color->g, stroke->color->b, stroke->color->a);
				}
			else
				{
					cairo_set_source_rgba (priv->cr, 0.0, 0.0, 0.0, 1.0);
				}
			if (stroke->style != NULL)
				{
					gdouble *dash = rpt_common_style_to_array (stroke->style);
					cairo_set_dash (priv->cr, dash, stroke->style->len, 0.0);
				}
		}
	else
		{
			cairo_set_source_rgba (priv->cr, 0.0, 0.0, 0.0, 1.0);
		}

	if (rotation != NULL)
		{
			RptSize *size;

			size = rpt_common_rptsize_new_with_values (rpt_common_value_to_points (priv->unit, to_p->x - from_p->x),
			                                           rpt_common_value_to_points (priv->unit, to_p->y - from_p->y));

			rpt_print_rotate (rpt_print, from_p, size, rotation->angle);

			g_free (size);
		}

	cairo_move_to (priv->cr,
	               rpt_common_value_to_points (priv->unit, from_p->x),
	               rpt_common_value_to_points (priv->unit, from_p->y) + to_add);
	cairo_line_to (priv->cr,
	               rpt_common_value_to_points (priv->unit, to_p->x),
	               rpt_common_value_to_points (priv->unit, to_p->y) + to_add);
	cairo_stroke (priv->cr);

	if (stroke != NULL && stroke->style != NULL)
		{
			cairo_set_dash (priv->cr, NULL, 0, 0.0);
		}
}

static void
rpt_print_border (RptPrint *rpt_print,
                  const RptPoint *position,
                  const RptSize *size,
                  const RptBorder *border,
                  const RptRotation *rotation)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	if (position == NULL || size == NULL || border == NULL)
		{
			g_warning ("Border position and size are mandatories.");
			return;
		}

	RptPoint *from_p = rpt_common_rptpoint_new ();
	RptPoint *to_p = rpt_common_rptpoint_new ();
	RptStroke *stroke = rpt_common_rptstroke_new ();

	if (border->top_width != 0.0)
		{
			from_p->x = position->x;
			from_p->y = position->y;
			to_p->x = position->x + size->width;
			to_p->y = position->y;
			stroke->width = border->top_width;
			stroke->color = border->top_color;
			stroke->style = border->top_style;
			rpt_print_line (rpt_print, from_p, to_p, stroke, NULL);
		}
	if (border->right_width != 0.0)
		{
			from_p->x = position->x + size->width;
			from_p->y = position->y;
			to_p->x = position->x + size->width;
			to_p->y = position->y + size->height;
			stroke->width = border->right_width;
			stroke->color = border->right_color;
			stroke->style = border->right_style;
			rpt_print_line (rpt_print, from_p, to_p, stroke, NULL);
		}
	if (border->bottom_width != 0.0)
		{
			from_p->x = position->x;
			from_p->y = position->y + size->height;
			to_p->x = position->x + size->width;
			to_p->y = position->y + size->height;
			stroke->width = border->bottom_width;
			stroke->color = border->bottom_color;
			stroke->style = border->bottom_style;
			rpt_print_line (rpt_print, from_p, to_p, stroke, NULL);
		}
	if (border->left_width != 0.0)
		{
			from_p->x = position->x;
			from_p->y = position->y;
			to_p->x = position->x;
			to_p->y = position->y + size->height;
			stroke->width = border->left_width;
			stroke->color = border->left_color;
			stroke->style = border->left_style;
			rpt_print_line (rpt_print, from_p, to_p, stroke, NULL);
		}

	g_free (from_p);
	g_free (to_p);
	g_free (stroke);
}

static gchar
*rpt_print_new_numbered_filename (const gchar *filename, int number)
{
	gchar *new_out_filename = NULL;

	gchar *filename_ext = g_strrstr (filename, ".");
	if (filename_ext == NULL)
		{
			new_out_filename = g_strdup_printf ("%s%d", filename, number);
		}
	else
		{
			new_out_filename = g_strdup_printf ("%s%d%s",
			                                    g_strndup (filename, strlen (filename) - strlen (filename_ext)),
			                                    number,
			                                    filename_ext);
		}

	return new_out_filename;
}

static void
rpt_print_rotate (RptPrint *rpt_print, const RptPoint *position, const RptSize *size, gdouble angle)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	gdouble tx = position->x + size->width / 2;
	gdouble ty = position->y + size->height / 2;

	cairo_translate (priv->cr, tx, ty);
	cairo_rotate (priv->cr, angle * G_PI / 180.);
	cairo_translate (priv->cr, -tx, -ty);
}

static void
rpt_print_gtk_begin_print (GtkPrintOperation *operation, 
                           GtkPrintContext *context,
                           gpointer user_data)
{
	RptPrint *rpt_print = (RptPrint *)user_data;
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	GtkPageSetup *page_set = gtk_page_setup_new ();

	gtk_page_setup_set_top_margin (page_set, 0.0, GTK_UNIT_POINTS);
	gtk_page_setup_set_bottom_margin (page_set, 0.0, GTK_UNIT_POINTS);
	gtk_page_setup_set_left_margin (page_set, 0.0, GTK_UNIT_POINTS);
	gtk_page_setup_set_right_margin (page_set, 0.0, GTK_UNIT_POINTS);

	gtk_print_operation_set_default_page_setup (operation, page_set);

	gtk_print_operation_set_unit (operation, GTK_UNIT_POINTS);
	gtk_print_operation_set_n_pages (operation, priv->pages->nodeNr);

	GtkPrintSettings *settings = gtk_print_operation_get_print_settings (operation);
	if (settings == NULL)
		{
			settings = gtk_print_settings_new ();
		}
	gtk_print_settings_set_n_copies (settings, priv->copies);
	gtk_print_operation_set_print_settings (operation, settings);
}

static void
rpt_print_gtk_request_page_setup (GtkPrintOperation *operation,
                                  GtkPrintContext *context,
                                  gint page_nr,
                                  GtkPageSetup *setup,
                                  gpointer user_data)
{
	GtkPaperSize *paper_size;
	gdouble swap;

	RptPrint *rpt_print = (RptPrint *)user_data;
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	rpt_print_get_xml_page_attributes (rpt_print, priv->pages->nodeTab[page_nr]);

	gtk_print_operation_set_use_full_page (operation, TRUE);
	swap = 0.0;
	if (priv->width > priv->height)
		{
			swap = priv->width;
			priv->width = priv->height;
			priv->height = swap;

			gtk_page_setup_set_orientation (setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
		}

	paper_size = gtk_paper_size_new_custom ("reptool",
	                                        "RepTool",
	                                        rpt_common_value_to_points (priv->unit, priv->width),
	                                        rpt_common_value_to_points (priv->unit, priv->height),
	                                        GTK_UNIT_POINTS);

	gtk_page_setup_set_paper_size (setup, paper_size);

	if (swap != 0)
		{
			swap = priv->width;
			priv->width = priv->height;
			priv->height = swap;
		}
}

static void
rpt_print_gtk_draw_page (GtkPrintOperation *operation,
                         GtkPrintContext *context,
                         gint page_nr,
                         gpointer user_data)
{
	RptPrint *rpt_print = (RptPrint *)user_data;
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->cr = gtk_print_context_get_cairo_context (context);
	priv->gtk_print_context = context;

	cairo_reset_clip (priv->cr);
	cairo_scale (priv->cr,
	             gtk_print_context_get_width (priv->gtk_print_context) / rpt_common_value_to_points (priv->unit, priv->width),
	             gtk_print_context_get_height (priv->gtk_print_context) / rpt_common_value_to_points (priv->unit, priv->height));

	if (priv->width != 0 && priv->height != 0)
		{
			rpt_print_page (rpt_print, priv->pages->nodeTab[page_nr]);
		}
}
