<?php
// $Id$

function field_json($field_id){
  if($_POST['__default_value']){
    if($fetch = db_query('SELECT tid, name FROM {fields_term} WHERE field_id = ? AND pid = ? ORDER BY weight ASC, tid ASC', array($field_id, $_POST['__default_value']))){
      foreach($fetch as $o){
        $json['contents'][] = array('tid' => $o->tid, 'name' => $o->name);
      }
    }
  }
  dd_set_header('Content-Type: text/plain; charset=utf-8');
  if($json){
    $json['error'] = 0;
    echo json_encode($json);
  }else{
    echo '{"error": "1"}';
  }
  exit;
}

/**
 * 字段浏览页面
 */
function field_page($field){
  dd_set_title(array($field->name));
  dd_set_breadcrumb(array(l($field->name, 'fields/'.$field->field_id), $term->name));
  return module_invoke($field->module, 'field_view', $field);
}

/**
 * select、radio、selects、checkbox、tags类型浏览页面
 */
 
function field_term_page($term){
  dd_set_title(array($term->name, $term->field->name));
  dd_set_breadcrumb(array(l($term->field->name, 'fields/'.$term->field->field_id), $term->name));
  return module_invoke($term->field->module, 'field_term_view', $term);
}

/**
 * 分类 rss
 */
function field_term_feed($term){
  return module_invoke($term->field->module, 'field_term_feed', $term);
}