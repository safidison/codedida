<?php
// $Id$

error_reporting(E_ALL ^ E_NOTICE);

define('DIDA_ROOT', getcwd());

if (is_file('sites/config.php')) {
  require_once 'sites/config.php';
  if ($config && $config[$_SERVER['HTTP_HOST']]) {
    $conf_dir = $config[$_SERVER['HTTP_HOST']];
  } else if (!empty($config['default'])) {
    $conf_dir = $config['default'];
  }
}

if (empty($conf_dir)) {
  if (!is_dir('sites/' . $_SERVER['HTTP_HOST'])) {
    $conf_dir = 'sites';
  } else {
    $conf_dir = 'sites/' . $_SERVER['HTTP_HOST'];
  }
}

$conf_file = $conf_dir .'/cache/conf.php';
$setting_file = $conf_dir .'/setting.php';

if (is_file($conf_file)) {
  require_once $conf_file;
}

if (is_file($setting_file)) {
	require_once $setting_file;
  
  if ($dir = trim(dirname($_SERVER['SCRIPT_NAME']), '\,/')) {
    $base_path = "/$dir";
    $base_path .= '/';
  } else {
    $base_path = '/';
  }
  
  if (!empty($installed)) {
    if (!empty($conf['status'])) {
      header('Location: ' . $base_path, true, 301);
    } else {
      header('Content-Type: text/html; charset=utf-8');
      echo $setting_file . ' 文件显示系统已经安装，或许这是一个旧的配置文件，请确认并清除该文件内容';
    }
    exit;
  }
}

require_once './includes/bootstrap.inc';
require_once './includes/database.inc';
require_once './includes/form.inc';
require_once './includes/cache.inc';

conf_init();

if (is_file('./install/install.php') && function_exists('install')) {
  call_user_func('install');
  exit;
}

$title = '检查安装环境';

if (!$error = dida_is_setup()) { // 检查安装环境
  if (empty($_GET['setup'])) {
    dd_goto(f('install.php?setup=1'));
  } else {
  	switch ($_GET['setup']) {
  	  case 1:
        $title = '选择数据库';
  	    $database_type = dida_setup_data_select(); // 选择数据库
  	  break;
  	  case 2:
        $title = '填写数据库信息';
  	    $database_form = dida_setup_data_form(); // 填写数据库信息
  	  break;
  	  case 3:
        $title = '测试数据库权限';
  	    $body = dida_setup_data_test(); // 测试数据库
  	  break;
  	  case 4:
        $title = '填写管理员信息';
  	    $body = dida_setup(); // 安装程序
  	  break;
  	  default:
  	  	dd_goto(f('install.php?setup=1'));
  	}
  }
}

function dida_is_setup() {
  $error = NULL;
  
  if (version_compare(PHP_VERSION, '5.2.0', '<')) {
    $error[] = 'PHP版本至少为 5.2';
  }
  if (!function_exists('json_decode')) {
    $error[] = '请开启 json 扩展';
  }
  if (!function_exists('mb_strlen')) {
    $error[] = '请开启 mb 扩展';
  }
  if (!class_exists('PDO')) {
    $error[] = '请开启 PDO 扩展';
  }
  if (!function_exists('gd_info')) {
    $error[] = '请开启 GD 扩展';
  }
  if (!is_writable('./sites/logs')) {
    $error[] = ' 日志目录(sites/logs)必须有读取权限 ';
  }
  
  if (!empty($error)) return $error;
  
  global $conf, $conf_dir, $conf_file, $setting_file;
  
  if (is_file($conf_file)) {
  	if (!is_writable($conf_file)) {
    	$error[] = $conf_file . ' 必须有读写权限';
    }
  } else {
    if (!is_dir($conf_dir.'/cache') && !mkdir($conf_dir.'/cache', 0750)) {
      $error[] = '无法创建目录 '.$conf_dir.'/cache，请手动创建，并将权限设置为可读写。';
    }
    if (is_file('sites/cache/default.conf.php')) {
      file_put_contents($conf_file, file_get_contents('sites/cache/default.conf.php'));
      chmod($conf_file, 0777);
    } else {
      $error[] = '请不要删除 sites/cache/default.conf.php';
    }
  }
  
  if (is_file($setting_file)) {
  	if (!is_writable($setting_file)) {
    	$error[] = $setting_file . ' 必须有读写权限';
    }
  } else if (!$handle = fopen($setting_file, "wb")) {
    $error[] = '配置文件('.$setting_file.')不存在且无法自动创建，请复制 sites/default.setting.php 并重命名为 setting.php。';
  } else {
    chmod($setting_file, 0777);
    fclose($handle);
  }
  
  return $error;
}

