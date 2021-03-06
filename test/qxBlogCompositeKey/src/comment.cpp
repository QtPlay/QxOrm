#include "../include/precompiled.h"

#include "../include/comment.h"
#include "../include/blog.h"

#include <QxMemLeak.h>

QX_REGISTER_CPP_QX_BLOG(comment)

namespace qx {
template <> void register_class(QxClass<comment> & t)
{
   t.id(& comment::m_id, comment::str_composite_key());

   t.data(& comment::m_text, "comment_text");
   t.data(& comment::m_dt_create, "date_creation");

   t.relationManyToOne(& comment::m_blog, blog::str_composite_key());
}}
