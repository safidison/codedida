<?php
//$Id$

function cache_del($type = 'cid', $cid = NULL, $table = 'cache') {
  switch($type){
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
      db_exec('DELETE FROM {'. $table .'} WHERE expire == 0');
  }
}

function cache_get($cid, $table = 'cache') {
  $cache = db_query('SELECT data, created, expire, serialized FROM {'. $table .'} WHERE cid = ?', array($cid), array('return' => 'one'));
  if (isset($cache->data)) {
    if (!$cache->expire || $cache->expire > $_SERVER['REQUEST_TIME']) {
      if ($cache->serialized) {
        $cache->data = unserialize($cache->data);
      }
      return $cache;
    }
  }
  return 0;
}

function cache_set($cid, $data, $table = 'cache', $expire = NULL) {
  $serialized = 0;
  if (is_object($data) || is_array($data)) {
    $data = serialize($data);
    $serialized = 1;
  }
  
  if($expire) {
    $expire += $_SERVER['REQUEST_TIME'];
  }else{
    $expire = 0;
  }
  
  if(!db_exec('UPDATE {'. $table .'} SET data = ?, created = ?, expire = ?, serialized = ? WHERE cid = ?', array($data, $_SERVER['REQUEST_TIME'], $expire, $serialized, $cid))){
    @db_exec('INSERT INTO {'. $table .'} (cid, data, created, expire, serialized) VALUES (?, ?, ?, ?, ?)', array($cid, $data, $_SERVER['REQUEST_TIME'], $expire, $serialized));
  }
}