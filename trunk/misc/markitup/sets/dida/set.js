// $Id$

Dida.markitup = {html: {}, tools: {}, cache: {}};

/*******html******/

Dida.markitup.html = function(obj) {
	var o = {
		options: {},
		extPreview: true
	};
	
	$.extend(o, obj);
	
	var opt = {
		nameSpace: 'html',
		onShiftEnter: {keepDefault:false, replaceWith:'<br />\n'},
		onCtrlEnter: {keepDefault:false, openWith:'\n<p>', closeWith:'</p>\n'},
		onTab: {keepDefault:false, openWith:''},
		previewAutoRefresh: false
	};
	
	var extSet = {
		code: function(h, type) {
			return '[code='+type+']\n' + h.selection + '\n[/code]\n';
		},
		ul: function(h, type) {
			var t = '<'+type+'>\n';
			
			if(h.selection) {
				var l = h.selection.split('\n');
				
				$(l).each(function(){
					if (this != '') {
						t += '  <li>'+this+'</li>\n';
					}
				});
				
			}
			
			t += '</'+type+'>\n';
			return t;
		}
	}
	
	// 按钮
	var sets = {
		h1: {name:'H1', key:'1', openWith:'<h1(!( class="[![Class]!]")!)>', closeWith:'</h1>'},
		h2: {name:'H2', key:'2', openWith:'<h2(!( class="[![Class]!]")!)>', closeWith:'</h2>'},
		h3: {name:'H3', key:'3', openWith:'<h3(!( class="[![Class]!]")!)>', closeWith:'</h3>'},
		h4: {name:'H4', key:'4', openWith:'<h4(!( class="[![Class]!]")!)>', closeWith:'</h4>'},
		h5: {name:'H5', key:'5', openWith:'<h5(!( class="[![Class]!]")!)>', closeWith:'</h5>'},
		h6: {name:'H6', key:'6', openWith:'<h6(!( class="[![Class]!]")!)>', closeWith:'</h6>'},
		p: {name:'P', openWith:'<p(!( class="[![Class]!]")!)>', closeWith:'</p>'  },
		strong: {name:'粗体', key:'B', openWith:'<strong>', closeWith:'</strong>' },
		em: {name:'斜体', key:'I', openWith:'<em>', closeWith:'</em>'  },
		u: {name:'下划线', key:'U', openWith:'<u>', closeWith:'</u>'  },
		del: {name:'删除线', key:'S', openWith:'<del>', closeWith:'</del>' },
		ul: {name:'无序列表', replaceWith: function(h) { return extSet.ul(h, 'ul'); }},
		ol: {name:'有序列表', replaceWith: function(h) { return extSet.ul(h, 'ol'); }},
		img: {name:'加图片', beforeInsert: function(h) { return Dida.markitup.tools.upload(h, o, 'image') } 	},
		a: {name:'超链接', beforeInsert: function(h) { return Dida.markitup.tools.upload(h, o, 'file') } },
		smiley: {name: '表情', beforeInsert: function(h) { return Dida.markitup.tools.smiley(h, o) }},
		blockquote: {name: '引用', key: 'Q', openWith: '<blockquote>', closeWith: '</blockquote>'},
		code: {name: '插入代码', dropMenu: 
			[
				 {name: 'PHP', replaceWith: function(h) { return extSet.code(h, 'php');} },
				 {name: 'HTML', replaceWith: function(h) { return extSet.code(h, 'html');} },
				 {name: 'ActionScript3', replaceWith: function(h) { return extSet.code(h, 'as3');} },
				 {name: 'Bash/shell', replaceWith: function(h) { return extSet.code(h, 'bash');} },
				 //{name: 'ColdFusion', replaceWith: function(h) { return extSet.code(h, 'cf');} },
				 {name: 'C#', replaceWith: function(h) { return extSet.code(h, 'csharp');} },
				 {name: 'C++', replaceWith: function(h) { return extSet.code(h, 'c');} },
				 {name: 'CSS', replaceWith: function(h) { return extSet.code(h, 'css');} },
				 {name: 'Delphi', replaceWith: function(h) { return extSet.code(h, 'delphi');} },
				 {name: 'Diff', replaceWith: function(h) { return extSet.code(h, 'diff');} },
				 //{name: 'Erlang', replaceWith: function(h) { return extSet.code(h, 'erl');} },
				 //{name: 'Groovy', replaceWith: function(h) { return extSet.code(h, 'groovy');} },
				 {name: 'JavaScript', replaceWith: function(h) { return extSet.code(h, 'js');} },
				 {name: 'Java', replaceWith: function(h) { return extSet.code(h, 'java');} },
				 {name: 'JavaFX', replaceWith: function(h) { return extSet.code(h, 'jfx');} },
				 {name: 'Perl', replaceWith: function(h) { return extSet.code(h, 'pl');} },
				 {name: 'Plain Text', replaceWith: function(h) { return extSet.code(h, 'plain');} },
				 //{name: 'PowerShell', replaceWith: function(h) { return extSet.code(h, 'ps');} },
				 {name: 'Python', replaceWith: function(h) { return extSet.code(h, 'py');} },
				 //{name: 'Ruby', replaceWith: function(h) { return extSet.code(h, 'ruby');} },
				 {name: 'SQL', replaceWith: function(h) { return extSet.code(h, 'sql');} },
				 {name: 'Visual Basic', replaceWith: function(h) { return extSet.code(h, 'vb');} },
				 {name: 'XML', replaceWith: function(h) { return extSet.code(h, 'xml');} }
			 ]},
		clean: {name:'清除格式', replaceWith:function(h) { return h.selection.replace(/<(.*?)>/g, "") }}
	}
	
	if (o.AutoSaveUrl) {
		sets.save = {name: o.AutoSaveName || '保存草稿', openWith: function(h) { Dida.markitup.tools.autoSave(h, o)}};
    window.setInterval(function() {
    	$.markItUp({openWith: function(h) { Dida.markitup.tools.autoSave(h, o)} });
    }, obj.AutoSaveTime || 60000);
	}
	
	if (!$.isEmptyObject(settings.markitupHtmlSets)) {
		// 添加全局按钮
		for (var attr in settings.markitupHtmlSets) {
			sets[attr] = settings.markitupHtmlSets[attr];
		}
	}
	
	if (o.sets) {
		// 添加自定义按钮
		for (var attr in o.sets) {
			sets[attr] = o.sets[attr];
		}
	}
	
	if (o.extPreview) {
		sets.preview = {name:'预览', closeWith: function(h) {Dida.markitup.tools.preview(h, o);}};
	} else {
		sets.preview = {name:'预览', call: 'preview'};
	}
	
	// 获取按钮
	opt.markupSet = Dida.markitup.tools.menus(sets, o.toolbar);
	
	if (!$.isEmptyObject(o.options)) {
		// 获取自定义编辑器配置，o.options 将直接覆盖默认配置
		for (var attr in o.options) {
			opt[attr] = o.options[attr];
		}
	}
	
	return opt;
}

