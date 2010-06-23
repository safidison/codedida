<?php
// $Id$

function user_default() {
  global $user;
  if (!$user->uid) {
    dd_goto('user/login');
  } else {
    dd_goto('user/' . $user->uid);
  }
}

/**
 * 用户 ajax 搜索 
 */
function user_page_json() {
  if ($_POST['value']) {
    $json['error'] = 0;
    if ($fetch = db_query('SELECT name FROM {users} WHERE name LIKE ?',
    array('%'.$_POST['value'].'%'), array('limit' => 10))) {
      foreach ($fetch as $o) {
        $json['contents'][] = $o->name;
      }
    }
    echo json_encode($json);
  } else {
    echo '{"error": 1}';
  }
  exit;
}

/**
 * 找回密码
 */
function user_page_pass() {
  switch ($_GET['type']) {
    case 'mail':
      if ($_GET['ajax'] == 'submit') {
        if ($_POST['name']) {
          $o = db_query('SELECT status, name, uid, mail FROM {users} WHERE name = ?', array($_POST['name']), array('return' => 'one'));
        } else if ($_POST['mail']) {
          $o = db_query('SELECT status, name, uid, mail FROM {users} WHERE mail =?', array($_POST['mail']), array('return' => 'one'));
        } else {
          dd_js_print('$(\'#ajax_description\').html("邮箱或昵称必填一项").show(300);');
        }
        if ($o->uid) {
          if ($o->status == 1) {
            if (_user_newpass($o->uid, $o->name, $o->mail)) {
              dd_js_print('$(\'#ajax_description\').html("找回密码的方式已发送到你的邮箱，请注意查收。").show(300);');
            } else {
              dd_js_print('$(\'#ajax_description\').html("找回失败，请重试。").show(300);');
            }
          } else if (!$o->status) {
            $str = addslashes('你的账号还未激活。'.l('立即激活', 'user/activation/').'。');
            dd_js_print('$(\'#ajax_description\').html("'.$str.'").show(300);');
          }
        }
      }
    exit();
    case 'newpass':
      if ($_GET['str'] && $_GET['uid']) {
        if ($uid = db_query('SELECT uid FROM {users} WHERE uid = ? AND newpass = ? AND active > ?', array($_GET['uid'], $_GET['str'], ($_SERVER['REQUEST_TIME'] - 86400)), array('return' => 'column'))) {
          db_exec('UPDATE {users} SET newpass = 0 WHERE uid = ?', array($uid));
          if (user_login(user_load($uid))) {
            setcookie(session_name(), session_id(), -1, '/', $cookie_domain);
            $_SESSION['oldpass'] = time();
            dd_set_message('请在 5 分钟内修改密码');
            dd_goto('user/newpass');
          } else {
            dd_goto('user/pass');
          }
        }
      } else {
        dd_set_message('错误的请求', 'error');
      }
    break;
  }
  $form['settings'] = array(
    '#ajax_validate' => true,
    '#description' => t('user', '请输入注册邮箱或昵称'),
    '#ajax_submit' => array(
      'des' => true,
      'options' => array(
        'dataType' => 'script',
        'url' => url('user/pass', array('query' => array('type' => 'mail', 'ajax' => 'submit'))),
      ),
    ),
  );
  $form['fields']['mail'] = array(
    '#type' => 'textfield',
    '#title' => '邮箱',
  );
  $form['fields']['name'] = array(
    '#type' => 'textfield',
    '#title' => '昵称',
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#value' => '确认'
  );
  return $form;
}

/**
 * 修改密码
 */