function dida_setup_data_select() {
  global $database, $setting_file;
  $type = db_info();
  
  if (!empty($_GET['clear'])) {
    if (file_put_contents($setting_file, '')) {
      $database = NULL;
    }
  }
  
  if (!empty($_POST['driver']) && !empty($type[$_POST['driver']])) {
    $setting_string = '<?php';
    $setting_string .= "\n";
    $setting_string .= '$database[\'default\'] = array(';
    $setting_string .= "\n  'driver' => '$_POST[driver]',\n);";
    if (file_put_contents($setting_file, $setting_string)) {
      dd_goto(f('install.php?setup=2'));
    }
  }
  
  if (!empty($database) && !empty($database['default']) 
  && !empty($database['default']['driver']) && !empty($type[$database['default']['driver']])) {
    dd_goto(f('install.php?setup=2'));
  }
  
  return $type;
}

function dida_setup_data_form() {
  global $database, $error, $conf_dir, $setting_file;
  
  $type = db_info();
  
  if (empty($database) || empty($database['default']) 
  || empty($database['default']['driver']) || empty($type[$database['default']['driver']])) {
    dd_goto(f('install.php?setup=1'));
  }
  
  require_once './includes/database/install.'.$database['default']['driver'].'.inc';
  
  $form = '<input type="hidden" name="driver" value="'.$database['default']['driver'].'" />';
  
  if ($_POST) {
    if (!$error = db_install_test($_POST)) {
      require_once './includes/cache.inc';
      if (cache_system_set_file('setting.php', 'database[\'default\']', $_POST, $conf_dir)) {
        dd_goto(f('install.php?setup=3'));
      } else {
        $error[] = '保存失败，请删除 ' . $setting_file;
      }
    } else {
      $database['default'] = $_POST;
    }
  } else {
    $database['default'] = NULL;
  }
  
  if ($fields = db_install_form($database['default'])) {
    foreach ($fields as $field) {
      $form .= '<div class="form_item">'.$field.'</div>';
    }
    return $form;
  }
}