/*******tools******/

/**
 * jquery ajaxfileupload 上传
 */
Dida.markitup.tools.ajaxUpload = function(obj, edit, op) {
	var url = '';
	if (op == 'image') {
		url = obj.filebrowserImageUploadUrl;
	} else {
		url = obj.filebrowserUploadUrl;
	}
	
	if (url == '') {
		return;
	}
	
	$.ajaxFileUpload({
		url: url,
		secureuri: false,
		fileElementId: 'markitup_file_form_field_upload',
		dataType: 'json',
		success: function (json, status){
			if (json.filepath) {
				
				$('#markitup_file_form_field_filepath').val(json.filepath);
				$('#markitup_file_form_field_upload').val("");
				
			} else {
				alert(json.error);
			}
		},
		error: function (data, status, e) {
			alert(e);
		}
	});
}

/**
 * 动态修改预览元素的位置
 */
Dida.markitup.tools.preview = function(edit, obj) {
	Dida.dialog({
		url: '',
		width: '95%',
		modal: true,
		open: function() {
			
			$(this).append('<div id="didaMarkItUpPreview" class="loading_image"></div>');
			
			if (!obj.options.previewParserPath ||( $.browser.msie &&  $.browser.version == '6.0')) {
				$('#didaMarkItUpPreview').hide();
				var data = $(edit.textarea).val();
				if (data) {
					data = data.replace(/\n/ig, '<br />');
				}
				$(this).append('<div id="didaMarkItUpPreviewBody">'+data+'</div>');
			} else {
				$(this).append('<iframe height="100%" width="100%" frameborder="no" border="0" name="didaMarkItUpPreviewIframe" id="didaMarkItUpPreviewIframe"></iframe>');
				$.ajax( {
					type: 'POST',
					url: obj.options.previewParserPath,
					data: 'data='+encodeURIComponent($(edit.textarea).val()),
					success: function(data) {
						var i;
						if (document.all){
							i = document.frames["didaMarkItUpPreviewIframe"].document;
						} else {
							i = document.getElementById("didaMarkItUpPreviewIframe").contentDocument;
						}
						i.open();
						i.write(data);
						i.close();
						i.documentElement.scrollTop = 0;
						$('#didaMarkItUpPreviewIframe').show();
						$('#didaMarkItUpPreview').hide();
					}
				});
			}
			
		},
		title: '预览'
	})
}

