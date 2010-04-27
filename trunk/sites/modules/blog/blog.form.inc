<?php
// $Id$

/**
 * 发布、更新日志表单
 */
function blog_node_form($blog = NULL){
  $tabs = array(l(t('blog', '创建日志'), 'admin/blogs/add'), l(t('blog', '日志列表'), 'admin/blogs'));
  
  if(!$blog->bid){
    global $user, $conf;
    if($conf['blog_autosave']){
      $autosave = url('tools/autosave/', array('query' => 'op=blog&type=insert'));
    }
    if($_GET['temp'] && $temp = temp_get($_GET['temp'])){
      if(user_access('admin blog edit') || $user->uid == $temp->uid){
        $form['#form_token'] = $_GET['temp'];
        $blog = (object)$temp->data;
        $pass = $blog->pass;
      }else{
        unset($temp);
      }
    }
    $pass_des = t('blog', '密码长度介于 4 - 64 字符之间');
  }else{
    $tabs[] = l(t('blog', '编辑日志'), $_GET['q']);
    
    $title_des = l(t('blog', '查看'), $blog->path, array('attributes' => array('target' => '_blank')));
    $pass_des = t('blog', '密码长度介于 4 - 64 字符之间，不修改请留空');
    $form['fields']['nid'] = $form['fields']['bid'] = array(
      '#type' => 'hidden',
      '#default_value' => $blog->bid,
      '#constant' => 1
    );
  }
  $tabs[] = l(t('blog', '草稿'), 'admin/blogs/draft');
  dd_set_tabs($tabs);
  
  $form['settings'] = array(
    '#ajax_validate' => 1,
    '#validate' => array('blog_node_form_validate'),
    '#redirect' => url('admin/blogs'),
  );
  
  $form['fields']['title'] = array(
    '#title' => t('blog', '标题'),
    '#type' => 'textfield',
    '#default_value' => $blog->title,
    '#required' => 1,
    '#weight' => -10,
    '#description' => $title_des,
    '#attributes' => array('size' => 100)
  );
  
  $blog->body = preg_replace_callback('/(<pre.*?>)(.*?)(<\/pre>)/ms', '_blog_pre_plain', $blog->body);
  
  $form['fields']['body'] = array(
    '#title' => t('blog', '内容'),
    '#type' => 'textarea',
    '#default_value' => $blog->body,
    '#attributes' => array(
      'cols' => 80,
      'rows' => 20,
      'id' => 'body'
    ),
  );
  $form['fields']['get_down_image'] = array(
    '#type' => 'checkbox',
    '#title' => t('blog', '保存远程图片'),
    '#description' => t('blog', '将内容中的远程图片保存到本地服务器')
  );
  $form['fields']['summary'] = array(
    '#fieldset_prefix' => 'desc',
    '#fieldset_legend' => t('field', '摘要'),
    '#fieldset_suffix' => 1,
    '#fieldset' => true,
    '#type' => 'textarea',
    '#description' => t('blog', '有助于读者更好的理解阅读，长度不超过 2000 字符，若留空将自动从正文中截取'),
    '#default_value' => $blog->summary,
    '#attributes' => array(
      'cols' => 80,
      'rows' => 4,
      'id' => 'summary'
    ),
  );
  $form['fields']['info'] = array(
    '#weight' => 99,
    '#fieldset_prefix' => 'asc',
    '#fieldset_legend' => t('field', '日志设置'),
    '#fieldset_suffix' => 1,
    '#fieldset' => true
  );
  $form['fields']['info']['top'] = array(
    '#title' => t('blog', '置顶'),
    '#type' => 'radio',
    '#default_value' => isset($blog->top) ? $blog->top : 0,
    '#required' => 1,
    '#options' => array(
      t('blog', '否'),
      t('blog', '是'),
    )
  );
  $form['fields']['info']['trackback'] = array(
    '#title' => t('blog', '通告'),
    '#type' => 'radio',
    '#default_value' => isset($blog->trackback) ? $blog->trackback : 0,
    '#required' => 1,
    '#options' => array(
      t('blog', '默认设置'),
      t('blog', '允许通告'),
      t('blog', '不允许通告'),
    )
  );
  $form['fields']['info']['comment'] = array(
    '#title' => t('blog', '评论'),
    '#type' => 'radio',
    '#default_value' => isset($blog->comment) ? $blog->comment : 0,
    '#required' => 1,
    '#options' => array(
      t('blog', '默认设置'),
      t('blog', '禁止评论'),
      t('blog', '所有人可评'),
      t('blog', '登录用户可评'),
    )
  );
  $form['fields']['info']['status'] = array(
    '#title' => t('blog', '状态'),
    '#type' => 'radio',
    '#default_value' => isset($blog->status) ? $blog->status : 1,
    '#required' => 1,
    '#options' => array(
      0 => t('blog', '暂不发布'),
      1 => t('blog', '立即发布'),
      2 => t('blog', '登录可见'),
      9 => t('blog', '密码访问'),
    )
  );
  $form['fields']['info']['pass'] = array(
    '#title' => t('blog', '密码'),
    '#type' => 'password',
    '#default_value' => $pass,
    '#attributes' => array('size' => 60),
    '#description' => $pass_des,
  );
  $form['fields']['info']['alias'] = array(
  	'#title' => t('blog', '别名'),
  	'#type' => 'textfield',
  	'#default_value' => $blog->alias,
  	'#description' => t('blog', '文章的访问路径，如：test/abcd、page.html等。优化的路径，可提升用户体验，且有利于搜索优化'),
  );
  $form['fields']['nid'] = $form['fields']['bid'] = array(
    '#type' => 'hidden',
    '#default_value' => $blog->bid,
    '#constant' => 1
  );
  field_get_node_form('blog', 'field', $blog, $form, 1);
  
  $form['fields']['format'] = filter_form_field((isset($blog->format) ? $blog->format : 2));
  $form['fields']['submit'] = array('#type' => 'submit', '#submit' => 'blog_node_form_submit', '#weight' => 100);
  
  $form['fields']['trackback_list'] = array(
    '#fieldset_legend' => t('blog', '发送 Trackbacks 到'),
    '#fieldset_prefix' => 'asc',
    '#type' => 'textarea',
    '#description' => t('blog', '每行一个通告地址。通告并不会立即发送，将由 cron 任务调度。每个通告最多重试三次，若三次后不成功，将不再发送。'),
    '#fieldset_suffix' => '</div>',
  );
  
  if($blog->bid && $fetch = db_query('SELECT trackback, status, tid FROM {blog_trackback} WHERE bid = ?', array($blog->bid))){
    $items = array(t('blog', '下列地址已发送：'));
    foreach($fetch as $track){
      if($track->status == 1){
        $status = '发送成功';
      }else if(!$track->status){
        $status = '未成功发送';
      }else{
        $status = '已作废';
      }
      
      $items[] = $track->trackback .'('.t('blog', $status).')';
    }
    $form['fields']['trackback_list']['#description'] .= theme('item_list', $items);
  }
  
  dd_get_editor('body', array('toolbar' => 'medium', 'upload_image' => 1, 'AutoSaveUrl' => $autosave, 'height' => 300,'AutoSaveTime' => $conf['blog_autosave_time']));
  
  dd_get_editor('summary', array('toolbar' => 'small', 'height' => 150));
  
  dd_add_js(dd_get_path('module', 'blog').'/blog.admin.js');
  dd_add_css(dd_get_path('module', 'blog').'/blog.css');
  module_alter_all('blog_form', $blog, $form, $v);
  return $form;
}

