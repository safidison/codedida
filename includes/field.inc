<?php
//$Id$

/*
 * 字段表单
 */
function field_form($call_type = NULL, $field = NULL) {
  $options = field_get_type(0);
  $call_info = module_invoke_all('field', 'call_type');
  $title = $call_info[$call_type];
  
  if ($field->fid) {
    $field_type = $field->field_type;
    $call_type = $field->call_type;
    $form_title = t('system', '编辑!name', array('!name' => $title));
    $description = '<div class="field_description">'.l(t('system', '返回所有!name', array('!name' => $title)), $_GET['q']).'</div>';
    
    $form['fields']['fid'] = array(
      '#type' => 'hidden',
      '#default_value' => $field->fid,
      '#constant' => 1
    );
  } else if (!$_GET['field_type']) {
    $form['settings']['#title'] = t('system', '请选择');
    $form['fields']['type'] = array(
      '#type' => 'select',
      '#title' => t('system', '类型'),
      '#required' => 1,
      '#prefix' => '<div id="field_form_type" class="form_item form_item_select">',
      '#default_value' => $field->type,
      '#options' => $options,
    );
    $form['fields']['submit'] = array(
      '#type' => 'submit',
      '#suffix' => '</div>',
      '#submit' => 'field_form_go'
    );
    return $form;
  } else if (!$options[$_GET['field_type']]) {
    dd_set_message(t('system', '不正确的访问'));
    dd_goto($_GET['q']);
  } else {
    $field_type = $_GET['field_type'];
    $form_title = t('system', '添加!name', array('!name' => $title));
    $description = '<div class="field_description">'.l(t('system', '返回!name类型选择', array('!name' => $title)), $_GET['q']).'</div>';
  }
  if (!$field->field_module) {
    $types = field_get_type();
    $field->field_module = $types[$field_type];
  }
  $form['settings'] = array(
    '#title' => $form_title,
    '#description' => $description,
    '#validate' => array('field_form_validate')
  );
  
  $form['fields']['field_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $field_type,
    '#constant' => 1
  );
  $form['fields']['call_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $call_type,
    '#constant' => 1
  );
  $form['fields']['field_module'] = array(
    '#type' => 'hidden',
    '#default_value' => $field->field_module,
    '#constant' => 1
  );
  
  $form['fields']['type_view'] = array(
    '#title' => t('system', '类型'),
    '#value' => $options[$field_type]
  );
  $form['fields']['name'] = array(
    '#title' => t('system', '名称'),
    '#type' => 'textfield',
    '#default_value' => $field->name
  );
  $form['fields']['weight'] = array(
    '#title' => t('system', '权重'),
    '#type' => 'weight',
    '#default_value' => $field->weight ? $field->weight : 0,
    '#start' => -20,
    '#end' => 20
  );
  $form['fields']['required'] = array(
    '#title' => t('system', '必填'),
    '#type' => 'checkbox',
    '#default_value' => $field->required
  );
  
	if ($custom = call_user_func($field->field_module.'_field', 'type_form', $field_type, $call_type, $field)) {
		if (is_array($custom)) {
      $form['fields']['value'] = array(
  	    '#fieldset_prefix' => 'asc',
  	    '#fieldset_legend' => t('system', '更多'),
        '#fieldset_suffix' => 1,
      );
      $form['fields']['value'] += $custom;
		}
	}
	
  if (function_exists($call_type.'_call_field')) {
    if ($t = call_user_func($call_type.'_call_field', 'type_form', $field_type, $field)) {
      if (is_array($t)) {
        $form['fields']['value'] = array_merge($form['fields']['value'], $t);
      }
    }
  }
  
  if (!$form['fields']['value']['description']) {
    $form['fields']['value']['description'] = array(
      '#type' => 'textfield',
      '#title' => t('system', '描述'),
      '#default_value' => $value->value['description'],
      '#attributes' => array('size' => 50)
    );
  }
  $form['fields']['submit']['#type'] = 'submit';
  return $form;
}

function field_form_go($form, $v) {
  dd_goto($_GET['q'], 'field_type='.$v['type']);
}