/**
 * 插入图片或链接
 */
Dida.markitup.tools.insertFilepath = function(obj, edit, op) {
	var filepath = $('#markitup_file_form_field_filepath').val();
	
	if (!filepath) {
		alert('地址不能为空');
		$('#markitup_file_form_field_filepath').focus();
		return;
	}
	
	if (op == 'file') {
		
		var title = $('#markitup_file_form_field_title').val();
		
		if (!title) {
			alert('链接标题不能为空');
			$('#markitup_file_form_field_title').focus();
			return;
		}
		
		var alt = $('#markitup_file_form_field_alt').val() || '';
		var target = $('#markitup_file_form_field_target').val();
		
		var link  = '<a href="'+filepath+'" alt="'+alt+'" title="'+title+'"';
		
		if (target) {
			link += ' target="'+target+'"';
		}
		
		link += '>'+title+'</a>';
		Dida.markitup.tools.insert(link);
		
	} else {
		
		if (obj.image_is_type && !Dida.isImage(filepath)) {
			// 验证图片格式
			alert('图片格式不正确，只能以 jpg、gif、png 结尾');
			return;
		}
			
		var alt = $('#markitup_file_form_field_alt').val() || '';
		var title = $('#markitup_file_form_field_title').val() || '';
		
		var image  = '<img src="'+filepath+'" alt="'+alt+'" title="'+title+'"';
		
		if ($('#markitup_file_form_field_size_width').val()) {
			image += ' width="' + parseInt($('#markitup_file_form_field_size_width').val()) + '"';
			$('#markitup_file_form_field_size_width').val("");
		}
		
		if ($('#markitup_file_form_field_size_height').val()) {
			image += ' height="' + parseInt($('#markitup_file_form_field_size_height').val()) + '"';
			$('#markitup_file_form_field_size_height').val("");
		}
		
		image += ' />';
		
		var target = $('#markitup_file_form_field_target').val();
		
		if (target) {
			target = ' target="'+target+'"';
		}
		
		if ($('#markitup_file_form_field_link').val()) {
			Dida.markitup.tools.insert('<a '+target+' href="'+$('#markitup_file_form_field_link').val()+'">' + image + '</a>', edit);
			$('#markitup_file_form_field_link').val("");
		} else {
			Dida.markitup.tools.insert(image, edit);
		}
		
	}
	
	$('#markitup_file_form_field_alt, #markitup_file_form_field_title, #markitup_file_form_field_filepath').val("");
	$('#markitup_dialog_wrapper').hide();
}

Dida.markitup.tools.dialog = function(edit, obj) {
	if (!$('#markitup_dialog_wrapper').size()) {
		$('body').append('<div id="markitup_dialog_wrapper"></div>');
	}
	
	$('#markitup_dialog_wrapper').show().children().hide();
	
	this.offset = $(edit.textarea).prev('.markItUpHeader').offset();
	$('#markitup_dialog_wrapper').css({'top': this.offset.top+22, 'left': this.offset.left+100}).show();
}

