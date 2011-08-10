<?php
// $Id$

/**
 * @file 
 * 定义一些命令行公用方法
 * 加载 bootstrap.inc
 * 设置一个全局变量 $_args，即解析为数据的命令行参数
 *
 * 调用本文件，必须在命令行下指定一个站点参数，即 --host www.test.com
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

// 使用指定站点的配置文件，必须有这个参数
$_SERVER['HTTP_HOST'] = $_args['host'];
$_SERVER['REQUEST_URI'] = '/scripts';

require_once DIDA_ROOT . '/includes/bootstrap.inc';

