<?php
// $Id$

/**
 * 数据库信息，扩展文件必须实现此接口
 */
function db_info(){
  $types = array();
  $hd = dir('includes/database');
  while(false !== ($name = $hd->read())){
    if(strpos($name, 'database') !== false){
      if($a = explode('.', $name)){
        require_once './includes/database/database.'.$a[1].'.inc';
        $function = $a[1].'_db_info';
        if(function_exists($function)){
          $types += $function();
        }
      }
    }
  }
  return $types;
}

/**
 * 数据库连接
 * @param (string) $target
 *  连接参数，默认为 $database['default'] 中提供的信息
 * @return (bool)
 *  根据连接参数调用相应的扩展文件，如：database.mysql.inc
 *  连接成功，返回true，并将连接对象写入全局变量 $db。失败则返回 false
 */
function db_connect($target = 'default'){
  global $database, $dbc, $db;
  if(!isset($db[$target]) && $dbc[$target]  = $database[$target]){
    $function = 'db_'.$dbc[$target]['driver'].'_connect';
    require_once './includes/database/database.'.$dbc[$target]['driver'].'.inc';
    if(function_exists($function)){
      if($db[$target] = $function($dbc[$target], $error)){
      	return true;
      }else{
      	dd_set_message(t('system', '数据库无法连接：@text', array('@text' => $error)), 'debug');
      }
    }
  }
}

/**
 * select 推荐使用
 * @param (string) $sql：语句，参数使用占位符
 * @param (array) $args：参数，对应语句中的占位符
 *
 * @param (array) $opt：属性设置
 *  $opt['target']：选择数据库连接
 *  $opt['fetch']：返回类型
 *   object - PDO::FETCH_OBJ，默认
 *   array - PDO::FETCH_ASSOC
 *   或其它 PDOStatement::setFetchMode() 允许的参数
 *
 *  $opt['return']：返回结果
 *    object - PDOStatement->fetchAll()，默认
 *    array - PDOStatement->fetch()
 *    column - PDOStatement->fetchColumn()
 *    result - PDOStatement
 *
 *  $opt['limit']：查询起始，array(0, 10) or 10
 *
 * @return object or array or string
 *
 */
function db_query($sql, $args = array(), $opt = array()){
  $opt = db_default_options($opt);
  $sql = db_prefix_tables($sql, $opt);
  $function = 'db_' . $opt['driver'].'_query';
  $fetch = $function($sql, $args, $opt);
  
  db_debug();
  return $fetch;
}

/**
 * insert、update、delete 推荐使用
 * @param (string) $sql：语句，参数使用占位符
 * @param (array) $args：参数，对应语句中的占位符
 * @param (array) $opt：属性设置
 *
 * @return (int) $count 影响的列数
 *
 */
function db_exec($sql, $args = array(), $opt = array()){
  $opt = db_default_options($opt);
  
  if($sql) $sql = db_prefix_tables($sql, $opt);
  
  $function = 'db_' . $opt['driver'].'_exec';
  $fetch = $function($sql, $args, $opt);
  
  db_debug();
  return $fetch;
}

/**
 * 检查表是否存在
 */
function db_is_table($table, $target = 'default'){
  global $dbc;
  
  if(empty($dbc[$target])) db_connect($target);
  
  //$table = $dbc[$target]['prefix'] . $table;
  
  return call_user_func('db_' . $dbc[$target]['driver'].'_is_table', $table, $target);
}

/**
 * 列出所有表
 */
function db_show_tables($target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  
  return call_user_func('db_' . $dbc[$target]['driver'].'_show_tables', $target);
}

/**
 * 获取数据库版本
 */
function db_get_version($target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  
  return call_user_func('db_' . $dbc[$target]['driver'].'_get_version', $target);
}

/**
 * 检查属性中是否有链接信息
 */
function db_default_options(array $opt = array()){
  global $dbc;
  if(empty($opt['target'])) $opt['target'] = 'default';
  if(empty($dbc[$opt['target']])) db_connect($opt['target']);
  $opt['driver'] = $dbc[$opt['target']]['driver'];
  return $opt;
}

/**
 * 为查询语句中的表名加上前缀
 */
function db_prefix_tables($sql, $opt){
  global $dbc;
  $prefix = $dbc[$opt['target']]['prefix'];
  return strtr($sql, array('{' => $prefix, '}' => ''));
}

/**
 * 获取最后一个自增主键
 */
function db_last_insert_id($target = 'default'){
  return $GLOBALS['db'][$target]->lastInsertId();
}

/**
 * @param $code：错误代码
 * @param $info：错误描述
 */
function db_error($code, $info){
  if($code != '0000'){
    $msg = 'Error('.$info[0].')：' . $info[2];
    
    text_log('database_error', dd_error_msg($msg));
    
    if(!$GLOBALS['conf']['debug']) return;
    if($GLOBALS['conf']['debug'] == 2 && $GLOBALS['user']->uid != 1) return;
    
    dd_set_message($msg, 'error');
    return true;
  }
}

/**
 * 创建表
 * @param (string)$name
 *  数据表名称
 * @param (array)$table
 *  字段名称
 *
 */
