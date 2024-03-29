<?php
// $Id$

function theme_init() {
  global $conf, $base_path, $base_theme_name, $base_theme_info;

  $base_theme_name = !$GLOBALS['user']->theme ? $conf['default_theme'] : $GLOBALS['user']->theme;
  
  $base_theme_info = $conf['themes'][$base_theme_name];

  $conf['base_theme'] = $base_path . $base_theme_info['path'];
  $conf['base_logo'] = $base_path . ($base_theme_info['logo'] ? $base_theme_info['logo'] : 'misc/images/logo.png');
  $conf['base_favicon'] = $base_path . ($base_theme_info['favicon'] ? $base_theme_info['favicon'] : 'misc/images/favicon.ico');

  if (is_file(DIDA_ROOT .'/'. $base_theme_info['path'] . '/template.php')) {
    require_once DIDA_ROOT .'/'. $base_theme_info['path'] . '/template.php';
  }

  if (empty($base_theme_info['styles_remove']['misc/dida.css'])) {
    dd_add_css('misc/dida.css', 'theme');
  }
  
  if (empty($base_theme_info['styles'])) {
    dd_add_css($base_theme_info['path'].'/css/style.css', 'theme');
  } else {
    foreach ($base_theme_info['styles'] as $style) {
      dd_add_css($base_theme_info['path'].'/'. $style, 'theme');
    }
  }
  
  if ($base_theme_info['scripts']) {
    foreach ($base_theme_info['scripts'] as $script) {
      dd_add_js($base_theme_info['path'].'/'. $script, 'theme');
    }
  }

  dd_add_js('misc/jquery.js', 'core');
  dd_add_js('misc/dida.js', 'core');
  dd_add_js('misc/jquery.dida.js', 'core');
}

/**
 * 缓存各模块定义的主题函数
 */
function theme_get_cache() {
  if ($themes = _module_invoke_all('theme')) {
    foreach ($themes as $filename => $module) {
      foreach ($module as $name => $theme) {
        $data[$name] = $theme;
        $data[$name]['module'] = $filename;
      }
    }
  } else {
    $data = array();
  }
  
  var_set('theme_cache', $data);
}

