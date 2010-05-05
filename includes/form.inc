<?php
// $Id$

/**
  $form['fields']['account'] = array(
    '#type' => 'textfield',
    '#title' => '用户名',
    '#required' => true,
    '#default_value' => $ac[0],
    '#prefix' => '包裹前缀',
    '#suffix' => '包裹后缀',
    '#field_prefix' => '字段前缀',
    '#field_suffix' => '字段后缀',
    '#attributes' => array(
      'id' => 'user_login',
      'size' => 50,
      'class' => "{required:true, messages:{required:'请输入用户名'}}"
    ),
    '#autocomplete' => array(
      'url' => url('user/tttt'),
    )
  );
*/

function dd_get_form($form_id) {
  global $conf, $user;
  dd_add_js('misc/form.js');
  
  $args = func_get_args();
  array_shift($args);
  
  if($_POST['form_id'] == $form_id){
  	$form_token = $_POST['form_token'];
  }else{
  	$form_token = md5(uniqid(mt_rand(), true));
  }
  
  $form = call_user_func_array($form_id, $args);
  
  if(!is_array($form['fields'])){
    return false;
  }
  
  if($form['#form_token']){
    $form_token = $form['#form_token'];
  }
  
  $form['__args'] = $args;
  
	module_alter_all('form_alter', $form_id, $form_token, $form);
  
  $form['#args'] = array(
  	'method' => 'post',
  	'action' => "",
  	'id' => $form_id,
  	'accept-charset' => 'UTF-8',
  );
  
  if($form['settings'] && is_array($form['settings']['#attributes'])){
  	$form['#args'] = array_merge($form['#args'], $form['settings']['#attributes']);
  }
  
  $form['#form_id'] = $form_id;
  $form['#form_token'] = $form_token;

  $settings = $form['settings'];
  
  if(!$settings['#redirect'] && $form['#args']['#action']){
  	$settings['#redirect'] = $form['#args']['#action'];
  }
  
  $fields = $form['fields'];
  
  //如果表单请求ajax验证，则载入以下js。
  if($settings['#ajax_validate']){
    dd_add_js('misc/jquery.validate.js');
    dd_add_js('misc/jquery.metadata.js');
    if(isset($settings['#ajax_submit'])){
      //如请求验证完成后ajax提交，则载入以下js。
      //ajax默认提交到当前url，可在表单中设置
      /*
        '#ajax_validate' => true,
        '#ajax_submit' => array(
          'des' => true,
          'reset' => true,
          'options' => array(
            'dataType' => 'script',
            'url' => url('user/register', array('query' => array('ajax' => 'submit'))),
          ),
        ),
      */
      //ajax验证和提交将跳过系统默认验证，应在ajax页面调用dd_ajax_validate()来验证
      if($settings['#ajax_submit']['des']){
        $output = '<div id="ajax_description" class="description"></div>';
      }
      dd_add_js('misc/jquery.form.js');
    }
    $value = array(
      'form_id' => $form_id,
      'ajax_submit' => isset($settings['#ajax_submit']),
      'options' => $settings['#ajax_submit']['options'],
    );
    dd_add_js(theme('form_ajax', $value), 'inline');
  }
  
  // 处理当前提交的表单
  if($_POST['form_id'] == $form_id){
  	// 基本验证：必填字段，选项字段数据是否被修改
    if(!$error = dd_form_required($form, $_POST)){
    	// 回调表单验证函数
    	
      if(is_array($settings['#validate'])){
        foreach($settings['#validate'] as $function){
          if($error = $function($form, $_POST)){
            break;
          }
        }
      }
      
      // 所有验证通过后再处理文件字段
      if(!$error && (!$error = file_form_validate($form))){
      	// 文件字段处理完毕后回调提交函数
      	
        //删除表单session，防止重复提交
        dd_del_form_cache($user->uid, $form_id, $form_token);
        
        //回调提交函数，处理数据。
        form_save_submit($form, $_POST);
        
        //重定向页面
        
        session_write_close();
        
        if($_GET['redirect']){
        	$redirect = $_GET['redirect'];
        }else if($settings['#redirect']){
        	$redirect = $settings['#redirect'];
        }else{
        	$redirect = url($_GET['q']);
        }
        
        header('Location: '. urldecode($redirect));
        exit();
      }
    }
  }else if(!$form['settings']['#noCache']){
  	dd_set_form_cache($form);
  }
  
  $fields = dd_form_sort(dd_form_field($form, $_POST, $error));
  
  $output .= '<form '.dd_attributes($form['#args']).'>';
  if($settings['#title']){
    $output .= '<h3>'.$settings['#title'].'</h3>';
  }
  if($settings['#description']) {
    $output .= '<div class="description form_description form_setting_description">'.$settings['#description'].'</div>';
  }
  
  $output .= dd_form_html($form, $fields);
  $output .= dd_form_hidden(array('#name' => 'form_id', '#value' => $form_id));
  $output .= dd_form_hidden(array('#name' => 'form_token', '#value' => $form_token));
  $output .= '</form>';
  return '<div class="'.$form_id.'">' .$output. '</div>';
}

