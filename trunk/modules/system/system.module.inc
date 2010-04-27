<?php
// $Id$

//模块管理
function system_modules_page(){
  switch(arg(2)){
    case 'update':
      global $conf;
      if($_GET['name'] && $conf['modules'][$_GET['name']]){
        $modules = array($_GET['name']);
      }else{
        $modules = array_keys($conf['modules']);
      }
      
      $query = module_set_enabled('enabled', 'module', $modules);
      
      if(!empty($query) && is_array($query)){
        dd_set_message('升级成功，升级中执行了以下 sql：<br>' .implode('<br>', $query));
      }else{
        dd_set_message(t('system', '没有模块需要升级'));
      }
      
      dd_goto('admin/modules');
    break;
    case 'uninstall':
      if($_GET['name'] && ($module = module_set_uninstall($_GET['name']))){
        dd_set_message(t('system', '%string 成功卸载', array('%string' => $module)));
      }
      dd_goto('admin/modules');
  }
  
  dd_set_tabs(array(l(t('system', '模块列表'), 'admin/modules'), l(t('system', '模块升级'), 'admin/modules/update'), l(t('system', '主题管理'), 'admin/themes')));
  
  $modules = module_list('module');
  
  foreach($modules as $name => $module){
    /*
    $schema = $data = array();
    $filepath = $module['path']. '/'. $name . '.install';
    if(is_file($filepath)){
      if ($fp = fopen($filepath, 'rb')){
        while (!feof($fp)) {
          $line = trim(fgets($fp));
          if(substr($line, 0, 8) == '$schema['){
            $schema[] = $line;
          }
        }
        fclose($fp);
        if($schema){
          foreach($schema as $val){
            if(preg_match('/schema\[\'(.*?)\'\]/i', $val, $mat)){
              $data[$mat[1]] = $mat[1];
            }
          }
        }
      }
    }
    */
    $form['fields']['modules'][$module['filename']]['status']['#value'] = $module['status'];
    $form['fields']['modules'][$module['filename']]['name']['#value'] = $module['info']['name'] ? $module['info']['name'] : $name;
    $form['fields']['modules'][$module['filename']]['path']['#value'] = $module['path'];
    $form['fields']['modules'][$module['filename']]['filename']['#value'] = $module['filename'];
    
    $form['fields']['modules'][$module['filename']]['ver']['#value'] = $module['info']['ver'];
    $form['fields']['modules'][$module['filename']]['install']['#value'] = $module['install'];
    $form['fields']['modules'][$module['filename']]['des']['#value'] = $module['info']['description'];
    $form['fields']['modules'][$module['filename']]['check'] = array(
      '#type' => 'checkbox',
      '#default_value' => $module['status'],
      '#attributes' => array(
        'title' => t('system', '启用这个模块'),
      ),
    );
    
    if($module['info']['dependencies']){
      foreach($module['info']['dependencies'] as $key => $dependencies){
        if(!$modules[$dependencies]){
          $form['fields']['modules'][$module['filename']]['check']['#disabled'] = true;
          $module['info']['dependencies'][$key] = $dependencies.'<span class="red">'.t('system', '(缺少)').'</span>';
        }else{
          $module['info']['dependencies'][$key] = t($dependencies, $modules[$dependencies]['info']['name']).'('.$dependencies.')';
        }
      }
      $form['fields']['modules'][$module['filename']]['dependencies']['#value'] = $module['info']['dependencies'];
    }
    
    if(in_array($name, module_core_module())){
      $form['fields']['modules'][$module['filename']]['check']['#disabled'] = true;
      $form['fields']['modules'][$module['filename']]['check']['#default_value'] = 1;
    }else if($module['status'] && $module['info']['uninstall']){
      $form['fields']['modules'][$module['filename']]['uninstall']['#value'] = true;
    }
  }
  
  $form['settings'] = array(
    '#validate' => array('system_modules_validate'),
    '#theme' => 'system_modules_theme'
  );
  $form['fields']['submit']['#type'] = 'submit';
  
  return $form;
}

function system_modules_validate($form, $value){
  global $conf;
  foreach($form['fields']['modules'] as $name => $info){
    if($value['modules'][$name]['check']){
      if(!$conf['modules'][$name]) $enabled[] = $name;
    }else if($conf['modules'][$name]){
      if(!in_array($name, module_core_module())) $disabled[] = $name;
    }
  }
  
  if($enabled && module_set_enabled('all', 'module', $enabled)){
    dd_set_message(t('system', '%string 启用成功', array('%string' => implode(',', $enabled))));
  }
  
  if($disabled && module_set_disabled($disabled)){
    dd_set_message(t('system', '%string 已禁用', array('%string' => implode(',', $disabled))));
  }
  
  if($enabled || $disabled){
    module_clear_cache();
    cache_del('cid', 'cache_hook');
    theme_get_cache();
  }
}

