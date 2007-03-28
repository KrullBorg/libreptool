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

#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>
#include <pango/pangocairo.h>

#include "rptprint.h"
#include "rptcommon.h"

enum
{
	PROP_0
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
                            RptPoint from_p,
                            RptPoint to_p,
                            RptStroke stroke);
static void rpt_print_border (RptPrint *rpt_print,
                              RptPoint position,
                              RptSize size,
                              RptBorder border);


#define RPT_PRINT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_RPT_PRINT, RptPrintPrivate))

typedef struct _RptPrintPrivate RptPrintPrivate;
struct _RptPrintPrivate
	{
		gint width;
		gint height;
		cairo_surface_t *surface;
		cairo_t *cr;
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
}

static void
rpt_print_init (RptPrint *rpt_print)
{
}

/**
 * rpt_print_new_from_xml:
 * @xdoc: an #xmlDoc.
 * @output_type:
 * @out_filename:
 *
 * Returns: the newly created #RptPrint object.
 */
RptPrint
*rpt_print_new_from_xml (xmlDoc *xdoc, RptPrintOutputType output_type, const gchar *out_filename)
{
	RptPrint *rpt_print = NULL;

	xmlNode *cur = xmlDocGetRootElement (xdoc);
	if (cur != NULL)
		{
			if (strcmp (cur->name, "reptool_report") == 0)
				{
					FILE *fout;
					RptPrintPrivate *priv;

					gint npage = 0;
				
					rpt_print = RPT_PRINT (g_object_new (rpt_print_get_type (), NULL));

					priv = RPT_PRINT_GET_PRIVATE (rpt_print);

					if (output_type != RPTP_OUTPUT_PNG)
						{
							fout = fopen (out_filename, "w");
							if (fout == NULL)
								{
									/* TO DO */
									return NULL;
								}
						}

					cur = cur->children;
					while (cur != NULL)
						{
							if (strcmp (cur->name, "page") == 0)
								{
									npage++;

									priv->width = atol (xmlGetProp (cur, (const xmlChar *)"width"));
									priv->height = atol (xmlGetProp (cur, (const xmlChar *)"height"));

									if (priv->width != 0 && priv->height != 0)
										{
											if (npage == 1)
												{
													switch (output_type)
														{
															case RPTP_OUTPUT_PNG:
																priv->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, priv->width, priv->height);
																break;
														
															case RPTP_OUTPUT_PDF:
																priv->surface = cairo_pdf_surface_create (out_filename, (double)priv->width, (double)priv->height);
																break;
		
															case RPTP_OUTPUT_PS:
																priv->surface = cairo_ps_surface_create (out_filename, (double)priv->width, (double)priv->height);
																break;
		
															case RPTP_OUTPUT_SVG:
																priv->surface = cairo_svg_surface_create (out_filename, (double)priv->width, (double)priv->height);
																break;
														}
												}

											if (cairo_surface_status (priv->surface) == CAIRO_STATUS_SUCCESS)
												{
													if (npage == 1)
														{
															priv->cr = cairo_create (priv->surface);
														}
													else
														{
															/* TO DO */
														}

													if (npage == 1 && output_type != RPTP_OUTPUT_PNG)
														{
															cairo_surface_destroy (priv->surface);
														}

													if (cairo_status (priv->cr) ==  CAIRO_STATUS_SUCCESS)
														{
															rpt_print_page (rpt_print, cur);
															if (output_type == RPTP_OUTPUT_PNG)
																{
																	cairo_surface_write_to_png (priv->surface, out_filename);
																	cairo_surface_destroy (priv->surface);
																}

															cairo_show_page (priv->cr);
														}
													else
														{
															/* TO DO */
														}												
												}
											else
												{
													/* TO DO */
												}
										}
									else
										{
											/* TO DO */
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
					if (output_type != RPTP_OUTPUT_PNG)
						{
							fclose (fout);
						}
				}
			else
				{
					/* TO DO */
				}
		}

	return rpt_print;
}

/**
 * rpt_print_new_from_file:
 * @filename: the path of the xml file to load.
 * @output_type:
 * @out_filename:
 *
 * Returns: the newly created #RptPrint object.
 */
RptPrint
*rpt_print_new_from_file (const gchar *filename, RptPrintOutputType output_type, const gchar *out_filename)
{
	RptPrint *rpt_print = NULL;

	xmlDoc *xdoc = xmlParseFile (filename);
	if (xdoc != NULL)
		{
			rpt_print = rpt_print_new_from_xml (xdoc, output_type, out_filename);
		}

	return rpt_print;
}

static void
rpt_print_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	RptPrint *rpt_print = RPT_PRINT (object);

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	switch (property_id)
		{
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
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
rpt_print_page (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	xmlNode *cur = xnode->children;

	while (cur != NULL)
		{
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

			cur = cur->next;
		}
}

static void
rpt_print_text_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	RptPoint position;
	RptSize size;
	RptAlign align;
	RptBorder border;
	RptFont font;
	RptColor color;

	PangoLayout *playout;
	PangoFontDescription *pfdesc;

	gchar *text = (gchar *)xmlNodeGetContent (xnode),
	      *prop,
	      *str_font;

	gdouble pad_top,
	        pad_right,
	        pad_bottom,
	        pad_left;

	rpt_common_get_position (xnode, &position);
	rpt_common_get_size (xnode, &size);
	rpt_common_get_align (xnode, &align);
	rpt_common_get_border (xnode, &border);
	rpt_common_get_font (xnode, &font);

	/* padding */
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-top");
	if (prop != NULL)
		{
			pad_top = atof (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-right");
	if (prop != NULL)
		{
			pad_right= atof (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-bottom");
	if (prop != NULL)
		{
			pad_bottom= atof (prop);
		}
	prop = xmlGetProp (xnode, (const xmlChar *)"padding-left");
	if (prop != NULL)
		{
			pad_left= atof (prop);
		}

	/* creating pango layout */
	playout = pango_cairo_create_layout (priv->cr);
	pango_layout_set_width (playout, size.width * PANGO_SCALE);

	str_font = g_strdup (font.name);
	if (font.bold)
		{
			str_font = g_strjoin (NULL, str_font, " bold", NULL);
		}
	if (font.italic)
		{
			str_font = g_strjoin (NULL, str_font, " italic", NULL);
		}
	/*if (font.underline)
		{
		}*/
	/*if (font.strike)
		{
		}*/
	if (font.size > 0)
		{
			str_font = g_strjoin (NULL, str_font, g_strdup_printf (" %f", font.size), NULL);
		}
	else
		{
			str_font = g_strjoin (NULL, str_font, " 12", NULL);
		}

	/* creating pango font description */
	pfdesc = pango_font_description_from_string (str_font);
	pango_layout_set_font_description (playout, pfdesc);
	pango_font_description_free (pfdesc);

	/* background */
	prop = xmlGetProp (xnode, (const xmlChar *)"background-color");
	if (prop != NULL)
		{
			rpt_common_parse_color (prop, &color);
		}

	/* drawing border */
	rpt_print_border (rpt_print, position, size, border);

	/* setting alignment */
	switch (align.h_align)
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

	switch (align.v_align)
		{
	 		case RPT_VALIGN_TOP:
	 			break;

	 		case RPT_VALIGN_CENTER:
	 			break;

	 		case RPT_VALIGN_BOTTOM:
	 			break;
		}

	/* setting clipping region */
	cairo_rectangle (priv->cr, position.x, position.y, size.width, size.height);
	cairo_clip (priv->cr);

	/* drawing text */
	cairo_set_source_rgba (priv->cr, font.color.r, font.color.g, font.color.b, font.color.a);
	cairo_move_to (priv->cr, position.x, position.y);
	pango_layout_set_text (playout, text, -1);
	pango_cairo_show_layout (priv->cr, playout);

	cairo_reset_clip (priv->cr);
}

static void
rpt_print_line_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint position,
	         from_p,
	         to_p;
	RptSize size;
	RptStroke stroke;

	rpt_common_get_position (xnode, &position);
	rpt_common_get_size (xnode, &size);
	rpt_common_get_stroke (xnode, &stroke);

	from_p.x = position.x;
	from_p.y = position.y;
	to_p.x = position.x + size.width;
	to_p.y = position.y + size.height;

	rpt_print_line (rpt_print, from_p, to_p, stroke);
}

static void
rpt_print_rect_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint position,
	         from_p,
	         to_p;
	RptSize size;
	RptStroke stroke;
	RptColor fill_color;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	rpt_common_get_position (xnode, &position);
	rpt_common_get_size (xnode, &size);
	rpt_common_get_stroke (xnode, &stroke);

	gchar *prop = xmlGetProp (xnode, (const xmlChar *)"fill-color");
	if (prop != NULL)
		{
			fill_color.r = 0.0;
			fill_color.g = 0.0;
			fill_color.b = 0.0;
			fill_color.a = 1.0;
			rpt_common_parse_color (prop, &fill_color);
		}

	/*cairo_set_line_width (priv->cr, stroke.width);*/
	cairo_rectangle (priv->cr, position.x, position.y, size.width, size.height);

	if (prop != NULL)
		{
			cairo_set_source_rgba (priv->cr, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
			cairo_fill_preserve (priv->cr);
		}

	cairo_set_source_rgba (priv->cr, stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
	cairo_stroke (priv->cr);
}

static void
rpt_print_ellipse_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint position;
	RptSize size;
	RptStroke stroke;
	RptColor fill_color;

	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	rpt_common_get_position (xnode, &position);
	rpt_common_get_size (xnode, &size);
	rpt_common_get_stroke (xnode, &stroke);

	gchar *prop = xmlGetProp (xnode, (const xmlChar *)"fill-color");
	if (prop != NULL)
		{
			fill_color.r = 0.0;
			fill_color.g = 0.0;
			fill_color.b = 0.0;
			fill_color.a = 1.0;
			rpt_common_parse_color (prop, &fill_color);
		}

	cairo_new_path (priv->cr);

	cairo_save (priv->cr);
	cairo_translate (priv->cr, position.x, position.y);
	cairo_scale (priv->cr, size.width, size.height);
	cairo_arc (priv->cr, 0., 0., 1., 0., 2. * M_PI);
	cairo_restore (priv->cr);
	
	if (prop != NULL)
		{
			cairo_set_source_rgba (priv->cr, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
			cairo_fill_preserve (priv->cr);
		}

	cairo_set_source_rgba (priv->cr, stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
	cairo_stroke (priv->cr);
}

static void
rpt_print_image_xml (RptPrint *rpt_print, xmlNode *xnode)
{
	RptPoint position;
	RptSize size;
	RptBorder border;

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

	rpt_common_get_position (xnode, &position);
	rpt_common_get_size (xnode, &size);
	rpt_common_get_border (xnode, &border);

	image = cairo_image_surface_create_from_png (filename);

	pattern = cairo_pattern_create_for_surface (image);

	cairo_matrix_init_identity (&matrix);
	if (strcmp (adapt, "none") != 0)
		{
			w = cairo_image_surface_get_width (image);
			h = cairo_image_surface_get_height (image);

			if (strcmp (adapt, "to-box") == 0)
				{
					cairo_matrix_scale (&matrix, w / size.width, h / size.height);
				}
			else if (strcmp (adapt, "to-image") == 0)
				{
					size.width = (gdouble)w;
					size.height = (gdouble)h;
				}
		}
	cairo_matrix_translate (&matrix, -position.x, -position.y);
	
	cairo_pattern_set_matrix (pattern, &matrix);
	cairo_set_source (priv->cr, pattern);

	cairo_rectangle (priv->cr, position.x, position.y, size.width, size.height);
	cairo_fill (priv->cr);

	rpt_print_border (rpt_print, position, size, border);
	
	cairo_pattern_destroy (pattern);
	cairo_surface_destroy (image);
}

static void
rpt_print_line (RptPrint *rpt_print, RptPoint from_p, RptPoint to_p, RptStroke stroke)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	/*cairo_set_line_width (priv->cr, stroke.width);*/
	cairo_set_source_rgba (priv->cr, stroke.color.r, stroke.color.g, stroke.color.b, stroke.color.a);
	cairo_move_to (priv->cr, from_p.x, from_p.y);
	cairo_line_to (priv->cr, to_p.x, to_p.y);
	cairo_stroke (priv->cr);
}

static void
rpt_print_border (RptPrint *rpt_print, RptPoint position, RptSize size, RptBorder border)
{
	RptPrintPrivate *priv = RPT_PRINT_GET_PRIVATE (rpt_print);

	RptPoint from_p,
	         to_p;
	RptStroke stroke;

	if (border.top_width != 0.0)
		{
			from_p.x = position.x;
			from_p.y = position.y;
			to_p.x = position.x + size.width;
			to_p.y = position.y;
			stroke.width = border.top_width;
			stroke.color = border.top_color;
			rpt_print_line (rpt_print, from_p, to_p, stroke);
		}
	if (border.right_width != 0.0)
		{
			from_p.x = position.x + size.width;
			from_p.y = position.y;
			to_p.x = position.x + size.width;
			to_p.y = position.y + size.height;
			stroke.width = border.right_width;
			stroke.color = border.right_color;
			rpt_print_line (rpt_print, from_p, to_p, stroke);
		}
	if (border.bottom_width != 0.0)
		{
			from_p.x = position.x;
			from_p.y = position.y + size.height;
			to_p.x = position.x + size.width;
			to_p.y = position.y + size.height;
			stroke.width = border.bottom_width;
			stroke.color = border.bottom_color;
			rpt_print_line (rpt_print, from_p, to_p, stroke);
		}
	if (border.left_width != 0.0)
		{
			from_p.x = position.x;
			from_p.y = position.y;
			to_p.x = position.x;
			to_p.y = position.y + size.height;
			stroke.width = border.left_width;
			stroke.color = border.left_color;
			rpt_print_line (rpt_print, from_p, to_p, stroke);
		}
}
