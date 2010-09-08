<?php
// $Id$

/**
 * 管理首页区块，提供钩子 hook_system_admin_default()
 * 模块可自定义一个区块在管理首页显示
 */
function system_admin_default() {
  global $conf;
  
  if ($_GET['op'] == 'system' && $_GET['view'] == 'php') {
    phpinfo();
    exit();
  }
  
  $output = '<h1>'.t('system', '欢迎进入 %string 管理系统', array('%string' => $conf['site_global']['name'])).'</h1>';
  if ($data = _module_invoke_all('system_admin_default')) {
    $i = 0;
    foreach ($data as $module => $content) {
      $html = '<div class="system_admin_default_block system_admin_default_block_'.$module.'">';
      if (is_array($content)) {
        if ($content['#title']) {
          $html .= '<h2 class="title">'.$content['#title'].'</h2>';
        }
        if ($content['#content']) {
          $html .= $content['#content'];
          if ($content['#weight']) {
            $weight = $content['#weight'];
          } else {
            $weight = $i;
          }
        }
      } else {
        $html .= $content;
        $weight = $i;
      }
      $html .= '</div>';
      $lists[] = array(
        '#content' => $html,
        '#weight' => $weight,
      );
      ++$i;
    }
    uasort($lists, 'dd_form_cmp');
    foreach ($lists as $d) {
      $output .= $d['#content'];
    }
  }
  return $output;
}

/**
 * @Implement of hook_system_admin_default()
 */
function system_system_admin_default() {
  global $conf;
  $item[] = dd_get_strong(t('system', '网站名称')). $conf['site_global']['name'] . l(t('system', '修改'), 'admin/settings/global');
  
  $item[] .= dd_get_strong(t('system', '程序版本')). DIDA_VERSION.l(t('system', '检查更新'), 'admin/status/update', dd_redirect());
  
  $item[] .= dd_get_strong(t('system', '站长邮箱')). $conf['site_global']['mail'] . l(t('system', '修改'), 'admin/settings/global');
  $item[] .= dd_get_strong(t('system', '创建时间')). format_date($conf['site_created']);
  
  if ($conf['cron_last_time']) {
    $cron = format_date($conf['cron_last_time']);
  } else {
    $cron = t('system', '从未执行');
  }
  if ($conf['cron_pass']) {
    $cron_url = f('cron.php?pass='.md5($conf['cron_pass']));
  } else {
    $cron_url = f('cron.php');
  }
  
  $cron .= l(t('system', '立即执行'), $cron_url, array('attributes' => array('target' => '_blank')));
  
  $item[] .= dd_get_strong(t('system', '上次 CRON')) . $cron;
  
  $item[] .= dd_get_strong(t('system', 'PHP 版本')) . PHP_VERSION. l(t('system', '查看'), 'admin/status/php');
  
  $item[] .= dd_get_strong(t('system', '数据库')) . $GLOBALS['database']['default']['driver'] .' - '. db_get_version();
  $item[] .= dd_get_strong(t('system', '服务器')). PHP_OS .' - ' .$_SERVER['SERVER_SOFTWARE'];
  
  $content = array(
    '#title' => t('system', '系统信息'),
    '#content' => theme('item_list', $item)
  );
  
  return $content;
}

/**
 * phpinfo
 */
function system_status_php() {
  phpinfo();
  exit;
}

/**
 * 系统日志、访问记录
 */
function system_admin_logs(){
  $header = array(
    array('data' => t('system', '用户'), 'field' => 'l.uid'),
    array('data' => t('system', '时间'), 'field' => 'l.sid', 'sort' => 'desc'),
    array('data' => t('system', '类型'), 'field' => 'l.type'),
    array('data' => t('system', '状态'), 'field' => 'l.status'),
    array('data' => t('system', '主机'), 'field' => 'l.host'),
    array('data' => t('system', '路径'), 'field' => 'l.url'),
  );
  
  $types = module_invoke_all('logs_info');
  $status = dd_logs_get_status();
  
  if($fetch = pager_query('SELECT l.*, u.name FROM {logs} l LEFT JOIN {users} u ON l.uid = u.uid'.
  table_sql($header), 50)){
    foreach($fetch as $o){
      $table[] = array(
        theme('username', $o),
        format_date($o->timestamp),
        $types[$o->type] ? $types[$o->type] : $o->type,
        $status[$o->status],
        $o->host,
        $o->url,
      );
      $table[][] = array(
        'data' => dd_get_strong(t('system', '说明')).$o->value,
        'colspan' => 5
      );
    }
    
    $output = theme('table', $header, $table);
    $output .= pager_view();
  }
  
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '系统日志')));
  return $output;
}

/**
 * 文件设置
 */
function system_site_files() {
  $form['fields']['file_directory_path'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#title' => t('system', '根目录'),
    '#description' => t('system', '相对于程序目录的系统路径，所有文件将存储于该文件夹'),
    '#default_value' => var_get('file_directory_path', $GLOBALS['conf_dir'].'/files')
  );
  $form['fields']['file_user_directory'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#attributes' => array('size' => 30),
    '#title' => t('system', '用户目录'),
    '#description' => t('system', '相对于根目录，用户文件默认存储于此。可使用 user、system 通配符，<a href="!url">通配符列表</a>', array('!url' => url('admin/token'))),
    '#default_value' => var_get('file_user_directory', 'user-[uid]/[date_Y]-[date_m]')
  );
  $form['fields']['user_file_size'] = array(
    '#type' => 'textfield',
    '#required' => 1,
    '#attributes' => array('size' => 10),
    '#title' => t('system', '用户默认容量'),
    '#field_suffix' => t('system', 'MB'),
    '#description' => t('system', '0 为不限制。也可按角色设置。容量限制仅在模块发出检查请求时生效，详见 file.inc'),
    '#default_value' => var_get('user_file_size', 30)
  );
  if ($roles = user_data_roles()) {
    $i = 0;
    foreach ($roles as $rid => $role) {
      if ($rid != var_get_key('user', 'anonymous_role', 1)) {
        $form['fields']['user_file_size_role_'.$rid] = array(
          '#type' => 'textfield',
          '#required' => 1,
          '#attributes' => array('size' => 10),
          '#title' => t('system', $role->name),
          '#field_suffix' => t('system', 'MB'),
          '#default_value' => var_get('user_file_size_role_'.$rid, 0)
        );
        if ($i == 0) {
          $form['fields']['user_file_size_role_'.$rid]['#fieldset_prefix'] = 'desc';
          $form['fields']['user_file_size_role_'.$rid]['#fieldset_legend'] = t('system', '按角色设置');
        }
        $i = $rid;
      }
    }
    $form['fields']['user_file_size_role_'.$i]['#fieldset_suffix'] = 1;
  }
  
  $type = 'form';
  
  module_alter_all('system_site_files', $type, $form);
  
  $form['settings'] = array(
    '#validate' => array('system_site_files_validate'),
    '#submit' => 'system_site_files_submit',
  );
  
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '文件设置')));
  return system_setting_form($form);
}

function system_site_files_validate($form, $v) {
  return module_invoke_all('system_site_files', 'validate', $form, $v);
}

function system_site_files_submit($form, $v) {
  module_invoke_all('system_site_files', 'submit', $form, $v);
  file_mkdir($v['file_directory_path']);
}

/**
 * 已执行、待执行的计划任务列表
 */

/**
function system_admin_cron() {
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '计划任务')));
}
*/

/**
 * 默认首页内容
 */
function system_admin_front() {
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '首页设置')));
  $form['fields']['value'] = array(
    '#type' => 'textarea',
    '#title' => t('system', '首页默认内容'),
    '#default_value' => custom_get('front_default'),
    '#description' => t('system', '当首页没有数据时，将显示该内容')
  );
  $form['fields']['submit'] = array(
  	'#type' => 'submit',
    '#submit' => 'system_admin_front_submit'
  );
  dd_get_editor('value', array('toolbar' => 'medium', 'height' => 200));
  return $form;
}

