<?php
// $Id$


function blog_admin_tabs(){
  dd_set_tabs(array(
    l(t('blog', '创建日志'), 'admin/blogs/add'),
    l(t('blog', '日志管理'), 'admin/blogs'),
    l(t('blog', '评论管理'), 'admin/blogs/comments'),
    l(t('blog', '日志设置'), 'admin/blogs/settings'),
    l(t('blog', '草稿'), 'admin/blogs/draft'),
  ));
}

/**
 * 日志管理列表
 */
function blog_admin_lists(){
  blog_admin_tabs();
  
  if(user_access('blog all updated')){
    $fetch = pager_query('SELECT b.*, u.name, a.alias FROM {blog} b INNER JOIN {users} u ON b.uid = u.uid LEFT JOIN {alias} a ON b.bid = a.tid AND a.type = ? ORDER BY b.bid DESC', 20, 0, NULL, array('blog'));
  }else{
    $fetch = pager_query('SELECT b.*, u.name, a.alias FROM {blog} b INNER JOIN {users} u ON b.uid = u.uid LEFT JOIN {alias} a ON b.bid = a.tid AND a.type = ? AND uid = ? ORDER BY b.bid DESC', 20, 0, NULL, array('blog', $GLOBALS['user']->uid));
  }
  
  if($fetch){
    foreach($fetch as $o){
      $table[] = array(
        l($o->title, ($o->alias ? $o->alias : 'node/'.$o->bid), array('attributes' => array('target' => '_blank'))), $o->name,
        blog_get_status($o->status),
        format_date($o->created),
        $o->updated ? format_date($o->updated) : 'NONE',
        dd_imp(array(
          l('编辑', 'admin/blogs/edit/'.$o->bid, dd_redirect()),
          l('删除', 'admin/blogs/delete/'.$o->bid, array('attributes' => array('class' => 'confirm')))
        ))
      );
    }
    $output .= theme('table', array('主题', '作者', '状态', '创建时间', '更新时间', '操作'), $table, array('id' => 'blog_admin_lists'));
    $output .= pager_view();
  }else{
    $output = system_no_content();
  }
  return $output;
}

/**
 * 评论管理列表
 */