function dida_setup_data_test() {
	global $error, $database;
	
	if (db_connect('default')) {
	  $table = array(
	    'description' => '测试',
	    'fields' => array(
	      'id' => array(
	        'type' => 'serial',
	        'unsigned' => TRUE,
	        'not null' => TRUE,
	      ),
	      'tid' => array(
	        'type' => 'int',
	        'unsigned' => TRUE,
	        'not null' => false,
	        'default' => 0,
	        'description' => '测试'
	      ),
	      'name' => array(
	        'type' => 'varchar',
	        'length' => 255,
	        'not null' => TRUE,
	        'default' => '',
	        'description' => '测试',
	      ),
	    ),
	    'indexes' => array(
	      'tid' => array('tid')
	    ),
	    'primary key' => array('id'),
	  );
	  
	  if (db_is_table('dida_test')) {
	  	if (!db_drop_table('dida_test')) {
	  		$error[] = '没有数据库表删除权限';
	  	}
	  }
	  
	  // 测试表
	  if (db_create_table('dida_test', $table)) {
	  	if (db_exec('INSERT INTO {dida_test} (tid, name) VALUES (:tid, :name)', array(':tid' => 10, ':name' => 'test'))) {
	  		if (db_query('SELECT * FROM {dida_test}')) {
	  			if (db_exec('UPDATE {dida_test} SET name = :name WHERE tid = :tid', array(':tid' => 10, ':name' => 'newtest'))) {
	  				if (db_exec('DELETE FROM {dida_test} WHERE tid = 10')) {
					  	if (!db_drop_table('dida_test')) {
					  		$error[] = '没有数据库表删除权限';
					  	} else if (db_is_table('system')) {
					  		$error[] = '系统配置表已经存在，该数据库中是否已经存在一个程序，建议使用全新数据库，或设置表前缀';
					  	}
	  				} else {
	  					$error[] = '没有数据删除权限';
	  				}
	  			} else {
	  				$error[] = '没有数据更新权限';
	  			}
	  		} else {
	  			$error[] = '没有数据查询权限';
	  		}
	  	} else {
	  		$error[] = '没有表写入权限';
	  	}
	  } else {
	  	$error[] = '没有表创建权限';
	  }
  } else {
  	$error[] = '连接数据库失败，请检查配置，或<a href="install.php?setup=1&clear=1">重新开始安装</a>';
  }
  
  if (empty($error)) {
    dd_goto(f('install.php?setup=4'));
  }
}

function dida_setup() {
  global $base_path, $error, $setting_file;
  
  if ($_POST) {
    if (empty($_POST['admin'])) {
      $error[] = '管理员帐号不能为空。';
    } else if (empty($_POST['mail'])) {
      $error[] = '邮箱不能为空。';
    } else if (empty($_POST['adminpass'])) {
      $error[] = '管理员帐号密码不能为空。';
    } else if ($_POST['adminpass'] != $_POST['adminpass2']) {
      $error[] = '两次输入的密码不一致。';
    } else if (empty($_POST['site_name'])) {
      $error[] = '网站名称不能为空。';
    } else if (empty($_POST['site_mail'])) {
      $error[] = '站长邮箱不能为空。';
    } else if (empty($_POST['status'])) {
      $error[] = '请设置网站访问状态。';
    } else if (db_connect('default')) {
      
      require_once './modules/system/system.install';
      require_once './includes/module.inc';
      require_once './includes/menu.inc';
      
      if (_system_install()) {
        if (module_set_list('theme') && module_set_list('module')) {
          db_exec('UPDATE {system} SET status = -1 WHERE filename = :name', array(':name' => 'default'));
          if ($messages = module_set_enabled('disabled', 'module', array('system', 'block', 'user', 'field'))) {
           
            _install_setting_chmod(); // 锁定安装文件，修改配置文件权限
            
            _install_bootstrap(); // 载入模块
            
            module_enabled_variable();//写入模块、主题列表
            
            menu_set_item();// 写入menu
            
            $admin = check_plain($_POST['admin']);
            
            $pass = user_get_salt_pass($_POST['adminpass']);

            //写入第一个用户
            db_query('INSERT INTO {users} (name, pass, salt, mail, created, status) VALUES(?, ?, ?, ?, ?, 1)',
            array($admin, $pass['pass'], $pass['salt'], $_POST['mail'], $_SERVER['REQUEST_TIME']));
            
            //写入匿名用户
            db_query('INSERT INTO {users} (name, created, status) VALUES(?, ?, 1)', array('匿名', $_SERVER['REQUEST_TIME']));
            
            //调整 uid
            db_query('UPDATE {users} SET uid = 0 WHERE name = ?', array('匿名'));
            
            //测试简洁链接
            $clean_url = 0;
            $goto = '?q=user/login';
            
            block_cache_lists('default'); // 导入区块
            
            $modules = array(
              'system' => 'modules/system',
              'user' => 'modules/user',
              'block' => 'modules/block',
              'category' => 'modules/category',
            );
            
            dd_set_lang('insert', $modules, array('zh-CN', 'zh-TW')); // 导入语言包
            
            $var_timestamp = serialize($_SERVER['REQUEST_TIME']);
            
            $var_args = array(
              'status', serialize($_POST['status']),
              'site_global', serialize(array(
                'logo' => $base_path.'misc/images/logo.png',
                'favicon' => $base_path.'misc/images/favicon.ico',
                'name' => trim($_POST['site_name']),
                'mail' => $_POST['site_mail']
              )),
              'clean_url', serialize($clean_url),
              'cache_css_and_js_timestamp', $var_timestamp,
              'site_created', $var_timestamp
            );
            
            db_query('INSERT INTO {variable} (name, value) VALUES (?, ?), (?, ?), (?, ?), (?, ?), (?, ?)', $var_args);
            require_once './includes/filter.inc';
            
            /**
             * 输入格式缓存
             */
            filter_get_filters();
            filter_set_cache();
            
            var_init();
            
            unset($_SESSION['messages']);
            foreach ($messages as $mmessage) {
              dd_set_message($message);
            }
            
            if (is_file('./install/install.custom.php') && function_exists('install_custom')) {
              call_user_func('install_custom');
            }
            
            user_login(user_load(1));
            dd_goto('user/login');
          }
        }
        $error[] = dd_get_message();
      } else {
        $error[] = '无法创建配置表';
      }
    } else {
	    $error[] = '连接数据库失败，请检查配置，或<a href="install.php?setup=1&clear=1">重新开始安装</a>';
    }
  }
}