function theme() {
  global $conf, $user;
  
  $temp = !$user->theme ? $conf['themes'][$conf['default_theme']] : $conf['themes'][$user->theme];
  $temp['path'] = DIDA_ROOT . '/' . $temp['path'];
  
  $args = func_get_args();
  $hook = array_shift($args);
  
  if (!$conf['theme_cache']) {
    theme_get_cache();
  }
  
  if (!$theme = $conf['theme_cache'][$hook]) {
    dd_set_message(t('system', '主题函数 %string 不存在', array('%string' => $hook)), 'error');
    return false;
  }
  
  $module_path = DIDA_ROOT . '/' . $conf['modules'][$theme['module']]['path'];
  
  /**
   * 包含文件
   * file：加载文件，相对于模块目录
   */
  if ($theme['file']) {
    $filepath = DIDA_ROOT .'/'. $module_path . '/' . $theme['file'];
    if (is_file($filepath)) include_once $filepath;
  }
  
  /**
   * tpl：加载一个模板文件，必须保存在 modulename/tpl/ 下。
   */
 
  /**
   * 载入 tpl
   * 在模块文件夹，以 tpl 命名文件夹，在tpl 下可分多级目录。在主题文件夹覆写，以模块名称命名文件夹
   */
  if ($theme['tpl']) {
    
    $var = array();
    if (is_array($theme['args'])) {
      foreach ($theme['args'] as $key => $val) {
        if ($val == 'theme_config') {
          dd_set_message(t('system', 'hook_theme 中 args 参数不能以 theme_config 命名，这是一个保留字符'), 'error');
          continue;
        }
        $var[$val] = $args[$key];
      }
    }
    
    $var['theme_config']->temp = $temp;
    $var['theme_config']->tpl[] = $theme['tpl'];
    
    $args = array(&$var, $hook);
    
    /**
     * 主题或模块可对 theme 进行覆写，顺序如下：
     */
    $functions = array(
      'template_preprocess',
      'template_preprocess_'.$hook,
      $theme['module'].'_template_preprocess_'.$hook,
      $temp['filename']. '_preprocess_'. $hook
    );
    
    foreach ($functions as $function) {
      if (function_exists($function)) {
        call_user_func_array($function, $args);
      }
    }
    
    /**
     * tpl_call：根据传递的参数动态调用 tpl。回调函数，函数返回一个 tpl
     */
    if ($theme['tpl_call']) {
      if (function_exists($theme['tpl_call'])) {
        $var['theme_config']->tpl = call_user_func_array($theme['tpl_call'], $args);
      }
    }
    
    
    if (!is_array($var['theme_config']->tpl)) {
      $var['theme_config']->tpl = array($var['theme_config']->tpl);
      $tpl_count = 1;
    } else {
      $tpl_count = count($var['theme_config']->tpl);
    }
    
    if ($tpl_count > 1) {
      $var['theme_config']->tpl = array_reverse($var['theme_config']->tpl);
    }
    
    if ($hook == 'page') {
      foreach ($var['theme_config']->tpl as $tpl) {
        $tpl = $temp['path'] . '/' .$tpl;
        if (is_file($tpl)) {
          return theme_render_tpl($tpl, $var, $hook);
        }
      }
    }
    
    foreach ($var['theme_config']->tpl as $tpl) {
      /**
       * 在当前主题中覆盖。将 tpl 文件复制到主题文件夹下，必须以模块名为目录名，即：themename/modulename/
       */
      if (is_file($temp['path'].'/'.$theme['module'].'/'.$tpl)) {
        
        return theme_render_tpl($temp['path'].'/'.$theme['module'].'/'.$tpl, $var, $hook);
        
      } else if (is_file($module_path.'/tpl/'.$tpl)) {
        /**
         * 调用模块目录中的 tpl
         */
        return theme_render_tpl($module_path.'/tpl/'.$tpl, $var, $hook);
        
      }
    }
    
  } else if ($theme['fun']) {
    /**
     * 回调函数处理 theme 请求
     * template.php 可覆写，如：default_template_table
     */
    $functions = array($temp['filename']. '_template_'. $hook, $theme['fun']);
    foreach ($functions as $function) {
      if (function_exists($function)) {
        return call_user_func_array($function, $args);
      }
    }
  }
}

/**
 * 传递给所有模板文件的变量，以 site_ 开头
 * 模块应尽量避免使用 site_ 开头来命名模板变量
 */
function template_preprocess(&$var, $hook) {
  static $count = array();
  $count[$hook] = isset($count[$hook]) && is_int($count[$hook]) ? $count[$hook] : 1;
  $var['zebra'] = ($count[$hook] % 2) ? 'odd' : 'even';
  $var['id'] = $count[$hook]++;
  
  if ($info = var_get('site_global')) {
    $var['site_global'] = (object) $info;
  }
  
  $var['site_global']->logo = $GLOBALS['conf']['base_logo'];
  $var['site_global']->favicon = $GLOBALS['conf']['base_favicon'];
  
  global $user;
  
  if ($user->uid > 0) {
    $var['is_login'] = 1;
  } else {
    $var['is_login'] = 0;
  }
  $var['base_theme'] = $GLOBALS['conf']['base_theme'];
  $var['base_path'] = $GLOBALS['base_path'];
}

