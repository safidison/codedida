<?php
// $Id$

/**
 * 初始化载入已启用模块
 */
function module_init() {
  global $conf;
  foreach ($conf['modules'] as $module => $info) {
    if (!module_load($module.'.module', $info['path'])) {
      module_invoke($module, 'init');
    }
  }
}

/**
 * 载入模块安装文件，若未指定模块名称，则载入全部
 */
function module_load_install($module = NULL) {
  global $conf;
  if ($module) {
    $info = $conf['modules'][$module];
    $filepath = './'.$info['path'] .'/'. $module. '.install';
    if (is_file($filepath)) include_once $filepath;
  } else {
    foreach ($conf['modules'] as $module => $info) {
      $filepath = './'.$info['path'] .'/'. $module. '.install';
      if (is_file($filepath)) include_once $filepath;
    }
  }
}

/**
 * 已启用模块、主题写入主配置表
 */
function module_enabled_variable($op = 'all') {
 if ($op == 'all' || $op == 'theme') {
    $themes = array();
    foreach (module_list('theme', 'enabled') as $name => $info) {
      if ($info['status'] == -1) {
        var_set('default_theme', $info['filename'], 0, 0);
      }
      $themes[$name] = array(
        'filename' => $info['filename'],
        'path' => $info['path'],
        'region' => $info['info']['region'],
        'styles' => $info['info']['styles'] ? $info['info']['styles'] : NULL,
        'scripts' => $info['info']['scripts'] ? $info['info']['scripts'] : NULL,
        'name' => ($info['info']['name'] ? $info['info']['name'] : $info['filename']),
        'language' => $info['info']['language']
      );
      if ($info['info']['logo']) {
        $themes[$name]['logo'] = $info['info']['logo'];
      }
      if ($info['info']['favicon']) {
        $themes[$name]['favicon'] = $info['info']['favicon'];
      }
      if ($info['info']['settings']) {
        $themes[$name]['settings'] = $info['info']['settings'];
      }
    }
    var_set('themes', $themes, 0, 0);
  }
  if ($op == 'all' || $op == 'module') {
    $modules = array();
    foreach (module_list('module', 'enabled') as $name => $info) {
      $modules[$name] = array(
        'filename' => $info['filename'],
        'path' => $info['path'],
        'name' => ($info['info']['name'] ? $info['info']['name'] : $info['filename']),
        'language' => $info['info']['language'],
      );
    }
    var_set('modules', $modules, 0, 0);
  }
}

/**
 * 模块、主题列表，实时读取数据库
 */
function module_list($type = 'module', $status = 'all') {
  switch ($status) {
    case 'enabled'://启用的模块
      $where = ' AND status != 0';
    break;
    case 'disabled'://禁用的模块
      $where = ' AND status = 0';
  }
  if ($fetch = db_query('SELECT * FROM {system} WHERE type = :type' . $where . ' ORDER BY weight DESC, filename DESC', array(':type' => $type), array('fetch' => 'array'))) {
    foreach ($fetch as $module) {
      $module['info'] = unserialize($module['info']);
      $modules[$module['filename']] = $module;
    }
    return $modules;
  }
}

/**
 * 注册模块、主题，或更新info信息
 */