/**
 * 提交按钮格式：
 * $form['submit'] = array(
 *  '#type' => 'submit',
 * 	'#submit' => 'user_register_submit',
 * 	'#value' => '确认注册'
 * );
 * 表单内允许多个提交按钮，但仅会处理当前被提交项。
 * 不论提交函数中的处理结果如何，均视为成功，并重定向。
 * 表单重定向顺序：
 * 	$_GET['redirect'] > $form['#redirect'] > $form['#action'] > $_GET['q']。
 * 基于此，数据合法性等应在表单验证环节处理，提交环节只负责数据写入或删除。
 * 可通过如下方式在提交函数中处理异常：
 * 	发现异常后重定向到某个页面，脱离默认表单处理流程。
*/
function form_save_submit($form, $value){
  foreach($form['fields'] as $key => $field){
    if(strpos($key, '#') === false){
      if(($field['#type'] == 'submit') && $value[$key]){
        if($function = $field['#submit']){
          return $function($form, $value);
        }
      }else if(is_array($field)){
        _form_save_submit($field, $value[$key], $value);
      }
    }
  }
}

function _form_save_submit($fields, $subvalue, $value){
  foreach($fields as $key => $field){
    if(strpos($key, '#') === false){
      if(($field['#type'] == 'submit') && $subvalue[$key]){
        if($function = $field['#submit']){
          return $function($form, $value);
        }
      }else if(is_array($field)){
        _form_save_submit($field, $subvalue[$key], $value);
      }
    }
  }
}

/**
 * ajax 验证
 * ___args settings 
 */
function dd_ajax_validate($uid, $form_id, $form_token){
  
	if($cache = dd_get_form_cache($uid, $form_id, $form_token)){
	  $form = call_user_func_array($form_id, $cache['__args']);
		module_alter_all('form_alter', $form_id, $form_token, $form);
		$form['#form_id'] = $form_id;
    $form['#form_token'] = $form_token;
    
		if($error = dd_form_required($form, $_POST)){
			return $error;
		}else if($cache['settings'] && is_array($cache['settings']['#validate'])){
      foreach($cache['settings']['#validate'] as $function){
        if($error = $function($form, $_POST)){
          return $error;
        }
      }
		}
    
    dd_del_form_cache($uid, $form_id, $form_token);
    
	}
}

/**
 * 获取表单缓存
 */
function dd_get_form_cache($uid, $form_id, $form_token){
  $cid = $uid.$form_id.$form_token;
  
	static $cache;
	if(!isset($cache[$cid])){
		$cache[$cid] = $_SESSION['form_token'][$cid];
	}
	return $cache[$cid];
}

/**
 * 生成表单缓存
 */
function dd_set_form_cache($form) {
  $data = array(
  	'form_token' => $form['#form_token'],
  	'settings' => $form['settings'],
  	'form_id' => $form['#form_id'],
  	'__args' => $form['__args'],
    '__timestamp' => $_SERVER['REQUEST_TIME']
  );
  
  $_SESSION['form_token'][$GLOBALS['user']->uid.$form['#form_id'].$form['#form_token']] = $data;
}

/**
 * 删除表单缓存
 */
function dd_del_form_cache($uid, $form_id, $form_token){
  unset($_SESSION['form_token'][$uid.$form_id.$form_token]);
}

/**
 * 默认验证，验证表单合法性、必填字段
 */
function dd_form_required($form, $value){
	global $user;
	
	if(!$form['settings']['#noCache']){
	  $cache = dd_get_form_cache($user->uid, $form['#form_id'], $_POST['form_token']); // 获取表单初始化缓存数据
    if(!$cache || ($_POST['form_token'] != $cache['form_token'])){
    	dd_set_message(t('system', '表单被非法修改，请重新进入页面'), 'error');
    	$error[] = t('system', '表单被非法修改，请重新进入页面');
      return $error;
    }
	}
  
  foreach($form['fields'] as $key => $field){
    if($field['#type'] || isset($field['#value'])){
      $field['#name'] = array($key);
      if($t = _dd_form_required($form, $field, $value)){
        $error[$key]['#error'] = $t;
      }
    }else if(is_array($field)){
      $sub_field = array();
      foreach($field as $child_key => $child){
        if(strpos($child_key, '#') === false){
          $sub_field[$child_key] = $child;
          $sub_field[$child_key]['#name'] = array($key);
        }
      }
      if($t = _dd_form_required_subfield($form, $sub_field, $value)){
        $error[$key] = $t;
      }
    }
  }
  return $error;
}

