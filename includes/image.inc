<?php
// $Id$

/**
 * 获取缩略图，若不存在，则生成，返回缩略图路径，若生成不成功，返回原图。原图不存在，返回错误图片
 * @param (int or string) $fid
 *  图片 id，一般指定 files 表 fid，也可自定义，用于创建缩略图存放目录
 * @param (string) $filepath
 *  图片路径，相对路径
 * @param (string) $size
 *  缩略图大小，如：100x100
 * @param (string) $style
 *  缩略图风格，scale、scale_resize、scale_and_crop、crop、resize 几种可选
 */
function image_get_thumb($fid, $filepath, $size, $style = 'scale') {
  static $files;
  if (!isset($files[$fid][$size][$style])) {
    if (is_file($filepath)) {
      $files[$fid][$size][$style] = $filepath;
      
      $filename = end(explode('/', $filepath));
      $ext = end(explode('.', basename($filepath)));
      $thumb = $size. '_'.$style;
      $thumb_filepath = file_mkdir(str_replace($filename, $fid, $filepath)) .'/'.$thumb .'.'. $ext;
      if (is_file($thumb_filepath)) {
        $files[$fid][$size][$style] = $thumb_filepath;
      } else {
        $t = explode('x', $size);
        if (is_file(file_directory_path(). '/org/'.$fid)) {
          $filepath = file_directory_path(). '/org/'.$fid;
        }
        $function = 'image_' . $style;
        if (function_exists($function)) {
          $true = $function($filepath, $thumb_filepath, $t[0], $t[1]);
        } else {
          $true = image_scale_resize($filepath, $thumb_filepath, $t[0], $t[1]);
        }
        if ($true) {
          $files[$fid][$size][$style] = $thumb_filepath;
        }
      }
    } else {
      $s = explode('x', $size);
      $files[$fid][$size][$style] = 'images/error?width='.$s[0].'&height='.$s[1];
    }
  }
  return $files[$fid][$size][$style];
}

function image_get_info($file) {
  $details = false;
  $data = @getimagesize($file);
  $file_size = @filesize($file);

  if (isset($data) && is_array($data)) {
    $extensions = array('1' => 'gif', '2' => 'jpg', '3' => 'png');
    $extension = array_key_exists($data[2], $extensions) ?  $extensions[$data[2]] : '';
    $details = array('width'     => $data[0],
                     'height'    => $data[1],
                     'extension' => $extension,
                     'file_size' => $file_size,
                     'mime_type' => $data['mime']);
  }
  return $details;
}

function image_scale_resize($source, $destination, $width, $height, $mark = NULL) {
  $info = image_get_info($source);
  $aspect = $info['height'] / $info['width'];
  if ($aspect != $height / $width) {
    if ($info['width'] >= $width || $info['height'] >= $height) {
      if (image_scale($source, $destination, $width, $height, $mark)) {
        $source = $destination;
        $info = image_get_info($destination);
      }
    }
    $dst = image_gd_open($source, $info['extension']);
    if (!$dst) {
      return false;
    }
    $im = imagecreatetruecolor($width, $height);
    $color = imagecolorallocate($im, 255, 255, 255);
    imagefill($im, 0, 0, $color);
    
    $dst_x = ($width - $info['width']) / 2;
    $dst_y = ($height - $info['height']) / 2;
    
    imagecopy($im, $dst, $dst_x, $dst_y, 0, 0, $info['width'], $info['height']);
    $result = image_gd_close($im, $source, $info['extension']);
    
    imagedestroy($im);
    imagedestroy($dst);
    return $result;
  } else {
    return image_gd_resize($source, $destination, $width, $height, $mark);
  }
}

function image_scale_and_crop($source, $destination, $width, $height, $mark = NULL) {
  $info = image_get_info($source);

  $scale = max($width / $info['width'], $height / $info['height']);
  $x = round(($info['width'] * $scale - $width) / 2);
  $y = round(($info['height'] * $scale - $height) / 2);

  if (image_gd_resize($source, $destination, $info['width'] * $scale, $info['height'] * $scale, 1)) {
    return image_gd_crop($destination, $destination, $x, $y, $width, $height, $mark);
  }
  return false;
}

function image_scale($source, $destination, $width, $height, $mark = NULL) {
  $info = image_get_info($source);
  
  if ($width >= $info['width'] && $height >= $info['height']) {
    return false;
  }
  $aspect = $info['height'] / $info['width'];
  if ($aspect < $height / $width) {
    $width = (int)min($width, $info['width']);
    $height = (int)round($width * $aspect);
  } else {
    $height = (int)min($height, $info['height']);
    $width = (int)round($height / $aspect);
  }
  
  return image_gd_resize($source, $destination, $width, $height, $mark);
}

function image_resize($source, $destination, $width, $height, $mark = NULL) {
  return image_gd_resize($source, $destination, $width, $height, $mark);
}

function image_rotate($source, $destination, $degrees, $background = 0x000000) {
  return image_gd_rotate($source, $destination, $degrees, $background);
}

