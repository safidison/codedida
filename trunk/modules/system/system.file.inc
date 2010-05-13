<?php
// $Id$

/**
 * 文件上传处理
 */
function system_file_upload(){
  switch($_GET['op']){
    case 'ckeditor': case 'swfupload':
      if($_GET['name'] && $_FILES[$_GET['name']]){
        
        if($_GET['image_size']){
          $validate['image']['size'] = $_GET['image_size'];
        }else if($_GET['type'] == 'image'){
          $validate['image'] = true;
        }
        
        $file = file_save_upload($_FILES[$_GET['name']], $validate);
        
        if($_GET['op'] == 'ckeditor'){
          if($file->filepath) {
            $filepath = f($file->filepath);
          }else{
            $error = t('system', '上传失败');
          }
          echo _dd_js_print('window.parent.CKEDITOR.tools.callFunction('.$_GET['CKEditorFuncNum'].', "'.$filepath.'", "'.$error.'")');
        }else{
          if($file->filepath) {
            echo f($file->filepath);
          }else{
            header("HTTP/1.1 404 Internal Server Error");
          }
        }
      }
    exit;
    case 'ckeditorbrowser': case 'custombrowser':
      if(user_access('file one view')){
      	global $user;
      	if($fetch = pager_query("SELECT fid, filepath, filename FROM {files} WHERE uid = ? AND filemime LIKE ? ORDER BY fid DESC", 20, 0, NULL, array($user->uid, 'image/%'))){
        	foreach($fetch as $o){
        		$thumb = image_get_thumb($o->fid, $o->filepath, '85x85');
        		$item[] = '<span class="files_view_editor_img">'.img($thumb) . '</span><span class="files_view_'.$_GET['op'].'_insert">'.l('插入小图', f($thumb), array('attributes' => array('title' => $o->filename, 'target' => '_blank', 'link' => f($o->filepath)))).l('插入大图', f($o->filepath), array('attributes' => array('title' => $o->filename, 'target' => '_blank'))).'</span>';
        	}
      		$output = theme('item_list', $item, NULL, 'ul', array('id' => 'files_view_editor'));
      		$output .= pager_view();
        }else{
          $output = system_no_content();
        }
      	echo theme('print', $output);
      	exit;
      }
  }
}