function _dd_form_required_subfield($form, $field, $value){
  static $error;
  foreach($field as $sub_key => $sub_field){
    $sub_field['#name'] = array_merge($sub_field['#name'], array($sub_key));
    if($sub_field['#type'] || isset($sub_field['#value'])){
      if($t = _dd_form_required($form, $sub_field, $value)){
        $error[$sub_key]['#error'] = $t;
      }
    }else if(is_array($sub_field)){
      $child_field = array();
      foreach($sub_field as $child_key => $child){
        if(strpos($child_key, '#') === false){
          $child_field[$child_key] = $child;
          $child_field[$child_key]['#name'] = $sub_field['#name'];
        }
      }
      _dd_form_required_subfield($form, $child_field, $value);
    }
  }
  return $error;
}

function _dd_form_required($form, $field, $value){
  $val = dd_get_value($value, $field['#name']);
  $name = array_shift($field['#name']);
  if(count($field['#name'])){
    $name .= '['. implode('][', $field['#name']) .']';
  }
  $field['#name'] = $name;
	
  if(($field['#type'] != 'file') && $field['#required'] && ($val == '')){
    $error[] =  t('system', '%text 不能为空', array('%text' => $field['#title']));
  }
  if($field['#options']){
    foreach($field['#options'] as $key => $k){
      $options[] = $key;
    }
  }
  switch($field['#type']){
    case 'select': case 'radio':
      if($val != '' && !in_array($val, $options)){
        $er = true;
      }
    break;
    case 'selects': case 'checkbox':
      if($options && is_array($val)){
        foreach($val as $check){
          if(!in_array($check, $options)){
            $er = true;
            break;
          }
        }
      }else if($val){
        if($val != 1){
          $er = true;
        }
      }
    break;
    case 'captcha':
      if($val != $_SESSION['captcha'][$form['#form_token'].$form['#form_id'] . $field['#name']]){
        $error[] = t('system', '验证码不正确');
      }
    break;
    case 'hidden':
      if($field['#constant'] && $field['#default_value'] != $val){
        $error[] = true;
        dd_set_message(t('system', $field['#name']. '%string 出现非法选择，请重新进入', array('%string' => $field['#name'])), 'error');
      }
  }
  
  if($er){
    $error[] = t('system', '%string 出现非法选项，请重新提交', array('%string' => $field['#title']));
  }
	
  return $error;
}

/**
 * 表单字段转换为 html
 */
function dd_form_html($form, $fields){
  if(!$form['settings']['#theme']){
    foreach($fields as $key => $field){
      if((strpos($key, '#') === false)){
        if($field['#html']){
          if(!$field['#theme']){
            $output .= $field['#html'];
          }else{
            $output .= call_user_func($field['#theme'], $field);
          }
        }else if(is_array($field)){
          $output .= _dd_form_fieldset($field, 'prefix');
          if(!$field['#theme']){
            $output .= dd_form_html($form, $field);
          }else{
            $output .= call_user_func($field['#theme'], $field);
          }
          $output .= _dd_form_fieldset($field, 'suffix');
        }
      }
    }
  }else{
    $output = call_user_func($form['settings']['#theme'], $form, _dd_form_html($fields));
  }
  return $output;
}

function _dd_form_html($fields){
  
  foreach($fields as $key => $field){
    if(strpos($key, '#') === false){
      if(is_array($field)){
        if($field['#html']){
          if(!$field['#theme']){
            $result[$key] = $field;
          }else if(function_exists($field['#theme'])){
            $result[$key] = call_user_func($field['#theme'], $field);
          }
        }else{
          $result[$key] = _dd_form_html($field);
        }
      }
    }
  }
  return $result;
}

//字段排序
function dd_form_sort($fields){
  foreach($fields as $key => $child){
    //if(isset($child['#children'])){
    if(strpos($key, '#') === false && is_array($child)){
      uasort($child, 'dd_form_cmp');
      //$child = array_reverse($child, true);
      $childs = dd_form_sort_child($child);
      $fields[$key] = $childs;
    }
  }
  uasort($fields, 'dd_form_cmp');
  //$fields = array_reverse($fields, true);
  return $fields;
}