function _blog_pre_plain($mat){
  return $mat[1].check_plain($mat[2]).$mat[3];
}

function blog_node_form_validate($form, $v){
  $v = array_merge($v, $v['info']);
  
  if($v['bid']){
    $blog = blog_load($v['bid']);
  }
  
  if(mb_strlen($v['title'], 'utf-8') > 128){
    $t['title']['#error'][] = t('blog', '标题过长，最多 128 字符');
  }else if($v['status'] == 9){
    $len = strlen($v['pass']);
    if(!$blog->bid && ($len < 4 || $len > 64)){
      $t['pass']['#error'][] = t('blog', '密码长度必须介于 4 - 64 字符之间');
    }
  }else if($v['alias']){
  	if(mb_strlen($v['alias'], 'utf-8') > 250){
  		$t['alias']['#error'][] = t('blog', '别名不大于 250 字符');
  	}else if(menu_check_reservation($v['alias'])){
  		$t['alias']['#error'][] = t('blog', '别名在受限的列表中，请换一个试试');
  	}else if($v['alias'] != $blog->alias && menu_get_path($v['alias'])){
  		$t['alias']['#error'][] = t('blog', '%string 已经存在', array('%string' => $v['alias']));
  	}
  }else if($v['summary'] && mb_strlen($v['summary'], 'utf-8') > 2000){
    $t['alias']['#error'][] = t('blog', '摘要不大于 2000 字符');
  }
  
  if(!$t){
    /**
     * 触发 hook_blog_validate()
     */
    $value = (object) $v;
    if(!$t = field_get_node_validate('blog', 'field', $value, $blog, $form)){
      $t = module_invoke_all('blog_validate', $value, $blog, $form);
    }
  }
  
  return $t;
}