function image_crop($source, $destination, $x, $y, $width, $height, $mark = NULL) {
  return image_gd_crop($source, $destination, $x, $y, $width, $height, $mark);
}

/**
 * 转换图片格式
 * @param (string) $source
 *  原图片
 * @param (string) $ext
 *  原扩展名
 * @param (string) $newext
 *  新扩展名
 * @param (string) $newsource
 *  新的位置
 */
function image_to_type_data($source, $ext, $newext, $newsource) {
  if ($ext != $newext) {
    $function = 'imagecreatefrom'.$ext;
    $functionnew = 'imagecreatefrom'.$newext;
    if (function_exists($function) && function_exists($functionnew)) {
      if ($im = $function($source)) {
        $functionnew($im, $newsource);
        imagedestroy($im);
      }
    }
  }
}

/**
 * GD 库接口
 */

/**
 * 裁剪，固定大小
 */
function image_gd_resize($source, $destination, $width, $height, $mark = false) {
  if (!file_exists($source)) {
    return false;
  }
  $info = image_get_info($source);
  if (!$info) {
    return false;
  }
  $im = image_gd_open($source, $info['extension']);
  if (!$im) {
    return false;
  }
  $res = imagecreatetruecolor($width, $height);
  if ($info['extension'] == 'png') {
    $transparency = imagecolorallocatealpha($res, 255, 255, 255, 127);
    imagealphablending($res, false);
    imagefilledrectangle($res, 0, 0, $width, $height, $transparency);
    imagealphablending($res, TRUE);
    imagesavealpha($res, TRUE);
  } elseif ($info['extension'] == 'gif') {
    $transparency_index = imagecolortransparent($im);
    if ($transparency_index >= 0) {
      $transparent_color = imagecolorsforindex($im, $transparency_index);
      $transparency_index = imagecolorallocate($res, $transparent_color['red'], $transparent_color['green'], $transparent_color['blue']);
      imagefill($res, 0, 0, $transparency_index);
      imagecolortransparent($res, $transparency_index);
      $number_colors = imagecolorstotal($im);
      imagetruecolortopalette($res, TRUE, $number_colors);
    }
  }
  imagecopyresampled($res, $im, 0, 0, 0, 0, $width, $height, $info['width'], $info['height']);
  /*
  if (!$mark && $width > 200 && $height > 200) {
    $watermark = imagecreatefrompng('misc/watermark.png');
    imagecopy($res, $watermark, $width-155, $height-35, 0, 0, 145, 25);
    imagedestroy($watermark);
  }
  */
  $result = image_gd_close($res, $destination, $info['extension']);

  imagedestroy($res);
  imagedestroy($im);

  return $result;
}

function image_gd_mark($filepath) {
  $info = image_get_info($filepath);
  if ($info['width'] > 200 && $info['height'] > 200) {
    $im = image_gd_open($filepath, $info['extension']);
    if (!$im) {
      return false;
    }
    $watermark = imagecreatefrompng('misc/watermark.png');
    imagecopy($im, $watermark, $info['width']-155, $info['height']-35, 0, 0, 145, 25);
    $result = image_gd_close($im, $filepath, $info['extension']);
    imagedestroy($watermark);
    imagedestroy($im);
    return $result;
  }
}

/**
 * 旋转图像
 */
function image_gd_rotate($source, $destination, $degrees, $background = 0xFFFFFF) {
  if (!function_exists('imageRotate')) {
    return false;
  }

  $info = image_get_info($source);
  if (!$info) {
    return false;
  }

  $im = image_gd_open($source, $info['extension']);
  if (!$im) {
    return false;
  }

  $res = imageRotate($im, $degrees, $background);
  $result = image_gd_close($res, $destination, $info['extension']);

  return $result;
}

/**
 * 裁剪图片
 */
function image_gd_crop($source, $destination, $x, $y, $width, $height, $mark = NULL) {
  $info = image_get_info($source);
  if (!$info) {
    return false;
  }

  $im = image_gd_open($source, $info['extension']);
  $res = imageCreateTrueColor($width, $height);
  imageCopy($res, $im, 0, 0, $x, $y, $width, $height);

  $result = image_gd_close($res, $destination, $info['extension']);

  imageDestroy($res);
  imageDestroy($im);

  return $result;
}

function image_gd_open($file, $extension) {
  $extension = str_replace('jpg', 'jpeg', $extension);
  $open_func = 'imageCreateFrom'. $extension;
  if (!function_exists($open_func)) {
    return false;
  }
  return $open_func($file);
}

function image_gd_close($res, $destination, $extension) {
  $extension = str_replace('jpg', 'jpeg', $extension);
  $close_func = 'image'. $extension;
  if (!function_exists($close_func)) {
    return false;
  }
  if ($extension == 'jpeg') {
    return $close_func($res, $destination, 100);
  } else {
    return $close_func($res, $destination);
  }
}