//子字段排序
function dd_form_sort_child($fields){
  foreach($fields as $key => $child){
    if(strpos($key, '#') === false && is_array($child)){
      uasort($child, "dd_form_cmp");
      //$child = array_reverse($child, true);
      $childs = dd_form_sort_child($child);
      $fields[$key] = $childs;
    }
  }
  return $fields;
}

/**
 * 字段按权重排序
 */
function dd_form_cmp($a, $b){
	
  $a_weight = isset($a['#weight']) ? $a['#weight'] : 0;
  $b_weight = isset($b['#weight']) ? $b['#weight'] : 0;
  if ($a_weight == $b_weight) {
    return 0;
  }
  return ($a_weight < $b_weight) ? -1 : 1;
}

/**
 * 解析表单
 */
function dd_form_field(&$form, $value, $error){
  
  foreach($form['fields'] as $key => $field){
    if($field['#type'] || isset($field['#value'])){
      $field['#name'] = array($key);
      $fields[$key] = _dd_form_element($form, $field, $value, $error);
    }else if(is_array($field)){
      $parents = $sub_field = array();
      foreach($field as $child_key => $child){
        if(strpos($child_key, '#') === false){
          $sub_field[$child_key] = $child;
          $sub_field[$child_key]['#name'] = array($key);
        }else{
          $parents[$child_key] = $child;
        }
      }
      $fields[$key] = dd_form_subfield($form, $sub_field, $value, $error);
      if($parents && $fields[$key]){
        $parents['#children'] = count($sub_field);
        $fields[$key] += $parents;
      }
      
			$fields[$key]['#weight'] = _dd_form_weight($form['#form_id'], $field['#weight']);
			
			//echo $fields[$key]['#weight'] . $key;
    }
  }
  return $fields;
}

/**
 * 多级
 */
function dd_form_subfield(&$form, $field, $value, $error){
  foreach($field as $sub_key => $sub_field){
    $sub_field['#name'] = array_merge($sub_field['#name'], array($sub_key));
    if($sub_field['#type'] || isset($sub_field['#value'])){
      $fields[$sub_key] = _dd_form_element($form, $sub_field, $value, $error);
    }else if(is_array($sub_field)){
      $parents = $child_field = array();
      foreach($sub_field as $child_key => $child){
        if(strpos($child_key, '#') === false){
          $child_field[$child_key] = $child;
          $child_field[$child_key]['#name'] = $sub_field['#name'];
        }else{
          $parents[$child_key] = $child;
        }
      }
      $fields[$sub_key] = dd_form_subfield($form, $child_field, $value, $error);
      if($parents && $fields[$sub_key]){
        $parents['#children'] = count($child_field);
        $fields[$sub_key] += $parents;
      }
      $fields[$sub_key]['#weight'] = _dd_form_weight($form['#form_id'], $sub_field['#weight']);
    }
  }
  return $fields;
}

function dd_get_value($value, $name){
  $count = count($name);
  for($i = 0; $i < $count; ++$i){
    if(isset($value[$name[$i]])){
      if(is_array($value[$name[$i]])){
        if($i != ($count -1)){
          $value = $value[$name[$i]];
        }else{
          return $value[$name[$i]];
        }
      }else{
        return $value[$name[$i]];
      }
    }
  }
  return false;
}

function dd_get_error($value, $name){
  $count = count($name);
  for($i = 0; $i < $count; ++$i){
    if($value[$name[$i]]){
      if($value[$name[$i]]['#error']){
        return $value[$name[$i]]['#error'];
      }else if(is_array($value[$name[$i]])){
        $value = $value[$name[$i]];
      }
    }
  }
  return false;
}

function _dd_form_weight($form_id, $weight = 0){
	static $i = array();
	if($weight == 0){
		$weight += $i[$form_id];
	}
	++$i[$form_id];
	return $weight;
}