function template_preprocess_page(&$var, $args) {
  global $conf, $user, $base_path;
  
  module_alter_all('template_preprocess_page', $var, $args);
  
  dd_set_html_head('<link rel="shortcut icon" href="'.$conf['base_favicon'].'" type="image/x-icon" />');
  dd_set_html_head('<link rel="bookmark" href="'.$conf['base_favicon'].'"/>');
  
  if ($block = dd_get_block()) {
    foreach ($block as $reg => $lists) {
      $var[$reg] = $lists;
    }
  }
  
  $var['menu'] = dd_get_menu();
  $var['head_title'] = dd_get_title();
  $var['messages'] = dd_get_message();
  $var['breadcrumb'] = dd_get_breadcrumb();
  $var['help'] = dd_get_help();
  $var['heads'] = dd_get_html_head();
  $var['styles'] = dd_get_css();
  $var['scripts'] = dd_get_js('header');
  $var['closure'] = dd_get_js('footer');
  $var['tabs'] = dd_get_tabs();
  $var['sub_tabs'] = dd_get_sub_tabs();
  $var['footer'] = dd_get_footer();
  
  if ($_SESSION['database_debug']) {
    
    $table = $_SESSION['database_debug'];
    
    if ($_SESSION['database_debug_count']) {
      $table[][] = array(
        'data' => theme('table', array(), $_SESSION['database_debug_count']),
        'colspan' => 3
      );
      unset($_SESSION['database_debug_count']);
    }
    
    $var['debug'] = theme('table', array(
      array('data' => 'sql('.(count($_SESSION['database_debug'])/2).')', 'class' => 'dd_database_debug_args0'),
      array('data' => 'args', 'class' => 'dd_database_debug_args1'),
      array('data' => 'options',  'class' => 'dd_database_debug_args2')
    ),
    $table, array('id' => 'dd_database_debug'));
    
    unset($_SESSION['database_debug']);
  }
  
  if (dd_is_front()) {
    $var['theme_config']->tpl[] = 'page_front.tpl.php';
    $var['args_id'] = 'page_wrapper_front';
  } else {
    $i = 0;
    $tpl = 'page';
    while ($arg = arg($i++)) {
      $arg = str_replace(array('/', '\\', '\0'), '', $arg);
      if (!is_numeric($arg)) {
        $args_id .= '_'.$arg;
        $args_value .= 'page_wrapper'.$args_id . ' ';
      }
      $var['theme_config']->tpl[] = $tpl .'_'. $arg . '.tpl.php';
      if (!is_numeric($arg)) {
        $tpl .= '_'. $arg;
      }
    }
    $var['args_id'] = rtrim($args_value, ' ');
  }
}

/**
 * 输出缓冲
 * @param (string) $tpl
 *  theme 文件
 * @param (array) $var
 *  取出的变量
 * @param (string) $hook
 *  theme 名称
 */
function theme_render_tpl($tpl, $var, $hook = NULL) {
  /**
   * php 执行信息，当前有执行时间，占用内存
   */
  if ($hook == 'page' && var_get('php_info', 1)) {
    if (var_get('php_info', 1) == 1 || $GLOBALS['user']->uid == 1) {
      // 向所有用户或 uid 为 1 的用户显示
      $var['debug'] .= '<p class="debug_php_info">';
      $var['debug'] .= '执行时间：' . (microtime(true)-DIDA_START_TIME) . ' 秒，';
      $size = memory_get_usage();
      $var['debug'] .= t('system', '占用内存 %size bytes，约为 %m MB', array(
        '%size' => $size,
        '%m' => round($size/1024/1024, 5)
      ));

      $var['debug'] .= '</p>';
    }
  }

  extract($var, EXTR_SKIP);

  ob_start();
  include $tpl;
  $contents = ob_get_contents();
  ob_end_clean();
 
  return $contents;
}

function theme_breadcrumb($breadcrumb) {
  return '<div class="breadcrumb">' . implode(' › ', $breadcrumb). '</div>';
}

function theme_help($help) {
  return '<div class="help">'.implode('', $help).'</div>';
}