Dida.markitup.tools.upload = function(edit, obj, op) {
	Dida.markitup.tools.dialog(edit, obj);
	
	if (!$('#markitup_file_wrapper').size()) {

		var html = '<div id="markitup_file_wrapper">';
		html += '<form method="post" enctype="multipart/form-data" action="" id="markitup_file_form">';
		
		html += '<div class="form_item markitup_file_filepath">';
		html += '<label class="dd_label">图片地址：</label><input type="text" value="" id="markitup_file_form_field_filepath" />';
		html += '<span class="form_description">必需</span>';
		
	  html += '<input type="button" value="选取" id="markitup_file_form_field_browser" />';
	  
		html += '</div>';
		
		html += '<div class="form_item markitup_file_alt">';
		html += '<label class="dd_label">图片ALT：</label><input type="text" value="" id="markitup_file_form_field_alt" />';
		html += '<span class="form_description">可选</span></div>';
		
		html += '<div class="form_item markitup_file_title">';
		html += '<label class="dd_label">图片TITLE：</label><input type="text" value="" id="markitup_file_form_field_title" />';
		html += '<span class="form_description">可选</span></div>';
		
		html += '<div class="form_item markitup_file_link">';
		html += '<label class="dd_label">图片链接：</label><input type="text" value="" id="markitup_file_form_field_link" />';
		html += '<span class="form_description">选填</span></div>';
		
		html += '<div class="form_item markitup_file_size"><label class="dd_label">图片尺寸：</label>';
		html += '<input type="text" value="" id="markitup_file_form_field_size_width" size="5" />x<input type="text" size="5" value="" id="markitup_file_form_field_size_height" />';
		html += '<span class="form_description">选填</span></div>'
			
	  html += '<div class="form_item markitup_file_target"><label class="dd_label">链接方式：</label>';
		html += '<select id="markitup_file_form_field_target">';
		html += '<option value="">当前窗口</option>';
		html += '<option value="_blank">新窗口</option>';
		html += '<option value="_parent">父窗口</option></select>';
		html += '<span class="form_description">可选</span></div>'
			
		html += '<div class="form_item markitup_file_upload"><label class="dd_label">本地上传：</label>';
		html += '<input type="file" name="upload" id="markitup_file_form_field_upload" />';
		html += '<input type="button" value="上传" id="markitup_file_form_field_upload_button" /></div>';
		
		html += '<div class="form_item markitup_file_button">';
		html += '<input type="button" id="markitup_file_form_field_confirm" value="确认插入" />';
		html += '<input type="button" id="markitup_file_form_field_cancel" value="关闭" /></div>';
		
		html += '</form></div>';
		
		$('#markitup_dialog_wrapper').append(html);
		
		$('#markitup_file_form').submit(function(){
			$('#markitup_file_form_field_confirm').trigger('click');
			return false;
		});
		
		$('#markitup_file_form_field_cancel').click(function(){
			$('#markitup_dialog_wrapper').hide();
		});
		
	}
	
	$('#markitup_file_wrapper').show();
	
	var op_name = op == 'image' ? '图片' : '链接';
	$('.markitup_file_filepath .dd_label').text(op_name+'地址');
	$('.markitup_file_alt .dd_label').text(op_name+'ALT');
	$('.markitup_file_title .dd_label').text(op_name+'TITLE');
	
	if (edit.selection) {
		$('#markitup_file_form_field_title').val(edit.selection);
	}
	
	if (op == 'image') {
		$('.markitup_file_size, .markitup_file_link').show();
		if (obj.filebrowserImageBrowseUrl) {
			$('#markitup_file_form_field_browser').show();
			$('#markitup_file_form_field_browser').unbind('click').bind('click', function(){
				Dida.dialog({
					url: obj.filebrowserImageBrowseUrl,
					iframe: true,
					modal: true,
					title: '图片库'
				});
				return false;
			});
		} else {
			$('#markitup_file_form_field_browser').hide();
		}
		
		if (obj.filebrowserImageUploadUrl) {
			$('.markitup_file_upload').show();
			$('#markitup_file_form_field_upload_button').unbind('click').bind('click', function(){
				Dida.markitup.tools.ajaxUpload(obj, edit, op);
				return false;
			});
		} else {
			$('.markitup_file_upload').hide();
		}
		
	} else {
		$('.markitup_file_size, .markitup_file_link').hide();
		if (obj.filebrowserBrowseUrl) {
			$('#markitup_file_form_field_browser').show();
			$('#markitup_file_form_field_browser').unbind('click').bind('click', function(){
				Dida.dialog({
					url: obj.filebrowserBrowseUrl,
					iframe: true,
					title: '文件库'
				});
				return false;
			});
		} else {
			$('#markitup_file_form_field_browser').hide();
		}
		
		if (obj.filebrowserUploadUrl) {
			$('.markitup_file_upload').show();
			$('#markitup_file_form_field_upload_button').unbind('click').bind('click', function(){
				Dida.markitup.tools.ajaxUpload(obj, edit, op);
				return false;
			});
		} else {
			$('.markitup_file_upload').hide();
		}
		
	}
	
	$('#markitup_file_form_field_confirm').unbind('click').bind('click', function(){
		Dida.markitup.tools.insertFilepath(obj, edit, op);
		return false;
	});
}

/**
 * 插入表情
 */
