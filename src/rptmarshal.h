
#ifndef ___rpt_marshal_MARSHAL_H__
#define ___rpt_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* STRING:STRING,POINTER,INT,POINTER,POINTER (reptool_marshal.list:1) */
extern void _rpt_marshal_STRING__STRING_POINTER_INT_POINTER_POINTER (GClosure     *closure,
                                                                     GValue       *return_value,
                                                                     guint         n_param_values,
                                                                     const GValue *param_values,
                                                                     gpointer      invocation_hint,
                                                                     gpointer      marshal_data);

G_END_DECLS

#endif /* ___rpt_marshal_MARSHAL_H__ */

