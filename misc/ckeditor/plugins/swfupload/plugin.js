CKEDITOR.plugins.add('swfupload',{
	init : function(a){
		a.addCommand('swfupload', new CKEDITOR.dialogCommand('swfupload'));
		a.ui.addButton( 'swfupload',{
				label : '批量上传',
				command : 'swfupload',
				icon : this.path + 'swfupload.gif'
			});
	}
});

var instanceName = null;

function ck_swfupload_error(file){
  window.setTimeout(function(){$('#' + file.id).slideUp('slow', function(){$(this).remove()});}, 3000); 
}

function ck_insert_img(src, title, alt, href){
	if(instanceName != null){
	  var a = CKEDITOR.instances[instanceName];
	  var html = '<img src="'+src+'" title="'+title+'" alt="'+alt+'" />';
	  if(href){
	    html = '<a href="'+href+'" target="_blank">'+html+'</a>';
	  }
	  a.insertHtml(html);
  }
}

function ck_swfupload_insert(file, path){
  ck_insert_img(path, file.name, file.name, 0);
  window.setTimeout(function(){$('#' + file.id).slideUp('slow', function(){$(this).remove()});}, 3000); 
}

CKEDITOR.dialog.add('swfupload', function (a) {
  return {
      title: '批量上传',
      minWidth: 650,
      minHeight: 300,
      contents: [{
          id: 'tab1',
          label: '',
          title: '',
          expand: true,
          padding: 0,
          elements: [{
              type: 'html',
              children: [{'width' : '650px', 'height' : '300px'}],
              html: '<div class="ckeditor_swfupload"><div id="swf_tabs"><a href="#" class="swf_view_click ck_tabs_ac">上传文件</a>'+(a.config.swf_Con.lists ? '<a href="#" class="files_view_click">浏览文件</a>' : '')+'</div><div id="files_view">loading</div><div class="ck_swfupload_wrapper"><div id="ck_swfupload"><div class="fieldset" id="fsUploadProgress"><span class="legend">文件队列</span></div><div class="su_content"><input type="button" class="su_stop_upload" value="暂停上传" /><input type="button" class="su_start_upload" value="开始上传" /><span id="swf_select_button"></span></div></div></div></div>'}]
      }],
      buttons: [CKEDITOR.dialog.okButton],
      onShow: function(){
        instanceName = a.name;
				$(function(){
          $('#fsUploadProgress tbody, .su_status, .su_loading').html('');
          $('.ck_swfupload_wrapper" input').attr('disabled', true).addClass('su_button_disabled');
          
          $('#files_view').hide().html('loading');
          $('.ck_swfupload_wrapper').show();
          $('.swf_view_click').addClass('ck_tabs_ac');
          $('.files_view_click').removeClass('ck_tabs_ac');
          
          $('.files_view_click').click(function(){
            
            $('#files_view').show();
            $('.ck_swfupload_wrapper').hide();
            
            $('#fsUploadProgress tbody, #fsUploadProgress .su_status').html('');
            $('.ck_swfupload_wrapper" input').attr('disabled', true).addClass('su_button_disabled');
            if($('#files_view').text() == 'loading'){
              var lists = a.config.swf_Con.lists || settings.base_path + 'misc/swfupload/lists.php';
              $('#files_view').html('<iframe src="'+lists+'" id="ck_files" name="ck_files"></iframe>');
            }
            $(this).addClass('ck_tabs_ac');
            $('.swf_view_click').removeClass('ck_tabs_ac');
            return false;
          });
          $('.swf_view_click').click(function(){
            $('#files_view').hide();
            $('.ck_swfupload_wrapper').show();
            $(this).addClass('ck_tabs_ac');
            $('.files_view_click').removeClass('ck_tabs_ac');
            return false;
          });
					$('.su_content').swfupload({
							flash_url : a.config.swf_Con.swf || settings.base_path + 'misc/swfupload/swfupload.swf',
							upload_url: a.config.swf_Con.url || settings.base_path + 'misc/swfupload/upload.php',
							post_params: a.config.swf_Con.params || {},
							file_size_limit : a.config.swf_Con.size_limit || "100 MB",
							file_types : a.config.swf_Con.types || '*.*',
							file_types_description : a.config.swf_Con.des || '所有文件',
							file_upload_limit : a.config.swf_Con.upload_limit || 100,
							file_queue_limit : a.config.swf_Con.queue_limit || 0,
							custom_settings : {
								wrapper : '#ck_swfupload',
				        fileLists: '#fsUploadProgress',
				        successCall: (a.config.swf_Con.successCall ? 'ck_swfupload_insert': 'undefined'),
                errorCall: 'ck_swfupload_error'
							},
				      minimum_flash_version : '9.0.28',
							debug: false,
							button_image_url:  a.config.swf_Con.button_image_url || settings.base_path + 'misc/swfupload/button.png',
							button_width: "70",
							button_height: "28",
							button_placeholder_id: 'swf_select_button',
				      button_placeholder : $('#swf_select_button')[0],
				      swfupload_loaded_handler: Su.swfLoaded,
				      file_dialog_start_handler: Su.dialogStart,
							file_queued_handler : Su.fileQueued,
							file_queue_error_handler : Su.fileQueueError,
				      file_dialog_complete_handler: Su.dialogComplete,
							upload_start_handler : Su.uploadStart, 
							upload_progress_handler : Su.uploadProgress,
							upload_error_handler : Su.uploadError,
							upload_success_handler : Su.uploadSuccess,
				      upload_complete_handler: Su.uploadComplete
				  });
				});
      }
  };
});