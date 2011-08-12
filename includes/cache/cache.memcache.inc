<?php
//$Id$

if (cache_memcache_onnect()) {
  /**
   * 缓存清除
   * @param (string) $type
   * 	操作类型：
   * 		cid - 按 cid 清除
   * 		all - 清空所有缓存表
   *    empty - 清空指定缓存表
   *    * - 按关键字模糊清除
   *    table - 清除指定表过期内容
   * @param (string) $cid
   * 	指定 cid
   * @param (string) $table
   *  指定缓存表
   */
  function cache_del($type = 'cid', $cid = NULL, $table = 'cache') {
    global $mem, $mem_prefix;
    switch ($type) {
      case 'cid':
        $mem->delete($mem_prefix . $table . '_' . $cid, 0);
      break;
      case 'all': // 全部清除
        $mem->flush();
      break;
      case 'empty': // 清除表中所有数据
        if ($keys = $mem->get('__table_' . $mem_prefix . $table . '_keys')) {
          foreach ($keys as $key => $k) {
            $mem->delete($key, 0);
          }
        }
      break;
      case '*': // 模糊匹配
        if ($keys = $mem->get('__table_' . $mem_prefix . $table . '_keys')) {
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

  /**
   * 获取缓存
   * @param (string) $cid
   * 	缓存 id
   * @param (string) $table
   * 	缓存表名
   * @return (object) $cache
   */
  function cache_get($cid, $table = 'cache') {
    global $mem, $mem_prefix;
    if ($data = $mem->get($mem_prefix . $table . '_' . $cid)) {
      $cache->data = $data;
      return $cache;
    }
  }

  /**
   * 写入缓存，若已存在，则更新
   * @param (string) $cid
   * 	缓存 id
   * @param (*) $data
   * 	待写入数据
   * @param (string) $table
   * 	缓存表名
   * @param (int) $expire
   * 	过期时间，从当前时间算起，以秒为单位
   */
  function cache_set($cid, $data, $table = 'cache', $expire = NULL) {
    global $mem, $mem_prefix;
    
    if (!$expire) {
      $expire = 2592000;
    }
    
    $key = $mem_prefix . $table.'_'.$cid;
    $mem_table = '__table_' . $mem_prefix . $table . '_keys';
    
    if ($keys = $mem->get($mem_table)) {
      if (empty($keys[$key])) {
        $keys[$key] = 1;
        $mem->set($mem_table, $keys, MEMCACHE_COMPRESSED, 2592000);
      }
    } else {
      $mem->set($mem_table, array($key => 1), MEMCACHE_COMPRESSED, 2592000);
    }
    
    $mem->set($key, $data, MEMCACHE_COMPRESSED, $expire);
  }

} else {
  // 无法连接 memcache，使用默认缓存
  require_once DIDA_ROOT . '/includes/cache.inc';
}

/**
 * memcache 连接
 */
function cache_memcache_onnect() {
  global $mem, $mem_prefix;
  
  if (!isset($mem)) {
    $mem = new Memcache;
    $mem_prefix = var_get('cache_memcache_prefix', '');
    if ($mem->pconnect(var_get('cache_memcache_host', 'localhost'), var_get('cache_memcache_port', 11211))) {
      $mem->setCompressThreshold(20000, 0.2);
      return true;
    }
  }

  return false;
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
  $form['fields']['cache_memcache_prefix'] = array(
    '#title' => t('system', 'memcache 前缀'),
    '#default_value' => var_get('cache_memcache_prefix', str_replace('.', '', $_SERVER['HTTP_HOST'])),
    '#description' => '当有多个站点连接同一个 memcache 服务时适用',
    '#type' => 'textfield',
    '#required' => 1
  );
}
