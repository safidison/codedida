<?php
// $Id$

/**
 * @方法 dd_save_cron()
 *  见 bootstrap.inc
 * @钩子 hook_cron($timestamp)
 *  每次执行计划任务时，优先读取 cron 表中所有未完成任务，予以执行
 *  其次才触发 hook_cron()，模块尽量选择将任务写入 cron 表，而不是实现 hook_cron
 *  这样有利于让系统在未来更合理的安排计划任务，让计划任务更有序地执行
 *  保留 hook_cron() 是考虑到某些模块，其内部可能有复杂繁琐的计划任务，
 *  或者其任务是一项长期且重复性的，比如清空临时表
 *  类似模块，在 hook_cron() 中自行处理更为合适
 * @param (int) $timestamp
 *  hook_cron() 将传递一个时间戳，即上一次 cron 执行的时间
 * @示例
 *  hook_cron($timestamp) {
 *    db_query('DELETE FROM {temp} ....'); // 定期清除临时表
 *  }
 */

require_once './includes/bootstrap.inc';
bootstrap('full');

/**
 * 获得 cron 的执行密码，默认为所有人可执行
 * 若设置了密码，则必须传递 URL 参数：pass
 * 密码生成方式：自定义字符串的 md5 值
 * cron 并不会泄露安全信息，故而这仅是一个很简单的验证
 */

if ($pass = var_get('cron_pass', false)) {
  if (!$_GET['pass'] || $_GET['pass'] != md5($pass)) {
    echo 'byebye';
    exit;
  }
}

// 获得上一次执行时间
$timestamp = var_get('cron_last_time', 0);

/**
 * 获得最小执行时间，默认为 3600 秒，防止频繁执行
 * $user->uid == 1 时，手动执行不受此限制
 */
if ($GLOBALS['user']->uid != 1) {
  $time_min = var_get('cron_min_time', 3600);

  if ($time_min && $timestamp > ($_SERVER['REQUEST_TIME'] - $time_min)) {
    echo 'byebye';
    exit;
  }
}

set_time_limit(600);

// 读取任务列表，每次最多 100 条
if ($fetch = db_query('SELECT * FROM {cron} WHERE status = 0 ORDER BY weight ASC, cid ASC', NULL, array('limit' => 100))) {
  foreach ($fetch as $o) {
    if (!$o->data) continue;
    
    $data = unserialize($o->data);
    
    if ($data['includes']) {
      foreach ($data['includes'] as $filepath) {
        include_once $filepath;
      }
    }
    if (function_exists($data['func']) && call_user_func_array($data['func'], $data['args'])) {
      db_exec('UPDATE {cron} SET status = 1 WHERE cid = ?', array($o->cid));
      if ($data['success'] && function_exists($data['success'])) {
        call_user_func_array($data['success'], $data['args']);
      }
    }
    
  }
}

// 触发 hook_cron()

module_invoke_all('cron', $timestamp);

// 写入运行时间
var_set('cron_last_time', time());

// 写入日志
dd_log('cron', t('system', '成功运行了计划任务'));

echo 'ok!';