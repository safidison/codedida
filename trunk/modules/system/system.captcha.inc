<?php
// $Id$

function captcha_page() {
  if ($_GET['form_id']) {
    $value = $_GET;
  } else if ($_POST['form_id']) {
    $value = $_POST;
  } else {
    echo 'ERROR!';
    exit;
  }
  
  if ($value['name'] && $value['form_id']) {
    $str = _captcha_get($value['name'], $value['form_id'], $value['tag'], $value['type']);
    header('Cache-Control: no-cache, must-revalidate');
    header('Expires: Sat, 26 Jul 1997 05:00:00 GMT');
    
    if ($value['image']) {
      if ($ttf = var_get('captcha_file_ttf')) {
        if (is_file($ttf)) {
          header('Content-type: image/png');
          list($width, $height) = explode('x', $value['image']);
          $len = mb_strlen($str, 'utf-8');
          
          if ($len*14 > $width) {
            $width = $len*14+40;
          }
          
          $im = @imagecreate($width, $height);
          imagecolorallocate($im, 255, 255, 255);
          $text_color = imagecolorallocate($im, 233, 12, 91);
          $font = $ttf;
          
          imagettftext($im, 12, 0, 5, $height-2, $text_color, $font, $str);
          imagepng($im);
          imagedestroy($im);
          unset($font);
          exit;
        }
      }
    }
    
    header('Content-Type: text/plain; charset=utf-8');
    print $str;
    exit;
  }
  print false;
  exit;
}
function captcha_get($name, $form_id, $form_token, $type, $image = 0) {
  if (!$image) {
    return _captcha_get($name, $form_id, $form_token, $type);
  } else {
    return '<img class="form_captcha_img" src="'.url('captcha', array('query' => array(
    'name' => $name, 'form_id' => $form_id, 'image' => $image, 'tag' => $form_token,
    'timestamp' => '1'))).'" />';
  }
}

function _captcha_get($name, $form_id, $form_token, $type) {
  switch ($type) {
    case 1:
      $array1 = range(1, 199);
      $array2 = range('a', 'z');
      $a = array_rand($array1, 2);
      $b = array_rand($array2, 2);
      $cap = $array1[$a[0]] . $array2[$b[0]] . $array1[$a[1]]  . $array2[$b[1]];
      $str = $cap;
    break;
    case 2:
      $cap = _captcha_zh();
      $str = $cap;
    break;
    case 3:
      $a = rand(1, 10);
      $b = rand(1, 10);
      $c = rand(1, 10);
      $d = rand(1, 10);
      $str = t('system', '请计算：!string', array('!string' => "{$a}+{$b}x{$c}-{$d} = ?"));
      $cap = $a + $b * $c - $d;
    break;
    case 4:
      $array = _captcha_ask();
      $cap = $array[1];
      $str = t('system', '请回答：!string', array('!string' => $array[0]));
    break;
    default:
      $a = rand(1, 10);
      $b = rand(1, 10);
      $c = rand(1, 10);
      $d = rand(1, 10);
      $cap = $a. $b. $c. $d . $d. $c;
      $cap = substr($cap, 0, 4);
      $str = $cap;
  }
  $_SESSION['captcha'][$form_token. $form_id . $name] = $cap;
  
  return $str;
}

function _captcha_zh() {
  $data = array(
    '西当太白有鸟道',
    '可以横绝峨眉巅',
    '上有六龙回日之高标',
    '下有冲波逆折之回川',
    '地崩山摧壮士死',
    '然后天梯石栈相钩连',
    '问君西游何时还',
    '连峰去天不盈尺',
    '枯松倒挂倚绝壁',
    '难于上青天',
    '嗟尔远道之人胡为乎来哉'
  );
  return $data[array_rand($data)];
}

function _captcha_ask() {
  $data = array(
    array('世界上含沙量最大的河是？', '黄河'),
    array('世界上最早的字典？', '说文解字'),
    array('世界上最大的石刻佛像？', '乐山大佛'),
    array('世界上最高的山峰？', '珠穆朗玛峰')
  );
  return $data[array_rand($data)];
}