function system_admin_front_submit($form, $v) {
  custom_set('front_default', $v['value']);
  dd_set_message(t('system', '操作成功'));
}

/**
 * 站点信息
 */
function system_site_global() {
  global $conf;
  
  $form['fields']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('system', '站点名称'),
    '#required' => true,
    '#default_value' => var_get_key('site_global', 'name', t('system', '我的网站')),
  );
  $form['fields']['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('system', '站长邮箱'),
    '#required' => true,
    '#default_value' => var_get_key('site_global', 'mail'),
  );
  $form['fields']['slogan'] = array(
    '#type' => 'textfield',
    '#title' => t('system', '站点口号'),
    '#attributes' => array('size' => 50),
    '#description' => t('system', '显示在站点名称的后面'),
    '#default_value' => var_get_key('site_global', 'slogan'),
  );
  $form['fields']['description'] = array(
    '#type' => 'textarea',
    '#attributes' => array('rows' => 3),
    '#title' => t('system', '站点描述'),
    '#default_value' => var_get_key('site_global', 'description'),
    '#description' => t('system', '不支持 HTML，用于 RSS 或其它站点简介的地方，应尽量简洁')
  );
  $form['fields']['footer'] = array(
    '#type' => 'textarea',
    '#title' => t('system', '页脚信息'),
    '#default_value' => var_get_key('site_global', 'footer'),
    '#description' => t('system', '支持 HTML')
  );
  
  $type = 'form';
  
  module_alter_all('system_site_global', $type, $form);
  
  $form['settings'] = array(
    '#validate' => array('system_site_global_validate'),
  );
  
  $form['fields']['submit'] = array(
    '#submit' => 'system_site_global_submit',
    '#type' => 'submit'
  );
  
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '站点信息')));
  return $form;
}

function system_site_global_validate($form, $v) {
  return module_invoke_all('system_site_global', 'validate', $form, $v);
}

function system_site_global_submit($form, $v) {
  foreach ($form['fields'] as $key => $field) {
    if ($field['#type'] && !in_array($field['#type'], array('hidden', 'submit', 'button', 'file', 'captcha'))) {
     $data[$key] = $v[$key];
    }
  }
  var_set('site_global', $data, 0);
  module_invoke_all('system_site_global', 'submit', $form, $v);
  var_init();
}

/**
 * 站点配置
 */
function system_site_settings() {
	global $conf;
  
  $form['fields']['status'] = array(
    '#type' => 'radio',
    '#title' => t('system', '网站状态'),
    '#required' => true,
    '#default_value' => var_get('status', 1),
    '#options' => array(
      t('system', '关闭'),
      t('system', '开启'),
      t('system', '登录可访问'),
      t('system', '超级管理员可访问')
    ),
  );
  $form['fields']['default_language'] = array(
    '#type' => 'select',
    '#title' => t('system', '默认语言'),
    '#required' => true,
    '#default_value' => var_get('default_language', 'zh-CN'),
    '#options' => $conf['languages'],
  );
  $form['fields']['clean_url'] = array(
    '#type' => 'radio',
    '#title' => t('system', '简洁链接'),
    '#required' => true,
    '#default_value' => var_get('clean_url', 1),
    '#options' => array(
      t('system', '关闭'),
      t('system', '开启'),
    ),
  );
  $form['fields']['sql_info'] = array(
    '#type' => 'radio',
    '#title' => t('system', 'SQL执行语句'),
    '#required' => true,
    '#default_value' => var_get('sql_info', 1),
    '#options' => array(t('system', '不显示'), t('system', '显示'), t('system', '仅向超级管理员显示')),
  );
  $form['fields']['history_enabled'] = array(
    '#type' => 'radio',
    '#title' => t('system', '访问记录'),
    '#required' => true,
    '#default_value' => var_get('history_enabled', 0),
    '#options' => array(t('system', '不记录'), t('system', '记录')),
    '#description' => t('system', '记录网站的每一次访问')
  );
  $form['fields']['history_enabled_limit'] = array(
    '#type' => 'textfield',
    '#title' => t('system', '保留最新'),
    '#required' => true,
    '#default_value' => var_get('history_enabled_limit', 100000),
    '#field_suffix' => t('system', '条访问记录，0 为不限制')
  );
  $form['fields']['site_mode'] = array(
    '#type' => 'radio',
    '#title' => t('system', '开发模式'),
    '#required' => true,
    '#default_value' => var_get('site_mode', 1),
    '#description' => t('system', '开发模式将禁用某些缓存，以便于调试'),
    '#options' => array(t('system', '否'), t('system', '是')),
  );
  $form['fields']['admin_custom_links'] = array(
    '#type' => 'radio',
    '#title' => t('system', '管理区块快捷方式'),
    '#required' => true,
    '#default_value' => var_get('admin_custom_links', 1),
    '#options' => array(
      t('system', '禁止'),
      t('system', '允许'),
    ),
    '#description' => t('system', '允许拥有后台管理权限的用户添加一些快捷链接，供自己使用'),
  );
  $form['fields']['cache_css_and_js'] = array(
    '#type' => 'radio',
    '#title' => t('system', '合并CSS和JS'),
    '#required' => true,
    '#default_value' => var_get('cache_css_and_js', 0),
    '#options' => array(t('system', '不合并'), t('system', '合并')),
  );
  $form['fields']['filter_cache'] = array(
    '#type' => 'radio',
    '#title' => t('system', '缓存输入法'),
    '#required' => true,
    '#default_value' => var_get('filter_cache', 0),
    '#options' => array(t('system', '不缓存'), t('system', '缓存')),
    '#description' => t('system', '可以将通过输入法过滤的内容缓存到数据库，减少 PHP 开销，但将数据库将增多'),
  );
  $form['fields']['alias'] = array(
    '#type' => 'radio',
    '#title' => t('system', '别名优先'),
    '#required' => true,
    '#default_value' => var_get('alias', 0),
    '#options' => array(t('system', '否'), t('system', '是')),
    '#description' => t('system', '若存在别名，通过系统路径访问时，跳转到别名'),
  );
  $form['fields']['cron_min_time'] = array(
    '#type' => 'textfield',
    '#title' => t('system', 'CRON 时间间隔'),
    '#required' => true,
    '#default_value' => var_get('cron_min_time', 3600),
    '#description' => t('system', '两次 cron 间隔最小时间，以秒为单位，防止cron被频繁执行'),
  );
  $form['fields']['cron_pass'] = array(
    '#type' => 'textfield',
    '#title' => t('system', 'CRON 执行密码'),
    '#default_value' => var_get('cron_pass'),
    '#description' => t('system', '不设置请留空'),
  );
  
  $form['fields']['custom_view'] = array(
    '#type' => 'select',
    '#title' => t('user', '网站首页'),
    '#required' => 1,
    '#default_value' => var_get('front_view_type'),
    '#options' => array(t('system', '系统默认'), t('system', '自定义')),
    '#description' => t('system', '若选择自定义，首页将不会有任何显示，仅传递可用变量，请自行在 front.tpl.php 中布局'),
  );
  
  $options = array(0 => t('system', '默认'));
  if ($modules = var_get('modules')) {
    foreach ($modules as $module => $info) {
      $options[$module] =  $info['name'];
    }
  }
  $form['fields']['front_view_module'] = array(
    '#type' => 'select',
    '#title' => t('system', '交给模块处理'),
    '#default_value' => var_get('front_view_module'),
    '#options' => $options,
    '#description' => t('system', '将网站首页完全交给某个模块处理，除非某个模块建议这么做，否则请不要选择'),
  );
  
  $type = 'form';
  
  module_alter_all('system_site_settings', $type, $form);
  
  // dd_get_editor('footer', array('toolbar' => 'medium', 'height' => 200));
  
  $form['settings'] = array(
    '#validate' => array('system_site_settings_validate'),
    '#submit' => 'system_site_settings_submit',
  );
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '站点设置')));
  return system_setting_form($form);
}

