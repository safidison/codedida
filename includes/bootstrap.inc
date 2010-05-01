<?php
// $Id$

define('DIDA_VERSION', '1.0 alpha 1');
/**
 * 初始化
 */
function bootstrap($op, $inc = NULL){
  static $called;
  
  if ($called) return;
  
  global $user, $database, $dbc, $db, $conf, $conf_dir, $installed, $cache_file;
  
  if(is_file('sites/config.php')){
    require_once 'sites/config.php';
    if($config && $config[$_SERVER['HTTP_HOST']]){
      $conf_dir = $config[$_SERVER['HTTP_HOST']];
    }
  }
  
  if(!$conf_dir){
    if(is_dir('sites/'.$_SERVER['HTTP_HOST'])){
      $conf_dir = 'sites/'.$_SERVER['HTTP_HOST'];
    }else{
      $conf_dir = 'sites';
    }
  }
  
  if(!is_file('./'.$conf_dir .'/setting.php')){
    dd_to_install();
  }
  
  require_once './'.$conf_dir.'/setting.php';
  
  if(!$installed) dd_to_install();
  
  if(is_file($conf_dir .'/cache/conf.php')){
    require_once './'. $conf_dir .'/cache/conf.php';
  }else{
    dd_to_install();
  }
  
  if(!$conf['status']) dd_get_online();
  
  set_error_handler('dd_php_error');
  
  switch ($op){
    case 'full':
      require_once './includes/database.inc';
      //初始化默认数据库连接
      db_connect('default');
      
      // session 及配置
      conf_init();
      
      require_once './includes/filter.inc';
      require_once './includes/theme.inc';
      require_once './includes/image.inc';
      require_once './includes/form.inc';
      require_once './includes/file.inc';
      require_once DD_CACHE_FILE;
      require_once './includes/module.inc';
      require_once './includes/menu.inc';
      require_once './includes/pager.inc';
      
      //用户验证
      dd_user_validate();
      //载入主题
      theme_init();
      //载入模块
      module_init();
      
      if($_POST['__SETPHPSESSID'] && $_POST['__SETUID']){
        if(!$user->uid && db_query('SELECT name FROM {users} WHERE session = ? AND uid = ? AND host = ?', array($_POST['__SETPHPSESSID'], $_POST['__SETUID'], ip_address()))){
          if(!$user = user_login(user_load($_POST['__SETUID']))){
            $user->__SETVALIDATE = NULL;
          }
        }
      }else{
        $user->__SETVALIDATE = NULL;
      }
      
      switch($conf['status']){
        case 2: //登录后可浏览
          if(!$user->uid && $_GET['q'] != 'user/login') dd_get_online();
        break;
        case 3: //超级管理员可浏览
          if($user->uid != 1 && $_GET['q'] != 'user/login') dd_get_online();
      }
    break;
    case 'data':
      /**
       * 仅载入数据库连接
       */
      require_once './includes/database.inc';
      //初始化默认数据库连接
      db_connect('default');
    break;
    case 'custom':
      /**
       * 跳过所有模块，默认仅加载数据库及缓存接口文件
       * 提供数据库连接，用户验证，全局配置
       * 可自定义载入其它核心文件
       */
      require_once './includes/database.inc';
      //初始化默认数据库连接
      db_connect('default');
      
      // session 及配置
      conf_init();
      
      require_once DD_CACHE_FILE;
      
      if(is_array($inc)){
        foreach($inc as $filename){
          $filepath = './includes/'.$filename.'.inc';
          if(is_file($filepath)) require_once $filepath;
        }
      }
      
      //用户验证
      dd_user_validate();
      
      switch($conf['status']){
        case 2: //登录后可浏览
          if(!$user->uid && $_GET['q'] != 'user/login') dd_get_online();
        break;
        case 3: //超级管理员可浏览
         if($user->uid != 1 && $_GET['q'] != 'user/login') dd_get_online();
      } 
  }
  $called = 1;
}

/**
 * 获取模块、主题文件物理路径
 * @param (string) $type
 *  类型，分为：theme、module
 * @param (string) $name
 *  模块或主题的系统名称
 */
function dd_get_path($type, $name){
  global $conf;
  switch($type){
    case 'module':
      $lists = $conf['modules'];
    break;
    case 'theme':
      $lists = $conf['themes'];
  }
  return $lists[$name]['path'];
}

/**
 * 写入一个任务到cron表，在调度 cron 时，优先执行该
 * @param (string) $module
 *  模块系统名称，错误的名称将无法写入
 * @param (string) $type
 *  任务类型，由模块自定义，便于自行查找
 * @param (array) $data
 *  任务数据。必须包含回调函数。参数(*表示必须，以下同)：
 *    *(string) func：执行该条任务时，将回调此函数。返回 true 为成功
 *    (array) args：传递给回调函数的参数
 *    (array) includes：需要加载的文件，完整的系统路径
 *    (string) success：成功执行回调的函数，并以 args 作为参数传递。
 * @param (int) $weight
 *  任务权重，可提高任务优先级。排序方式：weight ASC, cid ASC
 * @return
 *  写入成功则返回 cid
 */
function dd_save_cron($module, $type, $data = array(), $weight = 0){
  if($GLOBALS['conf']['modules'][$module]){
    if(db_exec('INSERT INTO {cron} (module, type, data, weight) VALUES (?, ?, ?, ?)',array($module, $type, serialize($data), $weight))){
      return db_last_insert_id();
    }
  }
}

/**
 * 站点关闭
 */
function dd_get_online(){
  header('Content-Type: text/html; charset=utf-8');
  if($output = @file_get_contents($GLOBALS['conf_dir'] . '/cache/site_offline_html.lang')){
    echo $output;
  }else{
    echo '站点维护中…';
  }
  exit;
}

/**
 * 获取完整 URL
 */
function dd_get_absolute_url(){
  return $GLOBALS['base_url'] . $_SERVER['REQUEST_URI'];
}

/**
 * 页面不存在
 */
function dd_get_not(){
  if($GLOBALS['conf']['page_not_history']){
    dd_set_message(t('system', '抱歉，%string 不存在', array('%string' => dd_get_absolute_url())), 'error');
    dd_goto(dd_get_history());
  }else{
    echo theme('print', custom_get('page_not_html'));
  }
  exit;
}

/**
 * 没有访问权限，如未登录，则显示登录表单
 */
function dd_get_access(){
  global $user;
  if($user->uid > 0){
    if($GLOBALS['conf']['page_access_history']){
      dd_set_message(t('system', '抱歉，没有权限访问该 %string', array('%string' => dd_get_absolute_url())), 'error');
      dd_goto(dd_get_history());
    }else{
      echo theme('print', custom_get('page_access_html'));
    }
  }else{
  	dd_set_message(t('system', '请登录确认权限'), 'error');
    echo theme('print', dd_get_form('user_login_form'));
  }
  exit;
}

/**
 * 上一页地址，若上一页非当前网站，则返回首页
 */
function dd_get_history(){
	if($_SERVER['HTTP_REFERER'] && strpos($_SERVER['HTTP_REFERER'], $GLOBALS['base_url']) !== false){
		return urlencode($_SERVER['HTTP_REFERER']);
	}else{
		return $GLOBALS['base_url'];
	}
}

//注销全局变量
function dd_unset_globals() {
  if (ini_get('register_globals')) {
    $allowed = array('_ENV' => 1, '_GET' => 1, '_POST' => 1, '_COOKIE' => 1, '_FILES' => 1, '_SERVER' => 1, '_REQUEST' => 1, 'GLOBALS' => 1);
    foreach ($GLOBALS as $key => $value) {
      if (!isset($allowed[$key])) {
        unset($GLOBALS[$key]);
      }
    }
  }
}

//页面跳转
function dd_goto($path = '', $query = NULL, $file = NULL, $fragment = NULL, $http_response_code = 301) {
  $url = url(urldecode($path), array('query' => $query, 'fragment' => $fragment, 'absolute' => true, 'file' => $file));
  session_write_close();
  header('Location: '. $url, true, $http_response_code);
  exit();
}

// 超链接格式化
function l($text, $path, $options = array()) {
  if(!$options['attributes']) $options['attributes'] = array();
  if(!$options['attributes']['title']){
    $options['attributes']['title'] = $text;
  }
  if($options['strlen'] && dd_strlen($text) > $options['strlen']){
    $text = dd_substr($text, 0, intval($options['strlen'])) . '...';
  }
  if($_GET['q'] == $path){
    $options['attributes']['class'] .= $options['attributes']['class'] ? ' active' : 'active';
  }
  if (isset($options['attributes']['title']) && strpos($options['attributes']['title'], '<') !== FALSE) {
    $options['attributes']['title'] = strip_tags($options['attributes']['title']);
  }
  return '<a href="'. url($path, $options) .'"'. dd_attributes($options['attributes']) .'>'. ($options['html'] ? $text : strip_tags($text)) .'</a>';
}

// 图片格式化
function img($path, $alt = '', $title = '', $attributes = NULL) {
  $url = _l_external($path) ? $path : f($path);
  return '<img src="'. $url .'" alt="'. strip_tags($alt) .'" title="'. strip_tags($title) .'" ' . dd_attributes($attributes) .' />';
}

function _l_external($path){
	
	if(substr($path, 0, 1) == '/') {
		return true;
	}
	
  if(strpos($path, ':') !== false){
  	return in_array(current(explode(':', $path, 2)), array('http', 'https', 'ftp', 'news', 'nntp', 'telnet', 'mailto', 'irc', 'ssh', 'sftp', 'webcal', 'rtsp'));
  }
}

// 输出物理文件地址
function f($filepath){
  return $GLOBALS['base_path'] . $filepath;
}