function system_modules_theme($form, $fields){
  $header = array(t('system', '启用'), t('system', '模块信息'), t('system', '说明'), t('system', '操作'));
  
  $rows = array();
  foreach($fields['modules'] as $module){
    $item = array(
      '<strong>'.t('system', '名称：').'</strong>' . $module['name']['#value'],
      '<strong>'.t('system', '版本：').'</strong>' . $module['ver']['#html'],
      '<strong>'.t('system', '路径：').'</strong>' . $module['path']['#value'],
    );
    if($module['dependencies']['#value']){
      $item[] = '<strong>'.t('system', '依赖：').'</strong>'. implode(', ', $module['dependencies']['#value']);
    }
    $v = module_get_ver($module['filename']['#value'], 1);
    
    $links = array();
    if($module['status']['#value'] == 1){
      if($module['install']['#value'] < $v['ver']){
        $links[] = l(t('system', '升级'), 'admin/modules/update', array('query' => 'name='.$module['filename']['#value']));
      }
      if($module['uninstall']['#value']){
        $links[] = l(t('system', '卸载'), 'admin/modules/uninstall', array('query' => 'name='.$module['filename']['#value'], 'attributes' => array('class' => 'confirm')));
      }
    }
    
    $row = array($module['check']['#html'], theme('item_list', $item), $module['des']['#value'], implode(' | ', $links));
    
    if($module['status']['#value'] == 1){
      $table[] = $row;
    }else{
      $rows[] = $row;
    }
  }
  
  if($rows){
    $table = array_merge($table, $rows);
  }
  
  $table[] = array(array('data' => $fields['submit']['#html'], 'colspan' => 6));
  return theme('table', $header, $table);
}

/**
 * 主题管理
 */
function system_themes_page(){
  global $conf;
  
  dd_set_tabs(array(l(t('system', '模块管理'), 'admin/modules'), l(t('system', '主题管理'), 'admin/themes')));
  
  if($_GET['op'] == 'default' && $_GET['name']){
    if($_GET['name'] != $GLOBALS['conf']['default_theme']){
      if(db_exec('UPDATE {system} SET status = -1 WHERE filename = ?', array($_GET['name']))){
        db_exec('UPDATE {system} SET status = 1 WHERE filename = ?', array($GLOBALS['conf']['default_theme']));
        cache_del('*', 'admin_menus_');
        var_set('default_theme',  $_GET['name']);
        dd_set_message(t('system', '%string 已被设为默认主题，请设置区块', array('%string' => $_GET['name'])));
        dd_goto('admin/blocks/settings/'.$_GET['name']);
        exit;
      }
    }
    dd_goto($_GET['q']);
  }
  foreach(module_list('theme') as $name => $theme){
    $filename = $theme['info']['name'] ? $theme['info']['name'] : $name;
    
    $form['fields']['themes'][$theme['filename']]['check'] = array(
      '#type' => 'checkbox',
      '#default_value' => $theme['status'],
      '#attributes' => array(
        'title' => t('system', '启用'),
      ),
    );
    if($theme['status'] == -1){
      $form['fields']['themes'][$theme['filename']]['check'] = array(
        '#type' => 'checkbox',
        '#default_value' => $theme['status'],
        '#description' => t('system', '当前默认'),
        '#disabled' => 1,
        '#attributes' => array(
          'title' => t('system', '当前默认'),
        ),
      );
    }else if($theme['status'] == 1){
      $form['fields']['themes'][$theme['filename']]['check'] = array(
        '#type' => 'checkbox',
        '#default_value' => $theme['status'],
        '#description' => '<a href="?op=default&name='.$theme['filename'].'">'.t('system', '设为默认').'</a>',
        '#attributes' => array(
          'title' => t('system', '禁用'),
        ),
      );
    }else{
      $form['fields']['themes'][$theme['filename']]['check'] = array(
        '#type' => 'checkbox',
        '#default_value' => $theme['status'],
        '#attributes' => array(
          'title' => t('system', '启用'),
        ),
      );
    }
    
    $form['fields']['themes'][$theme['filename']]['name']['#value'] = $filename;
    $form['fields']['themes'][$theme['filename']]['path']['#value'] = $theme['path'];
    $form['fields']['themes'][$theme['filename']]['ver']['#value'] = $theme['info']['ver'];
    $form['fields']['themes'][$theme['filename']]['des']['#value'] = $theme['info']['description'];
    if($theme['info']['preview_thumb']){
      $form['fields']['themes'][$theme['filename']]['thumb']['#value'] = l(img($theme['path'].'/'.$theme['info']['preview_thumb'], t('system', '预览')), f($theme['path'].'/'.$theme['info']['preview_original']), array('html' => 1));
    }else{
      $form['fields']['themes'][$theme['filename']]['thumb']['#value'] = t('system', '无');
    }
  }
  $form['settings'] = array(
    '#validate' => array('system_themes_validate'),
    '#theme' => 'system_themes_theme'
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('system', '确认操作'),
  );
  return $form;
}

function system_themes_theme($form, $fields){
  $header = array(t('system', '预览'), t('system', '名称'), t('system', '启用'), t('system', '路径'), t('system', '版本'), t('system', '说明'));
  
  foreach($fields['themes'] as $theme){
    $table[] = array(
      $theme['thumb']['#value'], $theme['name']['#value'], $theme['check']['#html'], $theme['path']['#value'], $theme['ver']['#value'], $theme['des']['#value']
    );
  }
  $table[] = array(array('data' => $fields['submit']['#html'], 'colspan' => 6));
  return theme('table', $header, $table);
}

function system_themes_validate($form, $value){
  foreach($form['fields']['themes'] as $name => $info){
    if($name == $GLOBALS['conf']['default_theme']){
      continue;
    }
    if($value['themes'][$name]['check']){
      if(db_exec('UPDATE {system} SET status = 1 WHERE filename = ?', array($name))){
        dd_set_message(t('system', '%string 已启用', array('%string' => $name)));
      }
    }else{
      if(db_exec('UPDATE {system} SET status = 0 WHERE filename = ?', array($name))){
        dd_set_message(t('system', '%string 已禁用', array('%string' => $name)));
      }
    }
  }
  module_enabled_variable('theme');
}