function system_site_settings_validate($form, $v) {
  if ($v['cache_css_and_js']) {
    file_mkdir(file_directory_path() . '/cache/css_js');
  }
  return module_invoke_all('system_site_settings', 'validate', $form, $v);
}

function system_site_settings_submit($form, $v) {
  module_invoke_all('system_site_settings', 'submit', $form, $v);
}

/**
 * 验证码设置
 */
function system_site_captcha() {
  $form['fields']['captcha_file_ttf'] = array(
    '#type' => 'textfield',
    '#default_value' => var_get('captcha_file_ttf', './tools/simkai.ttf'),
    '#title' => t('system', '字体文件'),
    '#description' => t('system', '图片验证码必须提供中文字体文件，请将字体文件放于程序可访问的目录(推荐 tools)，在此设置路径')
  );
  if ($fields = module_invoke_all('system_site_captcha', 'fields')) {
    foreach ($fields as $key => $field) {
      if (!is_array($field)) continue;
      
      $value = var_get_key('captcha', $key);
      $form['fields'][$key]['data'] = array(
        '#type' => 'radio',
        '#title' => t('system', '启用'),
        '#default_value' => $value['data'] ? $value['data'] : 0,
        '#options' => array(
          t('system', '否'),
          t('system', '是'),
        ),
        '#fieldset_prefix' => 'asc',
        '#fieldset_legend' => $field['title'],
      );
      if ($field['attributes']) {
        $form['fields'][$key]['data'] = array_merge($form['fields'][$key]['data'], $field['attributes']);
      }
      $form['fields'][$key]['type'] = array(
        '#type' => 'select',
        '#title' => t('system', '类型'),
        '#default_value' => $value['type'],
        '#options' => array(
          t('system', '纯数字'),
          t('system', '数字、字母混合'),
          t('system', '汉字'),
          t('system', '计算式'),
          t('system', '问答式'),
        )
      );
      $form['fields'][$key]['image'] = array(
        '#type' => 'select',
        '#title' => t('system', '样式'),
        '#default_value' => $value['image'],
        '#fieldset_suffix' => 1,
        '#options' => array(
          t('system', '文本'),
          t('system', '图片'),
        )
      );
    }
    $form['fields']['submit'] = array(
      '#type' => 'submit',
      '#submit' => 'system_site_captcha_submit'
    );
  }
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '验证码设置')));
  return $form;
}

function system_site_captcha_submit($form, $v) {
  var_set('captcha_file_ttf', $v['captcha_file_ttf'], 0);
  unset($v['captcha_file_ttf']);
  var_set('captcha', $v);
  dd_set_message(t('system', '保存成功'));
}

/**
 * 错误设置
 */
function system_site_error() {
  if ($image = var_get('image_error')) {
    $view = l(img($image, 
    t('system', '默认错误图片'), NULL, 
    array('width' => 100, 'height' => 100)), $image, array('html' => true));
  }
  $form['fields']['image_error'] = array(
    '#type' => 'file',
    '#title' => t('system', '错误图片'),
    '#description' => t('system', '当请求的图片不存在时，显示的图片') . $view,
    '#filesave' => true,// 系统处理上传文件
    '#validate' => array(
      'savepath' => 'error',
      'no_insert' => true,
      'title' => '默认错误图片',
      'filename' => 'image',
      'image' => array(
        'type' => array('jpg', 'png', 'gif'),
      ),
      'call_function' => '_system_image_error_save',
    ),
  );
  $form['fields']['debug'] = array(
    '#type' => 'radio',
    '#title' => t('system', '程序错误信息'),
    '#required' => true,
    '#default_value' => var_get('debug', 1),
    '#options' => array(
      t('system', '写入日志'),
      t('system', '写入日志并显示'),
      t('system', '写入日志并仅对超级管理员显示'),
    ),
  );
 
  $form['fields']['page_not_history'] = array(
    '#type' => 'radio',
    '#title' => t('system', '404错误'),
    '#required' => true,
    '#default_value' => var_get('page_not_history', 0),
    '#options' => array(
      t('system', '显示以下错误信息'),
      t('system', '返回上一页或首页'),
    )
  );
  
  $form['fields']['page_not_html'] = array(
    '#type' => 'textarea',
    '#title' => t('system', '404错误信息'),
    '#default_value' => custom_get('page_not_html', '当前页面不存在'),
    '#description' => t('system', '支持 HTML')
  );
  
  $form['fields']['page_access_history'] = array(
    '#type' => 'radio',
    '#title' => t('system', '403错误'),
    '#required' => true,
    '#default_value' => var_get('page_access_history', 0),
    '#options' => array(
      t('system', '显示以下错误信息'),
      t('system', '返回上一页或首页'),
    )
  );
  $form['fields']['page_access_html'] = array(
    '#type' => 'textarea',
    '#title' => t('system', '403错误信息'),
    '#default_value' => custom_get('page_access_html', '没有权限访问当前页面'),
    '#description' => t('system', '支持 HTML')
  );
  $form['fields']['site_offline_html'] = array(
    '#type' => 'textarea',
    '#title' => t('system', '站点维护信息'),
    '#default_value' => custom_get('site_offline_html', '站点维护中，请稍候访问…'),
    '#description' => t('system', '支持 HTML')
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#submit' => 'system_site_error_submit'
  );
  dd_set_breadcrumb(array(l(t('system', '系统管理'), 'admin'), t('system', '错误设置')));
  return $form;
}

function system_site_error_submit($form, $v) {
  unset($v['image_error'], $v['submit'], $v['form_id'], $v['form_toke']);
  
  foreach ($v as $key => $value) {
    if ($key == 'page_access_html' || $key == 'page_not_html' || $key == 'site_offline_html') {
      custom_set($key, $value);
    } else {
      var_set($key, $value, 0);
    }
  }
  
  file_put_contents($GLOBALS['conf_dir'] . '/cache/site_offline_html.lang', $v['site_offline_html']);
  
  var_init();
}

/**
 * 默认错误图片上传成功后回调函数
 * @param $form_id
 * @param $name
 * @param $file
 */
function _system_image_error_save($form_id, $name, $file) {
  var_set('image_error', $file->filepath);
}

function _system_cache_htaccess() {
  $filepath = $GLOBALS['conf_dir'].'/cache/.htaccess';
  if (!is_file($filepath)) {
    $h = "#禁止下载\n";
    $h .= '<FilesMatch "\.*$">';
    $h .= "\nOrder allow,deny\n";
    $h .= '</FilesMatch>';
    @file_put_contents($filepath, $h);
    chmod($filepath, 0644);
  }
}

/**
 * 缓存设置
 */
