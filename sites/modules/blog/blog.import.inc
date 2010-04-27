<?php
// $Id$

/**
 * 导入数据
 */
function blog_admin_import(){
  if($hooks = module_invoke_all('blog_admin_import')){
    $module = arg(3);
    foreach($hooks as $key => $hook){
      if(!$hook['func']) continue;
      if($module === $key && function_exists($hook['func'])){
        return call_user_func_array($hook['func'], $hook['args']);
      }
      $items[] = l($hook['name'], 'admin/blogs/import/'.$key, array('attributes' => array('target' => '_blank')));
    }
    $output = theme('item_list', $items, t('blog', '支持的导入服务'));
  }else{
    $output = system_no_content();
  }
  
  dd_set_tabs(array(
    l(t('blog', '导入数据'), 'admin/blogs/import'),
    l(t('blog', '创建日志'), 'admin/blogs/add'),
    l(t('blog', '日志管理'), 'admin/blogs'),
    l(t('blog', '评论管理'), 'admin/blogs/comments'),
    l(t('blog', '日志设置'), 'admin/blogs/settings'),
    l(t('blog', '草稿'), 'admin/blogs/draft'),
  ));
  
  return $output;
}

/**
 * @Implement of hook_blog_admin_import()
 */
function blog_blog_admin_import(){
  return array(
    'drupal6' => array(
      'name' => t('blog', '从 Drupal 6.x 导入'),
      'func' => 'blog_admin_import_drupal6',
      // 'args' => array(), // 传递的参数
    ),
  );
}

/**
 * 从 Drupal 导入
 */