// url 解析
function url($path = NULL, $options = array()) {
  if($options['file']){
    return f($path);
  }
  if (!isset($options['external'])) {
    $options['external'] = _l_external($path);
  }
  if ($options['external']) {
    return $path;
  }
  if($options['fragment']) {
    $options['fragment'] = '#'. $options['fragment'];
  }
  global $conf, $base_url, $base_path;
  if (is_array($options['query'])) {
    $options['query'] = dd_query_string_encode($options['query']);
  }else if(strpos($options['query'], '=') === false){
    $options['query'] = '';
  }
  
  if($conf['clean_url']) {
    $url = $base_path . $path;
    if($options['query']){
      if(strpos($options['query'], '?') === false){
        $url .= '?' . $options['query'];
      }else{
        $url .= '&' . $options['query'];
      }
    }
    $url .= $options['fragment'];
  }else{
    if (!empty($path)) {
      $path = '?q='. $path;
    }
    if ($options['query']) {
      $path = $path .'&'. $options['query'];
    }
    $url = $base_path . $path .$options['fragment'];
  }
  if(!$options['absolute']){
    return $url;
  }else{
    return $GLOBALS['base_url'] . $url;
  }
}

// html属性解析
function dd_attributes($attributes) {
  if (is_array($attributes)) {
    $t = '';
    foreach ($attributes as $key => $value) {
      if($value) $t .= " $key=".'"'. check_plain($value) .'"';
    }
    return $t;
  }
}

/**
 * 日志记录
 * @param $type = String
 *  类型，自定义
 * @param $value = String
 *  内容
 * @param $link = string
 *  发生地址，默认为当前地址
 * @param $status = int
 *  状态，0、普通，1、警告，2、严重
 * @param $uid = int
 *  用户 id，默认为当前用户
 */
function dd_log($type, $value, $link = NULL, $status = NULL, $uid = NULL){
  if($value){
    if(!$uid) $uid = $GLOBALS['user']->uid;
    if(!$status) $status = 0;
    
    db_exec('INSERT INTO {logs} (uid, type, status, value, url, timestamp, host) VALUES (?, ?, ?, ?, ?, ?, ?)', array($uid, $type, $status, $value, ($link ? $link : $_SERVER['REQUEST_URI']), $_SERVER['REQUEST_TIME'], ip_address()));
  }
}

/**
 * 写入配置，若存在则更新。初始化时将加载所有配置。
 * @param $name = String
 *  配置名称，不应大于 255 个字符
 * @param $value = *
 *  配置内容，序列化存储
 * $return 无返回值，但将即时更新全局变量 $conf
 * @param $clear = 1;
 *  清除配置文件缓存
 */
function var_set($name, $value, $clear = 1) {
  global $conf;
  $serialized_value = serialize($value);
  if(!db_exec('UPDATE {variable} SET value = ? WHERE name = ?', array($serialized_value, $name)) && !db_query('SELECT name FROM {variable} WHERE name = ?', array($name), array('return' => 'column'))) {
    db_exec('INSERT INTO {variable} (name, value) VALUES (?, ?)', array($name, $serialized_value));
  }
  $conf[$name] = $value;
  if($clear) var_init();
}

function custom_set($name, $value) {
  $serialized_value = serialize($value);
  if(!db_exec('UPDATE {custom} SET value = ? WHERE name = ?', array($serialized_value, $name)) && !db_query('SELECT name FROM {custom} WHERE name = ?', array($name), array('return' => 'column'))) {
    db_exec('INSERT INTO {custom} (name, value) VALUES (?, ?)', array($name, $serialized_value));
  }
}

/**
 * 读取配置
 * @param $name = string
 *  配置名称
 * @param $default = *
 *  默认值，若没有请求的配置，则返回此值
 * @return * 或 false;
 */
function var_get($name, $default = NULL) {
  global $conf;
  return isset($conf[$name]) ? $conf[$name] : $default;
}

function custom_get($name, $default = NULL) {
  static $custom;
  if(!isset($custom[$name])){
    if($value = db_query('SELECT value FROM {custom} WHERE name = ?', array($name), array('return' => 'column'))){
      $custom[$name] = unserialize($value);
    }else{
      $custom[$name] = $default;
    }
  }
  return $custom[$name];
}
/**
 * 读取子配置，针对数组配置，参数同 var_get()
 * @param $key = string
 *  子配置名称，即数组键名
 */
function var_get_key($name, $key, $default = false) {
  global $conf;
  return isset($conf[$name][$key]) ? $conf[$name][$key] : $default;
}

/**
 * 删除配置
 * @param $name = string
 *  配置名称
 */
function var_del($name, $clear = 1) {
  global $conf;
  db_exec('DELETE FROM {variable} WHERE name = ?', array($name));
  unset($conf[$name]);
  if($clear){
    var_init();
  }
}

function custom_del($name){
  db_exec('DELETE FROM {custom} WHERE name = ?', array($name));
}

/**
	* 更新配置缓存，无参数，无返回值
	*/
function var_init() {
  if($fetch = db_query('SELECT * FROM {variable}')){
    foreach ($fetch as $variable) {
      $v[$variable->name] = unserialize($variable->value);
    }
  }
  cache_system_set_file('conf.php', 'conf', $v);
}

/**
 * 多语言
 * @param $index = string
 *  语言字符串键名，若无对应值，则返回键名
 * @param $module = string
 *  默认系统名称，赋值则优先使用该模块定义的值
 * @param $args = array()
 *  要转换的字符串参数
 * @param $language = string
 *  指定语言
 * 所有模块的语言文件缓存在 ./sties/cache/lang_{$language}.lang
 */
function t($module, $index, $args = array()) {
	static $lang;
	if(!$language){
		$language = !$GLOBALS['user']->language ? $GLOBALS['conf']['default_language'] : $GLOBALS['user']->language;
	}
  
	if(!$module || $language != $GLOBALS['conf']['modules'][$module]['language']){
  	if(!isset($lang[$language])){
  	  if(is_file($GLOBALS['conf_dir'] . '/cache/languages/' .$language . '.php')){
  	    include $GLOBALS['conf_dir'] . '/cache/languages/' .$language . '.php';
  	    $lang[$language] = $l;
  		}else{
  			$lang[$language] = false;
  		}
  	}
    
  	if($module && $lang[$language] && $lang[$language][$index]) {
  		if(!is_array($lang[$language]['__'.$module]) || $lang[$language]['__'.$module][$index]){
        $index = $lang[$language][$index];
  		}else{
        $index = $lang[$language]['__'.$module][$index];
      }
  	}
    
	}
	
  if(empty($args)) {
    return $index;
  }else{
    foreach ($args as $key => $value) {
      switch ($key[0]) {
        case '@':
        	// 格式化
          $args[$key] = check_plain($value);
        break;
        case '%': 
          // 替换占位符 <em> </em>
          $args[$key] = '<em>'. $value . '</em>';
        break;
        //case '!': default:
          // 其它不做处理
      }
    }
    return strtr($index, $args);
  }
}

/**
 * 合并模块语言文件
 * @param $type = string
 *  合并方式：update - 保存现有字符串，insert - 全新导入
 * @param $lists = array()
 *  模块列表，默认为所有模块文件夹下的语言文件，modules/test/lang/*
 * @param $language = array()
 *  需导入的语言，默认为所有设定的语言
 * 语言文件缓存在 ./sties/cache/lang_{$language}.lang，合并规则：
 *  若有相同键名，取第一个保存为默认字符串，其它保存为特殊翻译串。
 * 特殊翻译保存在以模块系统名称加双下划线为键名的数组下
 * 如 __block，以此避免与模块名称翻译产生冲突。
 *  以此增加语言字符串重用率，减少语言文件体积。同时可为相同字符串提供多种翻译。
 * 示例：
 *  system.module 定义了 $lang['cache'] = '缓存'
 *  block.module 定义了 $lang['cache'] = '区块缓存'; // 假设
 *  在导入合并时，'缓存' 将作为 'cache' 的默认翻译，而 '区块缓存' 作为特殊翻译
 * 最终的语言文件如：
 *  $lang = array(
 			'cache' => '缓存',
 			'__block' => array(
 				'cache' => '区块缓存',
 			)
 		)
 */
function dd_set_lang($type = 'update', $lists = NULL, $language = array()){
	global $conf;
	if(empty($lists)){
		if($conf['modules']){
	    foreach ($conf['modules'] as $module => $info) {
	    	$lists[$module] = $info['path'];
	    }
    }
	}
	if(empty($language)){
		if($conf['languages']){
			foreach($conf['languages'] as $key => $name){
				$language[] = $key;
			}
		}else{
			$language[] = 'zh-hans';
		}
	}
	if($lists){
		foreach($language as $code){
			$l = array();
			
			if($type == 'update'){
				$cache = $GLOBALS['conf_dir'] . '/cache/languages/'.$code.'.php';
				if(is_file($cache)){
					include $cache;
				}
			}
			
			foreach($lists as $module => $path){
				$lang = array();
				$path = './' . $path . '/languages/' . $code . '.php';
				
				if(is_file($path)){
					include $path;
					if($lang){
						$newLang = array_diff_key($lang, $l);
						$reLang = array_intersect_key($lang, $l);
						
						if($newLang){
							$l = array_merge($l, $newLang);
						}
						if($reLang){
              foreach($reLang as $rekey => $resub){
                if($l[$rekey] == $resub){
                  unset($reLang[$rekey]);
                }
              }
              if($reLang){
                $moduleKey = '__'.$module;
  							if(!$l[$moduleKey]){
  								$l[$moduleKey] = $reLang;
  							}else{
  								$l[$moduleKey] = array_merge($reLang, $l[$moduleKey]);
  							}
              }
						}
					}
				}else{
					continue;
				}
			}
			
			if($l){
				// 合并为缓存文件
				cache_system_set_file($code.'.php', 'l', $l, $GLOBALS['conf_dir'].'/cache/languages');
				dd_set_message($code . ' 导入成功');
			}
		}
	}
}

/**
 * 调用 hook_token_replace 进行遍历替换
 * @param (string)$text
 *  要替换字符串
 * @param (*) $value
 *  包含一组替换值
 * @param (array)$module
 *  指定模块，若不指定，则调用所有模块的 hook_token_replace
 */
function dd_get_token($text, $value = NULL, $modules = NULL){
	if(!is_array($modules)) {
		$hooks = dd_get_hook('token_replace');
	}else{
		foreach($modules as $module){
			$hooks[$module] = $module . '_token_replace'; 
		}
	}
	
	if($hooks){
		foreach($hooks as $function){
			$text = $function($text, $value);
		}
	}
	return $text;
}

/**
 * hook 缓存，将hook函数缓存
 */