function system_cache_settings() {
  dd_set_tabs(array(
    l(t('system', '缓存管理'), 'admin/cache'),
    l(t('system', '缓存设置'), 'admin/cache/settings'),
    l(t('system', '页面缓存清理'), 'admin/cache/clearpage'),
  ));
  
  $options['./includes/cache.inc'] = t('system', '系统默认');
  
  if (is_dir('./includes/cache')) {
    if ($objs = glob('./includes/cache/*')) {
      foreach ($objs as $obj) {
        if (strpos($obj, '.inc') !== false) {
          $options[$obj] = next(explode('.', basename($obj), 3));
        }
      }
    }
  }
  
  $form['fields']['dd_cache_file'] = array(
    '#title' => t('system', '缓存方式'),
    '#type' => 'select',
    '#required' => 1,
    '#default_value' => var_get('dd_cache_file', './includes/cache.inc'),
    '#options' => $options,
  );
  
  $item[] = t('system', '开启静态页面，将跳过模块初始化，直接输出静态文件，所有动态逻辑判断均不会执行。');
  $item[] = t('system', '除非有极端性能需求，且对站点所使用的模块极为了解，否则不建议开启这个选项。');
  $item[] = t('system', '当然，开启这个选项，并不意味着所有页面都将缓存，模块可以根据实际情况，禁止或者允许某些页面缓存');
  $item[] = t('system', '在模块层，对于某个页面是否使用缓存，是完全可控的，参见 hook_cache_page_disable() 与  hook_cache_page_enabled()');
  $item[] = t('system', '这也意味着，当启用或禁用了一个模块后，应该清除静态页面，以应用可能的规则变化，除非你确认完全没有变化');
  $form['fields']['dd_cache_page'] = array(
    '#title' => t('system', '静态页面'),
    '#type' => 'select',
    '#required' => 1,
    '#default_value' => var_get('dd_cache_page', 0),
    '#options' => array(
      t('system', '关闭'),
      t('system', '对匿名用户开启'),
      t('system', '对所有用户开启'),
    ),
    '#description' => theme('item_list', $item)
  );
  
  $form['fields']['dd_cache_page_type'] = array(
    '#title' => t('system', '生成方式'),
    '#type' => 'select',
    '#required' => 1,
    '#options' => array(
      'disable' => t('system', '1.默认所有页面均生成静态，询问模块是否禁用'),
      'enabled' => t('system', '2.默认所有页面均禁用静态，询问模块是否启用'),
    ),
    '#default_value' => var_get('dd_cache_page_type', 'enabled'),
    '#description' => t('system', '此设置仅对匿名用户有效，对于登录用户，总是使用第 2 种方式')
  );
  
  $form['fields']['dd_cache_page_expire'] = array(
    '#title' => t('system', '有效时间'),
    '#type' => 'textfield',
    '#required' => 1,
    '#default_value' => var_get('dd_cache_page_expire', 600),
    '#description' => t('system', '默认静态文件缓存时间，以秒为单位，最大不超过 31536000，即 365 天')
  );
  
  if (function_exists('cache_settings_custom_form')) {
    cache_settings_custom_form($form);
  }
  
  return system_setting_form($form);
}

/**
 * @Implement of hook_system_cache_clear()
 */
function system_system_cache_clear($op = 'list', $v = NULL) {
  switch ($op) {
    case 'list':
    return array(
      'modules' => t('system', '清除模块信息缓存，当模块 info 信息变动时应选择此项'),
      'perms' => t('system', '清除用户权限缓存，当有新的权限注册时应选择此项'),
      'themes' => t('system', '清除主题信息缓存，当主题 info 信息变动时应选择此项'),
      'menu' => t('system', '清除menu信息缓存，当有模块注册新的 url 或旧的 url 权限变动时应选择此项'),
      'cache_menu' => t('system', '清除cache_menu缓存表，当旧的 url 权限变动时应选择此项'),
      'admin_menus' => t('system', '清除管理菜单缓存表，当有新的管理菜单注册时应选择此项'),
      'cache_css_and_js' => t('system', '清除js和css缓存'),
      'front_cache' => t('system', '清除首页缓存，仅当开启了页面缓存才需要此操作'),
    	'cache_hook' => t('system', '清除 hook 缓存，当有模块变动时应选择此项'),
      'cache_filter' => t('system', '清除 filter 缓存，当有输入法或过滤器有变动时应选择此项'),
    );
    // case 'submit':
  }
}

function system_cache_clearpage() {
  dd_set_tabs(array(
    l(t('system', '缓存管理'), 'admin/cache'),
    l(t('system', '缓存设置'), 'admin/cache/settings'),
    l(t('system', '页面缓存清理'), 'admin/cache/clearpage'),
  ));
  
  $form['fields']['all'] = array(
    '#title' => t('system', '清除全部'),
    '#type' => 'radio',
    '#options' => array(t('system', '否'), t('system', '是')),
    '#description' => t('system', '所有静态缓存将被清除，若缓存文件较多，建议通过 ftp 直接删除，位于：%dir',
    array('%dir' => $GLOBALS['conf_dir'].'/cache/page'))
  );
  
  dd_set_help(t('system', '一般情况下，各个模块将会自行判断何时应当清除它所创建的缓存，你不需要在此操作。当然，以下所有操作，并没有危险性，当某些页面迟迟不能更新时，你可以放心在此清除它'));
  
  $item[] = t('system', '完整路径，如：user/1，严格匹配，仅清除 user/1 单个页面');
  $item[] = t('system', '通配符 *，如：user/*，将清除 user/1、user/2、user/n的所有页面，但将跳过子文件夹，如：user/1/test');
  $item[] = t('system', '通配符 *all，如：user/*all，与 user/* 相同，但将删除所有子文件夹');
  $item[] = t('system', '特殊说明：[front] - 清除匿名用户的首页缓存，[front* - 清除所有用户的首页缓存');
  
  $form['fields']['path'] = array(
    '#title' => t('system', '按路径清除'),
    '#type' => 'textfield',
    '#description' => theme('item_list', $item)
  );
  
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#submit' => 'system_cache_clearpage_submit',
  );
  return $form;
}

function system_cache_clearpage_submit($form, $v) {
  if ($v['all']) {
    cache_clear_page('*');
  } else if ($v['path']) {
    cache_clear_page(trim($v['path']));
  }
  
  dd_set_message(t('system', '操作成功'));
}

/**
 * 缓存管理
 */
function system_cache_clear() {
  dd_set_tabs(array(
    l(t('system', '缓存管理'), 'admin/cache'),
    l(t('system', '缓存设置'), 'admin/cache/settings'),
    l(t('system', '页面缓存清理'), 'admin/cache/clearpage'),
  ));
  $form['fields']['value'] = array(
    '#type' => 'checkbox',
    '#options' => module_invoke_all('system_cache_clear', 'list'),
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#field_prefix' => dd_form_ajax_all_check(array('alt' => 'system_cache_clear_type_value')),
    '#value' => t('system', '清除缓存'),
    '#submit' => 'system_cache_clear_submit',
  );
  return $form;
}

function system_cache_clear_submit($form, $value) {
  if ($value['value']['modules']) {
    module_set_list('module', 'modules');
    module_enabled_variable('module');
    cache_del('cid', 'schema');
    dd_set_message(t('system', '模块信息缓存清除成功'));
  }
  if ($value['value']['themes']) {
    module_set_list('theme', 'themes');
    module_enabled_variable('theme');
    dd_set_message(t('system', '主题信息缓存清除成功'));
  }
  if ($value['value']['perms']) {
    cache_del('cid', 'user_perm');
    dd_set_message(t('system', '用户权限缓存清除成功'));
  }
  if ($value['value']['menu']) {
    menu_set_item();
    dd_set_message(t('system', 'menu 信息缓存清除成功'));
  }
  if ($value['value']['cache_hook']) {
    module_hook_set_cache();
    theme_get_cache();
    dd_set_message(t('system', 'hook 缓存清除成功'));
  }
  if ($value['value']['cache_filter']) {
    filter_set_cache();
    dd_set_message(t('system', 'filter 缓存清除成功'));
  }
  if ($value['value']['cache_menu']) {
    cache_del('empty', NULL, 'cache_menu');
    dd_set_message(t('system', 'menu 列表缓存清除成功'));
  }
  if ($value['value']['cache_css_and_js']) {
    $filepath = file_directory_path() . '/cache/css_js';
    if (is_dir($filepath)) {
      if ($objs = glob($filepath.'/*')) {
        foreach ($objs as $obj) {
          is_dir($obj) ? @rmdir($obj) : @unlink($obj);
        }
      }
    }
    var_set('cache_css_and_js_timestamp', $_SERVER['REQUEST_TIME'], 0);
    dd_set_message(t('system', 'js和css缓存清除成功'));
  }
  if ($value['value']['admin_menus']) {
    cache_del('*', 'admin_menus_');
    dd_set_message(t('system', '管理菜单缓存清除成功'));
  }
  if ($value['value']['front_cache']) {
    cache_clear_page('[front]');
  }
  
  _system_cache_htaccess();
  
  module_invoke_all('system_cache_clear', 'submit', $value);
  var_init();
}

