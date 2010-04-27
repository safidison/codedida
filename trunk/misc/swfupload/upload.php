<?php
	if (isset($_POST["PHPSESSID"])) {
		session_id($_POST["PHPSESSID"]);
	}
	session_start();
  
  if($_FILES['Filedata']['size'] > 30000){
    // 上传失败
  	// 403 权限不足
  	// 404 类型不符
  	// 405 文件过大
  	// 406 文件尺寸过小
  	//header("HTTP/1.1 404 Internal Server Error");
  }
  
  // 上传成功，返回图片路径
  echo 'http://file.100fo.com/upload/up_files/topic/front_content3_list_4601.gif';
	exit(0);
?>