function field_form_validate($form, $v) {
  if (dd_strlen($v['name']) > 60) {
    $t['name']['#error'][] = t('长度不超过 %length 字符', array('%length' => '60'));
  } else if (dd_strlen($v['value']['description']) > 100) {
    $t['value']['description']['#error'][] = t('长度不超过 %length 字符', array('%length' => '100'));
  } else {
  	$field = (object)$v;
  	$hook_function = $v['field_module'].'_field';
  	
  	// 回调 hook_field 验证
  	if (!$t = $hook_function('type_validate', $v['field_type'], $v['call_type'], $field)) {
  		
  		// 回调 hook_field::call_type 验证
			if (function_exists($v['call_type'].'_call_field')) {
				$call_function = $v['call_type'].'_call_field';
				if ($error = $call_function('type_validate', $v['field_type'], $field)) {
					return $error;
				}
			}
			
	    if ($v['fid']) {
	      // 编辑
	      if (db_exec('UPDATE {fields} SET name = ?, weight = ?, required = ?, value = ? WHERE fid = ?', array($v['name'], $v['weight'], $v['required'], serialize($v['value']), $v['fid']), array('return' => 1))) {
	        dd_set_message(t('编辑成功'));
	      } else {
	        dd_set_message(t('编辑失败'));
	        return true;
	      }
	    } else {
	      // 添加
	      if (db_exec('INSERT INTO {fields} (name, call_type, field_type, field_module, weight, required, value) VALUES (?, ?, ?, ?, ?, ?, ?)', array($v['name'], $v['call_type'], $v['field_type'], $v['field_module'], $v['weight'], $v['required'], serialize($v['value'])))) {
          $v['fid'] = db_last_insert_id();
	        dd_set_message(t('保存失败'));
	        return true;
	      } else {
	        dd_set_message(t('保存成功'));
	      }
	    }
	    
	    $field->fid = $v['fid'];
	    
	    $hook_function('type_save', $v['field_type'], $v['call_type'], $field);
	    if ($call_function) {
	    	$call_function('type_save', $v['field_type'], $field);
	    }
	  }
  }
  return $t;
}

/*
 * 获取所有模块定义的字段类型
 */
function field_get_type($all = 1) {
  static $options;
  $t = array();
  if (!isset($options)) {
    $options = _module_invoke_all('field_type_list');
  }
  if (is_array($options)) {
    if ($all) {
      foreach ($options as $module => $opt) {
        foreach ($opt as $key => $c) {
          if (!is_numeric($key)) {
            $t[$key] = $module;
          }
        }
      }
    } else {
      foreach ($options as $module => $opt) {
        foreach ($opt as $key => $c) {
          if (!is_numeric($key)) {
            $t[$key] = $c;
          }
        }
      }
    }
  }
  return $t;
}

/**
 * 根据 field_type 获取字段表单
 */
function field_get_field_list($call_type) {
  $call_info = module_invoke_all('field', 'call_type');
  
  switch ($_GET['field_op']) {
    case 'update':
      if ($field = db_query('SELECT * FROM {fields} WHERE fid = ?', array($_GET['fid']), array('return' => 'one'))) {
        if ($field->value) $field->value = unserialize($field->value);
        return dd_get_form('field_form', $call_type, $field, $call_info[$call_type]);
      } else {
        dd_set_message(t('system', '错误的访问'), 'error');
      }
    break;
    case 'delete':
      if ($field = db_query('SELECT * FROM {fields} WHERE fid = ?', array($_GET['fid']), array('return' => 'one'))) {
        if ($field->value) $field->value = unserialize($field->value);
        if (db_exec('DELETE FROM {fields} WHERE fid = ?', array($_GET['fid']))) {
          $function = $field->field_module .'_field';
          if (function_exists($function)) {
            $function('type_delete', $o->field_type, $o->call_type, $field);
          }
        }
        dd_goto($_GET['q']);
      } else {
        dd_set_message(t('system', '错误的访问'), 'error');
      }
  }
  
  $options = module_invoke_all('field', 'type_list');
  
  if ($fetch = db_query('SELECT * FROM {fields} WHERE call_type = ? ORDER BY weight ASC, fid DESC', array($call_type))) {
    foreach ($fetch as $o) {
      $table[] = array(
        $options[$o->field_type], $o->name,
        dd_get_required($o->required),
        dd_imp(array(
          l('system', '编辑', $_GET['q'], array('query' => 'field_op=update&fid='.$o->fid)),
          l('system', '删除', $_GET['q'], array('query' => 'field_op=delete&fid='.$o->fid, 'attributes' => array('class' => 'confirm')))
        ))
      );
    }
  }
  if ($table) {
    return theme('table', array(t('system', '类型'), t('system', '名称'), t('system', '是否必填'), t('system', '操作')), $table);
  } else {
    return theme('no_content', t('system', '还没有创建字段'));
  }
}
  