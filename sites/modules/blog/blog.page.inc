<?php
// $Id$

function blog_page($blog){
  if($blog->status == 0){
    dd_set_message(t('blog', '日志状态：未发布'));
  }
  if($GLOBALS['conf']['blog_visit']){
    db_exec('UPDATE {blog} SET visit = visit + 1 WHERE bid = ?', array($blog->bid));
  }
  
  $blog->comment_form = dd_get_form('blog_comment_form', $blog);
  $blog->comment_view = blog_comment_view($blog);
  
  global $conf;
  
  if($conf['blog_rss_node']){
    dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('blog', '订阅最新日志').'" href="'.f('custom.php?module=blog&op=feed&type=node').'" />');
  }
  
  if($conf['blog_rss_comment']){
    dd_set_html_head('<link rel="alternate" type="application/rss+xml" title="'.t('blog', '订阅本文评论').'" href="'.f('custom.php?module=blog&op=feed&type=comment&bid='.$blog->bid).'" />');
  }
  
  dd_set_breadcrumb(array($blog->title));
  dd_add_js(dd_get_path('module', 'blog').'/blog.js');
  dd_set_title(array($blog->title));
  dd_get_highlighter();
  
  return blog_view($blog);
}