/**
 * 通配符
 */
function system_admin_token() {
	if ($tokens = _module_invoke_all('token_list')) {
		foreach ($tokens as $module => $lists) {
			$table[][] = array('data' => $module, 'colspan' => 2);
			foreach ($lists as $key => $info) {
				$table[] = array($key, $info['title']);
			}
		}
		return theme('table', array(t('system', '通配符'), t('system', '替换为')), $table);
	}
}

function system_admin_lang() {
	global $conf;
  dd_set_tabs(array(l(t('system', '语言管理'), 'admin/lang'), l(t('system', '导入语言'), 'admin/lang/cache')));
  dd_set_help(t('system', '所有语言文件将被合并，保存在 %path 目录下', array('%path' => $GLOBALS['conf_dir'].'/cache/languages')));
	if (arg(2)) {
    if ($conf['languages'][arg(2)]) {
  		$lang = arg(2);
  		switch (arg(3)) {
  			case 'edit':
  				$edit = array('language' => $lang, 'name' => $conf['languages'][$lang]);
  			break;
  			case 'delete':
  				if ($lang != $conf['default_language']) {
  					db_exec('DELETE FROM {languages} WHERE language = ?', array($lang));
  					cache_del_file('lang_' . $lang . '.lang');
  					system_lang_cache();
  					dd_set_message(t('system', '删除成功'));
  				} else {
  					dd_set_message(t('system', '默认语言不能删除'));
  				}
  				dd_goto('admin/lang');
  				exit;
  		}
    } else if (arg(2) == 'cache') {
      return dd_get_form('system_admin_lang_cache');
    }
	}
  
  $output = dd_get_form('system_admin_lang_form', $edit);
  
  if ($conf['languages']) {
  	foreach ($conf['languages'] as $key => $name) {
  		$links = array(l(t('system', '编辑'), 'admin/lang/' . $key . '/edit'));
  		if ($conf['default_language'] == $key) {
  			$key .= t('system', '(当前默认语言)');
  		} else {
  			$links[] =  l(t('system', '删除'), 'admin/lang/' . $key . '/delete');
  		}
  		$table[] = array($key, $name, dd_imp($links));
  	}
  }
  
  $output .= theme('table', array(t('system', '语言代码'), t('system', '语言名称'), t('system', '操作')), $table);
  
	return $output;
}

// 语言设置
function system_admin_lang_form($lang = NULL) {
	global $conf;
  
	$form['settings']['#validate'] = array('system_admin_lang_form_validate');
  $form['fields']['language'] = array(
    '#type' => 'select',
    '#title' => t('system', '添加语言'),
    '#required' => 1,
    '#options' => system_get_languages_data(),
    '#default_value' => $lang['language'],
  );
  $form['fields']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('system', '语言名称'),
    '#default_value' => $lang['name'],
    '#required' => 1,
    '#description' => t('system', '用户看到的语言名称')
  );
  
  if ($lang['language']) {
    $form['fields']['update'] = array(
      '#type' => 'hidden',
      '#default_value' => $lang['language'],
      '#constant' => 1
    );
  }
  
  $form['fields']['submit']['#type'] = 'submit';
	return $form;
}

function system_admin_lang_form_validate($form, $v) {
	$v['language'] = trim($v['language']);
	$v['name'] = trim($v['name']);
	if (mb_strlen($v['language']) > 16) {
		$t['language']['#error'][] = t('system', '长度不超过 16 字符');
	} else if (mb_strlen($v['name']) > 32) {
		$t['name']['#error'][] = t('system', '长度不超过 32 字符');
	} else {
		if (!$v['update']) {
			if (db_query('SELECT COUNT(*) FROM {languages} WHERE language = ? OR name = ?', array($v['language'], $v['name']), array('return' => 'column'))) {
				$t['name']['#error'][] = t('system', '该语言已经存在');
			} else {
				db_exec('INSERT INTO {languages} (language, name) VALUES (?, ?)', array($v['language'], $v['name']));
				
				system_lang_cache();
				
				dd_set_lang('instrt', NULL, array($v['language'])); // 导入语言文件
				
				dd_set_message(t('system', '%string 添加成功', array('%string' => $v['name'])));
			}
		} else {
			db_exec('UPDATE {languages} SET language = ?, name = ? WHERE language = ?', array($v['language'], $v['name'], $v['update']));
			if ($v['language'] != $v['update']) {
				cache_del_file('lang_' . $v['update'] . '.php');
				if ($v['update'] == $GLOBALS['conf']['default_language']) {
					var_set('default_language', $v['language']);
				}
				dd_set_lang('instrt', NULL, array($v['language'])); // 导入语言文件
			}
			
			system_lang_cache();
			
			dd_goto('admin/lang');
		}
	}
	return $t;
}

/**
 * 语言文件更新
 */
function system_admin_lang_cache() {
	global $conf;
	$form['settings']['#validate'] = array('system_admin_lang_cache_validate');
  
  $form['fields']['language'] = array(
  	'#type' => 'select',
  	'#title' => t('system', '选择语言'),
  	'#options' => $conf['languages'],
  	'#required' => 1,
  );
  
  $form['fields']['value'] = array(
  	'#title' => t('system', '更新方式'),
    '#type' => 'select',
    '#default_value' => 'update',
    '#required' => 1,
    '#options' => array(
      'update' => t('system', '如有冲突，以现有字符串为准(推荐)'),
      'insert' => t('system', '全新导入，不保留现有字符串'),
    ),
    '#description' => '请备份你修改过的语言缓存文件',
  );
  
  $form['fields']['submit']['#type'] = 'submit';
  
	return $form;
}

function system_admin_lang_cache_validate($form, $v) {
	if ($v['language']) {
		$languages[] = $v['language'];
	} else {
		$languages = NULL;
	}
	dd_set_lang($v['value'], NULL, $languages);
}

/**
 * 输入设置
 */
