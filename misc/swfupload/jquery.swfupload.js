var Su = {};
(function($) {
	Su.endFile = {};
	Su.stopUpload = {};
	Su.loaded = {};
  Su.callFunc = function(cb) {
  	if (typeof cb == 'function') {
  		eval(cb);
  	} else {
      var func;
      func = eval(cb);
      return func.apply(null, Array.prototype.slice.call(arguments, 1));
    }
  }
	var defaultHandlers = ['swfupload_loaded_handler','file_queued_handler','file_queue_error_handler','file_dialog_start_handler','file_dialog_complete_handler','upload_start_handler','upload_progress_handler','upload_error_handler','upload_success_handler','upload_complete_handler','queue_complete_handler'];
	var additionalHandlers = [];
	
	$.fn.swfupload = function() {
		var args = $.makeArray(arguments);
		return this.each(function() {
			var swfu;
			if (args.length == 1 && typeof(args[0]) == 'object') {
				swfu = $(this).data('__swfu');
				if (!swfu) {
					var settings = args[0];
					
					var $magicUploadControl = $(this);
					var handlers = [];
					$.merge(handlers, defaultHandlers);
					$.merge(handlers, additionalHandlers);
					$(handlers).each(function(i, v) {
						settings[v] = eval(settings[v]);
					})
					$(this).data('__swfu', new SWFUpload(settings));
				}
			} else if (args.length > 0 && typeof(args[0]) == 'string') {
				var methodName = args.shift();
				swfu = $(this).data('__swfu');
				if (swfu && swfu[methodName]) {
					swfu[methodName].apply(swfu, args);
				}
			}
		});
	};
	
	$.swfupload = {
		additionalHandlers: function() {
			if (arguments.length === 0) {
				return additionalHandlers.slice();
			} else {
				$(arguments).each(function(i, v) {
					$.merge(additionalHandlers, $.makeArray(v));
				});
			}
		},
		defaultHandlers: function() {
			return defaultHandlers.slice();
		},
		getInstance: function(el) {
			return $(el).data('__swfu');
		}
	};
	
	Su.cancelUpload = function(movieName, fileID) {
	  var self = SWFUpload.instances[movieName];
	  self.cancelUpload(fileID, false);
	  var d = '#' + fileID;
	  $(d + ' .su_file_status_queued').text('已取消');
	  $(d + ' .su_file_op_queued').html(Su.getDel(d));
	  Su.setCount(self);
	}
	
	//初始化完成
	Su.swfLoaded = function() {
	  var s = this.customSettings;
	  if (Su.loaded[this.movieName] != 1) {
	  	var html = '<div class="su_files_table fieldset-wrapper"><table><thead><tr>';
	  	if (!s.not_filename) {
	  		html += '<th>文件名称</th>';
	  	}
	  	if (!s.not_filetype) {
	  		html += '<th>文件类型</th>';
	  	}
	  	if (!s.not_filesize) {
	  		html += '<th>文件大小</th>';
	  	}
	  	if (!s.not_status) {
	  		html += '<th>状态</th>';
	  	}
	  	if (!s.not_op) {
	  		html += '<th>操作</th>';
	  	}
	  	html += '</tr><thead><tbody></tbody></table></div>';
	  	
	  	$(s.fileLists).append(html + '<div class="su_status">请选择文件</div><div class="su_loading"></div><div class="su_loading_mark"></div>');
		  var opt = {'disabled' : true, 'alt' : this.movieName};
		  $(s.wrapper +' .su_start_upload').attr(opt).addClass('su_button_disabled').click(function() {
		  	Su.startUpload($(this).attr('alt'));
		  });
		  $(s.wrapper +' .su_stop_upload').attr(opt).addClass('su_button_disabled').click(function() {
		  	Su.stopUpload($(this).attr('alt'));
		  });
		}
	  Su.stopUpload[this.movieName] = 0;
	  Su.loaded[this.movieName] = 1;
	}

	//选择框打开前
	Su.dialogStart = function() {
	}

	//选择框关闭，成功文件
	Su.fileQueued = function(file) {
		Su.endFile[this.movieName] = file;
	  var s = this.customSettings;
	  var r = 0;
	  $(s.fileLists + ' .su_file_name_queued').each(function() {
	    if (file.name == $(this).text()) {
	      r = 1
	      return false;
	    }
	  });
	  var c = '';
	  if (r != 0) {
	  	c = ' su_file_list_cancel';
	  }
	  if (file.index%2 == 0) {
	  	c += ' even';
	  } else {
	  	c += ' odd';
	  }
	  var html = '<tr class="su_file_list su_file_list_queued'+c+'" id="'+file.id+'">';
	  if (!s.not_filename) {
	  	html += '<td class="su_file_name su_file_name_queued">'+file.name+'</td>';
	  }
	  if (!s.not_filetype) {
	  	html += '<td class="su_file_type su_file_type_queued">'+file.type+'</td>';
	  }
	  if (file.size) {
	  	if (!s.not_filesize) {
	  		html += '<td class="su_file_size su_file_size_queued">'+(file.size/1024).toFixed(2)+' k</td>';
	  	}
	    if (r == 0) {
	      html += '<td class="su_file_status su_file_status_queued">等待上传</td>';
	      html += '<td class="su_file_op su_file_op_queued"><a href="javascript:void(0);" class="su_cancel_upload" alt="'+file.id+'" title="取消" onclick="Su.cancelUpload(\''+this.movieName+'\', \''+file.id+'\');">取消</a></td>';
	    } else {
	      html += '<td class="su_file_status_error">已取消(<span class="su_error_msg">已在队列中</span>)</td>';
	      html += '<td class="su_file_op_queued">'+Su.getDel('#' + file.id)+'</td>';
	      this.cancelUpload(file.id, false);
	    }
	  } else {
	  	if (!s.not_filesize) {
	  		html += '<td class="su_file_size su_file_size_queued">0 k</td>';
	  	}
	    this.cancelUpload(file.id, false);
	    html += '<td class="su_file_status_error">已取消(<span class="su_error_msg">空文件</span>)</td>';
	    html += '<td class="su_file_op_queued">'+Su.getDel('#' + file.id)+'</td>';
	  }
	  html += '</tr>';
	  $(s.fileLists + ' tbody').append(html);
	}

	//选择框关闭，失败文件
	Su.fileQueueError = function(file, error, message) {
		Su.endFile[this.movieName] = file;
	  if (file !== null) {
	    var s = this.customSettings;
	    var html = '<tr class="su_file_list su_file_list_queued" id="'+file.id+'">';
	    if (!s.not_filename) {
	    	html += '<td class="su_file_name su_file_name_queued">'+file.name+'</td>';
	    }
	    if (!s.not_filetype) {
	    	html += '<td class="su_file_type su_file_type_queued">'+file.type+'</td>';
	    }
	    
	    if (!s.not_filesize) {
		    if (file.size) {
		      html += '<td class="su_file_size su_file_size_queued">'+(file.size/1024).toFixed(2)+' k</td>';
		    } else {
		      html += '<td class="su_file_size su_file_size_queued">0 k</td>';
		    }
	    }
	    if (!s.not_filestatus) {
	    	html += '<td class="su_file_status su_file_status_queued" colspan="2">加入失败(<span class="su_error_msg">'+Su.error(this, error)+'</span>)</td>';
	    }
	    html += '</tr>';
	    $(s.fileLists + ' tbody').append(html);
	  } else {
	    alert(Su.error(this, error));
	  }
	}

	//选择框关闭，文件总数
	Su.dialogComplete = function(sum, queued) {
	  if (sum > 0) {
	    Su.setCount(this);
	    if (this.getStats().files_queued > 0) {
	    	$(this.customSettings.wrapper + ' .su_start_upload').attr('disabled', false).removeClass('su_button_disabled');
	    }
	  }
	}

	//上传启动前
	Su.uploadStart = function(file) {
		
	}

	//开始上传
	Su.startUpload = function(movieName) {
		var self = SWFUpload.instances[movieName];
		Su.stopUpload[movieName] = 0;
	  self.startUpload();
	  $(self.customSettings.wrapper + ' .su_start_upload').attr('disabled', true).addClass('su_button_disabled');
	  $(self.customSettings.wrapper + ' .su_stop_upload').attr('disabled', false).removeClass('su_button_disabled');
	  $(self.customSettings.fileLists + ' .su_loading').show();
	}

	//停止上传
	Su.stopUpload = function(movieName) {
		var self = SWFUpload.instances[movieName];
	  Su.stopUpload[movieName] = 1;
	  $(self.customSettings.wrapper + ' .su_stop_upload').attr('disabled', true).addClass('su_button_disabled');
	  $(self.customSettings.wrapper + ' .su_start_upload').attr('disabled', false).removeClass('su_button_disabled');
	}

	//上传中
	Su.uploadProgress = function(file, bytes, total) {
		var w = parseInt(bytes/total*100) + '%';
		var html = '正在上传 ' + file.name + '<span class="su_loading_percentage">' + w +'</span><span class="su_loading_bytes">'+ bytes + '</span>/<span class="su_loading_total">' + total + '</span>';
		$(this.customSettings.fileLists + ' .su_loading').html(html);
		$(this.customSettings.fileLists + ' .su_loading_mark').css('width', w);
		$('#' + file.id).addClass('su_uploading');
	}

	//上传被终止或失败
	Su.uploadError = function(file, error, message) {
		var d = '#' + file.id;
	  $(d + ' .su_file_status_queued').html('上传失败(<span class="su_error_msg">'+Su.error(this, message)+'</span>)');
	  $(d + ' .su_file_op_queued').html(Su.getDel(d));
	  $(d).removeClass('su_uploading').addClass('su_upload_error');
  	if (this.customSettings.errorCall && this.customSettings.errorCall != 'undefined') {
  		Su.callFunc(this.customSettings.errorCall, file, error, message);
  	}
	  Su.setCount(this);
	}

	//上传成功
	Su.uploadSuccess = function(file, data) {
		var d = '#' + file.id;
	  $(d + ' .su_file_status_queued').text('上传成功');
	  $(d + ' .su_file_op_queued').html(Su.getDel(d));
	  $(d).removeClass('su_uploading').addClass('su_upload_ok');
	  Su.setCount(this);
  	if (this.customSettings.successCall && this.customSettings.successCall != 'undefined') {
  		Su.callFunc(this.customSettings.successCall, file, data);
  	}
	}

	//一个周期完成，即单个文件操作完全结束
	Su.uploadComplete = function(file) {
		var html;
	  if (file.id != Su.endFile[this.movieName].id) {
	  	html = file.name + ' 上传成功！';
	  } else {
	  	html = '上传完毕！';
	  	$(this.customSettings.wrapper +' .su_start_upload').attr('disabled', true).addClass('su_button_disabled');
	  	$(this.customSettings.wrapper +' .su_stop_upload').attr('disabled', true).addClass('su_button_disabled');
	  	if (this.customSettings.endCall && this.customSettings.endCall != 'undefined') {
	  		Su.callFunc(this.customSettings.endCall, this);
	  	}
	  }
	  $(this.customSettings.fileLists + ' .su_loading_mark').css('width', '0px');
	  $(this.customSettings.fileLists + ' .su_loading').html(html);
	  if (Su.stopUpload[this.movieName] == 0) {
	  	this.startUpload();
	  }
	  var top = parseInt($('.su_files_table').attr('scrollTop'));
	  $('.su_files_table').attr('scrollTop', (top + 20));
	}
	
	Su.uploadQueueComplete = function(obj) {
		alert('结束');
		alert(obj);
	}
	
	//debug
	Su.debug = function(message) {
	}

	Su.error = function(self, code) {
	  var m = '';
	  switch (code) {
	    case SWFUpload.QUEUE_ERROR.QUEUE_LIMIT_EXCEEDED:
	      m = '文件过多，最多允许 ' + self.settings.file_upload_limit + ' 个';
	    break;
	    case '405': case SWFUpload.QUEUE_ERROR.FILE_EXCEEDS_SIZE_LIMIT:
	      m = '最大 ' + self.settings.file_size_limit;
	    break;
	    case '404':
	    	m = '类型不符，允许 ' + self.settings.file_types;
	    break;
	    case '406':
	    	m = '图片尺寸过小';
	    break;
	    case '403':
	    	m = '权限不足';
	    break;
	    default: 
	      m = '上传错误';
	  }
	  return m;
	}

	Su.wrapperNum = function(str) {
	  return '<span class="su_num">' + str +'</span>';
	}

	Su.setCount = function(self) {
	  var s = self.getStats();
	  var html = '等待上传(' + Su.wrapperNum(s.files_queued)+')';
	  if (s.queue_errors) {
	    html += '，加入失败('+Su.wrapperNum(s.queue_errors)+')';
	  }
	  if (s.upload_cancelled) {
	    html += '，已取消('+Su.wrapperNum(s.upload_cancelled)+')';
	  }
	  if (s.successful_uploads) {
	    html += '，上传成功('+Su.wrapperNum(s.successful_uploads)+')';
	  }
	  if (s.upload_errors) {
	    html += '，上传失败('+Su.wrapperNum(s.upload_errors)+')';
	  }
	  $(self.customSettings.fileLists + ' .su_status').html(html);
	}
	Su.getDel = function(c) {
		return '<a href="javascript: void(0);" title="隐藏" onclick="Su.deleteTr(\''+c+'\');">隐藏</a>';
	}
	Su.deleteTr = function(c) {
		$(c).hide();
	}
})(jQuery);