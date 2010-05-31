<?php
//$Id$

cache_memcache_onnect();

function cache_del($type = 'cid', $cid = NULL, $table = 'cache') {
  global $mem;
  switch($type){
    case 'cid':
      $mem->delete($table.'_'.$cid, 0);
    break;
    case 'all': // 全部清除
      $mem->flush();
    break;
    case 'empty': // 清除表中所有数据
      if($keys = $mem->get('__table_'.$table.'_keys')){
        foreach($keys as $key => $k){
          $mem->delete($key, 0);
        }
      }
    break;
    case '*': // 模糊匹配
      if($keys = $mem->get('__table_'.$table.'_keys')){
        foreach($keys as $key => $k){
          if(strpos($key, $cid) !== false){
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
  if($data = $mem->get($table.'_'.$cid)){
    $cache->data = $data;
    return $cache;
  }
}

function cache_set($cid, $data, $table = 'cache', $expire = NULL) {
  global $mem;
  
  if(!$expire) {
    $expire = 259200;
  }
  
  $key = $table.'_'.$cid;
  if($keys = $mem->get('__table_'.$table.'_keys')){
    if(!$keys[$key]){
      $keys[$key] = 1;
      $mem->set('__table_'.$table.'_keys', $keys, MEMCACHE_COMPRESSED, 259200);
    }
  }else{
    $mem->set('__table_'.$table.'_keys', array($key => 1), MEMCACHE_COMPRESSED, 259200);
  }
  
  $mem->set($key, $data, MEMCACHE_COMPRESSED, $expire);
}

/**
 * memcache 连接
 */
function cache_memcache_onnect() {
  global $mem;
  if(!isset($mem)){
    $mem = new Memcache;
    if($mem->pconnect(var_get('cache_memcache_host', 'localhost'), var_get('cache_memcache_port', 11211))){
      $mem->setCompressThreshold(20000, 0.2);
    }
  }
}

/**
 * 缓存自定义配置
 * @param (array) $form
 */
function cache_settings_custom_form(&$form){
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