function module_set_list($type = 'module') {
  $array1 = module_tree_list($type);
  $array2 = module_list($type);
  
  if (is_array($array1)) {
    if ($array2) {
      /**
       * 清除文件已不存在的过期模块、主题
       * 模块目录共三个，若从一个移动到另一个，不清除
       */
      foreach ($array2 as $module => $old) {
        //if (!$array1[$module] || $array1[$module] != $old['path']) {
        if (!$array1[$module]) {
          db_exec('DELETE FROM {system} WHERE filename = :name', array(':name' => $module));
          unset($array2[$module]);
        }
      }
    }
    
    foreach ($array1 as $key => $path) {
      if ($array2[$key]) {
        unset($array1[$key]);
        
        if (!$info = module_get_info($key, './'.$path.'/'. $key .'.info')) {
          continue;
        }
        
        $install_file = './'.$path.'/'. $key .'.install';
        $info['uninstall'] = false;
        
        if (is_file($install_file)) {
          require_once $install_file;
          if (function_exists($key . '_uninstall')) {
            $info['uninstall'] = true;
          }
        }
        
        $info = array_merge($info, $array2[$key]['info']);
        
        db_exec('UPDATE {system} SET weight = :weight, info = :info, type = :type, path = :path WHERE filename = :name',
        array(
          ':path' => $path,
          ':type' => $type,
          ':weight' => $info['weight'],
          ':info' => serialize($info),
          ':name' => $key
        ));
      }
    }
  }
  if (count($array1) > 0) {
    foreach ($array1 as $key => $path) {
      $info = module_get_info($key, './'.$path .'/'. $key .'.info');
      if (!$info['ver']) {
        dd_set_message($key . ' 的info文件信息不完整，至少需要包含版本号及发布时间戳');
        return false;
      }
      
      $install_file = './'.$path.'/'. $key .'.install';
      $info['uninstall'] = false;
      if (is_file($install_file)) {
        require_once $install_file;
        if (function_exists($key . '_uninstall')) {
          $info['uninstall'] = true;
        }
      }
      
      $c = db_exec('INSERT INTO {system} (filename, path, type, status, install, weight, info) VALUES 
      (:name, :path, :type, 0, -1, :weight, :info)', 
      array(':name' => $key, ':path' => $path, ':type' => $type, ':weight' => $info['weight'], ':info' => serialize($info)));
    }
  }
  
  return true;
}

/**
 * 检查模块是否已启用
 */
function module_exists($module) {
  $list = array();
  $list = module_list('enabled');
  return array_key_exists($module, $list);
}

/**
 * 调用install创建或删除表
 */
function module_install_schema($type, $module) {
  $ret = array();
  $function = $module. '_schema';
  if (function_exists($function)) {
    $schema = $function();
    if ($type == 'install') {
      foreach ($schema as $name => $table) {
        if ($sql = db_create_table($name, $table)) {
        	$ret = array_merge($ret, $sql);
      	}
      }
    } else if ($type == 'uninstall') {
      // 外键约束，先从子表开始删除
      $schema = array_reverse($schema, true);
      foreach ($schema as $name => $table) {
        if (db_drop_table($name)) {
          $ret[] = $name;
        }
      }
    }
  }
  
  return $ret;
}

/**
 * 安装或升级模块数据表
 */
function module_set_install($v) {
  $updates = module_get_ver($v['filename']);
  
  $success['ver'] = $v['install'];
  
  if ($v['install'] != -1) {
    // 升级数据库
    if ($updates['ver'] > $v['install']) {
      ++$v['install'];
      ++$updates['ver'];
      $success['query'] = array();
      for ($i = $v['install']; $i < $updates['ver']; ++$i) {
        $function = $v['filename']. '_update_'.$i;
        if (function_exists($function)) {
          if ($sql = $function()) {
            if (is_array($sql)) {
              $success['query'] = array_merge($success['query'], $sql);
            }
          } else {
             return -1;
          }
        }
      }
      --$updates['ver'];
      $success['ver'] = $updates['ver'];
      return $success;
    }
  } else { // 首次安装数据库
    $function = $v['filename'] . '_install';
    if (function_exists($function)) {
      if ($sql = $function()) {
        $success['query'] = $sql;
        $success['ver'] = $updates['ver'];
        return $success;
      } else {
         return -1;
      }
    }
  }
  
  return false;
}

/**
 * 读取 info 文件
 */
function module_get_info($module, $filepath) {
  if (is_file($filepath)) {
    require $filepath;
    
    if (!in_array($module, module_core_module()) && $info['weight'] > 90) {
      $info['weight'] = 90;
    }
    
    if (!$info['weight'] || !is_numeric($info['weight'])) $info['weight'] = 0;
    return $info;
  }
}

/**
 * 系统必需的模块
 */
function module_core_module() {
  return array('system', 'user', 'block', 'field');
}

/**
 * 卸载模块
 */
function module_set_uninstall($module) {
  $lists = module_list('module', 'enabled');
  if (!$dependencies = module_is_dependencies($module, $lists)) {
    $install_file = $lists[$module]['path'] .'/'. $lists[$module]['filename'] .'.install';
    if (is_file($install_file)) {
      require_once $install_file;
      $function = $lists[$module]['filename'] . '_uninstall';
      if (function_exists($function)) $function();
    }
    
    if (db_exec('UPDATE {system} SET status = 0, install = -1 WHERE filename = ?', array($module))) {
      $return = t($module, $lists[$module]['info']['name']);
      module_clear_cache();
      return $return;
    }
  } else {
    dd_set_message(t('system', '不能卸载 %module，%name 依赖它', array('%name' => t($dependencies, $lists[$dependencies]['info']['name']), '%module' => t($module, $lists[$module]['info']['name']))));
    return false;
  }
}