function _dd_form_fieldset($field, $type){
  if($type == 'prefix'){
    if($field['#fieldset_prefix']){
      if(!$field['#fieldset_attributes'] || !$field['#fieldset_attributes']['class']){
        $field['#fieldset_attributes']['class'] = 'collapsible';
      }else{
        $field['#fieldset_attributes']['class'] .= ' collapsible';
      }
      if($field['#fieldset_prefix'] == 'asc'){
        $output .= '<fieldset '.dd_attributes($field['#fieldset_attributes']) .'><legend class="collapse-processed asc">';
    	}else{
        $field['#fieldset_attributes']['class'] .= ' fieldset-hide';
    		$output .= '<fieldset '.dd_attributes($field['#fieldset_attributes']) .'><legend class="collapse-processed desc">';
    	}
    	$output .= '<a href="#">'.$field['#fieldset_legend'].'</a></legend><div class="fieldset-wrapper">';
    }
    if(!$field['#type'] && $field['#description']){
      $output .= '<div class="description form_description form_setting_description">'.$field['#description'].'</div>';
    }
  }else if($field['#fieldset_suffix']){
  	$output = '</div></fieldset>';
  }
  
  return $output;
}

/**
 * 解析字段
 */
function _dd_form_element(&$form, $field, $value, $error){
  $field['#weight'] = _dd_form_weight($form['#form_id'], $field['#weight']);
  $field['#parents'] = $field['#name'];
  $name = array_shift($field['#name']);
  if(count($field['#name'])){
    $name .= '['. implode('][', $field['#name']) .']';
  }
  $field['#name'] = $name;
  
  if($value['form_id'] == $form['#form_id']){
    $val = dd_get_value($value, $field['#parents']);
    if($val !== false){
      $field['#value'] = $val;
    }
    $field['#error'] = dd_get_error($error, $field['#parents']);
  }else if(!isset($field['#value'])){
    $field['#value'] = isset($field['#default_value']) ? $field['#default_value'] : '';
  }
  
  if($field['#attributes']['class']){
    $class[] = $field['#attributes']['class'];
  }
  
  if($field['#required'] && $field['#type'] != 'checkbox'){
    $class[] = 'required';
  }
  
  if(in_array($field['#type'], array('textfield', 'password', 'captcha'))){
    $class[] = 'form_text';
  }
  
  if($class) $field['#attributes']['class'] = implode(' ', $class);
  $field['#form_id'] = $form['#form_id'];
  
  $field['__name'] = str_replace(array('[', ']'), array('_'), $field['#name']);
  
  if ($field['#disabled']) {
    $field['#attributes']['disabled'] = 'disabled';
  }
  
  if($field['#fieldset_prefix']){
    $field['#fieldset_prefix_html'] = _dd_form_fieldset($field, 'prefix');
  }
  if($field['#fieldset_suffix']){
    $field['#fieldset_suffix_html'] = '</div></fieldset>';
  }
  
  if(function_exists('dd_form_'.$field['#type'])){
    if($field['#type'] == 'file' && !$form['#args']['enctype']){
      $form['#args']['enctype'] = 'multipart/form-data';
    }
    if(!$field['#attributes']['id']){
      $field['#attributes']['id'] = $field['#form_id'].'_type_'.$field['__name'];
    }
    $element = call_user_func('dd_form_'.$field['#type'], $field, $form). $field['#field_suffix'];
  }else if($field['#value']){
    $form['#type'] = 'value';
    $element = dd_form_label($field) . $field['#value'] . $field['#field_suffix'];
  }
  if($field['#description']) {
    $element .= '<span class="description form_description">'.$field['#description'].'</span>';
  }
  if(isset($field['#prefix']) || isset($field['#suffix'])){
    $element =  $field['#fieldset_prefix_html'] . $field['#prefix'] .$element. $field['#suffix'] . $field['#fieldset_suffix_html'];
  }else if(!$form['settings']['#theme'] && $field['#type'] != 'hidden'){
    $element = $field['#fieldset_prefix_html'] . '<div id="'.$field['#form_id'].'_'.$field['__name'].'" class="form_item form_item_'.$field['#type'].'">'."\n" .$element. "\n</div>\n"  . $field['#fieldset_suffix_html'];
  }
  $field['#html'] = $element;
  form_set_ahah($field);
  return $field;
}

//字段错误信息
function dd_form_error($field){
  if($field['#error'][0]){
    return '<span class="form_error">'.implode('<br>', $field['#error'])."</span>\n";
  }
}

//单行文本字段
function dd_form_textfield($field){
  if($field['#autocomplete']){
    form_set_auto($field);
  }
  
  if(strpos($field['#attributes']['class'], 'form_text') === false) {
    $field['#attributes']['class'] = 'form_text';
    if($field['#required']){
      $field['#attributes']['class'] .= ' required';
    }
  }
  return dd_form_label($field) . '<input type="text" name="'.$field['#name'].'" value="'.check_plain($field['#value']).'"'.dd_attributes($field['#attributes']).'/>' . dd_form_error($field);
}