function _install_bootstrap() {
  require_once './includes/cache.inc';
  require_once './includes/menu.inc';
  require_once './includes/file.inc';
  foreach (module_list('module', 'enabled') as $module => $info) {
    if ($error = module_load($module.'.module', $info['path'])) {
      echo "模块 $module 主文件 $error 不存在。";
      exit;
    }
  }
}

function _install_setting_chmod() {
  global $database, $conf_dir, $setting_file;
  
  $text[] = '$database[\'default\'] = ' . var_export($database['default'], true).";\n\n";
  $text[] = '$installed = true; // 不允许运行 install.php ';
  $text[] = '$free_update = false; // 不允许匿名或普通用户执行升级';
  $text[] = 'define(\'DD_ADMIN_PATH\', \'admin\'); // 管理路径 ';
  $text[] = 'ini_set(\'arg_separator.output\', "&amp;");';
  $text[] = 'ini_set(\'magic_quotes_runtime\', 0);';
  $text[] = 'ini_set(\'magic_quotes_sybase\', 0);';
  $text[] = 'ini_set(\'session.cache_expire\', 2592000);';
  $text[] = 'ini_set(\'session.cache_limiter\', "none");';
  $text[] = 'ini_set(\'session.cookie_lifetime\', 31536000);';
  $text[] = 'ini_set(\'session.gc_maxlifetime\', 2592000);';
  $text[] = 'ini_set(\'session.use_only_cookies\', 1);';
  $text[] = 'ini_set(\'session.use_trans_sid\', 0);';
  $text[] = 'ini_set(\'url_rewriter.tags\', \'\');';
  
  if (cache_system_set_file('setting.php', NULL, implode("\n", $text), $conf_dir)) {
    if (!chmod($setting_file, 0644)) {
      dd_set_message("无法修改 $file 文件权限，为了安全，应将该文件修改为只读。", 'warning');
    }
  }
}

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <title><?php echo $title; ?> | <?php echo $conf['site_name']; ?></title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <style>
    body {
      color:#474747;
      font-size:14px;
      line-height:1.6;
    }
    a{
      color: #1A4686
    }
    .form_item{
      margin: 4px 0;
    }
    input.form_text{
      font-size: 14px;
      padding:2px 3px;
      font-style: normal;
    }
    span.form_required {
      color:#FFAE00;
    }
    #install_form{
      margin: 10px auto;
      border: 1px solid #ccc;
      padding: 0 12px;
      width: 700px;
    }
    #header{
      margin: 5px auto;
      border: 1px solid #ccc;
      padding: 12px;
      width: 700px;
      color: #3E5803
    }
    #error{
      width:700px;
      margin: 5px auto;
      background:#FFCCCC;
      color:#A30000;
      border:1px solid #DD7777;
      padding: 5px 12px;
    }
    label{
      text-align: right;
      display:-moz-inline-box;
      display:inline-block;
      width:120px;
      font-weight: bold;
      padding-right:6px;
    }
  </style>