function theme_item_list($items = array(), $title = NULL, $type = 'ul', $attributes = NULL, $wrapper = 1) {
  if (isset($attributes['class'])) {
    $class = ' item_'.$attributes['class'];
  }
  
  if ($wrapper) {
    $output = '<div class="item-list'.$class.'">';
  }
  
  if (isset($title)) {
    $output .= '<h3>'. $title .'</h3>';
  }
  
  if (!empty($items)) {
    $items = array_filter($items);
    $output .= "<$type". dd_attributes($attributes) .'>';
    $num_items = count($items);
    foreach ($items as $i => $item) {
      $attributes = array();
      $children = array();
      if (is_array($item)) {
        foreach ($item as $key => $value) {
          if ($key == 'data') {
            $data = $value;
          } elseif ($key == 'children') {
            $children = $value;
          } else {
            $attributes[$key] = $value;
          }
        }
      } else {
        $data = $item;
      }
      if (count($children) > 0) {
        $data .= theme_item_list($children, NULL, $type, $attributes);
      }
      if ($i == 0) {
        $attributes['class'] = empty($attributes['class']) ? 'first' : ($attributes['class'] .' first');
      }
      if ($i == $num_items - 1) {
        $attributes['class'] = empty($attributes['class']) ? 'last' : ($attributes['class'] .' last');
      }
      switch ($type) {
        case 'ul':
          $output .= '<li'. dd_attributes($attributes) .'>'. $data ."</li>\n";
        break;
        case 'dl':
          $output .= '<dd'. dd_attributes($attributes) .'>'. $data ."</dd>\n";
        break;
        case 'div':
          $output .= '<span'. dd_attributes($attributes) .'>'. $data ."</span>\n";
      }
    }
    $output .= "</$type>\n";
  }
  if ($wrapper) {
    $output .= "</div>\n";
  }
  
  return $output;
}

function theme_table($header, $rows, $attributes = array(), $caption = NULL) {
  $output = '<table'. dd_attributes($attributes) .">\n";

  if (isset($caption)) {
    $output .= '<caption>'. $caption ."</caption>\n";
  }

  if (count($header)) {
    $ts = table_init($header);
    $output .= (count($rows) ? ' <thead><tr>' : ' <tr>');
    foreach ($header as $cell) {
      $cell = table_header($cell, $header, $ts);
      $output .= _theme_table_cell($cell, TRUE);
    }
    $output .= (count($rows) ? " </tr></thead>\n" : "</tr>\n");
  } else {
    $ts = array();
  }

  if (count($rows)) {
    $output .= "<tbody>\n";
    $flip = array('even' => 'odd', 'odd' => 'even');
    $class = 'even';
    foreach ($rows as $number => $row) {
      $attributes = array();

      if (isset($row['data'])) {
        foreach ($row as $key => $value) {
          if ($key == 'data') {
            $cells = $value;
          } else {
            $attributes[$key] = $value;
          }
        }
      } else {
        $cells = $row;
      }
      if (count($cells)) {
        $class = $flip[$class];
        if (isset($attributes['class'])) {
          $attributes['class'] .= ' '. $class;
        } else {
          $attributes['class'] = $class;
        }
        $output .= ' <tr'. dd_attributes($attributes) .'>';
        $i = 0;
        foreach ($cells as $cell) {
          $cell = table_cell($cell, $header, $ts, $i++);
          $output .= _theme_table_cell($cell);
        }
        $output .= " </tr>\n";
      }
    }
    $output .= "</tbody>\n";
  }

  $output .= "</table>\n";
  return $output;
}

/*
  table 格式化
*/

function table_init($header) {
  $ts = table_get_order($header);
  $ts['sort'] = table_get_sort($header);
  $ts['query_string'] = table_get_querystring();
  return $ts;
}

/**
 * 获得 table 排序 sql
 * @param (array) $header
 *  table header
 * @param (string) $before
 *  在 by 之后添加内容，比如首先按照某个字段排序
 * @return (string)
 */
