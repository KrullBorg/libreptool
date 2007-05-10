/*
 * Copyright (C) 2006-2007 Andrea Zagli <azagli@inwind.it>
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

#include <gtk/gtk.h>
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
	PROP_OUTPUT_TYPE,
	PROP_OUTPUT_FILENAME
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


static gchar *rpt_print_new_numbered_filename (const gchar *filename, int number);
static void rpt_print_rotate (RptPrint *rpt_print, const RptPoint *position, const RptSize *size, gdouble angle);


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
		gdouble width;
		gdouble height;
		gdouble margin_top;
		gdouble margin_right;
		gdouble margin_bottom;
		gdouble margin_left;

		xmlDoc *xdoc;

		RptPrintOutputType output_type;
		gchar *output_filename;

		xmlNodeSet *pages;

		cairo_surface_t *surface;
		cairo_t *cr;
		GtkPrintContext *gtk_print_context;
	};

GType
rpt_print_get_type (void)
{
	static GType rpt_print_type = 0;

	if (!rpt_print_type)
		{
			static const GTypeInfo rpt_print_info =
			{
				sizeof (RptPrintClass),
				NULL,	/* base_init */
				NULL,	/* base_finalize */
				(GClassInitFunc) rpt_print_class_init,
				NULL,	/* class_finalize */
				NULL,	/* class_data */
				sizeof (RptPrint),
				0,	/* n_preallocs */
				(GInstanceInitFunc) rpt_print_init,
				NULL
			};

			rpt_print_type = g_type_register_static (G_TYPE_OBJECT, "RptPrint",
			                                         &rpt_print_info, 0);
		}

	return rpt_print_type;
}

static void
rpt_print_class_init (RptPrintClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (RptPrintPrivate));

	object_class->set_property = rpt_print_set_property;
	object_class->get_property = rpt_print_get_property;

	g_object_class_install_property (object_class, PROP_OUTPUT_TYPE,
	                                 g_param_spec_int ("output-type",
	                                                   "Output Type",
	                                                   "The output type.",
	                                                   RPTP_OUTPUT_PNG, RPTP_OUTPUT_GTK, RPTP_OUTPUT_PDF,
	                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
	g_object_class_install_property (object_class, PROP_OUTPUT_FILENAME,
	                                 g_param_spec_string ("output-filename",
	                                                      "Output File Name",
	                                                      "The output file's name.",
	                                                      "",
	                                                      G_PARAM_READWRITE));
}

static void
rpt_print_init (RptPrint *rpt_print)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->output_filename = g_strdup ("");
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
	RptPrint *rpt_print = NULL;

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
					/* TO DO */
					g_warning ("Not a valid RepTool print report format");
				}
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
	RptPrint *rpt_print = NULL;

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
rpt_print_set_output_type (RptPrint *rpt_print, RptPrintOutputType output_type)
{
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
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	priv->output_filename = g_strdup (output_filename);
}

/**
 * rpt_print_print:
 * @rpt_print: an #RptPrint object.
 *
 */