</head>
<body>
  <h2 id="header"><?php echo $title; ?></h2>
  <?php if ($error) : ?>
    <div id="error"><?php print implode('<br />', $error); ?><?php echo dd_get_message(); ?></div>
  <?php endif; ?>
  <form accept-charset="UTF-8" id="install_form" action="" method="post">
    <?php if ($_GET['setup'] == 1) { ?>
      <div class="form_item">
        <?php 
          echo dd_form_select(array(
          '#title' => '请选择数据库',
          '#name' => 'driver',
          '#required' => 1,
          '#options' => $database_type));
        ?>
        <input type="submit" value="确定" />
      </div>
    <?php } else if ($_GET['setup'] == 2) { ?>
      <h2>数据库信息：</h2>
      <div class="form_item">
        <label class="dd_label">数据库类型：<span title="此项不能为空。" class="form_required">*</span></label><?php echo $database['default']['driver']; ?>
        (<a href="install.php?setup=1&clear=1">重新选择</a>)
      </div>
      <?php echo $database_form; ?>
      <div class="form_item">
        <input type="submit" value="测试数据库" />
      </div>
    <?php } else if ($_GET['setup'] == 3) { ?>
        <a href="install.php?setup=2">返回重新填写数据库信息</a>
    <?php } else if ($_GET['setup'] == 4) { ?>
      <h2>注册第一个用户（超级管理员）：</h2>
      <div class="form_item">
        <label class="dd_label">邮箱：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="text" class="required form_text" size="50" value="<?php echo $_POST['mail'];?>" name="mail"/>
      </div>
      <div class="form_item">
        <label class="dd_label">昵称：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="text" class="form_text" size="50" value="<?php echo $_POST['admin'];?>" name="admin"/>
      </div>
      <div class="form_item">
        <label class="dd_label">密码：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="password" class="form_text" size="50" value="<?php echo $_POST['adminpass'];?>" name="adminpass"/>
      </div>
      <div class="form_item">
        <label class="dd_label">重复密码：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="password" class="required form_text" size="50" value="<?php echo $_POST['adminpass2'];?>" name="adminpass2"/>
      </div>
      <h2>站点设置：</h2>
      <div class="form_item">
        <label class="dd_label">名称：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="text" class="required form_text" size="50" value="<?php echo $_POST['site_name'];?>" name="site_name"/> 如：我的网站
      </div>
      <div class="form_item">
        <label class="dd_label">站长邮箱：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="text" class="required form_text" size="50" value="<?php echo $_POST['site_mail'];?>" name="site_mail"/>
      </div>
      <div class="form_item">
        <label class="dd_label">浏览：<span title="此项不能为空。" class="form_required">*</span></label>
        <input type="radio" name="status" value="1" <?php if ($_POST['status'] == 1) echo ' checked="checked"';?>/>允许浏览
        <input type="radio" name="status" value="3" <?php if ($_POST['status'] == 3) echo ' checked="checked"';?>/>暂时不允许浏览，设置好了再说
      </div>
      <div class="form_item">
        <input type="submit" value="开始安装" />
      </div>
    <?php }; ?>
  </form>
</body>
</html>