function blog_admin_comments(){
  switch($_GET['op']){
    case 'status':
      $return = 0;
      if($_GET['bid'] && $_GET['cid'] && ($_GET['status'] == 1 || $_GET['status'] == 0)){
        if(user_access('blog all updated')){
          if(db_exec('UPDATE {blog_comment} SET status = ? WHERE cid = ?', array($_GET['status'], $_GET['cid']))){
            $return = 1;
            blog_comment_count($_GET['bid']);
          }
        }else{
          if(db_query('SELECT bid FROM {blog} WHERE bid = ? AND uid = ?', array($_GET['bid'], $GLOBALS['user']->uid), array('return' => 'column')) && db_exec('UPDATE {blog_comment} SET status = ? WHERE cid = ? AND bid', array($_GET['status'], $_GET['cid']))){
            $return = 1;
            blog_comment_count($_GET['bid']);
          }
        }
      }
    echo $return;
    exit;
    case 'deleted':
      $return = 0;
      if($_GET['cid'] && $_GET['bid']){
        if(user_access('blog all updated')){
          if(db_exec('DELETE FROM {blog_comment} WHERE cid = ?', array($_GET['cid']))){
            $return = $_GET['cid'];
            blog_comment_count($_GET['bid']);
          }
        }else if($_GET['bid']){
          if(db_query('SELECT bid FROM {blog} WHERE bid = ? AND uid = ?', array($_GET['bid'], $GLOBALS['user']->uid), array('return' => 'column')) && db_exec('DELETE FROM {blog_comment} WHERE cid = ? AND bid', $_GET['cid'])){
            $return = $_GET['cid'];
            blog_comment_count($_GET['bid']);
          }
        }
      }
    echo '"'.$return.'"';
    exit;
  }
  blog_admin_tabs();
  
  $header = array(
    array('data' => t('blog', '日志'), 'field' => 'c.bid'),
    array('data' => t('blog', '时间'), 'field' => 'c.cid', 'sort' => 'desc'),
    array('data' => t('blog', '状态'), 'field' => 'c.status'),
    array('data' => t('blog', '类型'), 'field' => 'c.type')
  );
  
  if(user_access('blog all updated')){
    $fetch = pager_query('SELECT b.title, u.mail AS usermail, u.name AS username, c.* FROM {blog_comment} c INNER JOIN {blog} b ON c.bid = b.bid INNER JOIN {users} u ON c.uid = u.uid'.table_sql($header), 10);
  }else{
    $fetch = pager_query('SELECT b.title, u.name, u.account, c.* FROM {blog_comment} c INNER JOIN {blog} b ON c.bid = b.bid INNER JOIN {users} u ON c.uid = u.uid WHERE b.bid = ?'.table_sql($header), 10, 0, NULL, array($GLOBALS['user']->uid));
  }
  
  if($fetch){
    $type = array(t('blog', '评论'), t('blog', '通告'));
    foreach($fetch as $o){
      if(!$o->name){
        $o->name = $o->username;
      }
      
      if($o->status){
        $status = l(t('blog', '正常'), 'admin/blogs/comments', array('query' => 'op=status&cid='.$o->cid.'&bid='.$o->bid.'&status=0', 'attributes' => array('title' => t('blog', '点此修改为待审核'), 'replace' => t('blog', '待审核'), 'class' => 'confirmajax')));
      }else{
        $status = l(t('blog', '待审核'), 'admin/blogs/comments', array('query' => 'op=status&cid='.$o->cid.'&bid='.$o->bid.'&status=1' , 'attributes' => array('title' => t('blog', '点此通过审核'), 'replace' => t('blog', '正常'), 'class' => 'confirmajax')));
      }
      
      $items = array();
      $items[] = dd_get_strong(t('blog', '昵称')) . user_name($o);
      if($o->uid){
        $items[] = dd_get_strong(t('blog', '邮箱')). $o->usermail;
      }else if($o->mail){
        $items[] = dd_get_strong(t('blog', '邮箱')). $o->mail;
      }
      if($o->site){
        $items[] = dd_get_strong(t('blog', '主页')). l($o->site, $o->site, array('attributes' => array('target' => '_blank')));
      }
      
      $items[] = dd_get_strong(t('blog', '摘要')). mb_substr(strip_tags($o->body), 0, 200, 'utf-8') .'<span class="blog_admin_op_link">'. l(t('blog', '浏览'), 'node/'.$o->bid, array('fragment' => 'blog_comment')) . l(t('blog', '删除'), 'admin/blogs/comments', array('query' => 'op=deleted&cid='.$o->cid.'&bid='.$o->bid, 'fragment' => 'blog_comment', 'attributes' => array('class' => 'confirmajax', 'fun' => 'blog_comment_delete'))).'</span>';
      
      $table[] = array(
        'data' => array(
          array('data' => l($o->title, 'node/'.$o->bid), 'class' => 'blog_admin_comment_title'),
          array('data' => format_date($o->timestamp), 'class' => 'blog_admin_comment_date'),
          array('data' => $status, 'class' => 'blog_admin_comment_status'),
          array('data' => $type[$o->type], 'class' => 'blog_admin_comment_type')
        ),
        'class' => 'blog_admin_table_tr_'.$o->cid
      );
      
      $table[] = array(
        'data' => array(array(
          'class' => 'blog_admin_comment_body',
          'data' => theme('item_list', $items),
          'colspan' => 4
        )),
        'class' => 'blog_admin_table_tr_'.$o->cid
      );
    }
    
    $output = theme('table', $header, $table, array('id' => 'blog_admin_comment_list'));
    $output .= pager_view();
    dd_add_js(dd_get_path('module', 'blog').'/blog.admin.js');
  }
  return $output;
}

/**
 * 删除日志
 */
function blog_node_deleted($blog){
  if(db_exec('DELETE FROM {blog} WHERE bid = ?', array($blog->bid))){
    module_invoke_all('blog_deleted', $blog);
    dd_set_message(t('blog', '%string 已删除', array('%string' => $blog->title)));
  }else{
    dd_set_message(t('blog', '%string 删除失败', array('%string' => $blog->title)));
  }
  dd_goto_redirect(url('admin/blogs'));
}