Dida.markitup.tools.smiley = function(edit, obj) {
	if (!$('#markitup_smiley_wrapper').size()) {
		$('body').append('<div id="markitup_smiley_wrapper"><div class="contents"></div><div class="menu"></div></div>');
		
		$('#markitup_smiley_wrapper a.markitup_smiley_menu').live('click', function(){
			$('#markitup_smiley_wrapper .contents').html('<div class="loading_image loading_size"></div>');
			smiley($(this).attr('alt'));
			$(this).siblings('a.markitup_smiley_menu').removeClass('active');
			$(this).addClass('active');
			return false;
		});
		
		$('#markitup_smiley_wrapper .contents img').live('click', function(){
			Dida.markitup.tools.insert('[smiley]'+$(this).attr('alt')+'[/smiley]', edit);
			return false;
		});
		
		$('#markitup_smiley_close').live('click', function(){
			$('#markitup_smiley_wrapper').hide();
			return false;
		});
	}
	
	this.offset = $(edit.textarea).prev('.markItUpHeader').offset();
	$('#markitup_smiley_wrapper').css({'top': this.offset.top+22, 'left': this.offset.left+100}).show();
	
	var path = obj.root + 'sets/dida/face/';
	
	if (!$.isPlainObject(Dida.markitup.cache.smiley)) {
		Dida.markitup.cache.smiley = {};
	}
	
	function smiley(attr) {
		var items = Dida.markitup.cache.smiley[attr].items;
		if (!$.isEmptyObject(items)) {
			
			var html = '';
			$(items).each(function(){
				html += '<img src="'+path + this+'" alt="'+this+'"/>';
			});
			
			$('#markitup_smiley_wrapper .contents').html(html);
		}
		
	}
	
	if ($.isEmptyObject(Dida.markitup.cache.smiley)) {
		
		$('#markitup_smiley_wrapper .contents').html('<div class="loading_image loading_size"></div>');
		
		$.getJSON(path + 'face.php', function(json) {
			if (!json.error && json.contents) {
				Dida.markitup.cache.smiley = json.contents;
				
				var menu = '', i = 0;
				for (var attr in json.contents) {
					if (i == 0) {
						menu += '<a href="#" class="markitup_smiley_menu active" alt="'+attr+'">'+json.contents[attr].name+'</a>';
						smiley(attr);
					} else {
						menu += '<a href="#" class="markitup_smiley_menu" alt="'+attr+'">'+json.contents[attr].name+'</a>';
					}
					
					i++;
				}
				
				if (i < 2) {
					menu = '';
				}
				
				menu += '<a href="#" id="markitup_smiley_close">关闭</a>';
				$('#markitup_smiley_wrapper .menu').html(menu);
				
			} else {
				$('#markitup_smiley_wrapper').hide();
				alert('没有定义任何表情');
			}
		});
		
	}
	
}

/**
 * 向编辑区域添加内容
 */
Dida.markitup.tools.insert = function(html, edit) {
	
	// 若 edit.selection 不为空，则追加
	if (typeof(edit) == 'object' && edit.selection) {
		html = edit.selection + html;
	}
	
	$.markItUp({replaceWith: html});
}

/**
 * 后台保存
 */
Dida.markitup.tools.autoSave = function(edit, obj) {
	if (obj.AutoSaveUrl) {
    var s = $.param($(edit.textarea).closest('form').formToArray());
    if (this.AutoSaveString != s) { // 表单数据与上次请求保存时不一致时才执行
    	this.AutoSaveString = s;
      $.post(obj.AutoSaveUrl, s, function(data) {
        if (data == 'ok') {
          var myDate = new Date();
          Dida.messageShow(myDate.toLocaleString() + ' 草稿保存成功');
        } else {
          alert('草稿保存失败');
        }
      });
    };
	}
}

/**
 * 从文件浏览器中插入
 */
Dida.markitup.tools.filebrowserInsert = function(filepath, title, alt, link) {
	if (filepath) {
		$('#markitup_file_form_field_filepath').val(filepath);
		
		if (title) {
			$('#markitup_file_form_field_title').val(title);
		}
		if (alt) {
			$('#markitup_file_form_field_alt').val(alt);
		}
		if (link) {
			$('#markitup_file_form_field_link').val(link);
		}
	}
}

Dida.markitup.tools.menus = function(sets, menu) {
	
	if (typeof(menu) == 'object') {
		var l = menu.length, m = [];
		
		for (var i=0; i < l; i++) {
			if (sets[menu[i]]) {
				sets[menu[i]].className = 'markItUpButton_' + menu[i];
				m[i] = sets[menu[i]];
			}
		}
	} else {
		
		var i = 0, m = [];;
		for (var attr in sets) {
			sets[attr].className = 'markItUpButton_' + attr;
			m[i] = sets[attr];
			i++;
		}
		
	}
	return m;
}