//隐藏字段
function dd_form_hidden($field){
  return '<input type="hidden" name="'.$field['#name'].'" value="'.$field['#value'].'"'.dd_attributes($field['#attributes']).'/>';
}

function dd_form_weight($field){
  $start = $field['#start'] ? $field['#start'] : 0;
  $end = $field['#end'] ? $field['#end'] : 20;
  foreach(range($start, $end) as $v){
    $opt[$v] = $v;
  }
  
  if(!$field['#value'])	$field['#value'] = 0;
  
  $field['#options'] = $opt;
  
  if($field['#attributes']['class']){
    $field['#attributes']['class'] .= ' weight dd_form_ajax_field';
  }else{
    $field['#attributes']['class'] = 'weight';
  }
  
  return dd_form_select($field);
}

//密码字段
function dd_form_password($field){
  return dd_form_label($field) . '<input type="password" name="'.$field['#name'].'" value="'.$field['#value'].'"'.dd_attributes($field['#attributes']).'/>' . dd_form_error($field);
}

//多行文本框字段
function dd_form_textarea($field){
  if(!$field['#attributes']['rows']){
    $field['#attributes']['rows'] = 10;
  }
  if(!$field['#attributes']['cols']){
    $field['#attributes']['cols'] = 60;
  }
  return dd_form_label($field) . '<div class="form_textarea"><textarea name="'.$field['#name'].'"'.dd_attributes($field['#attributes']).'>'.$field['#value'].'</textarea></div>' . dd_form_error($field);
}

//验证码字段
function dd_form_captcha($field, $form = NULL){
  //require_once './modules/system/system.captcha.inc';
  /*
   array('type' => 1, 'image' => 100x100)
  */
  $array = array(
    'name' => $field['#name'],
    'form_id' => $field['#form_id'],
    'captcha' => $field['#captcha'],
  );
  
  if(!system_is_captcha_ttf()){
    unset($field['#captcha']['image']);
  }
  
  //$str = captcha_get($field['#name'], $form['#form_id'], $form['#form_token'], $field['#captcha']['type'], $field['#captcha']['image']);
  
  $key = $form['#form_token'] . $field['#name'] . $form['#form_id'];
  
  dd_add_js(array('captcha' => array($key => array(
    'name' => $field['#name'],
    'type' => $field['#captcha']['type'] ? $field['#captcha']['type'] : 0,
    'form_id' => $form['#form_id'],
    'tag' => $form['#form_token'],
    'image' => $field['#captcha']['image'] ? $field['#captcha']['image']: 0)
  )), 'setting');
  if($field['#attributes']['class']){
    if(strpos($field['#attributes']['class'], 'form_field_input_captcha') === false){
      $field['#attributes']['class'] .= ' form_field_input_captcha';
    }
  }else{
    $field['#attributes']['class'] = 'form_field_input_captcha';
  }
  return dd_form_label($field) . '<input type="text" name="'.$field['#name'].'" value="'.$field['#value'].'"'.dd_attributes($field['#attributes']).'/>' .'<span class="form_captcha" alt="'.$key.'" title="'.t('system', '点击换一个验证码').'">'.t('system', '获取验证码').'</span>'. dd_form_error($field);
}

/**
 * 复选框字段
 */
function dd_form_checkbox($field){
  $output = dd_form_label($field);
  
  if($field['#options']){
    $id = $field['#attributes']['id'];
    if($field['#attributes']['class']){
      $field['#attributes']['class'] .= ' '.$id;
    }else{
      $field['#attributes']['class'] = $id;
    }
    foreach($field['#options'] as $key => $op){
      $field['#attributes']['id'] = $id .'_'. $key;
      $output .= '<span class="form_checkbox_option"><input';
      if(is_array($field['#value']) && in_array($key, $field['#value'])) {
        $output .= ' checked="checked"';
      }
      $output .= ' value="'.$key.'" name="'.$field['#name'].'['.$key.']" type="checkbox"'.dd_attributes($field['#attributes']).'/><span class="option_label">'.$op ."</span></span>\n";
    }
  }else{
    $output .= '<input';
    if($field['#value']) $output .= ' checked="checked"';
    $output .= ' value="1" name="'.$field['#name'].'" type="checkbox"'.dd_attributes($field['#attributes'])."/>\n";
  }
  return $output . dd_form_error($field);
}

//全选
function dd_cp_checkbox($id, $att = NULL, $value = NULL){
  $att['class'] = $att['class'] ? $att['class'] : 'admin_delete_check';
  return dd_form_checkbox(array('#name' => 'check',  '#attributes' => $att, '#options' => array($id => ($value ? $value : ''))));
}