function blog_admin_drafts(){
  global $user;
  if($_GET['op'] == 'delete'){
    if(user_access('blog all deleted')){
      if(db_exec('DELETE FROM {temp} WHERE tid = ? AND type = ?', array($_GET['tid'], 'autoSaveBlog'))){
        echo 1;
      }
    }else if(db_exec('DELETE FROM {temp} WHERE tid = ? AND type = ? AND uid = ?', array($_GET['tid'], 'autoSaveBlog', $GLOBALS['user']->uid))){
      echo 1;
    }
    exit;
  }
  
  dd_set_tabs(array(l('日志', 'admin/blogs'), l('草稿', 'admin/blogs/draft')));
  
  if(user_access('blog all updated')){
    $fetch = pager_query('SELECT t.tid, t.data, t.timestamp, u.name FROM {temp} t INNER JOIN {users} u ON t.uid = u.uid WHERE t.type = ? ORDER BY t.timestamp DESC', 20, 0, NULL, array('autoSaveBlog'));
  }else{
    $fetch = pager_query('SELECT t.tid, t.data, t.timestamp, u.name FROM {temp} t INNER JOIN {users} u ON t.uid = u.uid WHERE t.type = ? AND t.uid = ? ORDER BY t.timestamp DESC', 20, 0, NULL, array('autoSaveBlog', $GLOBALS['user']->uid));
  }
  if($fetch){
    foreach($fetch as $o){
      $data = unserialize($o->data);
      $table[] = array(
        $data['title'] ? check_plain($data['title']) : '无主题', $o->name, format_date($o->timestamp),
        dd_imp(array(
          l('编辑', 'admin/blogs/add', array('query' => 'temp=' . $o->tid)),
          l('删除', 'admin/blogs/draft', array('query' => 'op=delete&tid=' . $o->tid, 'attributes' => array('class' => 'confirmajax')))
        ))
      );
    }
    $output .= theme('table', array('标题', '作者', '更新时间', '操作'), $table, array('id' => 'blog_admin_lists'));
  }
  return $output;
}

/**
 * 日志设置
 */