function system_admin_filter() {
  global $conf;
  
  $item = array(
    l(t('system', '输入法列表'), 'admin/filter'),
    l(t('system', '过滤器列表'), 'admin/format'),
    l(t('system', '添加输入法'), 'admin/filter/add', dd_redirect(url('admin/filter'))),
    l(t('system', '添加过滤器'), 'admin/format/add'),
  );
  $args = arg();
  switch ($args[2]) {
    case 'config':
      $item[] = l(t('system', '输入法配置'), $_GET['q']);
      $output .= dd_get_form('system_admin_filter_form', $args[3]);
    break;
    case 'add':
      $output .= dd_get_form('system_admin_filter_form');
    break;
    case 'delete':
      if (db_exec('DELETE FROM {filter_format} WHERE format = ?', array($args[3]))) {
        filter_set_cache(); // 清除配置文件缓存
        dd_set_message(t('system', '删除成功'));
      } else {
        dd_set_message(t('system', '删除失败'));
      }
      dd_goto('admin/filter');
    exit;
    case 'default':
      if ($conf['default_format'] != $args[3]) {
        if (db_query('SELECT format FROM {filter_format} WHERE format = ?', array($args[3]), array('return' => 'column'))) {
          var_set('default_format', $args[3]);
          dd_set_message(t('system', '操作成功'));
        }
      }
      dd_goto('admin/filter');
    exit;
    default:
      if ($fetch = db_query('SELECT * FROM {filter_format}')) {
        foreach ($fetch as $o) {
          $links = array(l(t('system', '设置'), 'admin/filter/config/'.$o->format), l(t('system', '删除'), 'admin/filter/delete/'.$o->format, array('attributes' => array('class' => 'confirm'))));
          if ($conf['default_format'] != $o->format) {
            $links[] = l(t('system', '设为默认'), 'admin/filter/default/'.$o->format);
          }
          
          $filters[] = $o->name . dd_imp($links) . '<span class="description form_description">'. check_plain($o->description). '</span>';
        }
      }
      $output .= theme('item_list', $filters, t('system', '输入法列表'));
  }
  
  dd_set_tabs($item);
  
  $output = $output;
  
  return $output;
}

/**
 * 过滤器列表
 */
function system_admin_format() {
  global $conf;
  
  $item = array(
    l(t('system', '输入法列表'), 'admin/filter'),
    l(t('system', '过滤器列表'), 'admin/format'),
    l(t('system', '添加输入法'), 'admin/filter/add', dd_redirect(url('admin/filter'))),
    l(t('system', '添加过滤器'), 'admin/format/add'),
  );
  $args = arg();
  switch ($args[2]) {
    case 'add':
      if ($lists = _module_invoke_all('filter', 'add')) {
        foreach ($lists as $module => $links) {
          foreach ($links as $delta => $link) {
            $filters[] = l($link['title'], 'admin/format/module/form/'.$module.'/'.$delta, dd_redirect(url('admin/format'))) . '<span class="description form_description">'. check_plain($link['description']). '</span>';
          }
        }
        $output .= theme('item_list', $filters);
      } else {
        dd_set_message(t('system', '没有可用的项目'));
      }
    break;
    case 'module':
      switch ($args[3]) {
        case 'form':
          // 添加过滤器
          if ($args[4] && is_numeric($args[5])) {
            $output .= dd_get_form('system_admin_format_form', $args[4], $args[5]);
          } else {
            dd_set_message(t('system', '没有可用的项目'));
          }
        break;
        case 'update':
          if ($filter = db_query('SELECT * FROM {filter_tags} WHERE fid = ?', array($args[4]), array('return' => 'one'))) {
            $output .= dd_get_form('system_admin_format_form', $filter->module, $filter->delta, $filter);
          } else {
            dd_set_message(t('system', '不正确的访问'));
            dd_goto('admin/format', NULL, NULL, NULL, 1);
          }
        break;
        case 'delete':
          // 删除过滤器
          if ($filter = db_query('SELECT * FROM {filter_tags} WHERE fid = ?', array($args[4]), array('return' => 'one'))) {
            db_exec('DELETE FROM {filter_tags} WHERE fid = ?', array($args[4]));
            db_exec('DELETE FROM {filters} WHERE filter = ?', array($filter->fid));
            if (function_exists($filter->module.'_filter')) {
              call_user_func($filter->module.'_filter', 'delete', $filter->delta, NULL, $filter);
            }
            
            filter_set_cache(); // 清除配置文件缓存
            cache_del('cid', 'filter_get_filters'); // 清除自定义过滤器缓存
            dd_set_message(t('system', '清除成功'));
          } else {
            dd_set_message(t('system', '清除失败'));
          }
          dd_goto_redirect('admin/format');
      }
    break;
    default:
      if ($lists = _module_invoke_all('filter', 'list')) {
        
        $allLists = array();
        foreach ($lists as $module => $filters) {
          foreach ($filters as $delta => $filter) {
            $filter['delta'] = $delta;
            $filter['module'] = $module;
            $allLists[$delta.'_'.$module] = (object)$filter;
          }
        }
        foreach ($allLists as $key => $filter) {
          $links = array();
          if ($filter->update) {
            if ($filter->update == 1) {
              $links[] = l(t('system', '设置'), 'admin/format/module/update/'.$filter->filter_id, dd_redirect());
            } else {
              $links[] = l(t('system', '设置'), $filter->update.'/'.$filter->delta, dd_redirect());
            }
          }
          if ($filter->delete) {
            if ($filter->delete == 1) {
              $links[] = l(t('system', '清除'), 'admin/format/module/delete/'.$filter->filter_id, dd_redirect());
            } else {
              $links[] = l(t('system', '清除'), $filter->delete.'/'.$filter->delta, dd_redirect());
            }
          }
          $filters[] = $filter->title . dd_imp($links) . '<span class="description form_description">'. check_plain($filter->description). '</span>';
        }
        $output .= theme('item_list', $filters, t('system', '过滤器列表'));
      }
  }
  
  dd_set_tabs($item);
  
  $output = $output;
  return $output;
}

/**
 * 过滤器表单
 */
function system_admin_format_form($module, $delta, $filter = NULL) {
  
  $add = call_user_func($module.'_filter', 'add', $delta);
  if (!$add[$delta] || !$add[$delta]['title']) {
    dd_set_message(t('system', '不正确的访问'));
    dd_goto('admin/filte/add_filter');
    exit;
  }
  $form['fields']['title'] = array(
    '#title' => t('system', '名称'),
    '#type' => 'textfield',
    '#required' => 1,
    '#default_value' => $filter->title,
    '#description' => t('system', '不超过 %length 字符', array('%length' => 60))
  );
  $form['fields']['description'] = array(
    '#title' => t('system', '描述'),
    '#type' => 'textfield',
    '#attributes' => array('size' => 60),
    '#default_value' => $filter->description,
    '#description' => t('system', '不超过 %length 字符', array('%length' => 255))
  );
  $form['fields']['fid'] = array(
    '#type' => 'hidden',
    '#default_value' => $filter->fid,
    '#constant' => true,
  );
  $form['fields']['module'] = array(
    '#type' => 'hidden',
    '#default_value' => $module,
    '#constant' => true,
  );
  $form['fields']['delta'] = array(
    '#type' => 'hidden',
    '#default_value' => $delta,
    '#constant' => true,
  );
  if ($k = call_user_func($module.'_filter', 'form', $delta, NULL, $filter)) {
    $form['fields'] += $k;
  }
  $form['settings'] = array(
    '#validate' => array('system_admin_format_form_validate'),
    '#title' => $add[$delta]['title']
  );
  $form['fields']['submit']['#type'] = 'submit';
  return $form;
}

function system_admin_format_form_validate($form, $v) {
  if (mb_strlen($v['title']) > 60) {
    $t['title']['#error'][] = t('system', '不超过 %length 字符', array('%length' => 60));
  } else if (mb_strlen($v['description']) > 255) {
    $t['description']['#error'][] = t('system', '不超过 %length 字符', array('%length' => 255));
  } else if (!$t = call_user_func($v['module'].'_filter', 'validate', $v['delta'], NULL, (object)$v)) {
    if ($v['fid']) {
      // 编辑
      if (!db_query('SELECT title FROM {filter_tags} WHERE title = ? AND fid != ?', array($v['title'], $v['fid']), array('return' => 'column'))) {
        db_exec('UPDATE {filter_tags} SET title = ?, description = ? WHERE fid = ?', array($v['title'], $v['description'], $v['fid']));
        dd_set_message(t('system', '编辑成功'));
      } else {
        $t['title']['#error'][] = t('system', '%string 已经存在', array('%string' => $v['title']));
      }
    } else if (!db_query('SELECT title FROM {filter_tags} WHERE title =?', array($v['title']), array('return' => 'column'))) {
      // 添加
      if (db_exec('INSERT INTO {filter_tags} (title, module, delta, description) VALUES (?, ?, ?, ?)', array($v['title'], $v['module'], $v['delta'], $v['description']))) {
        $v['fid'] = db_last_insert_id();
        dd_set_message(t('system', '保存成功'));
      } else {
        dd_set_message(t('system', '保存失败'));
        return true;
      }
    } else {
      $t['title']['#error'][] = t('system', '%string 已经存在', array('%string' => $v['title']));
    }
    call_user_func($v['module'].'_filter', 'save', $v['delta'], NULL, (object)$v);
    cache_del('cid', 'filter_get_filters'); // 清除自定义过滤器缓存
  }
  return $t;
}

