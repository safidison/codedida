<?php
// $Id$

/**
 * @file 从命令行执行脚本
 *
 * 加载 bootstrap.inc
 * 设置一个全局变量 $_args，即解析为数据的命令行参数
 *
 * 调用本文件，必须在命令行下指定一个站点参数，即 --host www.test.com
 *
 * 示例 1，加载 user 模块下的 script.inc 脚本：
 *  php common.inc --host www.test.com --module_scripts user --script_name script.inc
 */

/**
 * 
 * 将命令行参数解析为数组，约定使用 --
 * 如：php test.php --m test --k --p a -p b
 * 解析为 array('m' => 'test', 'k' => true, 'p' => array('a', 'b'));
 * 
 * @param (string) $key
 * 	获取指定参数
 * @return (array|*)
 */
function args($key = NULL) {
  static $args;
  
  if (!isset($args)) {
    $_args = array();
    for ($i = 1; $i < $_SERVER['argc']; ++$i) {
      if (preg_match('/--(\S+)/', $_SERVER['argv'][$i], $mat)) {
        $i++;
        if (substr($_SERVER['argv'][$i], 0, 2) != '--') {
          $_args[$mat[1]][] = $_SERVER['argv'][$i];
        } else {
          $i--;
          $_args[$mat[1]][] = true;
        }
      } else {
        $_args[$i] = $_SERVER['argv'][$i];
      }
    }
    
    $args = array();
    
    foreach ($_args as $_key => $v) {
      if (count($v) == 1) {;
        $args[$_key] = $v[0];
      } else {
        $args[$_key] = $v;
      }
    }
    
    unset($_args);
  }
  
  return isset($key) ? $args[$key] : $args;
}

// 解析命令行参数
$_args = args();

if (empty($_args['host'])) {
  print("必须指定站点，如：www.cmsns.com，才能加载配置文件\n");
  exit();
}

define('DIDA_ROOT', dirname(dirname(__FILE__)));
set_time_limit(0);

// 使用指定站点的配置文件，必须有这个参数
$_SERVER['HTTP_HOST'] = $_args['host'];
$_SERVER['REQUEST_URI'] = '/scripts';

require_once DIDA_ROOT . '/includes/bootstrap.inc';

// 初始化程序
if (!empty($_args['bootstrap'])) {
  bootstrap(($_args['bootstrap'] == 'data' ? 'data' : 'full'));
}

/**
 * 某些时候，涉及到大量数据操作的功能，耗时较长，可能需要在命令行下执行。
 * 模块可自行添加脚本，通过统一格式进行调用执行，如：
 *  sites/modules/mymodule/scripts/myscript.inc
 * 以如下方式调用执行：
 *  php -f common.inc --host www.cmsns.com --module_scripts mymodule --script_name myscript.inc
 */

// 需要加载的脚本文件名称
if (!empty($_args['script_name'])) {

  // 默认脚本文件目录为 scripts，可以指定一个模块名称
  if (!empty($_args['module_scripts'])) {
    $script_path = DIDA_ROOT . '/' . dd_get_path('module', $_args['module_scripts']) . '/scripts/' . $_args['script_name'];
  } else {
    $script_path = DIDA_ROOT . '/scripts/' . $_args['script_name'];
  }
  
  if (is_file($script_path)) {
    require_once $script_path;
  }
}