/**
 * @param $module
 *  模块系统名称
 * @param $lists
 *  模块列表
 * 搜索 $lists 中是否有依赖于 $module 的模块。
 * @return
 *  有依赖于 $module 的模块，则返回第一个模块的系统名称
 */
function module_is_dependencies($module, $lists = array()) {
  foreach ($lists as $key => $m) {
    if ($m['info'] && $m['info']['dependencies'] && in_array($module, $m['info']['dependencies'])) {
      return $key;
    }
  }
}

/**
 * 禁用模块
 */
function module_set_disabled($modules = array()) {
  $lists = module_list('module', 'enabled');
  $true = false;
  foreach ($modules as $module) {
    if ($lists[$module]) {
      if (!in_array($module, module_core_module())) {
        if (!$dependencies = module_is_dependencies($module, $lists)) {
          db_exec('UPDATE {system} SET status = 0 WHERE filename = ?', array($module));
          $install_file = $lists[$module]['path'] .'/'. $lists[$module]['filename'] .'.install';
          if (is_file($install_file)) {
            require_once $install_file;
            $function = $lists[$module]['filename'] . '_disabled';
            if (function_exists($function)) $function();
          }
          $true = true;
        } else {
          dd_set_message(t('system', '不能禁用 %module，%name 依赖它', array('%name' => t($dependencies, $lists[$dependencies]['info']['name']), '%module' => t($module, $lists[$module]['info']['name']))));
          return false;
        }
      } else {
        dd_set_message(t('system', '不能禁用 %string，系统必需', array('%string' => t($module, $lists[$module]['info']['name']))), 'error');
      }
    }
  }
  
  if ($true) return true;
  
}

/**
 * 启用或禁用模块，缓存清除
 */
function module_clear_cache() {
  module_enabled_variable('module');
  cache_del('cid', 'user_perm');
  cache_del('cid', 'cache_hook');
  cache_del('empty', NULL, 'cache_menu');
  cache_del('*', 'admin_menus_');
  cache_del('cid', 'schema');
  var_init();
  menu_set_item();
}

/**
 * 启用或升级模块
 */
function module_set_enabled($list, $type, $modules = array()) {
  $result = module_list($type, $list);
  
  foreach ($modules as $name) {
    if ($result[$name]) $module[$name] = $result[$name];
  }
  
  if (is_array($module)) {
    $query = array();
    foreach ($module as $v) {
      
      if (!$info = module_get_info($v['filename'], './'.$v['path'] .'/'. $v['filename'] .'.info')) {
        dd_set_message($type. ' ' .$v['filename'] .'缺少 info 文件。', 'error');
        return false;
      }
      
      if ($info['dependencies']) {
        foreach ($info['dependencies'] as $key => $dependencies) {
          if (!$result['modules'][$dependencies] && $result['modules'][$dependencies]['status'] == 1) {
            dd_set_message(t('system', '%name 依赖于 %module，请先启用 %module', array('%name' => t($v['filename'], $lists[$v['filename']]['info']['name']), '%module' => t($dependencies, $lists[$dependencies]['info']['name']))));
            return false;
          }
        }
      }
      
      require_once $v['path'] .'/'. $v['filename'] .'.module';
      $install_file = $v['path'] .'/'. $v['filename'] .'.install';
      
      if (is_file($install_file)) {
        require_once $install_file;
        
        if (function_exists($v['filename'] . '_uninstall')) {
          $info['uninstall'] = true;
        }
        
        if ($success = module_set_install($v)) {
          if ($success == -1) {
            dd_set_message(t('system', '无法升级或启用模块 %string ，请检查', array('%string' => t($v['filename'], $lists[$v['filename']]['info']['name']))), 'error');
            return false;
          }
        } else {
          $success['ver'] = $v['install'];
        }
      } else {
        $success['ver'] = -1;
      }
      
      $v['info'] = serialize($info);
      
      $v['install'] = $success['ver'];
      
      if (is_array($success['query'])) {
        $query = array_merge($success['query'], $query);
      }
      
      db_query('UPDATE {system} SET status = 1, install = ?, weight = ?, info = ? WHERE filename = ?', array($v['install'], $v['weight'], $v['info'], $v['filename']));
      
      if ($v['status'] != 1) {
        $function = $v['filename'] . '_enabled';
        if (function_exists($function)) $function();
      }
      
    }
    
    // 清除缓存
    module_clear_cache();
    
    if ($query) {
      return $query;
    } else {
      return array();
    }
  }
}