function dd_get_hook($hook, $clear = 0){
	static $hooks;
	if(!isset($hooks[$hook])){
		if(!$clear && ($cache = cache_get('hook_cache'))){
			$hooks = $cache->data;
		}
		if(!$cache->data || !is_array($hooks[$hook])) {
			$hooks[$hook] = array();
		  foreach ($GLOBALS['conf']['modules'] as $module => $info) {
		    $function = $module .'_'. $hook;
		    if(function_exists($function)) {
		      $hooks[$hook][$module] = $function;
		    }
		  }
		  cache_set('hook_cache', $hooks);
		}
	}
	return $hooks[$hook];
}

/**
 * 数组转为对象
 */
function array_to_object($array){
	if(is_array)
		return (object) $array;
}

//获取随机字符串
function dd_rand_str(){
  return md5(mt_rand(). time() . session_id());
}

// 获取客户端 ip
function ip_address() {
  static $ip_address = NULL;
  if (!isset($ip_address)) {
    $ip_address = $_SERVER['REMOTE_ADDR'];
    if (array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER)) {
      $ip_address = array_pop(explode(',', $_SERVER['HTTP_X_FORWARDED_FOR']));
    }
  }
  return $ip_address;
}

/**
 *  用户验证
 */
function dd_user_validate(){
  global $user, $cookie_domain;
  
  if($_COOKIE[session_name()] && $_SESSION['user'] && $_SESSION['user']->session == $_COOKIE[session_name()]){
    $user = $_SESSION['user'];
    $active = $_COOKIE['time'];
    $time = time();
    setcookie('time', $time, $time + 31536000, '/', $cookie_domain);
    if($active > ($time - 1800)){ 
      // 两次活动时间间隔 30 分钟内
      if(($active > ($user->active + 300)) && ($active < ($user->active + 600))){
        //每300秒写入一次用户在线时间。若距上次活动超过600秒，为不活动用户。
        $online = $active - $user->active; //增加在线时间
        db_exec('UPDATE {users} SET active = ?, online = online + ? WHERE uid = ?', array($time, $online, $user->uid));
        $user->active = $time;
        $user->exp += $exp;
        $user->online += $online;
      }
    }
    if(!$user->__SETVALIDATE){
      if($active > ($user->active + 600)){
        //每600秒验证一次用户session
        if(!db_query('SELECT uid FROM {users} WHERE uid = ? AND session = ?', array($user->uid, $_COOKIE[session_name()]), array('return' => 'column'))){
          $user = user_logout();  // 游客
          dd_set_message('登录超时，请重新登录', 'error');
        }else{
          $user->active = $time;
        }
      }
    }
  }
  
  if(!$user->timezone){
  	$user->timezone = 'Asia/Taipei';
  }
  if(!$user->uid){
    $user = user_anonymous();  // 游客
  }
  
  date_default_timezone_set(var_get('default_timezone', 'Asia/Taipei'));
}

/**
 * 验证超级管理员
 */
function dd_is_root_admin(){
  return $GLOBALS['user']->uid == 1;
}

/**
 * 注销，触发 hook_user_logout($ac)
 */
function user_logout() {
  global $user, $cookie_domain;
  
  module_invoke_all('user_logout', $user);
  
  setcookie(session_name(), '', -1, '/');
  session_destroy();
  $user = user_anonymous();
  dd_set_message(t('system', '退出成功'));
  dd_goto('');
}

/**
 * 游客信息
 */
function user_anonymous(){
  global $cookie_domain, $conf;
  setcookie('time', '', -1, '/', $cookie_domain);
  $user->uid = 0;
  $user->name = $user->roles[var_get_key('user', 'anonymous_role', 1)] = var_get_key('user', 'anonymous', '游客');
  $user->session = $_COOKIE[session_name()];
  $user->host = ip_address();
  return $user;
}

/**
 * 获取域名、路径、cookie 作用域、session id
 */