void
rpt_print_print (RptPrint *rpt_print)
{
	xmlXPathContextPtr xpcontext;
	xmlXPathObjectPtr xpresult;
	xmlNodeSetPtr xnodeset;

	FILE *fout;
	gchar *prop;
	gint npage = 0;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	xmlNode *cur = xmlDocGetRootElement (priv->xdoc);
	if (cur == NULL)
		{
			/* TO DO */
			g_warning ("Xml isn't a valid reptool print definition.");
			return;
		}
	else
		{
			if (xmlStrcmp (cur->name, (const xmlChar *)"reptool_report") != 0)
				{
					/* TO DO */
					g_warning ("Xml isn't a valid reptool print definition.");
					return;
				}
		}

	/* find number of pages */
	xpcontext = xmlXPathNewContext (priv->xdoc);

	xpcontext->node = xmlDocGetRootElement (priv->xdoc);
	xpresult = xmlXPathEvalExpression ((const xmlChar *)"child::page", xpcontext);
	if (!xmlXPathNodeSetIsEmpty (xpresult->nodesetval))
		{
			xnodeset = xpresult->nodesetval;
			priv->pages = xnodeset;
		}
	else
		{
			/* TO DO */
			g_warning ("No pages found in xml.");
			return;
		}

	if (priv->output_type == RPTP_OUTPUT_GTK)
		{
			gchar *locale_old;
			gchar *locale_num;
			GtkPrintOperation *operation;

			locale_old = setlocale (LC_ALL, NULL);
			gtk_init (0, NULL);

			operation = gtk_print_operation_new ();
			g_signal_connect (G_OBJECT (operation), "begin-print",
			                  G_CALLBACK (rpt_print_gtk_begin_print), (gpointer)rpt_print);
			g_signal_connect (G_OBJECT (operation), "request-page-setup",
			                  G_CALLBACK (rpt_print_gtk_request_page_setup), (gpointer)rpt_print);
			g_signal_connect (G_OBJECT (operation), "draw-page",
			                  G_CALLBACK (rpt_print_gtk_draw_page), (gpointer)rpt_print);

			locale_num = setlocale (LC_NUMERIC, "C");
			gtk_print_operation_run (operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);
			setlocale (LC_NUMERIC, locale_num);
			setlocale (LC_ALL, locale_old);
		}
	else
		{
			if (strcmp (g_strstrip (priv->output_filename), "") == 0)
				{
					switch (priv->output_type)
						{
							case RPTP_OUTPUT_PNG:
								priv->output_filename = g_strdup ("reptool.png");
								break;
							case RPTP_OUTPUT_PDF:
								priv->output_filename = g_strdup ("reptool.pdf");
								break;
							case RPTP_OUTPUT_PS:
								priv->output_filename = g_strdup ("reptool.ps");
								break;
							case RPTP_OUTPUT_SVG:
								priv->output_filename = g_strdup ("reptool.svg");
								break;
						}
				}
			if (priv->output_type != RPTP_OUTPUT_PNG && priv->output_type != RPTP_OUTPUT_SVG)
				{
					fout = fopen (priv->output_filename, "w");
					if (fout == NULL)
						{
							/* TO DO */
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
									if (priv->output_type == RPTP_OUTPUT_PNG)
										{
											priv->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, (int)priv->width, (int)priv->height);
										}
									else if (priv->output_type == RPTP_OUTPUT_PDF && npage == 0)
										{
											priv->surface = cairo_pdf_surface_create (priv->output_filename, priv->width, priv->height);
										}
									else if (priv->output_type == RPTP_OUTPUT_PS && npage == 0)
										{
											priv->surface = cairo_ps_surface_create (priv->output_filename, priv->width, priv->height);
										}
									else if (priv->output_type == RPTP_OUTPUT_SVG)
										{
											gchar *new_out_filename = rpt_print_new_numbered_filename (priv->output_filename, npage + 1);
											fout = fopen (new_out_filename, "w");
											if (fout == NULL)
												{
													/* TO DO */
													return;
												}
		
											priv->surface = cairo_svg_surface_create (new_out_filename, priv->width, priv->height);
										}
		
									if (cairo_surface_status (priv->surface) == CAIRO_STATUS_SUCCESS)
										{
											if (priv->output_type == RPTP_OUTPUT_PNG || priv->output_type == RPTP_OUTPUT_SVG)
												{
													priv->cr = cairo_create (priv->surface);
												}
											else if (npage == 0)
												{
													priv->cr = cairo_create (priv->surface);
												}
		
											if (priv->output_type != RPTP_OUTPUT_PNG && priv->output_type != RPTP_OUTPUT_SVG && npage == 0)
												{
													cairo_surface_destroy (priv->surface);
												}
		
											if (cairo_status (priv->cr) == CAIRO_STATUS_SUCCESS)
												{
													rpt_print_page (rpt_print, cur);
		
													if (priv->output_type == RPTP_OUTPUT_PNG)
														{
															gchar *new_out_filename = rpt_print_new_numbered_filename (priv->output_filename, npage + 1);
														
															cairo_surface_write_to_png (priv->surface,
																						new_out_filename);
															cairo_surface_destroy (priv->surface);
															cairo_destroy (priv->cr);
														}
													else
														{
															cairo_show_page (priv->cr);
														}
		
													if (priv->output_type == RPTP_OUTPUT_SVG)
														{
															cairo_surface_destroy (priv->surface);
															cairo_destroy (priv->cr);
															fclose (fout);
														}
												}
											else
												{
													/* TO DO */
													g_warning ("Cairo status not sucess: %d", cairo_status (priv->cr));
												}												
										}
									else
										{
											/* TO DO */
											g_warning ("Cairo surface status not sucess");
										}
								}
							else
								{
									/* TO DO */
									g_warning ("Page width or height cannot be zero");
								}
						}
					else
						{
							/* TO DO */
						}
		
					cur = cur->next;
				}
		
			if (priv->cr != NULL)
				{
					cairo_destroy (priv->cr);
				}
			if (priv->output_type != RPTP_OUTPUT_PNG && priv->output_type != RPTP_OUTPUT_SVG)
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
			case PROP_OUTPUT_TYPE:
				rpt_print_set_output_type (rpt_print, g_value_get_int (value));
				break;

			case PROP_OUTPUT_FILENAME:
				rpt_print_set_output_filename (rpt_print, g_value_get_string (value));
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
			case PROP_OUTPUT_TYPE:
				g_value_set_int (value, priv->output_type);
				break;

			case PROP_OUTPUT_FILENAME:
				g_value_set_string (value, priv->output_filename);
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
			priv->width = strtod (prop, NULL);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"height");
	if (prop != NULL)
		{
			priv->height = strtod (prop, NULL);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-top");
	if (prop != NULL)
		{
			priv->margin_top = strtod (prop, NULL);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-right");
	if (prop != NULL)
		{
			priv->margin_right = strtod (prop, NULL);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-bottom");
	if (prop != NULL)
		{
			priv->margin_bottom = strtod (prop, NULL);
		}
	prop = xmlGetProp (xml_page, (const xmlChar *)"margin-left");
	if (prop != NULL)
		{
			priv->margin_left = strtod (prop, NULL);
		}
}

static void
rpt_print_page (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	xmlNode *cur = xnode->children;

	/* clipping region for page's margins */
	cairo_rectangle (priv->cr,
	                 priv->margin_left,
					 priv->margin_top,
					 priv->width - priv->margin_left - priv->margin_right,
					 priv->height - priv->margin_top - priv->margin_bottom);
	cairo_clip (priv->cr);

	while (cur != NULL)
		{
			cairo_save (priv->cr);
			if (strcmp (cur->name, "text") == 0)
				{
					rpt_print_text_xml (rpt_print, cur);
				}
			else if (strcmp (cur->name, "line") == 0)
				{
					rpt_print_line_xml (rpt_print, cur);
				}
			else if (strcmp (cur->name, "rect") == 0)
				{
					rpt_print_rect_xml (rpt_print, cur);
				}
			else if (strcmp (cur->name, "ellipse") == 0)
				{
					rpt_print_ellipse_xml (rpt_print, cur);
				}
			else if (strcmp (cur->name, "image") == 0)
				{
					rpt_print_image_xml (rpt_print, cur);
				}
			cairo_restore (priv->cr);

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

	gchar *text = (gchar *)xmlNodeGetContent (xnode);
	gchar *prop;
	gchar *str_font;

	gdouble padding_top = 0.0;
	gdouble padding_right = 0.0;
	gdouble padding_bottom = 0.0;
	gdouble padding_left = 0.0;

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	rotation = rpt_common_get_rotation (xnode);
	align = rpt_common_get_align (xnode);
	border = rpt_common_get_border (xnode);
	font = rpt_common_get_font (xnode);

	if (position == NULL)
		{
			/* TO DO */
			return;
		}

	/* padding */
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-top");
	if (prop != NULL)
		{
			padding_top = atof (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-right");
	if (prop != NULL)
		{
			padding_right= atof (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-bottom");
	if (prop != NULL)
		{
			padding_bottom= atof (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-left");
	if (prop != NULL)
		{
			padding_left= atof (prop);
		}

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
			pango_layout_set_width (playout, (size->width - padding_left - padding_right) * PANGO_SCALE);
		}

	str_font = g_strdup (font->name);
	if (font->bold)
		{
			str_font = g_strconcat (str_font, " bold", NULL);
		}
	if (font->italic)
		{
			str_font = g_strconcat (str_font, " italic", NULL);
		}
	if (font->size > 0)
		{
			str_font = g_strconcat (str_font, g_strdup_printf (" %f", font->size), NULL);
		}
	else
		{
			str_font = g_strconcat (str_font, " 12", NULL);
		}

	/* creating pango font description */
	pfdesc = pango_font_description_from_string (str_font);
	pango_layout_set_font_description (playout, pfdesc);
	pango_font_description_free (pfdesc);

	/* setting layout attributes */
	if (font->underline != PANGO_UNDERLINE_NONE)
		{
			PangoAttribute *pattr;

			pattr = pango_attr_underline_new (font->underline);
			pattr->start_index = 0;
			pattr->end_index = strlen (text) + 1;

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
			pattr->end_index = strlen (text) + 1;

			if (lpattr == NULL)
				{
					lpattr = pango_attr_list_new ();
				}
			pango_attr_list_insert (lpattr, pattr);
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

			cairo_rectangle (priv->cr, position->x, position->y, size->width, size->height);
			cairo_set_source_rgba (priv->cr, color->r, color->g, color->b, color->a);
			cairo_fill_preserve (priv->cr);
		}

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

	/* TO DO */
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
			                 position->x + padding_left,
			                 position->y + padding_top,
			                 size->width - padding_left - padding_right,
			                 size->height - padding_top - padding_bottom);
			cairo_clip (priv->cr);
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

	cairo_move_to (priv->cr, position->x + padding_left, position->y + padding_top);

	pango_layout_set_text (playout, text, -1);
	pango_cairo_show_layout (priv->cr, playout);

	if (position != NULL && size != NULL)
		{
			cairo_reset_clip (priv->cr);
		}
}

static void
rpt_print_line_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint *position;
	RptPoint *from_p = (RptPoint *)g_malloc0 (sizeof (RptPoint));
	RptPoint *to_p = (RptPoint *)g_malloc0 (sizeof (RptPoint));
	RptSize *size;
	RptRotation *rotation;
	RptStroke *stroke;

	position = rpt_common_get_position (xnode);
	size = rpt_common_get_size (xnode);
	rotation = rpt_common_get_rotation (xnode);
	stroke = rpt_common_get_stroke (xnode);

	from_p->x = position->x;
	from_p->y = position->y;
	to_p->x = position->x + size->width;
	to_p->y = position->y + size->height;

	rpt_print_line (rpt_print, from_p, to_p, stroke, rotation);
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
			return;
		}
	if (stroke == NULL)
		{
			stroke = (RptStroke *)g_malloc0 (sizeof (RptStroke));
			stroke->width = 1.0;
			stroke->color = (RptColor *)g_malloc0 (sizeof (RptColor));
			stroke->color->a = 1.0;
			stroke->style = NULL;
		}

	prop = xmlGetProp (xnode, (const xmlChar *)"fill-color");
	if (prop != NULL)
		{
			fill_color = rpt_common_parse_color (prop);
		}

	if (rotation != NULL)
		{
			rpt_print_rotate (rpt_print, position, size, rotation->angle);
		}

	/* TO DO */
	/*cairo_set_line_width (priv->cr, stroke.width);*/
	cairo_rectangle (priv->cr, position->x, position->y, size->width, size->height);

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

	if (position == NULL || size == NULL)
		{
			return;
		}
	if (stroke == NULL)
		{
			stroke = (RptStroke *)g_malloc0 (sizeof (RptStroke));
			stroke->width = 1.0;
			stroke->color = (RptColor *)g_malloc0 (sizeof (RptColor));
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
	cairo_translate (priv->cr, position->x, position->y);
	cairo_scale (priv->cr, size->width, size->height);
	cairo_arc (priv->cr, 0., 0., 1., 0., 2. * M_PI);
	cairo_restore (priv->cr);
	
	if (prop != NULL && fill_color != NULL)
		{
			cairo_set_source_rgba (priv->cr, fill_color->r, fill_color->g, fill_color->b, fill_color->a);
			cairo_fill_preserve (priv->cr);
		}

	cairo_set_source_rgba (priv->cr, stroke->color->r, stroke->color->g, stroke->color->b, stroke->color->a);
	cairo_stroke (priv->cr);
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
			return;
		}

	adapt = xmlGetProp (xnode, (const xmlChar *)"adapt");
	if (adapt == NULL)
		{
			adapt = "none";
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
					cairo_matrix_scale (&matrix, w / size->width, h / size->height);
				}
			else if (strcmp (adapt, "to-image") == 0)
				{
					size->width = (gdouble)w;
					size->height = (gdouble)h;
				}
		}
	cairo_matrix_translate (&matrix, -position->x, -position->y);
	
	cairo_pattern_set_matrix (pattern, &matrix);
	cairo_set_source (priv->cr, pattern);

	cairo_rectangle (priv->cr, position->x, position->y, size->width, size->height);
	cairo_fill (priv->cr);

	rpt_print_border (rpt_print, position, size, border, rotation);
	
	cairo_pattern_destroy (pattern);
	cairo_surface_destroy (image);
}

static void
rpt_print_line (RptPrint *rpt_print, const RptPoint *from_p, const RptPoint *to_p, const RptStroke *stroke, const RptRotation *rotation)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	if (from_p == NULL || to_p == NULL) return;

	if (stroke != NULL)
		{
			/* TO DO */
			/*cairo_set_line_width (priv->cr, stroke.width);*/
			cairo_set_source_rgba (priv->cr, stroke->color->r, stroke->color->g, stroke->color->b, stroke->color->a);
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
			RptSize size;

			size.width = to_p->x - from_p->x;
			size.height = to_p->y - from_p->y;

			rpt_print_rotate (rpt_print, from_p, &size, rotation->angle);
		}

	cairo_move_to (priv->cr, from_p->x, from_p->y);
	cairo_line_to (priv->cr, to_p->x, to_p->y);
	cairo_stroke (priv->cr);

	if (stroke != NULL && stroke->style != NULL)
		{
			cairo_set_dash (priv->cr, NULL, 0, 0.0);
		}
}

static void
rpt_print_border (RptPrint *rpt_print, const RptPoint *position, const RptSize *size, const RptBorder *border, const RptRotation *rotation)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	if (position == NULL || size == NULL || border == NULL) return;

	RptPoint *from_p = (RptPoint *)g_malloc0 (sizeof (RptPoint));
	RptPoint *to_p = (RptPoint *)g_malloc0 (sizeof (RptPoint));
	RptStroke *stroke = (RptStroke *)g_malloc0 (sizeof (RptStroke));

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
}

static void
rpt_print_gtk_request_page_setup (GtkPrintOperation *operation,
                                  GtkPrintContext *context,
                                  gint page_nr,
                                  GtkPageSetup *setup,
                                  gpointer user_data)
{
	GtkPaperSize *paper_size;

	RptPrint *rpt_print = (RptPrint *)user_data;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	rpt_print_get_xml_page_attributes (rpt_print, priv->pages->nodeTab[page_nr]);
	paper_size = gtk_paper_size_new_custom ("reptool",
	                                        "RepTool",
	                                        priv->width,
	                                        priv->height,
	                                        GTK_UNIT_POINTS);

	gtk_page_setup_set_paper_size (setup, paper_size);
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

	if (priv->width != 0 && priv->height != 0)
		{
			rpt_print_page (rpt_print, priv->pages->nodeTab[page_nr]);
		}
}