/**
 * 调用指定模块实现的 hook
 * @param (string) $hook
 *  hook 名称
 * @param (array || string) $modules
 *  一个或一组模块的系统名称
 * @return
 *  模块返回值或递归合并，视 $modules 而定
 */
function module_invoke($modules, $hook) {
  
  $args = func_get_args();
  $modules = $args[0];
  $hook = $args[1];
  unset($args[0], $args[1]);
  
  if (is_string($modules)) {
    $function = $modules .'_'. $hook;
    
    if (function_exists($function)) {
      return call_user_func_array($function, $args);
    }
  } else if (is_array($modules)) {
    foreach ($modules as $module) {
      $function = $module .'_'. $hook;
      if (function_exists($function)) {
        $result = call_user_func_array($function, $args);
        if (isset($result) && is_array($result)) {
          $return = array_merge_recursive($return, $result);
        } else if (isset($result)) {
          $return[] = $result;
        }
      }
    }
    return $return;
  }
}

/**
 * 调用指定模块实现的 hook，引用传值
 * @param (string) $hook
 *  hook 名称
 * @param (array || string) $modules
 *  一个或一组模块的系统名称
 * @param (*) &$data, &$value1, &$value2, &$value3
 *  最多传递 4 个参数
 */
function module_alter($modules, $hook, &$data, &$value1 = NULL, &$value2 = NULL, &$value3 = NULL) {
  global $conf;
  
  if (is_string($modules)) {
    $function = $modules .'_'. $hook;
    if (function_exists($function)) {
      $function($data, $value1, $value2, $value3);
    }
  } else if (is_array($modules)) {
    foreach ($modules as $module) {
      $function = $module .'_'. $hook;
      if (function_exists($function)) {
        $function($data, $value1, $value2, $value3);
      }
    }
  }
}

/**
 * 调用所有已启用模块实现的 hook，引用传值
 * @param (string) $hook
 *  hook 名称
 * @param (*) &$data, &$value1, &$value2, &$value3
 *  最多传递 4 个参数
 */
function module_alter_all($hook, &$data, &$value1 = NULL, &$value2 = NULL, &$value3 = NULL) {
  global $conf;
  
  foreach ($conf['modules'] as $module => $info) {
    $function = $module .'_'. $hook;
    if (function_exists($function)) {
      $function($data, $value1, $value2, $value3);
    }
  }
}

/**
 * 调用所有已启用模块实现的 hook
 * @return
 *  返回值递归合并
 */
function module_invoke_all() {
  global $conf;
  $args = func_get_args();
  $hook = $args[0];
  unset($args[0]);
  $return = array();
  foreach ($conf['modules'] as $module => $info) {
    $function = $module .'_'. $hook;
    if (function_exists($function)) {
      $result = call_user_func_array($function, $args);
      if (is_array($result)) {
        $return = array_merge_recursive($return, $result);
      } else if (isset($result)) {
        $return[] = $result;
      }
    }
  }
  return $return;
}

/**
 * 调用所有已启用模块实现的 hook，与  module_invoke_all 不同的是，遇到首个返回值（包括false，不包括 NULL），则结束 
 * @return (*)
 */
function module_invoke_first() {
  global $conf;
  $args = func_get_args();
  $hook = $args[0];
  unset($args[0]);
  foreach ($conf['modules'] as $module => $info) {
    $function = $module .'_'. $hook;
    if (function_exists($function)) {
      $result = call_user_func_array($function, $args);
      if (isset($result)) {
        return $result;
      }
    }
  }
}

/**
 * 调用所有已启用模块实现的 hook
 * @return
 *  返回值按模块系统名称索引
 */