function conf_init(){
  global $cookie_domain, $base_url, $base_root, $base_path, $is_front;
  
  $is_front = dd_is_front();
  
  $base_root = (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') ? 'https' : 'http';
  $base_url = $base_root .= '://'. $_SERVER['HTTP_HOST'];
  if ($dir = trim(dirname($_SERVER['SCRIPT_NAME']), '\,/')) {
    $base_path = "/$dir";
    $base_url .= $base_path;
    $base_path .= '/';
  } else {
    $base_path = '/';
  }
  
  list( , $session_name) = explode('://', $base_url, 2);
  if (!empty($_SERVER['HTTP_HOST'])) {
    $cookie_domain = check_plain($_SERVER['HTTP_HOST']);
  }
  
  if (ini_get('session.cookie_secure')) {
    $session_name .= 'SSL';
  }
  
  $cookie_domain = ltrim($cookie_domain, '.');
  if (strpos($cookie_domain, 'www.') === 0) {
    $cookie_domain = substr($cookie_domain, 4);
  }
  
  $cookie_domain = explode(':', $cookie_domain);
  $cookie_domain = '.'. $cookie_domain[0];
  if (count(explode('.', $cookie_domain)) > 2 && !is_numeric(str_replace('.', '', $cookie_domain))) {
    ini_set('session.cookie_domain', $cookie_domain);
  }
  
  session_name('DIDA'. md5($session_name));
  
  session_start();
}

/**
 * 转至安装文件
 */
function dd_to_install(){
  if(!$to = trim(dirname($_SERVER['SCRIPT_NAME']), '\,/')){
    $to = '/';
  }else{
    $to .= '/';
  }
  header('Location: '.$to.'install.php');
  exit;
}

/**
 * 获得当前毫秒数
 */
function dd_get_microtime(){
  list($usec, $sec) = explode(" ", microtime());
  $time = ((float)$usec + (float)$sec);
  if(function_exists('bcmul')){
    return bcmul($time, 1000);
  }else{
    return $time;
  }
}

function format_date_week_cn($timestamp){
	$cn = array('日', '一', '二', '三', '四', '五', '六');
	return $cn[date('w', $timestamp)];
}

// 格式时间戳
function format_date($timestamp, $type = 'default', $format = '') {
  switch ($type) {
    case 'small':
      $format = 'Y/m/d';
    break;
    case 'large':
      $cn = array('日', '一', '二', '三', '四', '五', '六');
      $m = date('w', $timestamp);
      $format = 'Y/m/d H:i:s 星期'.$cn[$m];
    break;
    case 'custom':
      if(!$format) $format = 'Y/m/d H:i:s';
    break;
    case 'medium':
      $format = 'Y/m/d H:i:s';
    break;
    default:
      $format = 'Y/m/d H:i:s';
  }
  return date($format, $timestamp);
}

/**
 * 处理 PHP 错误信息
 */
function dd_php_error($errno, $message, $filename, $line = NULL, $context = NULL){
  if(error_reporting() == 0) return;
  if($errno & (E_ALL ^ E_NOTICE)){
    $msg = t('system', '!message。文件：%file(%line 行)', array('!message' => $message, '%file' => $filename, '%line' => $line));
    
    text_log('php_error', dd_error_msg($msg));
    
    if(!$GLOBALS['conf']['debug']) return;
    if($GLOBALS['conf']['debug'] == 2 && $GLOBALS['user']->uid != 1) return;
    
    dd_set_message($msg, 'error');
  }
}

/**
 * ping services
 */
function dd_set_ping($data, $service = array(), $method = 'weblogUpdates.extendedPing'){
  
  if(!$service){
    $service = array(
      'http://ping.baidu.com/ping/RPC2',
      'http://rpc.pingomatic.com/',
      'http://blog.yodao.com/ping/RPC2',
      // 'http://rpc.technorati.com/rpc/ping',
      // 'http://blogsearch.google.com/ping/RPC2' // google
    );
  }
  
  $xml = '<?xml version="1.0"?>';
  $xml .= '<methodCall><methodName>'.$method.'</methodName>';
  $xml .= '<params>';
  
  
  foreach($data['params'] as $param){
    $xml .= '<param><value>'.$param.'</value></param>';
  }
  
  $xml .= '</params>';
  $xml .= '</methodCall>';
  
  foreach($service as $ping){
    $result[] = dd_http_request($ping, $xml, 'POST', array('Content-Type' => 'text/xml'));
  }
  
  return $result;
}

/**
 * 执行一个 HTTP 请求
 *
 * @param (string) $url
 *  合法完整的 URL
 * @param (array) $headers
 *  HTTP header 数组
 * @param (string) $method
 *  请求方式：POST、GET、PUT
 * @param (*) $data
 *  写入的数据，字符串或数组、对象
 * @param (int) $retry
 *  重定向次数
 * @return
 *  返回一个包含执行结果的对象
 *  $result->error：错误提示
 *  $result->code：HTTP 响应代码，小于 0 则为请求失败
 *  $result->data：返回内容
 */
function dd_http_request($url, $data = NULL, $method = 'POST', $headers = array(), $retry = 3) {
  $result = new stdClass();
  
  $uri = parse_url($url);
  
  if ($uri == FALSE) {
    $result->error = t('system', '无法解析网址');
    $result->code = -1001;
    return $result;
  }
  
  if (!isset($uri['scheme'])) {
    $result->error = t('system', '无效的协议');
    $result->code = -1002;
    return $result;
  }
  
  switch ($uri['scheme']) {
    case 'http':
      $port = isset($uri['port']) ? $uri['port'] : 80;
      $host = $uri['host'] . ($port != 80 ? ':'. $port : '');
      $fp = @fsockopen($uri['host'], $port, $errno, $errstr, 15);
      break;
    case 'https':
      // 须编译 openssl
      $port = isset($uri['port']) ? $uri['port'] : 443;
      $host = $uri['host'] . ($port != 443 ? ':'. $port : '');
      $fp = @fsockopen('ssl://'. $uri['host'], $port, $errno, $errstr, 20);
      break;
    default:
      $result->error = t('system','%string 协议不存在', array('%string' => $uri['scheme']));
      $result->code = -1003;
      return $result;
  }
  
  // 服务器无响应
  if (!$fp) {
    $result->code = -$errno;
    $result->error = trim($errstr);
    return $result;
  }
  
  // 解析路径
  $path = isset($uri['path']) ? $uri['path'] : '/';
  if (isset($uri['query'])) {
    $path .= '?'. $uri['query'];
  }
  
  // 创建 HTTP 请求
  $defaults = array(
    'Host' => "Host: $host",
    'User-Agent' => 'User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; zh-CN; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 GTBDFff GTB7.0 ',
  );
  
  if(is_array($data) || is_object($data)){
    foreach($data as $key => $value){
      $http_data .= rawurlencode(trim($key)) .'=' . rawurlencode(trim($value)) . '&';
    }
    $http_data = substr($http_data, 0, -1);
  }else{
    $http_data = $data;
  }
  
  $content_length = strlen($http_data);
  if ($content_length > 0 || $method == 'POST' || $method == 'PUT') {
    $defaults['Content-Length'] = 'Content-Length: '. $content_length;
  }
  
  // 身份验证
  if (isset($uri['user'])) {
    $defaults['Authorization'] = 'Authorization: Basic '. base64_encode($uri['user'] . (!empty($uri['pass']) ? ":". $uri['pass'] : ''));
  }
  
  if($method == 'POST' && !$headers['Content-Type']){
    $defaults['Content-Type'] = 'Content-Type: application/x-www-form-urlencoded; charset=utf-8';
  }
  
  if($headers){
    foreach ($headers as $header => $value) {
      $defaults[$header] = $header .': '. $value;
    }
  }
  $request = $method .' '. $path ." HTTP/1.0\r\n";
  $request .= implode("\r\n", $defaults);
  $request .= "\r\n\r\n";
  $request .= $http_data;
  
  $result->request = $request;
  
  fwrite($fp, $request);
  
  // HTTP 响应
  $response = '';
  while (!feof($fp) && $chunk = fread($fp, 1024)) {
    $response .= $chunk;
  }
  fclose($fp);
  
  // 解析响应
  list($split, $result->data) = explode("\r\n\r\n", $response, 2);
  $split = preg_split("/\r\n|\n|\r/", $split);
  
  list($protocol, $code, $text) = explode(' ', trim(array_shift($split)), 3);
  $result->headers = array();
  
  // 响应头
  while ($line = trim(array_shift($split))) {
    list($header, $value) = explode(':', $line, 2);
    if(isset($result->headers[$header]) && $header == 'Set-Cookie') {
      // Cookie: 以逗号分隔取得的 Cookie
      $result->headers[$header] .= ','. trim($value);
    }else{
      $result->headers[$header] = trim($value);
    }
  }
  
  $responses = array(
    100 => 'Continue', 101 => 'Switching Protocols',
    200 => 'OK', 201 => 'Created', 202 => 'Accepted', 203 => 'Non-Authoritative Information', 204 => 'No Content', 205 => 'Reset Content', 206 => 'Partial Content',
    300 => 'Multiple Choices', 301 => 'Moved Permanently', 302 => 'Found', 303 => 'See Other', 304 => 'Not Modified', 305 => 'Use Proxy', 307 => 'Temporary Redirect',
    400 => 'Bad Request', 401 => 'Unauthorized', 402 => 'Payment Required', 403 => 'Forbidden', 404 => 'Not Found', 405 => 'Method Not Allowed', 406 => 'Not Acceptable', 407 => 'Proxy Authentication Required', 408 => 'Request Time-out', 409 => 'Conflict', 410 => 'Gone', 411 => 'Length Required', 412 => 'Precondition Failed', 413 => 'Request Entity Too Large', 414 => 'Request-URI Too Large', 415 => 'Unsupported Media Type', 416 => 'Requested range not satisfiable', 417 => 'Expectation Failed',
    500 => 'Internal Server Error', 501 => 'Not Implemented', 502 => 'Bad Gateway', 503 => 'Service Unavailable', 504 => 'Gateway Time-out', 505 => 'HTTP Version not supported'
  );
  
  if (!isset($responses[$code])) {
    $code = floor($code / 100) * 100;
  }
  
  switch ($code) {
    case 200: // 成功
    case 304: // 缓存页面
      break;
    case 301: // 永久重定向
    case 302: // 临时重定向
    case 307: // 临时重定向
      $location = $result->headers['Location'];
      if ($retry) {
        $result = dd_http_request($result->headers['Location'], $headers, $method, $data, --$retry);
        $result->redirect_code = $result->code;
      }
      $result->redirect_url = $location;
      break;
    default:
      $result->error = $text;
  }
  $result->code = $code;
  return $result;
}

// 文本错误信息
function dd_error_msg($msg){
  global $user;
  $error = array(
    '错误信息：'.$msg,
    '发生时间：'. format_date(time()),
    '当前用户：'.$GLOBALS['user']->mail,
    '用户主机：'. ip_address(),
    '当前页面：'.$_SERVER['REQUEST_URI']
  );
  return implode("\n", $error);
}

/**
 * 写入错误日志，保存在配置目录
 * @param (string) $filename
 *  文件名。最终文件名将附加年月
 * @param (string) $msg
 *  错误信息
 */
function text_log($filename, $msg){
  $filepath = './sites/logs/'.$_SERVER['HTTP_HOST'].'_'. $filename . '_'.date("Y") .'_'.date("m") . '.txt';
  $handle = fopen($filepath, 'ab');
  fwrite($handle, $msg. "\n<!-- 分隔符 -->\n");
  fclose($handle);
}

function dd_get_strong($text, $sufpix = '：'){
  return '<strong>'.$text.$sufpix.'</strong>';
}

/**
 * 邮件发送，默认使用 mail() 发送
 * @param (array) $mails
 *  数组，包含发件人等(键名区分大小写)：
 *    To：(array) 一组收件人的姓名和邮址
 *    例1：$mails = 'user@example.com'; // 仅一个邮址，若需提供姓名或一组：
 *    例2：$mails = array(array('user@c.com', 'name'), 'user1@example.com', );
 * @param (string) $subject
 *  邮件主题，换行符将被过滤
 * @param (string) $message
 *  邮件正文，每行不超过70
 * @param (string) $subject
 *  邮件主题，纯文本将在每70个字符后插入换行符，html 无此限制
 * @param (bool) $html
 *  发送为 html 内容
 * @param (array) $headers
 *  可提供更多邮件头信息，如抄送等
 * @param (array) $att
 *  更多配置，如可发送附件等。mail() 发送不支持
 * @return
 *  成功返回 true，失败返回 false，并发出一条警告
 */
//function dd_set_mail($to = array(), $subject, $message, $att = NULL, $filepath = NULL){
function dd_set_mail($mails, $subject, $message, $headers = NULL, $html = 1, $att = NULL){
  if(is_array($mails)){
    foreach($mails as $add){
      if(is_array($add)){
        // 邮址+姓名：array('user@example.com', 'name')
        if($add[1]){
          $tos[$add[0]] = $add[1].' <'.$add[0].'>';
        }else{
          $tos[$add[0]] = $add[0];
        }
      }else{
        $tos[$add] = $add;
      }
    }
    $to = implode(', ', $tos);
  }else{
    $to = $mails;
  }
  
  $data_header = array();
  if($headers){
    foreach($headers as $key => $value){
      $data_header[$key] = $key.':'.$value;
    }
    $header = implode("\r\n", $data_header);
  }
  
  if(!$html){
    $message = wordwrap($message, 70, "\n", true);
  }else{
    $header .= 'MIME-Version: 1.0' . "\r\n";
    $header .= 'Content-type: text/html; charset=utf-8' . "\r\n";
  }
  if(!$headers['Form']){
    $header .= 'Form: '.$GLOBALS['site_global']['name'].' ('.var_get_key('site_global', 'mail', 'admin@'.$_SERVER['HTTP_HOST']).")\r\n";
  }
  
  if(mail($to, $subject, $message, $header)){
    return true;
  }else{
    dd_set_message(t('system', '邮件发送失败'), 'error');
    return false;
  }
  /*
  date_default_timezone_set('America/Toronto');
  require_once('./tools/class.phpmailer.php');
  $mail = new PHPMailer();
  if($message){
    $body = eregi_replace("[\]", '', $message);
  }else{
    if($filepath && is_file($filepath)){
      $body = file_get_contents($filepath);
      $body = eregi_replace("[\]", '', $body);
    }
  }
  $mail->IsSMTP();
  $mail->Host       = 'ssl://smtp.gmail.com';
  $mail->SMTPDebug  = 0;
  $mail->SMTPAuth   = true;
  $mail->Port       = 465; 
  $mail->Username   = "yd2004@gmail.com";
  $mail->Password   = "......";
  $mail->SetFrom('yd2004@gmail.com', $GLOBALS['conf']['site_name']);
  
  $mail->Subject = $subject;
  
  $mail->AltBody = '你的邮箱不支持 HTML 格式。'; 
  
  $mail->MsgHTML($body);
  
  $name = $to[1] ? $to[1] : end(explode('@', $to[0]));
  $mail->AddAddress($to[0], $name);
  
  if($att){
    foreach($att as $file){
      $mail->AddAttachment($file);
    }
  }

  if(!$mail->Send()) {
    text_log('mail', '邮件发送错误: ' . $mail->ErrorInfo);
  } else {
    return true;
  }
  
  */
}

/**
 * 纯真QQ IP。
 */
function qqwry_getip($ip, $true = false) {
  static $ips;
  if(!isset($ips[$ip])){
	  $return = '';
    if($ip){
    	if(preg_match("/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/", $ip)) {
    		$iparray = explode('.', $ip);
    		if($iparray[0] == 10 || $iparray[0] == 127 || ($iparray[0] == 192 && $iparray[1] == 168) || ($iparray[0] == 172 && ($iparray[1] >= 16 && $iparray[1] <= 31))) {
    			$t = '本机地址'; 
    		} elseif($iparray[0] > 255 || $iparray[1] > 255 || $iparray[2] > 255 || $iparray[3] > 255){
    			$t = 'IP不存在';
    		} else {
    			$t = qqwry_data($ip);
    		}
    	}else{
    		$t = 'IP不合法';
  	  }
    }else{
      $t = '没有IP';
    }
    if($true){
      $ips[$ip] = $ip . '<br />(' . $t .')';
    }else{
  	  $ips[$ip] = $t;
    }
  }
  return $ips[$ip];
}

function qqwry_data($ip, $file = './tools/QQWry.Dat') {

	if(!$fd = @fopen($file, 'rb')) {
		return false;
	}

	$ip = explode('.', $ip);
	$ipNum = $ip[0] * 16777216 + $ip[1] * 65536 + $ip[2] * 256 + $ip[3];

	if(!($DataBegin = fread($fd, 4)) || !($DataEnd = fread($fd, 4)) ) return;
	@$ipbegin = implode('', unpack('L', $DataBegin));
	if($ipbegin < 0) $ipbegin += pow(2, 32);
	@$ipend = implode('', unpack('L', $DataEnd));
	if($ipend < 0) $ipend += pow(2, 32);
	$ipAllNum = ($ipend - $ipbegin) / 7 + 1;

	$BeginNum = $ip2num = $ip1num = 0;
	$ipAddr1 = $ipAddr2 = '';
	$EndNum = $ipAllNum;

	while($ip1num > $ipNum || $ip2num < $ipNum) {
		$Middle= intval(($EndNum + $BeginNum) / 2);

		fseek($fd, $ipbegin + 7 * $Middle);
		$ipData1 = fread($fd, 4);
		if(strlen($ipData1) < 4) {
			fclose($fd);
			return false;
		}
		$ip1num = implode('', unpack('L', $ipData1));
		if($ip1num < 0) $ip1num += pow(2, 32);

		if($ip1num > $ipNum) {
			$EndNum = $Middle;
			continue;
		}

		$DataSeek = fread($fd, 3);
		if(strlen($DataSeek) < 3) {
			fclose($fd);
			return false;
		}
		$DataSeek = implode('', unpack('L', $DataSeek.chr(0)));
		fseek($fd, $DataSeek);
		$ipData2 = fread($fd, 4);
		if(strlen($ipData2) < 4) {
			fclose($fd);
			return false;
		}
		$ip2num = implode('', unpack('L', $ipData2));
		if($ip2num < 0) $ip2num += pow(2, 32);

		if($ip2num < $ipNum) {
			if($Middle == $BeginNum) {
				fclose($fd);
				return '未知';
			}
			$BeginNum = $Middle;
		}
	}

	$ipFlag = fread($fd, 1);
	if($ipFlag == chr(1)) {
		$ipSeek = fread($fd, 3);
		if(strlen($ipSeek) < 3) {
			fclose($fd);
			return false;
		}
		$ipSeek = implode('', unpack('L', $ipSeek.chr(0)));
		fseek($fd, $ipSeek);
		$ipFlag = fread($fd, 1);
	}

	if($ipFlag == chr(2)) {
		$AddrSeek = fread($fd, 3);
		if(strlen($AddrSeek) < 3) {
			fclose($fd);
			return false;
		}
		$ipFlag = fread($fd, 1);
		if($ipFlag == chr(2)) {
			$AddrSeek2 = fread($fd, 3);
			if(strlen($AddrSeek2) < 3) {
				fclose($fd);
				return false;
			}
			$AddrSeek2 = implode('', unpack('L', $AddrSeek2.chr(0)));
			fseek($fd, $AddrSeek2);
		} else {
			fseek($fd, -1, SEEK_CUR);
		}

		while(($char = fread($fd, 1)) != chr(0))
		$ipAddr2 .= $char;

		$AddrSeek = implode('', unpack('L', $AddrSeek.chr(0)));
		fseek($fd, $AddrSeek);

		while(($char = fread($fd, 1)) != chr(0))
		$ipAddr1 .= $char;
	} else {
		fseek($fd, -1, SEEK_CUR);
		while(($char = fread($fd, 1)) != chr(0))
		$ipAddr1 .= $char;

		$ipFlag = fread($fd, 1);
		if($ipFlag == chr(2)) {
			$AddrSeek2 = fread($fd, 3);
			if(strlen($AddrSeek2) < 3) {
				fclose($fd);
				return false;
			}
			$AddrSeek2 = implode('', unpack('L', $AddrSeek2.chr(0)));
			fseek($fd, $AddrSeek2);
		} else {
			fseek($fd, -1, SEEK_CUR);
		}
		while(($char = fread($fd, 1)) != chr(0))
		$ipAddr2 .= $char;
	}
	fclose($fd);

	if(preg_match('/http/i', $ipAddr2)) {
		$ipAddr2 = '';
	}
	$ipaddr = "$ipAddr1 $ipAddr2";
	$ipaddr = preg_replace('/CZ88\.NET/is', '', $ipaddr);
	$ipaddr = preg_replace('/^\s*/is', '', $ipaddr);
	$ipaddr = preg_replace('/\s*$/is', '', $ipaddr);
	if(preg_match('/http/i', $ipaddr) || $ipaddr == '') {
		$ipaddr = '未知';
	}
  return iconv("gb2312", "UTF-8", $ipaddr);
}

/**
 * 载入ui，
 * $theme：(sring) 载入的主题
 */

function dd_jqui($theme = 'base'){
  dd_add_js('misc/jquery.bgiframe.js');
  dd_add_js('misc/ui/jquery.ui.js');
  dd_add_css('misc/ui/themes/base/jquery.ui.css');
}

/**
 * 载入jqedit
 */
function dd_jqedit($v = array()){
  $args = array(
    'indicator' => '<img src="/misc/loading.gif">',
    'submit' => '确定',
    'cancel' => '取消',
    'tooltip' => '点此编辑',
  );
  
  $v['opt'] = array_merge($args, $v['opt']);
  
  if($v['limit']) $v['opt']['charcounter']['characters'] = $v['limit'];
  dd_add_js(array('edit' => $v), 'setting');
  dd_add_js('misc/jquery.jeditable.mini.js');
  switch($v['opt']['type']){
    case 'charcounter':
      dd_add_js('misc/jquery.jeditable.charcounter.js');
      dd_add_js('misc/jquery.charcounter.js');
    break;
    case 'autogrow':
      dd_add_js('misc/jquery.jeditable.autogrow.js');
      dd_add_js('misc/jquery.autogrow.js');
    break;
    case 'masked':
      dd_add_js('misc/jquery.jeditable.masked.js');
      dd_add_js('misc/jquery.maskedinput.js');
    break;
    case 'datepicker':
      dd_add_js('misc/jquery.jeditable.datepicker.js');
      dd_add_js('misc/jquery.datepicker.js');
      dd_add_js('misc/data.js');
    break;
    case 'timepicker':
      dd_add_js('misc/jquery.jeditable.timepicker.js');
      dd_add_js('misc/jquery.timepicker.js');
    break;
    case 'ajaxupload':
      dd_add_js('misc/jquery.jeditable.ajaxupload.js');
      dd_add_js('misc/jquery.ajaxfileupload.js');
    break;
    case 'time':
      dd_add_js('misc/jquery.jeditable.time.js');
      dd_add_js('misc/jquery.dimensions.js');
  }
}

/**
 * 弹出窗口
 */
function dd_dialog_att($height = 400, $width = 600, $iframe = 'true', $inlineId = NULL, $reload = NULL){
  $str = "width=$width&height=$height";
  if($iframe){
    $str .= '&iframe=true&modal=true';
  }
  if($inlineId){
    $str .= '&inlineId='.$inlineId;
  }
  if($reload){
    $str .= '&reload=true';
  }
  dd_jqui();
  return array('query' => $str, 'attributes' => array('class' => 'dialog', 'target' => '_blank'));
}

/**
 * flash上传
 */
function dd_swfup($value){
  dd_add_js('misc/swfupload/swfupload.js');
  dd_add_js('misc/swfupload/swfupload.queue.js');
  dd_add_js('misc/swfupload/fileprogress.js');
  dd_add_js('misc/swfupload/handlers.js');
  dd_add_css('misc/upload/uploadify.css');
  
  if(!$value['flash_id']) $value['flash_id'] = 'spanButtonPlaceHolder';
  if(!$value['target']) $value['target'] = 'fsUploadProgress'; 
  if(!$value['cancel']) $value['cancel'] = 'btnCancel'; 
  
  dd_add_js(theme('form_swfu', $value), 'inline');
  $t = '<div class="form_swfup"><div class="form_swfup_button">';
  $t .= '<div><span id="'.$value['flash_id'].'"></span></div>';
  $t .= '<input id="startUpload" class="form_swfu_button" type="button" onclick="swfu.startUpload()" value="开始上传" disabled="disabled" />';
  $t .= '<input id="'.$value['cancel'].'" type="button" value="取消上传" onclick="swfu.cancelQueue();" disabled="disabled" class="form_swfu_button" /></div>';
  $t .= '<span id="spanButtonPlaceHolder"></span>';
  $t .= '<div id="divStatus"></div><div class="fieldset flash" id="'.$value['target'].'"></div></div>';
  
  return $t;
}

/**
 * 写入提示信息
 */
function dd_set_message($message = NULL, $type = 'status', $repeat = TRUE) {
  if($message) {
    if($type == 'sql_bug' && !$GLOBALS['conf']['debug']){
      text_log('sql_bug', $message);
      if(!$GLOBALS['conf']['debug']){
        return false;
      }
      $type = 'debug';
    }
    if (!isset($_SESSION['messages'])) {
      $_SESSION['messages'] = array();
    }
    if (!isset($_SESSION['messages'][$type])) {
      $_SESSION['messages'][$type] = array();
    }
    if ($repeat || !in_array($message, $_SESSION['messages'][$type])) {
      $_SESSION['messages'][$type][] = $message;
    }
  }
}

/**
 * 获取提示信息
 */
function dd_get_message() {
  if($_SESSION['messages']){
    foreach ($_SESSION['messages'] as $type => $messages) {
      $output .= "<div class=\"messages $type\">\n";
      if (count($messages) > 1) {
        $output .= " <ul>\n";
        foreach ($messages as $message) {
          $output .= '  <li>'. $message ."</li>\n";
        }
        $output .= " </ul>\n";
      }else {
        $output .= $messages[0];
      }
      $output .= "</div>\n";
    }
    unset($_SESSION['messages']);
    return $output;
  }
}

function dd_array_to_string($array){
  if(is_array($array)){
    foreach($array as $key => $value){
      if(is_array($value)){
        dd_array_to_string($value);
      }else{
        $args .= '['.$key.'] => ' . $value;
      }
    }
    return $args;
  }else{
    return $array;
  }
}

function dd_get_help() {
  if($help = module_invoke_all('help', $_GET['q'])){
    return '<div class="help">'.implode('', $help).'</div>';
  }
}

function dd_set_footer($item = NULL){
  static $items;
  if($item){
    $items[] = $item;
  }else{
    return $items;
  }
}

function dd_get_footer(){
  if($items = dd_set_footer()){
    return theme('item_list', $items, NULL, 'ul', array('id' => 'module_invoke_footer'));
  }
}

function dd_set_tabs($tab = NULL) {
  static $tabs;
  if($tab){
    if(is_array($tab)){
      foreach($tab as $t){
        $tabs[] = $t;
      }
    }else{
      $tabs[] = $tab;
    }
  }else{
    return $tabs;
  }
}

function dd_get_sub_tabs(){
  if($tabs = dd_set_sub_tabs()){
    return theme('item_list', $tabs, NULL, 'ul', array('class' => 'sub_tabs'));
  }
}

function dd_set_sub_tabs($tab = NULL) {
  static $tabs;
  if($tab){
    if(is_array($tab)){
      foreach($tab as $t){
        $tabs[] = $t;
      }
    }else{
      $tabs[] = $tab;
    }
  }else{
    return $tabs;
  }
}

function dd_get_tabs(){
  if($tabs = dd_set_tabs()){
    return theme('item_list', $tabs, NULL, 'ul', array('class' => 'tabs'));
  }
}

function dd_set_title($title = NULL) {
  static $stored_title;
  if(isset($title)){
    $stored_title = $title;
  }else{
    return $stored_title;
  }
}

function dd_set_block($value = NULL, $name = 'left') {
  static $block;
  if ($value) {
    $block[$name] = $value;
  }else{
    return $block;
  }
}

function dd_set_breadcrumb($breadcrumb = NULL, $front = 1){
  static $value;
  if(is_array($breadcrumb)){
    if($front) array_unshift($breadcrumb, l(t('system', '首页'), NULL));
    $value = $breadcrumb;
  }else{
    return $value;
  }
}

function dd_get_menu($menus = NULL){
  if(!isset($menus)){
    $menus = array();
    module_alter_all('site_menu', $menus);
    $calss = 'site_menu';
  }else{
    $calss = 'site_menu_children';
  }
  
  if($menus){
    static $i;
    if($i == 0){
      dd_add_js('misc/jquery.menu.js');
      dd_add_js('$(\'.site_menu\').menuLevel();', 'inline');
    }
    foreach($menus as $key => $lists){
      if(is_array($lists)){
        if($lists['#data']){
          $data = '';
          $data .= $lists['#data'];
          if($lists['#childrens']){
            $data .= dd_get_menu($lists['#childrens']);
          }
          $items[] = array(
            '#data' => array('data' => $data, 'class' => 'site_menu_list site_menu_list_'.$key),
            '#weight' => ($lists['#weight'] ? $lists['#weight'] : $weight)
          );
        }else if(!$lists['#childrens']){
          foreach($lists as $k => $child){
            if(is_array($child)){
              if($child['#data']){
                $data = '';
                $data .= $child['#data'];
                if($child['#childrens']){
                  $data .= dd_get_menu($child['#childrens']);
                }
                $items[] = array(
                  '#data' => array('data' => $data, 'class' => 'site_menu_list site_menu_list_'.$k),
                  '#weight' => ($child['#weight'] ? $child['#weight'] : $weight)
                );
              }
            }else{
              $items[] = array(
                '#data' => array('data' => $child, 'class' => 'site_menu_list site_menu_list_'.$k),
                '#weight' => $i
              );
            }
            ++$i;
          }
        }
      }else{
        $items[] = array(
          '#data' => array('data' => $lists, 'class' => 'site_menu_list site_menu_list_'.$key),
          '#weight' => $i,
        );
      }
      ++$i;
    }
    if($items){
      uasort($items, 'dd_form_cmp');
      foreach($items as $d){
        $item[] = $d['#data'];
      }
      $output = theme('item_list', $item, NULL, 'ul', array('class' => $calss), 0);
    }
    return $output;
  }
}

function dd_get_title() {
  if($title = dd_set_title()){
    $head_title = implode(' - ', array_filter($title)) .' - ';
  }
  $head_title .= $GLOBALS['conf']['site_global']['name'];
  if($GLOBALS['conf']['site_global']['slogan']){
    $head_title .= ' - ' . $GLOBALS['conf']['site_global']['slogan'];
  }
  return $head_title;
}

function dd_get_breadcrumb(){
  if($breadcrumb = dd_set_breadcrumb()){
    return '<div class="breadcrumb">' . implode(' › ', array_filter($breadcrumb)). '</div>';
  }
}

function dd_set_header($header = NULL) {
  static $stored_headers = array();
  if (strlen($header)) {
    header($header);
    $stored_headers[] = $header;
  }
  return implode("\n", $stored_headers);
}
function dd_set_html_head($data = NULL) {
  static $stored_head = '';
  if (!is_null($data)) {
    $stored_head .= $data ."\n";
  }
  return $stored_head;
}

function dd_get_html_head() {
  return dd_set_html_head();
}

function dd_add_css($path = NULL, $type = 'module', $media = 'all') {
  static $css = array();
  if (isset($path)) {
    if (!isset($css[$media])) {
      $css[$media] = array('core' => array(), 'module' => array(), 'theme' => array());
    }
    $css[$media][$type][$path] = $path;
  }
  return $css;
}

function dd_get_css($css = NULL) {
  global $base_path;
  if (!isset($css)) {
    $css = dd_add_css();
  }
  
  foreach ($css as $media => $types) {
    foreach ($types as $type => $files) {
      if(count($files)){
        foreach ($types[$type] as $file) {
          if($GLOBALS['conf']['cache_css_and_js']){
            $css_path[$type][] = $file;
            $filename .= $file;
          }else{
            $gets[$type] .= '<link type="text/css" rel="stylesheet" media="'. $media .'" href="'. $base_path . $file.'?'.$GLOBALS['conf']['cache_css_and_js_timestamp'].'" />'."\n";
         }
        }
      }
    }
  }
  
  if($css_path && $filename){
    $filepath = file_directory_path() . '/cache/css_js/' . md5($filename) . '.css';
    if(!is_file($filepath)){
      require_once './includes/css_js_min.inc';
      foreach(array('core', 'module', 'theme') as $key){
        if($css_path[$key]){
          foreach($css_path[$key] as $path){
            $data = file_get_contents($path);
            $base = $base_path . dirname($path) .'/';
            _dd_build_css_path(NULL, $base);
            $contents .= preg_replace_callback('/url\([\'"]?(?![a-z]+:|\/+)([^\'")]+)[\'"]?\)/i', '_dd_build_css_path', $data);
          }
        }
      }
      _file_save_data(cssmin::minify($contents), $filepath);
    }
    return '<link type="text/css" rel="stylesheet" media="all" href="'. $base_path . $filepath.'?'.$GLOBALS['conf']['cache_css_and_js_timestamp'].'" />'."\n";
  }
  return $gets['core'] . $gets['module'] . $gets['theme'];
}

//css背景图片路径替换
function _dd_build_css_path($matches, $base = NULL) {
  static $_base;
  if (isset($base)) {
    $_base = $base;
  }
  $path = $_base . $matches[1];
  
  /*
  $last = '';
  while ($path != $last) {
    $last = $path;
    $path = preg_replace('`(^|/)(?!../)([^/]+)/../`', '$1', $path);
  }
  */
  return 'url('. $path .')';
}

function dd_js_print($js){
  print "$(function(){\n". $js . "\n});";
  exit;
}

function _dd_js_print($js){
  return '<script type="text/javascript">'.$js.'</script>';
}

function dd_js_goto($limit){
  if($q = dd_query_string_encode($_REQUEST, array_merge(array('q', 'js_limit'), array_keys($_COOKIE)))){
    $q = '&' . $q;
  }
  
  if($_GET['js_limit']) $limit += $_GET['js_limit'];
  
  $path = url($_GET['q'], array('query' => 'js_limit='.$limit.$q));
  echo _dd_js_print('location.href = "'.$path.'";');
  exit;
}

function dd_imp($v = array(), $str = ' | '){
  if($v) return '<span class="dd_imp">'.implode($str, $v).'</span>';
}

function dd_add_js($data = NULL, $type = 'module', $cache = true, $scope = 'header', $defer = false) {
  static $javascript = array();
  if (isset($data)) {
    if (isset($scope) && !isset($javascript[$scope])) {
      $javascript[$scope] = array('core' => array(), 'module' => array(), 'theme' => array(), 'inline' => array());
    }
    if (isset($type) && isset($scope) && !isset($javascript[$scope][$type])) {
      $javascript[$scope][$type] = array();
    }
    switch ($type) {
      case 'inline': case 'inline_nojq':
        $javascript[$scope][$type][] = array('code' => $data, 'defer' => $defer);
      break;
      case 'setting':
        $javascript[$scope][$type][] = $data;
      break;
      default:
        $javascript[$scope][$type][$data] = array('defer' => $defer, 'cache' => $cache);
    }
  }
  if (isset($scope)) {
    if (isset($javascript[$scope])) {
      return $javascript[$scope];
    }else {
      return array();
    }
  }else {
    return $javascript;
  }
}

function dd_get_js($scope = 'header', $javascript = NULL) {
  global $conf, $base_path;
  static $init;
  if(!$init) {
    $js = array(
      'cache_css_and_js_timestamp' => $conf['cache_css_and_js_timestamp'],
      'base_theme' => $conf['base_theme'],
      'base_path' => $base_path,
    );
    dd_add_js($js, 'setting');
  }
  
  if(!isset($javascript)){
    $javascript = dd_add_js(NULL, NULL, NULL, $scope);
  }
  
  $init = 1;
  
  if (empty($javascript)) return '';
  foreach ($javascript as $type => $data) {
    if (!$data) continue;
    switch ($type) {
      case 'setting':
        $settings = 'var settings = ' . dd_to_js(call_user_func_array('array_merge_recursive', $data)) . ';';
      break;
      case 'inline':
      	$code_js = '';
        foreach ($data as $info) {
          $code_js .= $info['code'];
        }
        $output['inline'] .= '$(function(){'. $code_js . '});';
      break;
      case 'inline_nojq':
      	$code_js = '';
        foreach ($data as $info) {
          $code_js .= $info['code'].';';
        }
        $output['inline_nojq'] .= $code_js;
      break;
      default:
        foreach ($data as $path => $info) {
          if($path){
            if($GLOBALS['conf']['cache_css_and_js']){
              if($info['cache']){
                $js_path[] = $path;
                $filename .= $path;
              }else{
                if(strpos($path, 'http://') === false && strpos($path, 'https://') === false){
                  $path = $base_path .$path .'?'.$GLOBALS['conf']['cache_css_and_js_timestamp'];
                }
                $preprocess .= '<script type="text/javascript"'. ($info['defer'] ? ' defer="defer"' : '') .' src="'. $path."\"></script>\n";
              }
            }else{
              if(strpos($path, 'http://') === false && strpos($path, 'https://') === false){
                $path = $base_path .$path .'?'.$GLOBALS['conf']['cache_css_and_js_timestamp'];
              }
              $preprocess .= '<script type="text/javascript"'. ($info['defer'] ? ' defer="defer"' : '') .' src="'. $path."\"></script>\n";
            }
          }
        }
    }
  }
  if($js_path && $filename){
    $filepath = file_mkdir(file_directory_path() . '/cache/css_js') . '/' . md5($filename) . '.js';
    if(!is_file($filepath)){
      require_once './includes/css_js_min.inc';
      foreach($js_path as $path){
        $contents .= file_get_contents($path).';';
      }
      _file_save_data(JSMin::minify($contents), $filepath);
    }
    $preprocess .= '<script type="text/javascript" src="'. $base_path .$filepath .'?'.$conf['cache_css_and_js_timestamp'].'"></script>'."\n";
  }
  if($output){
    $output = "<script type=\"text/javascript\">\n<!--//--><![CDATA[//><!--\n" . $output['inline'] . $output['inline_nojq'] . "\n//--><!]]>\n</script>";
  }
  if($settings){
    $settings = "<script type=\"text/javascript\">\n<!--//--><![CDATA[//><!--\n" . $settings . "\n//--><!]]>\n</script>\n";
  }
  return $settings . $preprocess . $output;
}

/*
  view end
*/
function dd_to_js($var) {
  switch (gettype($var)) {
    case 'boolean':
      return $var ? 'true' : 'false';
    case 'integer':
    case 'double':
      return $var;
    case 'resource':
    case 'string':
      $str = '"'. str_replace(array("\r", "\n", "<", ">", "&"),
                              array('\r', '\n', '\x3c', '\x3e', '\x26'),
                              addslashes($var)) .'"';
    	if(strpos($var, ':::code:::') === false){
      	return '"'. str_replace(array("\r", "\n", "<", ">", "&"),
                              array('\r', '\n', '\x3c', '\x3e', '\x26'),
                              addslashes($var)) .'"';
    	}else{
      	return str_replace(array("\r", "\n", "<", ">", "&", ':::code:::'),
                              array('\r', '\n', '\x3c', '\x3e', '\x26', ''),
                              addslashes($var));
    	}
    case 'array':
      if (empty ($var) || array_keys($var) === range(0, sizeof($var) - 1)) {
        $output = array();
        foreach ($var as $v) {
          $output[] = dd_to_js($v);
        }
        return '[ '. implode(', ', $output) .' ]';
      }
    case 'object':
      $output = array();
      foreach ($var as $k => $v) {
        $output[] = dd_to_js(strval($k)) .': '. dd_to_js($v);
      }
      return '{ '. implode(', ', $output) .' }';
    default:
      return 'null';
  }
}

/*
  字符串处理
*/
function dd_substr($text, $start, $length = NULL) {
  if(function_exists('mb_strlen')) {
    return $length === NULL ? mb_substr($text, $start, NULL, 'utf-8') : mb_substr($text, $start, $length, 'utf-8');
  }else{
    $strlen = strlen($text);
    $bytes = 0;
    if ($start > 0) {
      $bytes = -1; $chars = -1;
      while ($bytes < $strlen && $chars < $start) {
        $bytes++;
        $c = ord($text[$bytes]);
        if ($c < 0x80 || $c >= 0xC0) {
          $chars++;
        }
      }
    } else if ($start < 0) {
      $start = abs($start);
      $bytes = $strlen; $chars = 0;
      while ($bytes > 0 && $chars < $start) {
        $bytes--;
        $c = ord($text[$bytes]);
        if ($c < 0x80 || $c >= 0xC0) {
          $chars++;
        }
      }
    }
    $istart = $bytes;
    if ($length === NULL) {
      $bytes = $strlen - 1;
    }else if ($length > 0) {
      $bytes = $istart; $chars = 0;
      while ($bytes < $strlen && $chars < $length) {
        $bytes++;
        $c = ord($text[$bytes]);
        if ($c < 0x80 || $c >= 0xC0) {
          $chars++;
        }
      }
      $bytes--;
    }else if ($length < 0) {
      $length = abs($length);
      $bytes = $strlen - 1; $chars = 0;
      while ($bytes >= 0 && $chars < $length) {
        $c = ord($text[$bytes]);
        if ($c < 0x80 || $c >= 0xC0) {
          $chars++;
        }
        $bytes--;
      }
    }
    $iend = $bytes;
    return substr($text, $istart, max(0, $iend - $istart + 1));
  }
}

function dd_strlen($text) {
  if (function_exists('mb_strlen')) {
    return mb_strlen($text, 'utf-8');
  }else {
    return strlen(preg_replace("/[\x80-\xBF]/", '', $text));
  }
}

function dd_strtoupper($text) {
  if (function_exists('mb_strlen')) {
    return mb_strtoupper($text);
  }else {
    $text = strtoupper($text);
    $text = preg_replace_callback('/\xC3[\xA0-\xB6\xB8-\xBE]/', '_unicode_caseflip', $text);
    return $text;
  }
}

function dd_strtolower($text) {
  if (function_exists('mb_strlen')) {
    return mb_strtolower($text);
  }else {
    $text = strtolower($text);
    $text = preg_replace_callback('/\xC3[\x80-\x96\x98-\x9E]/', '_unicode_caseflip', $text);
    return $text;
  }
}

/**
 * 将字符串转换为数组，以换行符分割
 * @param (string) $string
 *  待处理的字符串
 * @param (bool) $repeat
 *  若为 true，则去除数组中空白和重复值
 */
function dd_line_to_array($string, $repeat = NULL){
  if(!$string) return false;
  if(strpos($string, "\r\n") !== false){
    $string = str_replace("\r\n", "\n", $string);
  }
  if(strpos($string, "\n") !== false){
    $string = trim($string);
    if($arr = explode("\n", $string)){
      if($repeat){
        $arr = array_filter($arr);
        $arr = array_unique($arr);
      }
      return $arr;
    }
  }else{
    return array(trim($string));
  }
  return false;
}

function dd_query_string_encode($query, $exclude = array(), $parent = '') {
  $params = array();
  foreach ($query as $key => $value) {
    $key = dd_urlencode($key);
    if ($parent) {
      $key = $parent .'['. $key .']';
    }
    if (in_array($key, $exclude)) {
      continue;
    }
    if (is_array($value)) {
      $params[] = dd_query_string_encode($value, $exclude, $key);
    }
    else {
      $params[] = $key .'='. dd_urlencode($value);
    }
  }
  return implode('&', $params);
}

function decode_entities($text, $exclude = array()) {
  static $table;
  if (!isset($table)) {
    $table = array_flip(get_html_translation_table(HTML_ENTITIES));
    $table = array_map('utf8_encode', $table);
    $table['&apos;'] = "'";
  }
  $newtable = array_diff($table, $exclude);
  return preg_replace('/&(#x?)?([A-Za-z0-9]+);/e', '_decode_entities("$1", "$2", "$0", $newtable, $exclude)', $text);
}

function _decode_entities($prefix, $codepoint, $original, &$table, &$exclude) {
  if (!$prefix) {
    if (isset($table[$original])) {
      return $table[$original];
    }else {
      return $original;
    }
  }
  if ($prefix == '#x') {
    $codepoint = base_convert($codepoint, 16, 10);
  }else {
    $codepoint = preg_replace('/^0+/', '', $codepoint);
  }
  if ($codepoint < 0x80) {
    $str = chr($codepoint);
  }else if ($codepoint < 0x800) {
    $str = chr(0xC0 | ($codepoint >> 6))
         . chr(0x80 | ($codepoint & 0x3F));
  }else if ($codepoint < 0x10000) {
    $str = chr(0xE0 | ( $codepoint >> 12))
         . chr(0x80 | (($codepoint >> 6) & 0x3F))
         . chr(0x80 | ( $codepoint       & 0x3F));
  }else if ($codepoint < 0x200000) {
    $str = chr(0xF0 | ( $codepoint >> 18))
         . chr(0x80 | (($codepoint >> 12) & 0x3F))
         . chr(0x80 | (($codepoint >> 6)  & 0x3F))
         . chr(0x80 | ( $codepoint        & 0x3F));
  }
  if (in_array($str, $exclude)) {
    return $original;
  }
  else {
    return $str;
  }
}

/**
 * 字符串检查，去除html标签
 */
function check_plain($text) {
  static $php525;

  if(!isset($php525)) {
    $php525 = version_compare(PHP_VERSION, '5.2.5', '>=');
  }
  if($php525) {
    return htmlspecialchars($text, ENT_QUOTES, 'UTF-8');
  }
  return (preg_match('/^./us', $text) == 1) ? htmlspecialchars($text, ENT_QUOTES, 'UTF-8') : '';
}

// 是否utf8编码
function dd_validate_utf8($text) {
  if (strlen($text) == 0) {
    return TRUE;
  }
  return (preg_match('/^./us', $text) == 1);
}

// url编码
function dd_urlencode($text) {
  global $conf;
  if ($conf['clean_url']) {
    return str_replace(array('%2F', '%26', '%23', '//'), array('/', '%2526', '%2523', '/%252F'), rawurlencode($text));
  }else {
    return str_replace('%2F', '/', rawurlencode($text));
  }
}

// 检查url
function check_url($uri) {
  return filter_xss_bad_protocol($uri, FALSE);
}

function filter_xss_bad_protocol($string, $decode = TRUE) {
  static $allowed_protocols;
  if (!isset($allowed_protocols)) {
    $allowed_protocols = array_flip(var_get('filter_allowed_protocols', array('http', 'https', 'ftp', 'news', 'nntp', 'telnet', 'mailto', 'irc', 'ssh', 'sftp', 'webcal', 'rtsp')));
  }

  if ($decode) {
    $string = decode_entities($string);
  }do {
    $before = $string;
    $colonpos = strpos($string, ':');
    if ($colonpos > 0) {
      $protocol = substr($string, 0, $colonpos);
      if (preg_match('![/?#]!', $protocol)) {
        break;
      }
      if (!isset($allowed_protocols[strtolower($protocol)])) {
        $string = substr($string, $colonpos + 1);
      }
    }
  } while ($before != $string);
  return check_plain($string);
}

// 时间格式化
function format_interval($timestamp, $granularity = 5) {
  $units = array(31536000 => '年', 604800 => '周', 86400 => '天', 3600 => '小时', 60 => '分钟', 1 => '秒');
  $output = '';
  foreach ($units as $key => $value) {
    if ($timestamp >= $key) {
      $output .= floor($timestamp / $key) . $value;
      $timestamp %= $key;
      $granularity--;
    }
    if ($granularity == 0) {
      break;
    }
  }
  return $output ? $output : '0 秒';
}

// iso 日期转时间戳
function format_date_iso($iso){
  if(strpos($iso, '-') !== false){
    $d = explode('-', $iso);
    if(count($d) == 3) return mktime(0, 0, 0, $d[1], $d[2], $d[0]);
  }
}

// 获取当前日期零点时间戳
function format_get_current(){
  static $time;
  if(!isset($time)){
    $time = format_date_iso(format_date(time(), 'custom', 'Y-m-d'));
  }
  return $time;
}

//字母数字混合模式的随机数
function dd_range_str(){
  $a = range(a, z);
  $b = range(A, Z);
  return $a[rand(0, 25)]  . rand(1, 99) . $b[rand(0, 25)]. rand(99, 999);
}

//不缓存文本header
function _dd_text_cache(){
  dd_set_header('Content-Type: text/plain; charset=utf-8');
  dd_set_header('Cache-Control: no-cache, must-revalidate');
  dd_set_header('Expires: Sat, 26 Jul 1997 05:00:00 GMT');
}

/**
 * 获取$_GET['q']
 */
function arg($index = NULL, $path = NULL) {
  static $arguments;
  if (!isset($path)) {
    $path = $_GET['q'];
  }
  if (!isset($arguments[$path])) {
    $arguments[$path] = explode('/', $path);
  }
  if (!isset($index)) {
    return $arguments[$path];
  }
  if (isset($arguments[$path][$index])) {
    return $arguments[$path][$index];
  }
}

/**
 * 递归合并数组
 */
function dd_array_multi2single($multi, &$array = array()){
  foreach($multi as $value){
    if(is_array($value)){
      dd_array_multi2single($value, $array);
    }else if($value != ''){
      $array[] = $value;
    }
  }
  return $array;
}

/**
 * 多维转一维
 */
function dd_array2_to($op = 'value', $array, &$arr = array()){
  foreach($array as $key => $val){
    if(is_array($val)){
      dd_array2_to($op, $val, $arr);
    }else{
      $arr[] = $op == 'value' ? $val : $key;
    }
  }
  return $arr;
}

/**
 * 判断当前页是否为首页
 */
function dd_is_front(){
  return !isset($_GET['q']) || $_GET['q'] == $GLOBALS['conf']['front'];
}

/**
 * 获取跳转地址
 */
function dd_redirect($url = NULL){
  return array('query' => 'redirect='. ($url ? $url : url($_GET['q'])));
}

/**
 * 跳转至 redirect 或 指定路径
 */
function dd_goto_redirect($url = NULL){
  if(!$url) $url = $_GET['q'];
  dd_goto($_GET['redirect'] ? $_GET['redirect'] : $url);
}

/**
 * utf-8 字符转 ascii 编码
 */
function dd_encode_ascii($str, $imp = '.'){
  if(!$str) return false;
  static $cache;
  if(!isset($cache[$str])){
    $len = strlen($str);
    for($i = 0; $i < $len; ++$i){
      $ascii = ord(substr($str, $i, 1));
      if($ascii > 128){
        ++$i;
        $ascii .= ord(substr($str, $i, 1));
        ++$i;
        $ascii .= ord(substr($str, $i, 1));
      }
      
      $data[] = dechex($ascii);
    }
    $cache[$str] = implode($imp, $data);
  }
  return $cache[$str];
}

/**
 * ascii 编码转 utf-8 字符
 */
function dd_decode_ascii($str, $imp = '.'){
  if(!$str) return false;
  static $cache;
  if(!isset($cache[$str])){
    $array = explode($imp, $str);
    foreach($array as $temp){
      $temp = hexdec($temp);
      $len = strlen($temp);
      for ($i = 0;$i < $len;$i += 3){
        $cache[$str] .= chr(substr($temp, $i, 3));
      }
    }
  }
  return $cache[$str];
}

function cache_system_set_file($filename, $var_name, $value, $path = 1){
  $info = '//$Id$' . "\n\n";
  
  if(!is_string($value)){
    $value = var_export($value, true);
  	if(strpos($value, 'stdClass::__set_state') !== false){
  		$value = str_replace('stdClass::__set_state', '(object)', $value);
  	}
    $value .= ';';
  }
  if($path == 1){
    $filepath = $GLOBALS['conf_dir'] . '/cache/' . $filename;
  }else{
    $filepath = $path . '/' . $filename;
  }
  if($var_name){
    $value = '$'. $var_name .' = '. $value;
  }
  return file_put_contents($filepath, "<?php \n $info" . $value . "\n\n?>");
}

function cache_array_to_object($value){
  return (object) $value;
}

function dd_get_required($required){
  if($required) return t('system', '是');
}

function cache_system_get_file($filename, $var_name){
  if(is_file($GLOBALS['conf_dir'] . '/cache/' . $filename)){
    include_once $GLOBALS['conf_dir'] . '/cache/' . $filename;
    return ${$var_name};
	}
}

/**
 * 删除缓存文件
 */
function cache_del_file($filename){
	if(strpos($filename, '/') === false){
		$filename = $GLOBALS['conf_dir'] . '/cache/' . $filename;
	}
	return @unlink($filename);
}

function temp_get($tid){
  $temp = db_query('SELECT data, type, timestamp, uid, serialized FROM {temp} WHERE tid = ?', array($tid), array('return' => 'one'));
  if (isset($temp->data)) {
    if ($temp->serialized) {
      $temp->data = unserialize($temp->data);
    }
    return $temp;
  }
  return 0;
}

function temp_set($tid, $data, $type = NULL){
  $serialized = 0;
  if (is_object($data) || is_array($data)) {
    $data = serialize($data);
    $serialized = 1;
  }
  
  if(!db_exec('UPDATE {temp} SET data = ?, timestamp = ?, type = ?, serialized = ? WHERE tid = ?', array($data, $_SERVER['REQUEST_TIME'], $type, $serialized, $tid))){
    return db_exec('INSERT INTO {temp} (tid, data, type, serialized, uid, timestamp) VALUES (?, ?, ?, ?, ?, ?)', array($tid, $data, $type, $serialized, $GLOBALS['user']->uid, $_SERVER['REQUEST_TIME']));
  }else{
    return true;
  }
}

function temp_del($tid){
  return db_exec('DELETE FROM {temp} WHERE tid = ?', array($tid));
}

function dd_rss_feed($items, $channel = array(), $args = array()) {
  global $conf, $base_url;
  
  $namespaces = array(
    'xmlns:dc' => 'http://purl.org/dc/elements/1.1/',
    'xml:base' => $base_url
  );
  
  $channel_defaults = array('version' => '2.0', 'root' => 'channel');
  
  if($channel['namespaces']){
    $namespaces = array_merge($namespaces, $channel['namespaces']);
  }
  
  if($channel){
    $channel = array_merge($channel_defaults, $channel);
  }else{
    $channel = $channel_defaults;
  }
  
  $output = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  $output .= "<rss version=\"". $channel["version"] ."\" ". dd_attributes($namespaces) .">\n";
  $output .= "<$channel[root]>\n";
  if($args){
  	$output .= dd_xml_elements($args);
  }
  
  $output .= $items;
  $output .= "</$channel[root]>\n";
  $output .= "</rss>\n";
  
  return $output;
}

function dd_rss_item($fields, $item = 'item', $args = array()) {
  $output = "<$item>\n";
  if(is_array($fields)){
  	foreach($fields as $key => $val){
  		if(!is_array($val)){
        if(strpos($val, '<') !== false){
          $val = '<![CDATA[ '.$val.' ]]> ';
        }
  			$output .= " <$key>". $val ."</$key>\n";
  		}else{
  			$val['key'] = $key;
  			$output .= dd_xml_elements($val);
  		}
  	}
  }else if($fields){
  	$output .= $fields;
  }
  
  if($args) $output .= dd_xml_elements($args);
  
  $output .= "</$item>\n";
  return $output;
}

function dd_xml_elements($array){
  $output = '';
  foreach ($array as $key => $value) {
    if(is_numeric($key)) {
      if($value['key']) {
        $output .= ' <'. $value['key'];
        if (isset($value['attributes']) && is_array($value['attributes'])) {
          $output .= dd_attributes($value['attributes']);
        }
        if($value['value'] != ''){
          $output .= '>'. (is_array($value['value']) ? dd_xml_elements($value['value']) : check_plain($value['value'])) .'</'. $value['key'] .">\n";
        }else {
          $output .= " />\n";
        }
      }
    }else{
      $output .= ' <'. $key .'>'. (is_array($value) ? dd_xml_elements($value) : check_plain($value)) ."</$key>\n";
    }
  }
  return $output;
}