function blog_node_form_submit($form, $v){
  global $user, $conf;
  
  $v = array_merge($v, $v['info']);
  
  if($v['bid']){
    $blog = blog_load($v['bid']);
  }
  
  if($v['status'] == 9) $v['status'] = $v['pass'] ? md5($v['pass']) : $blog->pass; // 访问密码
  
  if($v['get_down_image'] && $v['body']){
    $v['body'] = file_down_image($v['body']);
    if($v['summary']){
      $v['summary'] = file_down_image($v['summary']);
    }
  }
  
  if($v['body'] && (!$v['summary'] 
  || strpos($v['summary'], '<!-- summary -->') !== false 
  || !trim(str_replace('&nbsp;', '', strip_tags($v['summary']))))){
     $v['summary'] = '<!-- summary -->'.blog_summary($v['body'], var_get('blog_summary_num', 500), $v['format']);
  }
  
  if($blog->bid){ // 更新
    db_exec('UPDATE {blog} SET title = ?, body = ?, summary = ?, status = ?, updated = ?, format = ?, comment = ?, top = ?, trackback = ? WHERE bid = ?', array($v['title'], $v['body'], $v['summary'], $v['status'], $_SERVER['REQUEST_TIME'], $v['format'], $v['comment'], $v['top'], $v['trackback'], $blog->bid));
    
    blog_save_trackback($v['trackback_list'], $blog->bid);
    
    if($blog->alias){
    	if($v['alias']){
    		if($v['alias'] != $blog->alias)
          menu_update_alias($v['alias'], $blog->alias); // 更新别名
    	}else{
        // 删除别名
        menu_delete_path('node/'. $blog->bid);
      }
    }else if($v['alias']){
    	menu_set_alias('node/'. $blog->bid, $v['alias'], 'blog', $blog->bid); // 写入别名
    }
    
    dd_set_message(t('blog', '%string 更新成功', array('%string' => $v['title'])));
    module_invoke_all('blog_update', (object)$v, $form, $blog);
  }else if(db_exec('INSERT INTO {blog} (title, body, summary, status, uid, created, format, comment, top, trackback) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)', array($v['title'], $v['body'], $v['summary'], $v['status'], $user->uid, $_SERVER['REQUEST_TIME'], $v['format'], $v['comment'], $v['top'], $v['trackback']))){
    if($blog->bid = db_last_insert_id()){
      
      blog_save_trackback($v['trackback_list'], $blog->bid, true);
      
      dd_save_cron('blog', 'ping', array('func' => 'blog_set_ping', 'args' => array($blog->bid)), 100);
      
      if($conf['cron_pass']){
        $cron_url = f('cron.php?pass='.md5($conf['cron_pass']));
      }else{
        $cron_url = f('cron.php');
      }
      
    	if($v['alias']){
    		menu_set_alias('node/'. $blog->bid, $v['alias'], 'blog', $blog->bid); // 写入别名
    	}
	    dd_set_message(t('blog', '%string 创建成功，你可以手动执行 cron 以发送 trackback 和 ping： !link，或等待定时任务稍后执行', array('%string' => $v['title'], '!link' => l(t('system', '立即执行'), $cron_url, array('attributes' => array('target' => '_blank'))))));
      
	    db_exec('DELETE FROM {temp} WHERE tid = ?', array($v['form_token']));
	    
	    module_invoke_all('blog_insert', (object)$v, $form);
    }
  }else{
    dd_set_message(t('blog', '%string 保存失败', array('%string' => $v['title'])), 'error');
  }
  if($blog->bid){
    $v['nid'] = $blog->bid;
    field_get_node_save('blog', 'field', (object)$v, $blog, $form);
    custom_del('blog_front_cache');
  }
}