<?php
// $Id$

// 请不要修改和删除本文件
// 复制本文件到 sites/ 或 sites/demo.mysite.com 目录，并重命名为 setting.php

/**
 * 如何实现同一套程序多个站点？通过配置文件来完成。
 * 假设通过域名 demo.mysite.com 访问，优先读取 sites/demo.mysite.com/setting.php
 * 假设通过域名 test.mysite.com 访问，优先读取 sites/test.mysite.com/setting.php
 * 若无，则读取默认配置文件 sites/setting.php
 */

/**
 * 安装状态
 * 若为 true，将无法访问安装文件(install.php)
 */

$installed = false;

/**
 * 缓存文件，相对路径。以此实现不同的缓存方式，比如 memcache
 * 默认文件 includes/cache.inc。
 * 不推荐修改，不正确的修改将导致程序无法运行
 */

define('DD_CACHE_FILE', 'includes/cache.inc');

// 默认数据库连接，安装时按提示输入，或直接在此填写

define('DD_ADMIN_PATH', 'admin'); // 管理路径

/**
 * mysql 为例
 */
$database['default'] = array(
  'driver' => 'mysql',// 或其它 PDO 支持的数据库，
  'host' => 'localhost', // 主机
  'port' => 3306, // 端口
  'dbname' => 'test', // 数据库名称
  'dbuser' => 'root', // 数据库用户
  'dbpass' => 'root', // 密码
  'prefix' => '' // 表前缀
);

// 可提供多个连接

/**
	$database['more'] = array(
	  'driver' => 'mysql',
	  'host' => 'localhost',
	  'port' => 3306, 
	  'dbname' => 'test2',
	  'dbuser' => 'root',
	  'dbpass' => 'root',
	  'prefix' => ''
	);
 */

/**
 * 使用连接：
 *  db_query($sql, $args);// 用默认连接执行
 *  db_query($sql, $args, array('target' => 'more'));// 用 more 链接查询
 * 或直接使用：
 *  global $db;
 *  $db['default']->query($sql);
 *  $db['more']->exec($sql);
 */

ini_set('arg_separator.output', "&amp;");
ini_set('magic_quotes_runtime', 0);
ini_set('magic_quotes_sybase', 0);
ini_set('session.cache_expire', 2592000);
ini_set('session.cache_limiter', "none");
ini_set('session.cookie_lifetime', 31536000);
ini_set('session.gc_maxlifetime', 2592000);
ini_set('session.use_only_cookies', 1);
ini_set('session.use_trans_sid', 0);
ini_set('url_rewriter.tags', '');