function user_page_newpass() {
  global $user;
  if ($_GET['ajax'] == 'submit') {
    if (!$_POST['pass']) {
      $error[] = t('user', '新密码不能为空');
    } else if ($_POST['pass'] != $_POST['pass2']) {
      $error[] = t('user', '两次输入的新密码不一致');
    } else {
    	
    	$_POST['uid'] = $user->uid;
    	$_POST['name'] = $user->name;
    	$_POST['mail'] = $user->mail;
    	
    	if (!$error = module_invoke_all('user', 'newpass', (object)$_POST)) {
        $ok = true;
	      if (!$_SESSION['oldpass'] || $_SESSION['oldpass'] < time() - 300) {
          if ($_POST['pass'] != $_POST['oldpass']) {
	          $ok = db_exec('UPDATE {users} SET pass = ? WHERE uid =? AND pass = ?', array(md5($_POST['pass']), $user->uid, md5($_POST['oldpass'])));
          }
	      } else {
	        db_exec('UPDATE {users} SET pass = ? WHERE uid = ?', array(md5($_POST['pass']), $user->uid));
          
	        unset($_SESSION['oldpass']);
	      }
	      if ($ok) {
	        setcookie(session_name(), session_id(), -1, '/', $cookie_domain);
	        dd_set_message(t('user', '密码修改成功'));
	        dd_js_print('window.location.href = "'.url('user/newpass').'"');
	        exit();
	      } else {
	        $error[] = t('user', '旧密码不匹配');
	      }
      }
    }
    dd_js_print('$(\'#ajax_description\').html("'.addslashes(implode('<br />', dd_array_multi2single($error))).'").show(300);');
    exit();
  }
  
  dd_set_tabs(array(
    l(t('user', '我的主页'), 'user/'.$ac->uid),
    l(t('user', '编辑资料'), 'user/edit'),
    l(t('user', '修改密码'), 'user/newpass'),
  ));
  
  $form['settings'] = array(
    '#ajax_validate' => true,
    '#ajax_submit' => array(
      'des' => true,
      'options' => array(
        'dataType' => 'script',
        'url' => url('user/newpass', array('query' => array('ajax' => 'submit'))),
      ),
    ),
  );
  if (!$_SESSION['oldpass'] || $_SESSION['oldpass'] < time() - 300) {
    $form['fields']['oldpass'] = array(
      '#type' => 'password',
      '#title' => t('user', '旧密码'),
      '#required' => true,
    );
  }
  $form['fields']['pass'] = array(
    '#type' => 'password',
    '#title' => t('user', '新密码'),
    '#required' => true,
    '#attributes' => array(
      'id' => 'password',
    ),
  );
  $form['fields']['pass2'] = array(
    '#type' => 'password',
    '#title' => t('user', '重复新密码'),
    '#required' => true,
    '#attributes' => array(
      'class' => "{equalTo:'#password'}"
    ),
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#weight' => 100,
  );
  return $form;
}

/**
 * 激活帐号，触发 hook_user_status($old, &$ac)
 */
function user_page_activation() {
  if ($_GET['validate'] && $_GET['uid']) {
    if ($uid = db_query('SELECT uid FROM {users} WHERE validate = ? AND uid = ?', array($_GET['validate'], $_GET['uid']), array('return' => 'column'))) {
      if (db_exec('UPDATE {users} SET status = 1, validate = ? WHERE uid = ?', array('', $uid))) {
        
        $user = user_load($uid);
        
        module_invoke_all('user_status', 0, $user);
        
        if (user_login($user) && ($_GET['ajax'] != 'submit')) {
          dd_set_message('激活成功，欢迎加入 '. $GLOBALS['conf']['site_name']);
          dd_goto(dd_get_token(var_get_key('user', 'redirect', 'user/' . $user->uid), $user, array('user', 'system')));
        }
      }
    } else {
      dd_set_message('不正确的激活链接', 'error');
    }
  }
  $form['settings'] = array(
    '#validate' => array('user_activation_validate'),
  );
  $form['fields']['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('user', '邮箱'),
    '#required' => true,
    '#description' => t('user', '请输入你注册时填写的邮箱，系统将为你重新发送激活链接'),
  );
  $form['fields']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('user', '发送激活链接')
  );
  return $form;
}

function user_activation_validate($form, $v) {
  if ($msg = form_vali_mail($v['mail'])) {
    $t['mail']['#error'][] = $msg;
  } else if (!$o = db_query('SELECT uid, name, created FROM {users} WHERE mail = ? AND status = 0', array($v['mail']), array('return' => 'one'))) {
    $t['mail']['#error'][] = t('user', '不正确的邮箱');
  } else {
    _user_activation($o->uid, $o->name, $v['mail'], $o->created);
    dd_set_message(t('user', '激活链接已发送，请注意查收'));
    dd_goto('user/activation');
  }
  return $t;
}

/**
 * 修改资料
 */
function user_page_edit() {
  
  return dd_get_form('user_edit_form', $GLOBALS['user']);
}

/**
 * 注册
 */