/**
 * 输入法配置
 */
function system_admin_filter_form($fid = NULL) {
  if ($fid && (!$format = db_query('SELECT * FROM {filter_format} WHERE format = ?', array($fid), array('return' => 'one')))) {
    dd_set_message(t('system', '不正确的访问'));
    dd_goto('admin/filter');
    exit;
  }
  
  if ($format->name) {
    dd_set_title(array(t(0, $format->name), t('system', '输入法配置'), t('system', '输入设置')));
  } else {
    dd_set_title(array(t('system', '添加输入法'), t('system', '输入设置')));
  }
  
  if ($format->roles) {
    $format->roles = unserialize($format->roles);
  } else {
    $format->roles = array();
  }
  $form['fields']['name'] = array(
    '#title' => t('system', '名称'),
    '#type' => 'textfield',
    '#required' => 1,
    '#default_value' => $format->name,
    '#description' => t('system', '不超过 %length 字符', array('%length' => 60))
  );
  $form['fields']['description'] = array(
    '#title' => t('system', '描述'),
    '#type' => 'textfield',
    '#attributes' => array('size' => 60),
    '#default_value' => $format->description,
    '#description' => t('system', '不超过 %length 字符', array('%length' => 255))
  );
  if ($lists = _module_invoke_all('filter', 'list')) {
    $oldLists = $allLists = array();
    
    foreach ($lists as $module => $filters) {
      foreach ($filters as $delta => $filter) {
        $filter['delta'] = $delta;
        $filter['module'] = $module;
        $allLists[$delta.'_'.$module] = (object)$filter;
      }
    }
    if ($fid) {
      if ($fetch = db_query('SELECT * FROM {filters} WHERE format = ? ORDER BY weight ASC, fid DESC', array($fid))) {
        foreach ($fetch as $o) {
          $key = $o->delta.'_'.$o->module;
          if ($allLists[$key]) {
            $allLists[$key]->fid = $o->fid;
            $allLists[$key]->weight = $o->weight;
          }
        }
      }
  	  $form['fields']['fid'] = array(
  	    '#type' => 'hidden',
  	    '#default_value' => $fid,
  	    '#constant' => true,
  	  );
    }
    $form['fields']['roles'] = array(
      '#type' => 'checkbox',
      '#title' => t('system', '角色'),
      '#default_value' => $format->roles,
      '#description' => t('system', '允许使用此输入法'),
      '#options' => user_roles_opt()
    );
    $form['fields']['filter'] = array(
	    '#fieldset_prefix' => 'asc',
	    '#fieldset_legend' => t('system', '过滤器列表'),
      '#fieldset_suffix' => 1,
      '#description' => t('system', '权重将决定过滤器调用的先后顺序，多个过滤器之间可能存在冲突')
    );
    foreach ($allLists as $key => $filter) {
      $op = array();
      if ($filter->update) {
        if ($filter->update == 1) {
          $op[] = l(t('system', '设置'), 'admin/format/module/update/'.$filter->filter_id, dd_redirect());
        } else {
          $op[] = l(t('system', '设置'), $filter->update.'/'.$filter->delta, dd_redirect());
        }
      }
      if ($filter->delete) {
        if ($filter->delete == 1) {
          $op[] = l(t('system', '清除'), 'admin/format/module/delete/'.$filter->filter_id, dd_redirect());
        } else {
          $op[] = l(t('system', '清除'), $filter->delete.'/'.$filter->delta, dd_redirect());
        }
      }
      
      $form['fields']['filter'][$key] = array(
        '#fieldset_prefix' => 'asc',
        '#fieldset_legend' => t('system', $filter->title),
        '#fieldset_suffix' => 1,
        '#description' => dd_imp($op) . check_plain($filter->description),
        '#fieldset' => true,
      );
      $form['fields']['filter'][$key]['filter_id'] = array(
          '#type' => 'hidden',
          '#constant' => 1,
          '#default_value' => $filter->filter_id,
        );
      $form['fields']['filter'][$key]['check'] = array(
          '#title' => t('system', '启用'),
          '#prefix' => '<div>',
          '#type' => 'checkbox',
          '#default_value' => $filter->fid ? 1 : 0,
        );
      $form['fields']['filter'][$key]['weight'] = array(
          '#title' => t('system', '权重'),
          '#type' => 'weight',
          '#start' => -30,
          '#suffix' => '</div>',
          '#default_value' => $filter->weight ? $filter->weight : 0,
          '#end' => 30,
        );
    }
  }
  $form['settings']['#validate'] = array('system_admin_filter_form_validate');
  $form['fields']['submit']['#type'] = 'submit';
  
  return $form;
}

function system_admin_filter_form_validate($form, $v) {
  if (mb_strlen($v['name']) > 60) {
    $t['name']['#error'][] = t('system', '不超过 %length 字符', array('%length' => 60));
  } else if (mb_strlen($v['description']) > 255) {
    $t['description']['#error'][] = t('system', '不超过 %length 字符', array('%length' => 255));
  } else if ($v['fid']) {
    if (!db_query('SELECT name FROM {filter_format} WHERE name = ? AND format != ?', array($v['name'], $v['fid']), array('return' => 'column'))) {
      db_exec('UPDATE {filter_format} SET name = ?, description = ?, roles = ? WHERE format = ?', array($v['name'], $v['description'], serialize($v['roles']), $v['fid']));
      db_exec('DELETE FROM {filters} WHERE format = ?', array($v['fid']));
      $fid = $v['fid'];
      dd_set_message(t('system', '编辑成功'));
    } else {
      $t['name']['#error'][] = t('system', '%string 已经存在', array('%string' => $v['name']));
    }
  } else if (!db_query('SELECT name FROM {filter_format} WHERE name =?', array($v['name']), array('return' => 'column'))) {
    if (db_exec('INSERT INTO {filter_format} (name, description, roles) VALUES (?, ?, ?)', array($v['name'], $v['description'], serialize($v['roles'])))) {
      $fid = db_last_insert_id();
      dd_set_message(t('system', '添加成功'));
    }
  } else {
    $t['name']['#error'][] = t('system', '%string 已经存在', array('%string' => $v['name']));
  }
  
  if ($fid) {
    if ($v['filter']) {
      foreach ($v['filter'] as $key => $filter) {
        if (is_array($filter)) {
          list($delta, $module) = explode('_', $key, 2);
          if ($filter['check'] == 1) {
            if (!$filter['filter_id']) $filter['filter_id'] = 0;
            db_exec('INSERT INTO {filters} (format, filter, delta, module, weight) VALUES (?, ?, ?, ?, ?)', array($fid, $filter['filter_id'], $delta, $module, $filter['weight']));
          }
        }
      }
    }
    filter_set_cache();
  }
  
  return $t;
}

/**
 * @reference
 *  http://people.w3.org/rishida/names/languages.html
 */
