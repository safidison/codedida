<?php
// $Id$
?>
var swfu;
window.onload = function() {
  var swfu_o = {};
  swfu_o.session = "<?php echo session_id(); ?>";
  <?php
    if($value['opt'])
    foreach($value['opt'] as $key => $val){
      if($val){
        echo 'swfu_o.'. $key  . '= "' . $val . '";';
      }
    }
  ?>
	var settings = {
		flash_url : "/misc/swfupload/swfupload.swf",
		upload_url: "<?php echo $value['url']; ?>",
		post_params: swfu_o,
		file_size_limit : "<?php if(!$value['size_limit']) { echo '100'; }else {echo $value['size_limit'];}  ?> MB",
		file_types : "<?php if(!$value['type']) { echo '*.jpg;*.png;*.gif;*.jpeg'; }else {echo $value['type'];}  ?>",
		file_types_description : "<?php if(!$value['des_text']) { echo '选择图片…'; }else {echo $value['des_text'];}  ?>",
		file_upload_limit : <?php if(!$value['upload_limit']) { echo 100; }else {echo $value['upload_limit'];}  ?>,
		file_queue_limit : 0,
		custom_settings : {
			progressTarget : "<?php echo $value['target'];  ?>",
			cancelButtonId : "<?php echo $value['cancel'];  ?>"
		},
    auto_upload: false,
		debug: false,
    
		button_image_url: "/misc/swfupload/TestImageNoText_65x29.png",
		button_width: "65",
		button_height: "29",
		button_placeholder_id: "<?php echo $value['flash_id'];  ?>",
		button_text: '<span class="theFont"><?php if(!$value['text']) { echo '点此选择'; }else {echo $value['text'];}  ?></span>',
		button_text_style: ".theFont { font-size: 12; color: #AAA;}",
		button_text_left_padding: 7,
		button_text_top_padding: 4,
		
		file_queued_handler : fileQueued,
		file_queue_error_handler : fileQueueError,
		file_dialog_complete_handler : fileDialogComplete,
		upload_start_handler : uploadStart,
		upload_progress_handler : uploadProgress,
		upload_error_handler : uploadError,
		upload_success_handler : uploadSuccess,
		upload_complete_handler : uploadComplete,
		queue_complete_handler : queueComplete
	};
	swfu = new SWFUpload(settings);
};