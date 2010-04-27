<?php
require_once 'ckeditor.inc';

$v['extraPlugins'] = 'mycolor,swfupload';
// swfupload 配置
$v['swf_Con'] = array('des' => '图片', 'types' => '*.gif;*.png;*.jpg', 'url' => 'upload.php', 'lists' => 'lists.php');

$v['toolbar'] = 'full';

$js = get_fck('body', $v);

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>上传</title>
<link href="swfupload/css.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="ckeditor/ckeditor.js"></script>
<script type="text/javascript" src="swfupload/swfupload.js"></script>
<script type="text/javascript" src="swfupload/jquery.swfupload.js"></script>
</head>
<body>
  <form id="form1" action="index.php" method="post" enctype="multipart/form-data">
    <textarea id="body" rows="20" cols="80" name="body"></textarea>
  </form>
  <script type="text/javascript">
  <!--//--><![CDATA[//><!--
  <?php echo $js; ?>
  //--><!]]>
  </script>
</body>
</html>
