<?php
//$Id$

/**
 * 删除缓存
 * @param (string) $type
 *  删除模式：
 *   cid => 删除指定键名
 *   * => 通配符模式
 *   all => 清空全部缓存表
 *   empty => 清空指定表
 *   table => 删除指定表除永久缓存外的所有数据
 * @param (string) $cid
 *  缓存键名
 * @param (string) $table
 *  表名
 */
function cache_del($type = 'cid', $cid = NULL, $table = 'cache') {
  switch ($type) {
    case 'cid':
      db_exec('DELETE FROM {'. $table .'} WHERE cid = ?', array($cid));
    break;
    case 'all':
      db_exec('DELETE FROM {cache}');
      db_exec('DELETE FROM {cache_menu}');
      db_exec('DELETE FROM {cache_page}');
      db_exec('DELETE FROM {cache_form}');
    break;
    case 'empty':
      db_exec('DELETE FROM {'. $table .'}');
    break;
    case '*':
      db_exec('DELETE FROM {'. $table .'} WHERE cid LIKE ?', array('%'.$cid.'%'));
    break;
    case 'table':
      db_exec('DELETE FROM {'. $table .'} WHERE expire = 0');
  }
}

/**
 * 获取缓存
 * @param (string) $cid
 *  缓存键名
 * @param (string) $table
 *  表名
 * @reutrn object or 0
 */
function cache_get($cid, $table = 'cache') {
  $cache = db_query('SELECT data, created, expire, serialized FROM {'. $table .'} WHERE cid = ?', array($cid), array('return' => 'one'));
  if (isset($cache->created)) {
    if (!$cache->expire || $cache->expire > $_SERVER['REQUEST_TIME']) {
      if ($cache->serialized) {
        $cache->data = unserialize($cache->data);
      }
      return $cache;
    }
  }
  return 0;
}

/**
 * 写入缓存
 * @param (string) $cid
 *  缓存键名，推荐以模块名称开头，如：system_data_cache
 * @param (*) $data
 *  需缓存的数据，任意类型
 * @param (string) $table
 *  存储的表名
 * @param (int) $expire
 *  过期时间，以当前日期为基础，秒为单位。如：500，表示 500 秒后过期。默认为 0，即永不过期
 */
function cache_set($cid, $data, $table = 'cache', $expire = NULL) {
  $cache = array();
  
  $cache['serialized'] = 0; // 序列化
  
  if (is_object($data) || is_array($data)) {
    $data = serialize($data);
    $cache['serialized'] = 1;
  }
  
  $cache['data'] = $data;
  
  if ($expire) {
    $cache['expire'] += $_SERVER['REQUEST_TIME'];
  } else {
    $cache['expire'] = 0;
  }
  
  $cache['cid'] = $cid;
  $cache['created'] = $_SERVER['REQUEST_TIME'];
  
  return db_replace($table, $cache);
}

/**
 * 写入静态页面
 * @param (string) $path
 *  将要保存的页面，md5 值做为缓存唯一标识
 * @param (string) $value
 *  页面内容
 * @param (int) $expire
 *  过期时间，以秒为单位
 */
function cache_set_page($path, $value, $expire = NULL) {
  $oldpath = $path;
  
  // 以路径的 md5 值做为唯一标识
  $cid = md5($path);
  
  // 静态文件保存到 cache/page 目录，确认  cache 目录可写
  $filepath = $GLOBALS['conf_dir'].'/cache/page/';
  
  /**
   * 缓存目录结构，将 md5 值的前 16 位，等分为 4，做为目录，避免一个目录文件过多
   */
  for ($i = 0; $i < 4; ++$i) {
    $filepath .= substr($cid, $i*4, 4).'/';
  }
  
  // 检查目录，不存在则创建
  
  if (file_mkdir($filepath)) {
    // 后 16 位做为文件名，文件名无后缀
    
    $end16 = substr($cid, 16);
    
    if (!$GLOBALS['user']->uid) {
      $filepath .= $end16;
    } else {
      // 为每个登录用户单独生成一份缓存，这将成倍加大缓存空间，建议关闭针对登录用户的页面缓存
      $filepath = file_mkdir($filepath . $end16 .'_user').'/'. $GLOBALS['user']->uid.'_'.$end16;
    }
    
    if ($fp = fopen($filepath, 'wb')) {
      if (flock($fp, LOCK_EX)) { // 锁定
        
        if (!$expire) {
          $expire = var_get('dd_cache_page_expire', 600);
        }
        
        // 将当前时间和缓存过期时间添加在内容最前面
        fwrite($fp, ($_SERVER['REQUEST_TIME'] + $expire) .$_SERVER['REQUEST_TIME']. $value);
        flock($fp, LOCK_UN); // 释放
      }
      fclose($fp);
      
      /**
       * 静态文件均取目录部分，与系统路径对应，用于清除缓存
       */ 
      
      $dirname = dirname($filepath);
      
      if (!db_query('SELECT path FROM {cache_page_list} WHERE dirname = ? AND path = ?',
      array($dirname, $oldpath), array('return' => 'column'))) {
        db_exec('INSERT INTO {cache_page_list} (path, dirname) VALUES (?, ?)', array($oldpath, $dirname));
      }
        
    }
    
  }
}