function _module_invoke_all() {
  global $conf;
  $args = func_get_args();
  $hook = $args[0];
  unset($args[0]);
  $return = array();
  foreach ($conf['modules'] as $module => $info) {
    $function = $module .'_'. $hook;
    if (function_exists($function)) {
      if ($result = call_user_func_array($function, $args)) {
        $return[$module] = $result;
      }
    }
  }
  return $return;
}

/**
 * 获取模块install文件中的升级序号
 */
function module_get_ver($module, $load = NULL) {
  if ($load) module_load_install($module);
  $updates = array('ver' => 0);
  $functions = get_defined_functions();
  foreach ($functions['user'] as $function) {
    if (strpos($function, $module .'_update_') === 0) {
      $version = substr($function, strlen($module .'_update_'));
      if (is_numeric($version)) {
        $updates[] = $version;
      }
    }
  }
  if (count($updates) == 0) {
    return $updates;
  }
  sort($updates, SORT_NUMERIC);
  $array['ver'] = end($updates);
  $array['list'] = $updates;
  return $array;
}
/**
 * 遍历模块、主题文件夹
 */
function module_tree_list($type) {
  if ($type == 'module') {
    $dirs[] = 'modules';
    if (is_dir('sites/modules')) {
      $dirs[] = 'sites/modules';
    }
    if (is_dir($GLOBALS['conf_dir'].'/modules')) {
      $dirs[] = $GLOBALS['conf_dir'].'/modules';
    }
  } else {
    $dirs[] = 'themes';
    if (is_dir('sites/themes')) {
      $dirs[] = 'sites/themes';
    }
    if (is_dir($GLOBALS['conf_dir'].'/themes')) {
      $dirs[] = $GLOBALS['conf_dir'].'/themes';
    }
  }
  $modules = array();
  foreach ($dirs as $dir) {
    _module_tree_list($type, $dir, $modules);
  }
  return $modules;
}

function _module_tree_list($type, $dir, &$modules = NULL) {
  if (is_dir($dir)) {
    $d = dir($dir);
    while (false !== ($entry = $d->read())) {
      if (preg_match('/[a-z]/i', substr($entry, 0, 1)) && strlen($entry) < 32) {
        $sub = $dir . '/'. $entry;
        if (!is_dir($sub)) {
          if (strpos($entry, '.')  !== false) {
            $t = explode('.', $entry);
            if (strlen($t[0]) < 64) {
              $ext = end($t);
              if ($type == 'module') {
                if ($ext == 'info' || $ext == 'module') {
                  if ($t[0] == $z[$t[0]]) {
                    $modules[$t[0]] = $dir;
                  }
                  $z[$t[0]] = $t[0];
                }
              } else if ($ext == 'info') {
                $modules[$t[0]] = $dir;
              }
            }
          }
        } else {
          _module_tree_list($type, $sub, $modules);
        }
      }
    }
    $d->close();
  }
  return $modules;
}

/**
 * 载入文件
 */
function module_load($filename, $path) {
  $filepath = './'.$path .'/'. $filename;
  if (is_file($filepath)) {
    include_once $filepath;
    return false;
  }
  return $filepath;
}

/**
 * 获取指定模块表结构
 */
function module_get_table($module, $table = NULL) {
  module_load_install($module);
  $function = $module . '_schema';
  if (function_exists($function)) {
    $schema = $function();
    return !$table ? $schema : $schema[$table];
  }
}

function module_get_schema($table, $clear = NULL) {
  static $tables;
  
  if (!isset($tables)) {
    
    if ($clear || !$cache = cache_get('schema')) {
      
      module_load_install();
      $_tables = db_show_tables();
      
      if ($lists = _module_invoke_all('schema')) {
        
        foreach ($lists as $module => $ts) {
          module_alter_all('alter_schema', $module, $ts);
          foreach ($ts as $name => $fields) {
            if (in_array($name, $_tables)) {
              if ($fields['fields']) {
                $_fields = array('module' => $module);
                foreach ($fields['fields'] as $field_name => $field) {
                  unset($field['description']);
                  $_fields['fields'][$field_name] = $field;
                }
                $tables[$name] = $_fields;
              }
            }
          }
        }
      }
      
      cache_set('schema', $tables, 'cache', 0, 0);
      
    } else {
      $tables = $cache->data;
    }
  }
  
  return $tables[$table] ? $tables[$table] : NULL;
}