function _dd_cp_checkbox(){
  return dd_form_checkbox(array('#name' => 'check',  '#attributes' => array('disabled' => 'disabled')));
}

function dd_form_ajax_form($url, $title = '确认提交', $att = NULL){
 $att['alt'] = $url;
 $att['class'] = 'dd_form_ajax_form_button';
 dd_add_js('misc/jquery.form.js');
	return dd_form_button(array(
		'#name' => 'dd_form_ajax_form_button[]',
		'#value' => $title,
		'#attributes' => $att
	));
}

function dd_cp_delete($url, $title = NULL, $class = 0){
  return dd_form_button(array('#name' => 'admin_delete_button[]', '#value' => ($title ? $title : '确认删除'), '#attributes' => array('alt' => $url, 'class' => 'admin_delete_button', 'rel' => $class ? $class : 'admin_delete_check')));
}

function dd_form_allcheck($alt, $str = '全选', $att = NULL){
  return '<input type="checkbox" class="form_all_check '.$alt.'" alt="'.$alt.'" '.dd_attributes($att).' />'.$str;
}

//下拉框字段
function dd_form_select($field){
  $output = dd_form_label($field) . '<select name="'.$field['#name'].'"'.dd_attributes($field['#attributes']).'>';
  foreach($field['#options'] as $key => $op){
    $output .= '<option';
    if($field['#value'] == (string) $key) $output .= ' selected="selected"';
    $output .= ' value="'.$key.'">'.$op."</option>\n";
  }
  $output .= "</select>\n";
  return $output . dd_form_error($field);
}

//下拉复选框字段
function dd_form_selects($field){
  $output = dd_form_label($field) . '<select name="'.$field['#name'].'[]"'.dd_attributes($field['#attributes']).' multiple="multiple">';
  
  foreach($field['#options'] as $key => $op){
    $output .= '<option';
    if($field['#value'] && in_array($key, $field['#value'])) $output .= ' selected="selected"';
    $output .= ' value="'.$key.'">'.$op."</option>\n";
  }
  $output .= "</select>\n";
  return $output . dd_form_error($field);
}

//单选框字段
function dd_form_radio($field){
  $output = dd_form_label($field);
  if(is_array($field['#options'])){
    $id = $field['#attributes']['id'];
    if($field['#attributes']['class']){
      $field['#attributes']['class'] .= ' '.$id;
    }else{
      $field['#attributes']['class'] = $id;
    }
    foreach($field['#options'] as $key => $op){
      $field['#attributes']['id'] = $id .'_'. $key;
      $cd = '';
      if($field['#value'] == (string) $key) $cd = ' checked="checked"';
      $output .= '<span class="form_radio_field"><input'.$cd.' value="'.$key.'" name="'.$field['#name'].'" type="radio"'.dd_attributes($field['#attributes']).'/><span class="form_radio_text">'.$op .'</span></span>';
    }
  }else{
    if($field['#value']) $cd = ' checked="checked"';
    $output .= '<span class="form_radio_field"><input'.$cd.' value="1" name="'.$field['#name'].'" type="radio"'.dd_attributes($field['#attributes']).'/><span class="form_radio_text">'.$op .'</span></span>';
  }
  return $output . dd_form_error($field);
}

//上传字段
function dd_form_file($field){
  return file_get_form($field);
}

//按钮字段
function dd_form_button($field){
  return dd_form_label($field). '<input type="button" name="'.$field['#name'].'" value="'.($field['#value'] ? $field['#value'] : 'Button').'"'.dd_attributes($field['#attributes']).'/>';
}

//提交按钮字段
function dd_form_submit($field){
  return '<input type="submit" name="'.$field['#name'].'" value="'.($field['#value'] ? $field['#value'] : '确认提交').'"'.dd_attributes($field['#attributes']).'/>';
}

//图片按钮字段
function dd_form_image($field){
  return '<input type="image" name="'.$field['#name'].'" src="'.$field['#src'].'"'.dd_attributes($field['#attributes']).'/>';
}

function dd_form_label($field){
  if($field['#title'] || $field['#required']){
    $t = '<label class="dd_label" for="'.$field['#name'].'">';
    if($field['#title']) $t .= $field['#title'].'：';
    if($field['#required']) $t .= '<span class="form_required" title="此项不能为空。">*</span>';
    $t .= "</label>\n";
  }
  $t .= $field['#field_prefix'];
  return $t;
}