function blog_admin_settings(){
  blog_admin_tabs();
  $form['fields']['blog_autosave'] = array(
    '#type' => 'radio',
    '#title' => t('blog', '自动保存'),
    '#required' => true,
    '#default_value' => var_get('blog_autosave', 1),
    '#options' => array(t('blog', '关闭'), t('blog', '开启')),
    '#description' => t('blog', '日志自动保存可减少浏览器崩溃等意外造成的损失'),
  );
  $form['fields']['blog_autosave_time'] = array(
    '#type' => 'textfield',
    '#title' => t('blog', '间隔时间'),
    '#default_value' => var_get('blog_autosave_time', 60000),
    '#description' => t('blog','两次保存间隔不低于10000（即 10 秒），不大于 3600000（即 1 小时）'),
  );
  $form['fields']['blog_visit'] = array(
    '#type' => 'radio',
    '#title' => t('blog', '浏览统计'),
    '#required' => true,
    '#default_value' => var_get('blog_visit', 0),
    '#options' => array(t('blog', '关闭'), t('blog', '开启')),
    '#description' => t('blog', '记录日志访问次数'),
  );
  $form['fields']['blog_trackback'] = array(
    '#title' => t('blog', '日志通告'),
    '#type' => 'radio',
    '#default_value' => var_get('blog_trackback', 0),
    '#required' => 1,
    '#options' => array(
      t('blog', '不允许通告'),
      t('blog', '允许通告'),
    )
  );
  $form['fields']['blog_trackback_verify'] = array(
    '#title' => t('blog', '通告审核'),
    '#type' => 'radio',
    '#required' => 1,
    '#default_value' => var_get('blog_trackback_verify', 2),
    '#options' => array(
      t('blog', '所有通告均不需要'),
      t('blog', '曾通过审核的网站不需要'),
      t('blog', '所有通告均需要'),
    )
  );
  $form['fields']['blog_comment'] = array(
    '#title' => t('blog', '评论设置'),
    '#type' => 'radio',
    '#default_value' => var_get('blog_comment', 1),
    '#required' => 1,
    '#options' => array(
      t('blog', '禁止评论'),
      t('blog', '所有人可评'),
      t('blog', '登录用户可评'),
    )
  );
  
  $form['fields']['blog_comment_pager_count'] = array(
    '#title' => t('blog', '评论分页'),
    '#type' => 'textfield',
    '#default_value' => var_get('blog_comment_pager_count', 20),
    '#required' => 1,
    '#description' => t('blog', '每页显示评论数目')
  );
  
  $form['fields']['blog_comment_pager_order'] = array(
    '#title' => t('blog', '评论排列'),
    '#type' => 'radio',
    '#default_value' => var_get('blog_comment_pager_order', 0),
    '#required' => 1,
    '#options' => array(
      t('blog', '旧的优先'),
      t('blog', '新的优先'),
    )
  );
  
  $form['fields']['blog_comment_format'] = filter_form_field(1, 1, t('blog', '评论默认输入格式'));
  
  $form['fields']['blog_comment_verify'] = array(
    '#title' => t('blog', '评论审核'),
    '#type' => 'radio',
    '#required' => 1,
    '#default_value' => var_get('blog_comment_verify', 1),
    '#options' => array(
      t('blog', '所有评论均不审核'),
      t('blog', '游客评论需审核'),
      t('blog', '所有评论均审核'),
    )
  );
  $form['fields']['blog_is_anonymous'] = array(
    '#title' => t('blog', '游客评论'),
    '#type' => 'checkbox',
    '#default_value' => var_get('blog_is_anonymous'),
    '#options' => array(
      1 => t('blog', '必须输入昵称'),
      2 => t('blog', '必须输入邮箱'),
      3 => t('blog', '必须输入主页'),
    )
  );
  $form['fields']['blog_anonymous_name'] = array(
    '#type' => 'textfield',
    '#title' => t('blog', '默认昵称'),
    '#required' => 1,
    '#default_value' => var_get('blog_anonymous_name'),
    '#description' => t('blog','以游客身份评论时默认昵称'),
  );
  $form['fields']['blog_summary_num'] = array(
    '#title' => t('blog', '摘要长度'),
    '#type' => 'textfield',
    '#default_value' => var_get('blog_summary_num', 500),
    '#required' => 1,
  );
  $form['fields']['blog_rss_node'] = array(
    '#title' => t('blog', '日志RSS'),
    '#type' => 'radio',
    '#default_value' => var_get('blog_rss', 1),
    '#required' => 1,
    '#options' => array(
      t('blog', '禁用'),
      t('blog', '输出摘要'),
      t('blog', '输出全文'),
    )
  );
  $form['fields']['blog_rss_comment'] = array(
    '#title' => t('blog', '评论RSS'),
    '#type' => 'radio',
    '#default_value' => var_get('blog_rss', 1),
    '#required' => 1,
    '#options' => array(
      t('blog', '禁用'),
      t('blog', '输出'),
    )
  );
  $form['fields']['blog_rss_node_num'] = array(
    '#title' => t('blog', '日志RSS条目'),
    '#type' => 'textfield',
    '#default_value' => var_get('blog_rss_node_num', 10),
    '#description' => t('blog', '最新日志 RSS 输出几条'),
    '#required' => 1,
  );
  $form['fields']['blog_rss_comment_num'] = array(
    '#title' => t('blog', '评论RSS条目'),
    '#type' => 'textfield',
    '#default_value' => var_get('blog_rss_comment_num', 10),
    '#description' => t('blog', '最新评论 RSS 输出几条'),
    '#required' => 1,
  );
  $form['fields']['blog_front_count'] = array(
    '#type' => 'textfield',
    '#title' => t('blog', '首页日志'),
    '#required' => true,
    '#default_value' => var_get('blog_front_count', 10),
    '#description' => t('blog', '每页显示数目，0 为不显示'),
  );
  $form['fields']['blog_front_style'] = array(
    '#type' => 'radio',
    '#title' => t('blog', '显示方式'),
    '#required' => true,
    '#default_value' => var_get('blog_front_style', 0),
    '#options' => array(t('blog', '摘要'), t('blog', '标题')),
    '#description' => t('blog', '在首页显示日志的方式'),
  );
  $form['settings'] = array(
    '#validate' => array('blog_admin_settings_validate')
  );
  return system_setting_form($form);
}

function blog_admin_settings_validate($form, $v){
  if($v['blog_autosave'] && !$v['blog_autosave_time']){
    $t['blog_autosave_time']['#error'][] = t('blog', '请设置保存间隔时间');
  }else if(preg_match('/[^0-9]/', $v['blog_autosave_time']) || $v['blog_autosave_time'] < 10000 || $v['blog_autosave_time'] > 3600000){
    $t['blog_autosave_time']['#error'][] = t('blog','间隔时间不正确');
  }
  return $t;
}

/**
 * 日志字段
 */
function blog_admin_fields(){
	$tabs = array(
		l(t('blog', '字段管理'), 'admin/blogs/fields'),
		l(t('blog', '添加字段'), 'admin/blogs/fields/add'),
    l(t('blog', '日志管理'), 'admin/blogs'),
	);
  $args = arg();
  switch($args[3]){
  	case 'add':
      $field->type = 'field';
      $field->module = 'blog';
      $field->redirect = url('admin/blogs/fields');
  		$output = dd_get_form('field_form', $field);
  	break;
  	default:
  		$output = field_lists('blog', 'field');
  }
  
  dd_set_tabs($tabs);
  return $output;
}