/**
 * 读取静态页面
 * @param (string) $path
 *  系统路径
 */
function cache_get_page($path) {
  $cid = md5($path);
  
  $filepath = $GLOBALS['conf_dir'].'/cache/page/';
  
  for ($i = 0; $i < 4; ++$i) {
    $filepath .= substr($cid, $i*4, 4).'/';
  }
  
  if (!$GLOBALS['user']->uid) {
    $filepath .= substr($cid, 16);
  } else {
    $filepath .= substr($cid, 16) .'_user/'. $GLOBALS['user']->uid.'_'.substr($cid, 16);
  }
  
  if (is_file($filepath)) {
    $handle = fopen($filepath, 'rb');
    
    // 读取过期时间
    $timestamp = fread($handle, 10);
    
    if ($timestamp > $_SERVER['REQUEST_TIME']) {
      
      // 读取缓存创建时间 
      $timestamp = fread($handle, 10);
      
      // 发送 header，检查客户端版本
      dd_page_cache_header($timestamp);
      
      return fread($handle, filesize($filepath));
    }
    
    fclose($handle);
  }
}

/**
 * 清除页面缓存
 * @param (string) $path
 *  设置清除路径，示例：
 *   * => 全部缓存
 *    当有新的权限注册或权限重新分配时，适用。缓存较多时，将影响效率，建议使用 ftp 删除
 *   user/1 => 清除 user/1 的缓存页面，严格匹配，user/1?page=1、user/1/test 则不会被清除
 *    适用于单一页面
 *   user/1/* => 清除 user/1 及其下的所有缓存页面，不包括子文件夹
 *    适用于路径有规律的页面，比如：node/*，则将清除  node/1 node/2 ... node/x
 *   user/1/*all => 清除 user/1 及其下的所有缓存页面，包括所有子文件夹
 *    适用于 hook_menu 发生变动
 */
function cache_clear_page($path) {
  if ($path == '*') {
    
    $filepath = $GLOBALS['conf_dir'].'/cache/page';
    file_delete_dir($filepath);
    
  } else if (strpos($path, '*all') !== false) {
    
    $path = str_replace(array('/*all', '*all'), '%', $path);
    
    if ($fetch = db_query('SELECT dirname FROM {cache_page_list} WHERE path LIKE ?', array($path))) {
      foreach ($fetch as $o) {
        file_delete_dir($o->dirname);
      }
    }
    
  } else if (strpos($path, '*') !== false) {
    $path = str_replace(array('/*', '*'), '%', $path);
    
    if ($fetch = db_query('SELECT dirname FROM {cache_page_list} WHERE path LIKE ?', array($path))) {
      
      foreach ($fetch as $o) {
        file_delete_dir($o->dirname, 0);
      }
      
    }
    
  } else {
    $cid = md5($path);
    
    $filepath = $GLOBALS['conf_dir'].'/cache/page/';
    
    for ($i = 0; $i < 4; ++$i) {
      $filepath .= substr($cid, $i*4, 4).'/';
    }
    
    $end16 = substr($cid, 16);
    
    $filename = $filepath . $end16;
    
    if (is_file($filename)) @unlink($filename);
    
    file_delete_dir($filepath . $end16 .'_user');
  }
}