function table_sql($header, $before = '') {
  $ts = table_init($header);
  if ($ts['sql']) {
    $field = preg_replace('/[^A-Za-z0-9_.]+/', '', $ts['sql']);

    $sort = mb_strtoupper($ts['sort']);
    $sort = in_array($sort, array('ASC', 'DESC')) ? $sort : '';
    
    return " ORDER BY $before $field $sort";
  }
}

function table_header($cell, $header, $ts) {
  if (is_array($cell) && isset($cell['field'])) {
    $title = t('system', '按 !title排序', array('!title' => $cell['data']));
    if ($cell['data'] == $ts['name']) {
      
      $class = 'header_ac';
      if ($ts['sort'] == 'asc') {
        $ts['sort'] = 'desc';
        $class .= ' sort_active_desc';
      } else {
        $ts['sort'] = 'asc';
        $class .= ' sort_active_asc';
      }
      
      if (isset($cell['class'])) {
        $cell['class'] .= ' active';
      } else {
        $cell['class'] = 'active';
      }
      
    } else {
      $ts['sort'] = 'desc';
    }
    
    if (!empty($ts['query_string'])) {
      $ts['query_string'] = '&'. $ts['query_string'];
    }
    
    $cell['data'] = l($cell['data'], $_GET['q'], array('attributes' => array('title' => $title, 'class' => $class), 'query' => 'sort='. $ts['sort'] .'&order='. urlencode($cell['data']) . $ts['query_string'], 'html' => TRUE));

    unset($cell['field'], $cell['sort']);
  }
  return $cell;
}

function table_cell($cell, $header, $ts, $i) {
  if (isset($header[$i]['data']) && $header[$i]['data'] == $ts['name'] && !empty($header[$i]['field'])) {
    if (is_array($cell)) {
      if (isset($cell['class'])) {
        $cell['class'] .= ' active';
      } else {
        $cell['class'] = 'active';
      }
    } else {
      $cell = array('data' => $cell, 'class' => 'active');
    }
  }
  return $cell;
}

function table_get_querystring() {
  return dd_query_string_encode($_REQUEST, array_merge(array('q', 'sort', 'order', 'page'), array_keys($_COOKIE)));
}

function table_get_order($headers) {
  $order = isset($_GET['order']) ? $_GET['order'] : '';
  foreach ($headers as $header) {
    if (isset($header['data']) && $order == $header['data']) {
      return array('name' => $header['data'], 'sql' => isset($header['field']) ? $header['field'] : '');
    }

    if (isset($header['sort']) && ($header['sort'] == 'asc' || $header['sort'] == 'desc')) {
      $default = array('name' => $header['data'], 'sql' => isset($header['field']) ? $header['field'] : '');
    }
  }

  if (isset($default)) {
    return $default;
  } else {
    if (is_array($headers[0])) {
      $headers[0] += array('data' => NULL, 'field' => NULL);
      return array('name' => $headers[0]['data'], 'sql' => $headers[0]['field']);
    } else {
      return array('name' => $headers[0]);
    }
  }
}

function table_get_sort($headers) {
  if (isset($_GET['sort'])) {
    return ($_GET['sort'] == 'desc') ? 'desc' : 'asc';
  } else {
    foreach ($headers as $header) {
      if (is_array($header) && array_key_exists('sort', $header)) {
        return $header['sort'];
      }
    }
  }
  return 'asc';
}

function _theme_table_cell($cell, $header = FALSE) {
  $attributes = '';
  if (is_array($cell)) {
    $data = isset($cell['data']) ? $cell['data'] : '';
    $header |= isset($cell['header']);
    unset($cell['data']);
    unset($cell['header']);
    $attributes = dd_attributes($cell);
  } else {
    $data = $cell;
  }

  if ($header) {
    $output = "<th$attributes>$data</th>";
  } else {
    $output = "<td$attributes>$data</td>";
  }

  return $output;
}