function user_page_register() {
  if ($_GET['ajax'] == 'submit') {
    $form_values = array();
    if (!$error = dd_ajax_validate(0, 'user_register_form', $_POST['form_token'])) {
      if ($account = user_insert($_POST['_dida_values'])) {
        if ($_GET['redirect'] && strpos($_GET['redirect'], 'user/login') === false && strpos($_GET['redirect'], 'user/register') === false) {
          $rediect = $_GET['redirect'];
        } else {
          $rediect = url(dd_get_token(var_get_key('user', 'redirect', 'user/' . $account->uid), $account, array('user', 'system')));
        }
        if ($account->status == 1) {
          dd_set_message(t('user', '恭喜，注册成功了'));
          user_login(user_load($account->uid));
        } else if ($GLOBALS['conf']['user']['status'] == 0) {
          dd_set_message(t('user', '恭喜，注册成功了，你需要通过邮箱激活帐号'));
          $rediect = url('user/activation');
        } else if ($GLOBALS['conf']['user']['status'] == -1) {
          dd_set_message(t('user', '恭喜，注册成功了，请等待管理员激你的帐号'));
          $rediect = $GLOBALS['base_path'];
        }
        
        dd_js_print('window.location.href = "'.urldecode($rediect).'"');
      } else {
        dd_js_print('alert(\''.t('user', '抱歉，出现问题，注册失败了').'\')');
      }
    } else {
      dd_js_print('$(\'#ajax_description\').html("'. addslashes(implode('<br>', dd_array_multi2single($error))).'").show(300);');
    }
  }
  $v = dd_get_form('user_register_form');
  return $v;
}

/**
 * 登录
 */
function user_page_login() {
  if ($_GET['ajax'] == 'submit') {
    if (!$error = dd_ajax_validate(0, 'user_login_form', $_POST['form_token'])) {
      if ($_SESSION['user']->uid > 0) {
        if (!$_POST['his'][1]) {
          global $cookie_domain;
          setcookie(session_name(), session_id(), -1, '/', $cookie_domain);
        }
        if ($_GET['redirect'] && strpos($_GET['redirect'], 'user/login') === false && strpos($_GET['redirect'], 'user/register') === false) {
          $rediect = $_GET['redirect'];
        } else {
          $rediect = url(dd_get_token(var_get_key('user', 'redirect', 'user/' . $_SESSION['user']->uid), $_SESSION['user'], array('user', 'system')));
        }
        dd_js_print('location.href = "'.urldecode($rediect).'"');
      } else {
        dd_js_print('alert(\''.t('user', '抱歉，登录失败了').'\')');
      }
    } else {
      dd_js_print('$(\'#ajax_description\').html("'.addslashes(implode('<br>', dd_array_multi2single($error))).'").show(300);');
    }
  }
  $v = dd_get_form('user_login_form');
  return $v;
}

function user_page_login_ajax() {
  if ($_GET['ajax'] == 'submit') {
    if (!$error = dd_ajax_validate(0, 'user_login_form', $_POST['form_token'])) {
      if ($_SESSION['user']->uid > 0) {
        if (!$_POST['his'][1]) {
          global $cookie_domain;
          setcookie(session_name(), session_id(), -1, '/', $cookie_domain);
        }
        switch ($_GET['call']) {
          case 'to':
            //关闭本页，刷新父页
            dd_js_print('self.parent._tb_remove()');
          break;
          case 'function':
            //执行指定函数
            $_GET['function'] ? dd_js_print('self.parent.tb_remove(); self.parent.'. $_GET['function'] .'();') : dd_js_print('self.parent.tb_remove();');
          break;
          case 'close':
            //关闭层
            dd_js_print('self.parent.tb_remove();');
          break;
          default:
            $url = $_GET['redirect'] ? $_GET['redirect'] : url(dd_get_token(var_get_key('user', 'redirect', 'user/' . $_SESSION['user']->uid), $_SESSION['user'], array('user', 'system')));
            dd_js_print('window.location.href = "'.urldecode($url).'"');
        }
      } else {
        dd_js_print('alert(\''.t('user', '抱歉，登录失败了').'\')');
      }
    } else {
      dd_js_print('$(\'#ajax_description\').html("'.addslashes(implode('<br>', dd_array_multi2single($error))).'").show(100);');
    }
    exit;
  }
  echo theme('print', dd_get_form('user_login_form'));
  exit;
}