function db_create_table($name, $table, $target = 'default'){
  global $dbc, $db;
  
  if(empty($dbc[$target])) db_connect($target);
  
  if(!db_is_table($name, $target)){
    if($sql = call_user_func('db_' . $dbc[$target]['driver'].'_create_table', $name, $table, $target)){
      return $sql;
    }else{
      dd_set_message(t('system', '表 @table 创建失败', array('@table' => $name)), 'error');
    }
  }else{
    dd_set_message(t('system', '表 @table 已经存在', array('@table' => $name)), 'error');
  }
}

/**
 * 表更名
 * @param (string)$name
 *  原表名
 * @param (string)$new_name
 *  新表名
 * @param (string)$target
 *  数据库
 */
function db_rename_table($name, $new_name, $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  
	if(db_is_table($name, $target)){
		if(!db_is_table($new_name, $target)){
	    if(call_user_func('db_' . $dbc[$target]['driver'].'_rename_table', $name, $new_name, $target)){
	      dd_set_message(t('system', '@table 更名成功', array('@table' => $name)));
	      return true;
	    }else{
	      dd_set_message(t('system', '@table 更名失败', array('@table' => $name)), 'error');
	    }
		}else{
			dd_set_message(t('system', '@table 已经存在', array('@table' => $new_name)), 'error');
		}
	}else{
		dd_set_message(t('system', '@table 不存在', array('@table' => $name)), 'error');
	}
}

/**
 * 删除表
 */
function db_drop_table($table, $target = 'default'){
  global $dbc;
  
  if(empty($dbc[$target])) db_connect($target);
  if(db_is_table($table, $target)){
    if(call_user_func('db_' . $dbc[$target]['driver'].'_drop_table', $table, $target)){
      dd_set_message(t('system', '@table 删除成功', array('@table' => $table)));
      return true;
    }else{
      dd_set_message(t('system', '@table 删除失败', array('@table' => $table)), 'error');
    }
  }else{
    dd_set_message(t('system', '@table 不存在', array('@table' => $table)), 'error');
  }
}

/**
 * 添加字段
 */
function db_add_field($table, $field, $spec, $keys_new = array(), $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_add_field', $table, $field, $spec, $keys_new, $target);
}

/**
 * 删除字段
 */
function db_drop_field($table, $field, $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_drop_field', $table, $field, $target);
}

/**
 * 设置字段默认值
 * @param $table
 *  表名
 * @param $field
 *  字段名
 * @param $value
 *  默认值
 */
function db_field_set_default($table, $field, $value, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_field_set_default', $table, $field, $value, $target);
}

/**
 * 删除字段默认值
 */
function db_field_drop_default($table, $field, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_field_del_default', $table, $field, $target);
}

/**
 * 添加主键
 */
function db_add_primary($table, $fields, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_add_primary', $table, $fields, $target);
}

/**
 * 删除主键
 */
function db_drop_primary($table, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_drop_primary', $table, $target);
}

/**
 * 添加外键
 */
function db_add_foreign($table, $field, $foreign, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_add_foreign', $table, $field, $foreign, $target);
}

/**
 * 删除外键
 */
function db_drop_foreign($table, $field, $parent_table, $parent_field, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_drop_foreign', $table, $field, $parent_table, $parent_field, $target);
}

/**
 * 添加唯一值
 */
function db_add_unique($table, $name, $fields, $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_add_unique', $table, $name, $fields, $target);
}

/**
 * 删除唯一值
 */
function db_drop_unique($table, $name, $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_drop_unique', $table, $name, $fields, $target);
}

/**
 * 添加索引
 */
function db_add_index($table, $name, $fields, $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_add_index', $table, $name, $fields, $target);
}

/**
 * 删除索引
 */
function db_drop_index($table, $name, $target = 'default') {
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_drop_index', $table, $name, $target);
}

/**
 * 更新字段
 */
function db_change_field($table, $field, $field_new, $spec, $keys_new, $target = 'default'){
  global $dbc;
  if(empty($dbc[$target])) db_connect($target);
  return call_user_func('db_' . $dbc[$target]['driver'].'_change_field', $table, $field, $field_new, $spec, $keys_new, $target);
}

/**
 * 数组解析为占位符
 */
function db_get_placeholder($array = array()){
	return implode(',', array_fill(0, count($array), '?'));
}

/**
 * 在页脚显示查询语句
 */
function db_debug(){
  if(!$GLOBALS['conf']['sql_info']) return;
  if($GLOBALS['conf']['sql_info'] == 2 && $GLOBALS['user']->uid != 1) return;
  
  if($debug = debug_backtrace()){
    foreach($debug as $de){
      if($de['function'] == 'db_query' || $de['function'] == 'db_exec'){
        $rows[] = array('data' => $de['args'][0], 'class' => 'dd_database_debug_args0');
        
        if($de['args'][1]){
          $rows[] = array('data' => dd_array_to_string($de['args'][1]), 'class' => 'dd_database_debug_args1');
        }else{
          $rows[] = '';
        }
        if($de['args'][2]){
          $rows[] = array('data' => dd_array_to_string($de['args'][2]), 'class' => 'dd_database_debug_args2');
        }else{
          $rows[] = '';
        }
        
        $_SESSION['database_debug'][] = $rows;
        
        $_SESSION['database_debug'][][] = array('data' => t('system', '文件：@file(@line 行)', array('@file' => $de['file'], '@line' => $de['line'])),'colspan' => 3);
      }
    }
  }
}