function system_get_languages_data($lang = NULL) {
  $languages = array(
    'aa' => 'Afar',
    'ab' => 'Abkhazian(аҧсуа бызшәа)',
    'ae' => 'Avestan',
    'af' => 'Afrikaans',
    'ak' => 'Akan',
    'am' => 'Amharic(አማርኛ)',
    'ar' => 'Arabic(العربية)',
    'as' => 'Assamese',
    'av' => 'Avar',
    'ay' => 'Aymara',
    'az' => 'Azerbaijani(azərbaycan)',
    'ba' => 'Bashkir',
    'be' => 'Belarusian(Беларуская)',
    'bg' => 'Bulgarian(Български)',
    'bh' => 'Bihari',
    'bi' => 'Bislama',
    'bm' => 'Bambara(Bamanankan)',
    'bn' => 'Bengali',
    'bo' => 'Tibetan',
    'br' => 'Breton',
    'bs' => 'Bosnian(Bosanski)',
    'ca' => 'Catalan(Català)',
    'ce' => 'Chechen',
    'ch' => 'Chamorro',
    'co' => 'Corsican',
    'cr' => 'Cree',
    'cs' => 'Czech(Čeština)',
    'cu' => 'Old Slavonic',
    'cv' => 'Chuvash',
    'cy' => 'Welsh(Cymraeg)',
    'da' => 'Danish(Dansk)',
    'de' => 'German(Deutsch)',
    'dv' => 'Maldivian',
    'dz' => 'Bhutani',
    'ee' => 'Ewe(Ɛʋɛ)',
    'el' => 'Greek(Ελληνικά)',
    'en' => 'English',
    'eo' => 'Esperanto',
    'es' => 'Spanish(Español)',
    'et' => 'Estonian(Eesti)',
    'eu' => 'Basque(Euskera)',
    'fa' => 'Persian(فارسی)',
    'ff' => 'Fulah(Fulfulde)',
    'fi' => 'Finnish(Suomi)',
    'fj' => 'Fiji',
    'fo' => 'Faeroese',
    'fr' => 'French(Français)',
    'fy' => 'Frisian(Frysk)',
    'ga' => 'Irish(Gaeilge)',
    'gd' => 'Scots Gaelic',
    'gl' => 'Galician(Galego)',
    'gn' => 'Guarani',
    'gu' => 'Gujarati',
    'gv' => 'Manx',
    'ha' => 'Hausa',
    'he' => 'Hebrew(עברית)',
    'hi' => 'Hindi(हिन्दी)',
    'ho' => 'Hiri Motu',
    'hr' => 'Croatian(Hrvatski)',
    'hu' => 'Hungarian(Magyar)',
    'hy' => 'Armenian(Հայերեն)',
    'hz' => 'Herero',
    'ia' => 'Interlingua',
    'id' => 'Indonesian(Bahasa Indonesia)',
    'ie' => 'Interlingue',
    'ig' => 'Igbo',
    'ik' => 'Inupiak',
    'is' => 'Icelandic(Íslenska)',
    'it' => 'Italian(Italiano)',
    'iu' => 'Inuktitut',
    'ja' => 'Japanese(日本語)',
    'jv' => 'Javanese',
    'ka' => 'Georgian',
    'kg' => 'Kongo',
    'ki' => 'Kikuyu',
    'kj' => 'Kwanyama',
    'kk' => 'Kazakh(Қазақ)',
    'kl' => 'Greenlandic',
    'km' => 'Cambodian',
    'kn' => 'Kannada(ಕನ್ನಡ)',
    'ko' => 'Korean(한국어)',
    'kr' => 'Kanuri',
    'ks' => 'Kashmiri',
    'ku' => 'Kurdish(Kurdî)',
    'kv' => 'Komi',
    'kw' => 'Cornish',
    'kg' => 'Kirghiz(Кыргыз)',
    'la' => 'Latin(Latina)',
    'lb' => 'Luxembourgish',
    'lg' => 'Luganda',
    'ln' => 'Lingala',
    'lo' => 'Laothian',
    'lt' => 'Lithuanian(Lietuvių)',
    'lv' => 'Latvian(Latviešu)',
    'mg' => 'Malagasy',
    'mh' => 'Marshallese',
    'mi' => 'Maori',
    'mk' => 'Macedonian(Македонски)',
    'ml' => 'Malayalam(മലയാളം)',
    'mn' => 'Mongolian',
    'mo' => 'Moldavian',
    'mr' => 'Marathi',
    'ms' => 'Malay(Bahasa Melayu)',
    'mt' => 'Maltese(Malti)',
    'my' => 'Burmese',
    'na' => 'Nauru',
    'nd' => 'North Ndebele',
    'ne' => 'Nepali',
    'ng' => 'Ndonga',
    'nl' => 'Dutch(Nederlands)',
    'nb' => 'Norwegian Bokmål(Bokmål)',
    'nn' => 'Norwegian Nynorsk(Nynorsk)',
    'nr' => 'South Ndebele',
    'nv' => 'Navajo',
    'ny' => 'Chichewa',
    'oc' => 'Occitan',
    'om' => 'Oromo',
    'or' => 'Oriya',
    'os' => 'Ossetian',
    'pa' => 'Punjabi',
    'pi' => 'Pali',
    'pl' => 'Polish(Polski)',
    'ps' => 'Pashto(پښتو)',
    'pt-pt' => 'Portuguese, Portugal(Português)',
    'pt-br' => 'Portuguese, Brazil(Português)',
    'qu' => 'Quechua',
    'rm' => 'Rhaeto-Romance',
    'rn' => 'Kirundi',
    'ro' => 'Romanian(Română)',
    'ru' => 'Russian(Русский)',
    'rw' => 'Kinyarwanda',
    'sa' => 'Sanskrit',
    'sc' => 'Sardinian',
    'sd' => 'Sindhi',
    'se' => 'Northern Sami',
    'sg' => 'Sango',
    'sh' => 'Serbo-Croatian',
    'si' => 'Sinhala(සිංහල)',
    'sk' => 'Slovak(Slovenčina)',
    'sl' => 'Slovenian(Slovenščina)',
    'sm' => 'Samoan',
    'sn' => 'Shona',
    'so' => 'Somali',
    'sq' => 'Albanian(Shqip)',
    'sr' => 'Serbian(Српски)',
    'ss' => 'Siswati',
    'st' => 'Sesotho',
    'su' => 'Sudanese',
    'sv' => 'Swedish(Svenska)',
    'sw' => 'Swahili(Kiswahili)',
    'ta' => 'Tamil(தமிழ்)',
    'te' => 'Telugu(తెలుగు)',
    'tg' => 'Tajik',
    'th' => 'Thai(ภาษาไทย)',
    'ti' => 'Tigrinya',
    'tk' => 'Turkmen',
    'tl' => 'Tagalog',
    'tn' => 'Setswana',
    'to' => 'Tonga',
    'tr' => 'Turkish(Türkçe)',
    'ts' => 'Tsonga',
    'tt' => 'Tatar(Tatarça)',
    'tw' => 'Twi',
    'ty' => 'Tahitian',
    'ug' => 'Uighur',
    'uk' => 'Ukrainian(Українська)',
    'ur' => 'Urdu(اردو)',
    'uz' => "Uzbek(o'zbek)",
    've' => 'Venda',
    'vi' => 'Vietnamese(Tiếng Việt)',
    'wo' => 'Wolof',
    'xh' => 'Xhosa(isiXhosa)',
    'yi' => 'Yiddish',
    'yo' => 'Yoruba(Yorùbá)',
    'za' => 'Zhuang',
    'zh-hans' => 'Chinese, Simplified(简体中文)',
    'zh-hant' => 'Chinese, Traditional(繁體中文)',
    'zu' => 'Zulu(isiZulu)',
  );
  return $lang ? $languages[$lang] : $languages;
}