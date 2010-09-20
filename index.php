<?php
// $Id$

/**
 * 工作目录
 * 建议包含文件，使用绝对路径。
 */
define('DIDA_ROOT', getcwd());

require_once DIDA_ROOT . '/includes/bootstrap.inc';

// 程序初始化
bootstrap('full');

// 获取当前路径($_GET['q'])输出
$return = menu_get_item();

if (is_int($return)) {
  switch ($return) {
    case MENU_ACCESS_DENIED:
      // 无权限访问
      dd_get_access();
    break;
    case MENU_NOT_FOUND:
      // 路径未定义
      dd_get_not();
  }
  exit;
}

// 打印数据
print theme('page', $return);