function blog_admin_import_drupal6(){
  global $conf, $database;
  echo '----';
  exit;
  if($conf['blog_import_drupal6_database']){
    $database['drupal6'] = $conf['blog_import_drupal6_database'];
    $data_opt = array('target' => 'drupal6');
  }
  
  $tabs = array(
    'data' => t('blog', '设置数据库'),
    'site' => t('blog', '导入站点信息'),
    'user' => t('blog', '导入用户'),
    'term' => t('blog', '导入分类'),
    'node' => t('blog', '导入节点'),
    'end' => t('blog', '导入结束，清除临时数据')
  );
  
  $cache_key = 'blog_import_drupal6_'.$_GET['op'].'_'.$database['drupal6']['dbname'];
  
  switch($_GET['op']){
    case 'user':
      exit;
      if(!$conf[$cache_key]){
        if($fetch = db_query('SELECT mail, name, pass, created, login FROM {users} ORDER BY uid ASC', NULL, array('target' => 'drupal6', 'limit' => array($_GET['js_limit'], 100)))){
          $true = false;
          foreach($fetch as $o){
            
            $true = true;
          }
          if($true){
            dd_js_goto(100);
          }
        }
      }else{
        dd_set_message(t('blog', '这个数据库似乎曾导入'), 'error');
        $output = blog_import_confirm_button($cache_key);
      }
    break;
    case 'term':
      if(!$conf[$cache_key]){
        if($fetch = db_query('SELECT * FROM {vocabulary}', NULL, $data_opt)){
          foreach($fetch as $o){
            $field = new stdClass();
            $field->type = 'field';
            $field->module = 'blog';
            $field->name = $o->name;
            $field->required = $o->required;
            $field->data['description'] = $o->description;
            if($o->tags){
              $field->field_type = 'tags';
            }else if($o->multiple){
              $field->field_type = 'checkbox';
            }else{
              $field->field_type = 'select';
            }
            $field->weight = $o->weight;
            field_save($field);
            
            if($field->field_id){
              /**
               * 导入所有子分类
               */
              if($terms = db_query('SELECT tid, name, description, weight FROM {term_data} WHERE vid = ?', array($o->vid), $data_opt)){
                
                foreach($terms as $term){
                  $newterm = field_save_term($field, (object)array(
                    'name' => $term->name,
                    'description' => $term->description,
                    'weight' => $term->weight
                  ), 1);
                  $import_var[$term->tid] = $newterm->tid;
                }
              }
              
            }
          }
        }
        
        // 写入分类导入标记
        var_set($cache_key, true);
        
        if($import_var){
          // 记录两个数据库中分类id，导入节点时使用
          cache_set($cache_key, $import_var);
          
          field_get_cache_field(NULL, 1);
          field_set_cache_term();
        }
      }else{
        dd_set_message(t('blog', '这个数据库似乎曾导入'), 'error');
        $output = blog_import_confirm_button($cache_key);
      }
    break;
    case 'node':
      if(!$conf[$cache_key]){
        if($fetch = db_query('SELECT t.*, n.comment, n.created, n.changed, n.status FROM {node} n INNER JOIN {node_revisions} t ON n.nid = t.nid ORDER BY nid ASC', NULL, array('target' => 'drupal6', 'limit' => array(($_GET['js_limit'] ? $_GET['js_limit']: 0), 20)))){
          if($cache = cache_get($cache_key)){
            $vars = $cache->data;
          }else{
            $vars = array();
          }
          $true = false;
          $i = $j = 0;
          
          if($cache = cache_get('blog_import_drupal6_term_'.$database['drupal6']['dbname'])){
            $terms = $cache->data;
          }
          
          foreach($fetch as $o){
            if(strpos($o->body, '<!--more-->') !== false){
              $o->body = str_replace('<!--more-->', '<!-- break -->', $o->body);
            }
            if($o->format == 1){
              $format = 2;
            }else{
              $format = 0;
            }
            if(db_exec('INSERT INTO {blog} (title, body, status, uid, created, updated, format, comment, top, trackback, summary) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)', array($o->title, $o->body, $o->status, 1, $o->created, $o->changed, $format, $o->comment, 0, 0, $o->teaser))){
              if($bid = db_last_insert_id()){
                $vars[$o->nid] = $bid;
                ++$i;
                
                /**
                 * 导入节点下的评论
                 */
                if($o->comment){
                  if($comments = db_query('SELECT uid, comment, hostname, timestamp, name, homepage, mail, status FROM {comments} WHERE nid = ?', array($o->nid), $data_opt)){
                    foreach($comments as $c){
                      ++$j;
                      if($c->format == 1){
                        $format = 2;
                      }else{
                        $format = 0;
                      }
                      db_exec('INSERT INTO {blog_comment} (bid, name, mail, site, body, uid, format, status, type, timestamp, host) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)', array($bid, $c->name, $c->mail, $c->homepage, $c->comment, $c->uid, $format, $c->status, 0, $c->timestamp, $c->hostname));
                    }
                  }
                }
                
                /**
                 * 导入节点分类
                 */
                if($terms && $cat = db_query('SELECT tid FROM {term_node} WHERE nid = ?', array($o->nid), $data_opt)){
                  foreach($cat as $t){
                    if($terms[$t->tid]){
                      db_exec('INSERT INTO {fields_term_node} (tid, nid) VALUES (?, ?)', array($terms[$t->tid], $bid));
                    }
                  }
                }
              }
            }
            $true = true;
          }
          if($vars){
            cache_set($cache_key, $vars);
          }
          if($i){
            $_SESSION[$cache_key]['node'] += $i;
          }
          
          if($j){
            $_SESSION[$cache_key]['comment'] += $j;
          }
          
          unset($_SESSION['database_debug']);
          
          if($true){
            dd_js_goto(20);
          }else{
            dd_set_message(t('blog', '%node 篇日志，%comment 则评论成功导入', array('%node' => $_SESSION[$cache_key]['node'], '%comment' => $_SESSION[$cache_key]['comment'])));
            unset($_SESSION[$cache_key]);
          }
        }
      }else{
        dd_set_message(t('blog', '这个数据库似乎曾导入'), 'error');
        $output = blog_import_confirm_button('blog_import_drupal6_node_'.$database['drupal6']['dbname']);
      }
    break;
    case 'data':
      $types = db_info();
      $output = '<form accept-charset="UTF-8" action="" method="post">';
      
      if($_POST['driver'] && in_array($_POST['driver'], $types)){
        require_once './includes/database/install.'.$_POST['driver'].'.inc';
        
        if($fields = db_install_form($_POST)){
          
          if($_POST['dbname'] && $_POST['driver']){
            var_set('blog_import_drupal6_database', ($_POST ? $_POST : $conf['blog_import_drupal6_database']));
            dd_set_message(t('blog', '保存成功'));
          }
          
          foreach($fields as $field){
            $output .= '<div class="form_item">'.$field.'</div>';
          }
          $output .= dd_form_hidden(array(
          '#name' => 'driver',
          '#value' => $_POST['driver']
          ));
          $output .= dd_form_submit(array());
        }
      }else{
        $output .= dd_form_select(array(
        '#title' => t('blog', '请选择数据库'),
        '#name' => 'driver',
        '#required' => 1,
        '#options' => $types));
        $output .= dd_form_submit(array());
        $output .= '</form>';
      }
    break;
    case 'site':
      dd_set_message(t('blog', '仅导入站点名称、口号、描述、站长信箱、页脚信息'));
      if($fetch = db_query('SELECT * FROM {variable} WHERE name LIKE ?', array('sit%'), $data_opt)){
        $vars = $conf['site_global'];
        foreach($fetch as $o){
          if($o->value){
            $value = unserialize($o->value);
          }else{
            continue;
          }
          switch($o->name){
            case 'site_mail':
              $vars['mail'] = $value;
            break;
            case 'site_mission':
              $vars['description'] = $value;
            break;
            case 'site_slogan':
              $vars['slogan'] = $value;
            break;
            case 'site_footer':
              $vars['footer'] = $value;
            break;
            case 'site_name':
              $vars['name'] = $value;
          }
        }
        if($vars){
          dd_set_message(t('blog', '导入成功'));
          var_set('site_global', $vars);
        }
      }
    break;
    case 'end':
      db_exec('DELETE FROM {variable} WHERE name LIKE ?', array('blog_import_drupal6_%'));
      cache_del('*', 'blog_import_drupal6_');
      unset($conf[$name]);
      var_init();
      dd_set_message(t('blog', '临时数据清除成功'));
  }
  
  dd_set_title(array($tabs[$_GET['op']], t('blog', '从 Drupal 6.x 导入')));
  dd_set_breadcrumb(array(t('blog', '数据导入'), t('blog', '从 Drupal 6.x 导入'), $tabs[$_GET['op']]));
  
  $i = 1;
  foreach($tabs as $key => $tab){
    $items[$key] = '第 '.$i.' 步：' . l($tab, $_GET['q'], array('query' => 'op='.$key));
    ++$i;
  }
  
  $output = theme('item_list', $items) . $output;
  
  echo theme('print', $output);
  exit;
}

function blog_import_confirm_button($name){
  if($_POST[$name]){
    var_del($name);
  }
  return '<form accept-charset="UTF-8" action="" method="post"><input type="submit" name="'.$name.'" value="'.t('blog', '确认操作').'" /></form>';
}