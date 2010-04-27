<?php
// $Id$

/**
 * 全站或用户或分类最新日志 rss
 */
function blog_get_feed_node($uid = NULL, $tid = NULL){
  global $conf;
  
  if(!$conf['blog_rss_node']){
    return blog_get_feed(t('blog', 'RSS 已禁用'), t('blog', 'RSS 已禁用'));
  }
  
  $title = '';
  
  if($uid){
    $fetch = db_query('SELECT b.*, u.uid, u.name, a.alias FROM {blog} b INNER JOIN {users} u ON b.uid = u.uid LEFT JOIN {alias} a ON b.bid = a.tid AND a.type = ? WHERE b.uid = ? AND b.status > 0 ORDER BY b.top DESC, b.bid DESC', array('blog', $uid), array('limit' => $conf['blog_rss_node_num']));
    $link = $base_url;
    $title = t('blog', '最新日志');
  }else if($tid){
    $fetch = db_query('SELECT b.*, u.uid, u.name, a.alias FROM {blog} b INNER JOIN {users} u ON b.uid = u.uid INNER JOIN {fields_term_node} t ON b.bid = t.nid LEFT JOIN {alias} a ON b.bid = a.tid AND a.type = ? WHERE b.status > 0 AND t.tid = ? ORDER BY b.top DESC, b.bid DESC', array('blog', $tid), array('limit' => $conf['blog_rss_node_num']));
  }else{
    $fetch = db_query('SELECT b.*, u.uid, u.name, a.alias FROM {blog} b INNER JOIN {users} u ON b.uid = u.uid LEFT JOIN {alias} a ON b.bid = a.tid AND a.type = ? WHERE b.status > 0 ORDER BY b.top DESC, b.bid DESC', array('blog'), array('limit' => $conf['blog_rss_node_num']));
  }
  
  if($fetch){
    foreach($fetch as $blog){
      if(!$title){
        if($tid){
          $term = field_term_load($tid);
          $title = $term->name .'_'. t('blog', '分类');
        }else{
          $link = url('blog/'.$blog->uid);
          $title = t('blog', '!string 的最新日志', array('!string' => $blog->name));
        }
      }
      
    	if(!$blog->alias){
    		$blog->path = url('node/' . $blog->bid, array('absolute' => 1));
    	}else{
    		$blog->path = url($blog->alias, array('absolute' => 1));
    	}
      
      if($conf['blog_rss_node'] == 1){
        $body = $blog->summary;
      }else{
        $body = $blog->body;
      }
      
      $items .= dd_rss_item(
        array(
          'title' => check_plain($blog->title),
          'description' => $body,
          'link' => $blog->path,
          'comments' => $blog->path.'#blog_comment'
        ), 'item',
        array(
            array('key' => 'pubDate', 'value' => gmdate('r', $blog->created)),
            array('key' => 'dc:creator', 'value' => $blog->name),
            array('key' => 'guid', 'value' => $blog->bid . ' at ' . $base_url, 'attributes' => array('isPermaLink' => 'false'))
         )
      );
    }
  }else{
    $items = t('blog', '没有数据');
  }
  
  return blog_get_feed($items, $title, $link);
}

/**
 * 全站或用户或单篇日志的最新评论 rss
 */
function blog_get_feed_comment($bid = NULL, $uid = NULL){
  global $conf;
  if(!$conf['blog_rss_comment']){
    return blog_get_feed(t('blog', 'RSS 已禁用'), t('blog', 'RSS 已禁用'));
  }
  
  $title  = '';
  
  if($bid){
    $fetch = db_query('SELECT c.bid, c.name, c.cid, c.body, u.name AS username, b.title FROM {blog_comment} c INNER JOIN {users} u ON c.uid = u.uid INNER JOIN {blog} b ON c.bid = b.bid WHERE c.bid = ? AND c.status = 1 ORDER BY c.cid DESC', array($bid), array('limit' => $conf['blog_rss_comment_num']));
  }else if($uid){
    $fetch = db_query('SELECT c.bid, c.name, c.cid, c.body, u.name AS username, b.title FROM {blog_comment} c INNER JOIN {users} u ON c.uid = u.uid INNER JOIN {blog} b ON c.bid = b.bid WHERE c.status = 1 AND b.uid = ? ORDER BY c.cid DESC', array($uid), array('limit' => $conf['blog_rss_comment_num']));
  }else{
    $fetch = db_query('SELECT c.bid, c.name, c.cid, c.body, u.name AS username, b.title FROM {blog_comment} c INNER JOIN {users} u ON c.uid = u.uid INNER JOIN {blog} b ON c.bid = b.bid WHERE c.status = 1 ORDER BY c.cid DESC', array($bid), array('limit' => $conf['blog_rss_comment_num']));
    $title = t('blog', '最新评论');
  }
  
  if($fetch){
    foreach($fetch as $o){
      if(!$o->name) $o->name = $o->username;
      
      $path = url('node/'.$o->bid, array('absolute' => 1));
      
      if(!$title){
        if($bid){
          $title = t('blog', '针对 !string 的最新评论', array('!string' => $o->title));
          $link = $path.'#blog_comment';
        }else{
          $title = t('blog', '!string 的最新评论', array('!string' => db_query('SELECT name FROM {users} WHERE uid = ?', array($uid), array('return' => 'column'))));
          $path = url('blog/'.$uid, array('absolute' => 1));
        }
      }
      
      $items .= dd_rss_item(
        array(
          'title' => t('blog', '#日志：!string', array('!string' => check_plain($o->title))),
          'description' => $o->body,
          'link' => $path,
          'comments' => $path.'#blog_comment'
        ), 'item',
        array(
            array('key' => 'pubDate', 'value' => gmdate('r', $o->timestamp)),
            array('key' => 'dc:creator', 'value' => $o->name),
            array('key' => 'guid', 'value' => $o->cid . ' at ' . $base_url, 'attributes' => array('isPermaLink' => 'false'))
         )
      );
    }
  }else{
    $items = t('blog', '没有数据');
  }
  
  return blog_get_feed($items, $title, $link);
}

function blog_get_feed($items, $title, $link = NULL, $des = NULL){
  global $conf, $base_url;
  
  if(strpos($conf['site_global']['logo'], 'http') === false){
    $logo = $base_url . $conf['site_global']['logo'];
  }else{
    $logo = $conf['site_global']['logo'];
  }
  
  return dd_rss_feed($items, NULL, array(
    'title' => $title .'_'. $conf['site_global']['name'],
    'description' => ($des ? $des :$conf['site_global']['description']),
    'link' => ($link ? $link : $base_url),
    'image' => array(
      'url' => $logo,
      'title' => $conf['site_global']['name'],
      'link' => $base_url,
    ),
  ));
}