function form_set_auto($field = NULL){
  if($field['#autocomplete']){
    dd_jqui();
    foreach($field['#autocomplete'] as $key => $value){
      if($value){
        $auto_val[$key] = $value;
      }
    }
    dd_add_js(array('auto' => $auto_val), 'setting');
  }
}

function form_set_ahah($field = NULL) {
  if (isset($field['#ahah']['url']) && !isset($field['#ahah']['event'])) {
    switch ($field['#type']) {
      case 'textfield':
        $field['#ahah']['event'] = 'keyup';
      break;
      case 'select': case 'selects': case 'radio': case 'checkbox':
        $field['#ahah']['event'] = 'change';
      break;
      case 'submit': case 'image': case 'button':
        $field['#ahah']['event'] = 'click';
    }
  }
  if (isset($field['#attributes']['id']) && isset($field['#ahah']['url']) && isset($field['#ahah']['event'])) {
    dd_add_js('misc/ahah.js');
    dd_add_js('misc/jquery.form.js');
    /*
      event => 触发事件
      url => 请求url
      type => 表单提交类型
      beforeSubmit => 提交前回调函数
      success => 返回值处理函数
      dataType => 返回值类型
      clearForm => 表单提交成功后是否清除表单
      target => 页面更新的元素
    */
    foreach($field['#ahah'] as $key => $value){
      if($value){
        $ahah_val[$key] = $value;
      }
    }
    $ahah_val['id'] = $field['#attributes']['id'];
    dd_add_js(array('ahah' => array($field['#attributes']['id'] => $ahah_val)), 'setting');
  }
}

/**
 * 验证邮箱
 */
function form_vali_mail($mail){
  if(!preg_match('/([a-z0-9_-])+@([a-z0-9_-])+.[a-z].*/i', $mail)){
    return t('system', '邮箱格式不正确');
  }
}

/**
 * 验证手机号码
 */
function form_vali_mobile($mobile){
	if(strlen($mobile) != 11){
		$error = t('system', '手机号码有误');
	}else if(preg_match('/[^0-9]/', $mobile)){
		$error = t('system', '手机号码只能是数字');
	}
  return $error;
}

function dd_form_inline($form){
  return '<div class="dd_form_inline">'.$form.'</div>';
}

function form_time_select($title, $type = 'get'){
  if($type == 'get'){
    $start = $_GET['time_start'];
    $end = $_GET['time_end'];
  }else{
    $start = $_POST['time_start'];
    $end = $_POST['time_end'];
  }
  $time_start = array(
    '#name' => 'time_start',
    '#title' => $title,
    '#value' => $start,
    '#attributes' => array('class' => 'form_text ui_data_click {dateISO: true}')
  );
  $time_end = array(
    '#name' => 'time_end',
    '#value' => $end,
    '#attributes' => array('class' => 'form_text ui_data_click {dateISO: true}')
  );
  dd_add_js('misc/form.js');
  dd_add_js(array('uidata' => array(array('dom' => '.ui_data_click', 'start' => 'input[name="time_start"]', 'end' => 'input[name="time_end"]'))), 'setting');
  dd_jqui();
  
  return dd_form_textfield($time_start) .' -- '. dd_form_textfield($time_end);
}


function dd_label($v, $class = NULL){
  return '<span class="dd_label'.($class ? ' '. $class :  '').'">'.$v.'</span>';
}

/**
 * meta 表单
 */
function dd_get_form_meta($sort = 'desc', $value, $title = 'meta 参数'){
  $form = array(
    '#fieldset_prefix' => $sort,
    '#fieldset_legend' => $title,
    '#fieldset_suffix' => 1,
    '#fieldset' => true,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => 'title',
    '#attributes' => array('size' => 50),
    '#default_value' => $value['title'],
  );
  $form['keywords'] = array(
    '#type' => 'textfield',
    '#title' => 'keywords',
    '#attributes' => array('size' => 50),
    '#default_value' => $value['keywords'],
  );
  $form['robots'] = array(
    '#type' => 'textfield',
    '#title' => 'robots',
    '#attributes' => array('size' => 50),
    '#default_value' => $value['robots'],
    '#description' => t('system', 'all|none|index|noindex|follow|nofollow')
  );
  $form['author'] = array(
    '#type' => 'textfield',
    '#title' => 'author',
    '#attributes' => array('size' => 50),
    '#default_value' => $value['author'],
  );
  $form['generator'] = array(
    '#type' => 'textfield',
    '#title' => 'generator',
    '#attributes' => array('size' => 50),
    '#default_value' => $value['generator'],
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => 'description',
    '#default_value' => $value['description'],
  );
  return $form;
}