<?php 
  switch ($_GET['op']) {
    case 'save':
      echo 'ok';
    exit;
    case 'upload':
      echo '{"filepath": "http://www.dongge.org/misc/images/logo.png", "title": "标题标题"}';
    exit;
    case 'list':
      echo '测试页面，没有文件。';
    exit;
    case 'preview':
      echo '测试';
    exit;
  }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>markItUp 演示 | demo</title>

<script type="text/javascript">
<!--//--><![CDATA[//><!--
var settings = {"cache_css_and_js_timestamp": 1280644983};
//--><!]]>
</script>

<script type="text/javascript" src="./../jquery.js"></script>
<script type="text/javascript" src="./../dida.js"></script>
<script type="text/javascript" src="./../jquery.dida.js"></script>
<script type="text/javascript" src="./../jquery.form.js"></script>
<script type="text/javascript" src="./../jquery.ajaxfileupload.js"></script>

<script type="text/javascript" src="jquery.markitup.js"></script>
<script type="text/javascript" src="sets/dida/set.js"></script>
<script type="text/javascript" src="./../jquery.bgiframe.js"></script>
<script type="text/javascript" src="./../ui/jquery.ui.js"></script>

<link rel="stylesheet" type="text/css" href="skins/dida/style.css" />
<link rel="stylesheet" type="text/css" href="/modules/system/system.css" />
<link type="text/css" rel="stylesheet" media="all" href="./../ui/themes/base/jquery.ui.css" />

</head>
<body>
<script type="text/javascript">
<!--
$(document).ready(function()  {
  $('#markItUp').markItUp(Dida.markitup.html({
  	filebrowserImageUploadUrl: '?op=upload',
  	filebrowserImageBrowseUrl: '?op=list',
  	filebrowserBrowseUrl: '?op=list',
  	filebrowserUploadUrl: '?op=upload',
    AutoSaveUrl: '?op=save',
    AutoSaveTime: 10000,
    options: {previewParserPath: '/custom.php?module=blog&op=preview&bootstrap=full&type=body'}
  }));
  
  $('#markItUp2').markItUp(Dida.markitup.html({
    toolbar: ['img', 'a']
  }));
});
-->
</script>

<form method="post" action="">
<textarea name="test" id="markItUp" cols="80" rows="20">
完整按钮，自动保存与文件上传为测试，不会保存在服务器。
[code=js]
$('#markItUp').markItUp(Dida.markitup.html({
  filebrowserImageUploadUrl: '?op=upload',
  filebrowserImageBrowseUrl: '?op=list',
  filebrowserBrowseUrl: '?op=upload',
  filebrowserUploadUrl: '?op=list',
  AutoSaveUrl: '?op=save',
  AutoSaveTime: 10000
}));
[/code]

[code=php]
// test
echo 'test';
[/code]
</textarea>
</form>

<form method="post" action="">
<textarea name="abcd" id="markItUp2" cols="80" rows="20">
自定义标签：
$('#markItUp2').markItUp(Dida.markitup.html({
  toolbar: ['img', 'a'],
}));
</textarea>
</form>

</body>
</html>
