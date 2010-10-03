<?php
//$Id$

cache_memcache_onnect();

function cache_del($type = 'cid', $cid = NULL, $table = 'cache') {
  global $mem;
  switch ($type) {
    case 'cid':
      $mem->delete($table.'_'.$cid, 0);
    break;
    case 'all': // 全部清除
      $mem->flush();
    break;
    case 'empty': // 清除表中所有数据
      if ($keys = $mem->get('__table_'.$table.'_keys')) {
        foreach ($keys as $key => $k) {
          $mem->delete($key, 0);
        }
      }
    break;
    case '*': // 模糊匹配
      if ($keys = $mem->get('__table_'.$table.'_keys')) {
        foreach ($keys as $key => $k) {
          if (strpos($key, $cid) !== false) {
            $mem->delete($key, 0);
          }
        }
      }
    break;
    case 'table': // 清除表中已到过期缓存，memcache 会自动清除
  }
}

function cache_get($cid, $table = 'cache') {
  global $mem;
  if ($data = $mem->get($table.'_'.$cid)) {
    $cache->data = $data;
    return $cache;
  }
}

function cache_set($cid, $data, $table = 'cache', $expire = NULL) {
  global $mem;
  
  if (!$expire) {
    $expire = 2592000;
  }
  
  $key = $table.'_'.$cid;
  if ($keys = $mem->get('__table_'.$table.'_keys')) {
    if (!$keys[$key]) {
      $keys[$key] = 1;
      $mem->set('__table_'.$table.'_keys', $keys, MEMCACHE_COMPRESSED, 2592000);
    }
  } else {
    $mem->set('__table_'.$table.'_keys', array($key => 1), MEMCACHE_COMPRESSED, 2592000);
  }
  
  $mem->set($key, $data, MEMCACHE_COMPRESSED, $expire);
}

/**
 * memcache 连接
 */
function cache_memcache_onnect() {
  global $mem;
  if (!isset($mem)) {
    $mem = new Memcache;
    if ($mem->pconnect(var_get('cache_memcache_host', 'localhost'), var_get('cache_memcache_port', 11211))) {
      $mem->setCompressThreshold(20000, 0.2);
    }
  }
}

/**
 * 缓存自定义配置
 * @param (array) $form
 */
function cache_settings_custom_form(&$form) {
  $form['fields']['cache_memcache_host'] = array(
    '#title' => t('system', 'memcache 主机'),
    '#default_value' => var_get('cache_memcache_host', 'localhost'),
    '#type' => 'textfield',
    '#required' => 1
  );
  $form['fields']['cache_memcache_port'] = array(
    '#title' => t('system', 'memcache 端口'),
    '#default_value' => var_get('cache_memcache_port', 11211),
    '#type' => 'textfield',
    '#required' => 1
  );
}

/**
 * 以下静态页面缓存，与 cache.inc 相同
 * 静态页面数据较大，且多，不适合使用 memcache
 */

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
      //dd_page_cache_header($timestamp);
      
      dd_page_header();
      
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