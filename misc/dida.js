// $Id$
var Dida = {cache: {}};
Dida.dejson = function(data) {
  if ((data.substring(0, 1) != '{') && (data.substring(0, 1) != '[')) {
    return false;
  }
  return eval('(' + data + ');');
};

Dida.getck = function(name) {
  var arr = document.cookie.match(new RegExp("(^| )"+name+"=([^;]*)(;|$)"));
  if (arr != null) {
  	return decodeURIComponent(arr[2]);
  } else {
  	return null;
  }
};

Dida.gettime = function() {
  var d = new Date();
  return d.getTime();
};

Dida.getsize = function() {
	var de = document.documentElement;
	var w = window.innerWidth || self.innerWidth || (de&&de.clientWidth) || document.body.clientWidth;
	var h = window.innerHeight || self.innerHeight || (de&&de.clientHeight) || document.body.clientHeight;
	arrayPageSize = [w,h];
	return arrayPageSize;
};

Dida.external = function(path) {
	
	if (path.substr(0, 1) == '/') {
		return true;
	}
	
	if (path.indexOf(':') != -1) {
		
		var par = path.split(':');
		var types = {
					'http': 1, 'https': 1, 'ftp': 1, 'news': 1, 'nntp': 1,
					'telnet': 1, 'mailto': 1, 'irc': 1, 'ssh': 1, 'sftp': 1, 'webcal': 1, 'rtsp': 1
				};
  	return types[par[0]];
  }
}

Dida.url = function(q, opt) {
  var url = q;
  var external = Dida.external(q);
  
  if (!external) {
  	if (settings.clean_url || url.indexOf('?') != -1) {
  		url = settings.base_path + q;
  	} else {
  		url = settings.base_path + '?q=' + q;
  	}
  }
  
  if (opt) {
  	
    if (url.indexOf('?') == -1) {
      url += '?';
    } else {
      url += '&';
    }
    for (var attr in opt) {
      url += attr + '=' +opt[attr] + '&';
    }
    url = url.substr(0, url.length - 1);
  }
  return url;
};

Dida.favorite = function(title, url) {
  if ($.browser.msie) {
    window.external.addFavorite(url, title);
  } else {
    window.sidebar.addPanel(title, url, '');
  }
};

Dida.loca = function(url) {
  if (url == 1) url = '';
  location.href = url ? url : location.href;
};

Dida.onbe = function(msg) {  
  window.onbeforeunload = onbeforeunload_handler;
  function onbeforeunload_handler() {
    var warning = msg ? msg : '确认退出？';
    return warning;
  }
};

Dida.onun = function(msg) {  
  window.onunload = onunload_handler;
  function onunload_handler() {
      alert(msg); 
  }
};

Dida.parseQuery = function(query) {
   var Params = {};
   if (!query) {return Params;}
   var Pairs = query.split(/[;&]/);
   
   for ( var i = 0; i < Pairs.length; i++ ) {
      var KeyVal = Pairs[i].split('=');
      if ( ! KeyVal || KeyVal.length != 2 ) {continue;}
      var key = unescape( KeyVal[0] );
      var val = unescape( KeyVal[1] );
      val = val.replace(/\+/g, ' ');
      Params[key] = val;
   }
   return Params;
};

Dida.getUrl = function() {
  var url = location.href;
  return url.replace(/^[^\?]*\??/,'');
}

Dida.messageShow = function(text, opt) {
  var o;
  if (opt) {
    o = {
      timeOut: opt.timeOut || 5000,
      status: opt.status || 'status'
    };
  } else {
    o = {timeOut: 5000, status: 'status'};
  }
  
  $('body').append('<div style="position:absolute;top:0;right:0;z-index: 1000;" class="js_messageShow '+o.status+'">'+text+'</div>');
  window.setTimeout(function() {$('.js_messageShow').remove();}, o.timeOut);
}

Dida.callFunc = function(cb) {
	if (typeof cb == 'function') {
		eval(cb);
	} else {
    var func;
    func = eval(cb);
    return func.apply(null, Array.prototype.slice.call(arguments, 1));
  }
}

Dida.isImage = function(p) {
	var regu = /.*(jpg|gif|png)$/i;
	return p && regu.test(p);
}

Dida.dialog = function(opt) {
	
	var o = {
		width: 700,
		height: 450,
		title: '', 
		url: '',
		closeText: '关闭',
		autoOpen: true,
		modal: false,
		//bgiframe: true,
		reload: false,
		closeCall: false,
		iframe: false
	}
	
	$.extend(o, opt);
	
	if ($.isPlainObject(settings.dialogOptions)) {
		$.extend(o, settings.dialogOptions);
	}
	
	if ($('#dialog_wrapper').size()) {
		$('#dialog_wrapper').dialog('destroy').remove();
	}
	
	$('body').append('<div id="dialog_wrapper" style="display: none;"></div>');
	
	if (isNaN(o.url)) {
		
		var queryString = o.url.replace(/^[^\?]*\??/,'');
		var params = Dida.parseQuery(queryString);
		if (params) {
			o.params = params;
			if (params['width']) {
				o.width = parseInt(params['width']);
			}
			if (params['height']) {
				o.height =  parseInt(params['height']);
			}
			if (params['inlineId']) {
				o.inlineId = params['inlineId'];
			}
			if (params['iframe']) {
				o.iframe = true;
			}
			if (params['closeCall']) {
				o.closeCall = params['closeCall'];
			}
			if (params['reload']) {
				o.reload = params['reload'];
			}
			if (params['modal']) {
				o.modal = params['modal'];
			}
		}
		
	}
	
	if (!$.isFunction(o.close)) {
		o.close = function(event, ui) {
	    
	    if (o.closeCall) {
	      Dida.callFunc(o.closeCall, o, event, ui);
	    }
	    
			if (o.reload) {
				location.reload();
			}
		}
	}
	
  if (o.inlineId) {

		$('#'+o.inlineId).dialog(o);
		
  } else if (o.iframe) {
  	if (!$.isFunction(o.open)) {
			o.open = function() {
				var h = '<div id="dialog_wrapper_loading">';
				h += '<img align="absmiddle" src="'+settings.base_path+'misc/images/loading.gif" />加载中，请稍候…</div>';
				h += '<iframe id="dialog_iframe_wrapper" frameborder="no" border="0" src="'+o.url+'" width="100%"';
				h += ' height="100%" style="display:none"></iframe>';
				
				$(this).append(h);
				
				$('#dialog_iframe_wrapper').load(function() {
	        $('#dialog_wrapper_loading').hide();
	        $('#dialog_iframe_wrapper').show();
	      });
			}
  	}
		$('#dialog_wrapper').dialog(o);
		
  } else if (Dida.isImage(o.url)) {
  	if (!$.isFunction(o.open)) {
			o.open = function() {
				var _s = Dida.getsize();
				var h = '<div id="dialog_wrapper_loading">';
				h += '<img align="absmiddle" src="'+settings.base_path+'misc/images/loading.gif" />加载中，请稍候…</div>';
				h += '<img id="dialog_image_wrapper" src="'+o.url+'" />';
				$(this).append(h);
				$('#dialog_image_wrapper').load(function() {
					var _sw = $(this).width()+160 < _s[0] ? $(this).width()+80 : _s[0]-80;
					var _sh = $(this).height()+160 < _s[1] ? $(this).height()+80 : _s[1]-80;
					$('#dialog_wrapper').dialog( "option", {
						'width': _sw,
						'height': _sh,
						'position': 'center'
						});
					
		      $('#dialog_wrapper_loading').hide();
		    });
			}
  	}
  	
		$('#dialog_wrapper').dialog(o);
		
  } else {
  	if (!$.isFunction(o.open)) {
			o.open = function() {
				var h = '<div id="dialog_wrapper_loading">';
				h += '<img align="absmiddle" src="'+settings.base_path+'misc/images/loading.gif" />加载中，请稍候…</div>';
				h += '<div id="dialog_ajax_wrapper" style="display:none"></div>';
				
				$(this).append(h);
				$.post(o.url, o.ajax_vars, function(html) {
					$('#dialog_wrapper_loading').hide();
					$('#dialog_ajax_wrapper').html(html).show();
				});
			}
  	}
		$('#dialog_wrapper').dialog(o);
		
  }
  
	$("#dialog_wrapper").prev('.ui-widget-header').find(".ui-dialog-titlebar-close")
	.before('<a href="#" title="最小化" class="ui-icon ui-icon-minusthick ui-icon-dialog-max-min"></a>');
	
	$(".ui-icon-dialog-max-min").click(function() {
		var obj = $(this).parents(".ui-dialog").find("#dialog_wrapper");
		if (obj.css('display') == 'none') {
			
			obj.show();
			$(this).attr('title', '最小化').removeClass('ui-icon-plusthick').addClass('ui-icon-minusthick')
			.parents(".ui-dialog").css('height', 'auto');
			
			$('#dialog_wrapper').dialog("option", "position", 'center');
			
		} else {
			obj.hide();
			
			$(this).attr('title', '最大化').removeClass('ui-icon-minusthick').addClass('ui-icon-plusthick')
			.parents(".ui-dialog").css('height', '35px');
			
			$('#dialog_wrapper').dialog("option", "position", ['right', 'bottom']);
		}
		
		return false;
	});
	
	return false;
}

Dida.dialog_colse = function() {
	$('#dialog_wrapper').dialog('destroy');
	$('#dialog_wrapper').remove();
	location.reload();
}

Dida.php = {
	in_array: function(str, arr) {
	  for (i = 0; i < arr.length; i++) {
	    var thisEntry = arr[i].toString();
	    if (thisEntry == str) {
	      return true;
	    }
	  }
	  return false;
	}
};

Dida.ajaxSuccess = function(obj, data, type) {
	
	obj.removeClass('ja_loading');
	
  if (obj.attr('type') != 'js') {
  	var level = obj.attr('level');
  	
  	if (level && data == 1) {
  		data = level;
  	}
  	
  	var fun = obj.attr('fun');
  	
  	if (fun) {
  		
  		// 调用函数，依次传递：返回值、当前元素、类型
  		Dida.callFunc(fun, data, obj, type);
  		
  	} else {
  	
	    switch (data) {
	      case 'parent':
	      	// 删除父级
	      	obj.parent().remove();
	      break;
	      case 'two':
	      	// 删除祖级
	      	obj.parent().parent().remove();
	      break;
	      case 'own':
	      	// 删除本身
	      	obj.remove();
	      break;
	      case 'tr':
	      	// 删除上级中第一个匹配的 tr
	      	obj.closest('tr').remove();
	      break;
	      case 'replace':
	      	// 替换
          var text = obj.attr('replace') ? obj.attr('replace') : '成功';
          if (text) {
          	obj.after('<span class="red msgjs">' + text + '</span>');
            obj.remove();
          }
        break;
	      default:
	        if (data == 1) {
	        	
	        	var text = obj.attr('replace') ? obj.attr('replace') : '成功';
	          if (text) {
	          	obj.after('<span class="red msgjs">' + text + '</span>');
	            obj.remove();
	          }
	          
	        } else if (type == 'a') {
	          alert(data ? data : '操作失败');
	        } else {
	        	obj.after('<span class="msgjs">' + (data ? data : '操作失败') + '</span>');
	        }
	    }
  	}
  } else {
  	
    eval(data);
    
  }
}

$(function() {
	
  $('.button_goto').click(function() {
    location.href = $(this).attr('href');
  });
  
  $('.homepage_button').click(function() {
    if ($.browser.msie) {
      this.style.behavior='url(#default#homepage)';this.setHomePage($(this).attr('href'))
    } else {
      alert('你的浏览器安全设置过高，不支持此操作。');
    }
    return false;
  });
  
  $('.favorites_button').click(function() {
  	if (!$.browser.mozilla) {
	    Dida.favorite($(this).attr('title'), $(this).attr('href'));
	    return false;
  	}
  });
  
  $('.confirm').live('click', function() {
    msg = $(this).attr('alt');
    if (!confirm((msg ? msg : '确认此操作吗？'))) {
      return false;
    }else if ($(this).attr('type') == 'button') {
      location.href = $(this).attr('href');
    }
  });
  
  $('.confirmajax').live('click', function() {
    msg = $(this).attr('alt');
    if (confirm((msg ? msg : '确认此操作吗？'))) {
    	$(this).addClass('ja_loading');
      var $$ = $(this);
      var url = $$.attr('href');
      $.get(url, {'timestamp': Dida.gettime()}, function(data) {
      	Dida.ajaxSuccess($$, data, 'a');
      })
    }
    return false;
  });
  
  $('.dd_form_ajax_field').change(function() {
  	var href = $(this).attr('href');
  	
  	if ($(this).hasClass('changeconfirm') && !confirm('确认此操作吗？')) {
  		return false;
  	}
  	
  	if (href) {
  		var $$ = $(this);
  		$.ajax({
        type: 'POST',
        url: href,
        dataType: 'html',
        data: 'id='+$(this).attr('alt')+'&value=' + $(this).val(),
        success: function(data) {
  				if (data == -1) {
  					alert('操作失败');
  				}else if (data == 'two') {
  					$$.parent().parent().remove();
  				}
        }
  		});
  	}
  });
  
  $('.form_all_check').click(function() {
    dom = $(this).attr('alt');
    $('.' + dom).attr('checked', this.checked ? true : false);
  });
  
  $('a.all_menu_ext').click(function() {
    dom = $(this).attr('alt');
    if ($(this).attr('rel') == 1) {
      $(this).attr('rel', 0).text('闭合');
      $('.' + dom).removeClass('div_none');
    } else {
      $(this).attr('rel', 1).text('展开');
      $('.' + dom).addClass('div_none');
    }
    return false;
  });
  
  $('.dc_form_ajax_form_button').click(function() {
  	var s = $.param($(this).parents('form').formToArray());
  	var getUrl = $(this).attr('alt');
  	if (getUrl) {
	  	$.ajax({
	  		type: 'POST',
	  		url: getUrl,
	  		dataType: 'script',
	  		data: s,
	  		error: function(e) {
	  			alert('error');
	  		}
	  	});
  	};
  	return false;
  });
  
  $('.fieldset-hide').children('.fieldset-wrapper').hide();
  $('fieldset.collapsible > legend.collapse-processed').click(function() {
    $(this).toggleClass('asc').toggleClass('desc').parent().toggleClass('fieldset-hide');
    $(this).next('.fieldset-wrapper').slideToggle(100);
    return false;
  });
  
	$('.thickbox, .dialog').click(function() {
		var o = {};
		o.url = $(this).attr('href');
		o.title = $(this).attr('title') || $(this).text();
		
		Dida.dialog(o);
		
		return false;
	});
  
  $('input[class="admin_delete_button"]').click(function() {
    if (confirm('确认此操作吗？')) {
      var href = $(this).attr('alt');
      var c = $(this).attr('rel');
      $('.' + c).not($('input.form_all_check')).each(function() {
        if (this.checked) {
          var $$ = $(this);
          $$.after('<span class="ja_loading"></span>');
          url = $$.attr('href') ? $$.attr('href') : href;
          var opt = {};
          opt.timestamp = Dida.gettime();
          if (!$$.attr('href')) {
            opt.id = $(this).val();
            opt.name = $(this).attr('alt');
          }
          $.get(url, opt, function(data) {
          	$$.next('ja_loading').remove();
          	Dida.ajaxSuccess($$, data, 'input');
          })
        }
      })
    }
    return false;
  });
  
  $('.login_msg').click(function() {
    if (confirm('你需要登录才能进行此操作，立即登录？')) {
      location.href = Dida.url('user/login', {redirect: $(this).attr('href') ? $(this).attr('href') : location.pathname});
    }
    return false;
  });
  
  $('.confirm_msg').click(function() {
    alert($(this).attr('title'));
    return false;
  });
  
  if (settings.multi) {
    for (var attr in settings.multi) {
      var element = settings.multi[attr];
      $(element).each(function() {
      	$('#multi_' + this.dom).MultiFile({'list' : '#multi_list_' + this.dom});
      });
    }
  }
  if (settings.edit) {
    $(settings.edit).each(function() {
    	$(this.dom).editable(null, this.opt);
    });
  }
  
  $('.pager_form_go_input').blur(function() {
    var id = $(this).attr('alt');
    var s = settings.pager[id];
    var t = parseInt($(this).val());
    var go = 0;
    if (t) {
      t = t - 1;
      if (t != s.current && t < s.sum) {
        go =  t > s.sum ? s.sum : t;
        var h = location.href;
        
        if (go > 0) {
        	if (h.indexOf('page=') != -1) {
        		var re = /page=\d*/i;
        		h = h.replace(re, 'page=' + go);
        	} else {
        		h = Dida.url(h, {'page': go});
        	}
        } else {
        	var re = /[\?|&]page=\d*/i;
          h = h.replace(re, '');
        }
        location.href = h;
      }
    }
  });
  if (settings.farbtastic) {
    for (var attr in settings.farbtastic) {
      var element = settings.farbtastic[attr];
      $(element).each(function() {
			  var f = $.farbtastic(this.dom);
			  var p = $(this.dom).css('opacity', 0.25);
			  var selected;
			  $(this.items)
			    .each(function () { f.linkTo(this); $(this).css('opacity', 0.35); })
			    .focus(function() {
			      if (selected) {
			        $(selected).css('opacity', 0.35).removeClass('colorwell-selected');
			      }
			      f.linkTo(this);
			      p.css('opacity', 1);
			      $(selected = this).css('opacity', 1).addClass('colorwell-selected');
			    });
      });
    }
  };
  
  //自动完成
  if (settings.auto) {
  	var ui_auto = {};
    $(settings.auto).each(function(i, item) {
      if (item.dom) {
      	ui_auto[i] = item;
      	if (ui_auto[i].url) {
      		ui_auto[i].cache = {};
      		ui_auto[i].source = function(request, response) {
      			if (ui_auto[i].cache[request.term] != undefined) {
      				response(ui_auto[i].cache[request.term]);
      				return;
      			}
      			
      			$.ajax({
      				url: ui_auto[i].url,
      				dataType: 'json',
      				type: 'POST',
      				data: {value: request.term},
      				success: function(data) {
      					if (!data.error && data.contents) {
      						ui_auto[i].cache[request.term] = data.contents;
	    						response(data.contents);
      					} else {
      						if (ui_auto[i].range && ($.browser.mozilla || $.browser.msie)) {
      							$(ui_auto[i].dom).val("");
      						}
      						ui_auto[i].cache[request.term] = [];
      						response([]);
      					}
      				}
      			})
      		}
      	}
      	$(ui_auto[i].dom).autocomplete(ui_auto[i]);
      	if ($.browser.mozilla) {
      		/**
      		 * firefox 中文输入法 bug
      		 */
	      	$(ui_auto[i].dom).bind("text", function() {
	      		$(this).autocomplete('search');
	      	});
      	}
      }
    });
  }
  
  if (settings.sort) {
    var element = settings.sort;
    $(element).each(function(i) {
      var $$ = this;
      $(this.wid).after('<span class="ui-icon ui-icon-arrowthick-2-n-s"></span>');
      $(this.dom).after('<div class="messages sort_messages sort_messages_'+ i +'" style="display: none"></div>');
      $(this.dom).sortable({
         change: function(event, ui) {$('.sort_messages').show().text('提示：排序已变动，请提交保存'); $(this.dom).sortable("serialize"); }
      });
    });
  };
  
  if (settings.markItUp && typeof(Dida.markitup) == 'object') {
  	$(settings.markItUp).each(function() {
  		if (this.dom) {
	  		var $$ = this;
	  		var type = $$.type || 'html';
	  		$(this.dom).markItUp(Dida.markitup[type]($$.options));
  		}
  	});
  }
  
  // ajax 验证
  if (settings.ajax_validate) {
  	
  	$(settings.ajax_validate).each(function(i) {
  		var o = this;
  		
  		if (o.ajax_submit) {
  			o.submitHandler = function(form) {
  				$(form).ajaxSubmit(o.options);
  			};
  		}
  		
  		$('#'+ o.form_id).validate(o);
  		
  	});
  	
  	$.validator.addMethod("required", function(value, element, param) {
  		if ( !this.depend(param, element) )
  			return "dependency-mismatch";
  		switch( element.nodeName.toLowerCase() ) {
	  		case 'select':
	  			var val = $(element).val();
	  			return val && val.length > 0;
	  		case 'input':
	  			if ( this.checkable(element) ) {
	  				if (element.type == 'checkbox') {
	  					var name = $(element).parent('.form_checkbox_option').attr('alt');
	  					if (name) {
  							cd = false;
  							var errorClass = this.settings.errorClass;
  							$(element).parents('form').find('.'+name).each(function() {
	  							if ($(this).attr('checked')) {
	  								cd = true;
	  								$(element).parents('form').find('.'+name).each(function() {
	  									$(this).next('label.'+errorClass).hide();
	  								});
	  								return cd;
	  								
	  							}
	  						});
	  						
	  						return cd;
	  					}
	  				}
	  				
	  				return this.getLength(value, element) > 0;
	  			}
	  		default:
	  			return $.trim(value).length > 0;
  		}
  	});
  	
  };
  
  //时间控件
  if (settings.uidata) {
    $.datepicker.setDefaults($.datepicker.regional['zh-CN']);
    var element = settings.uidata;
    $(element).each(function(i) {
      var o = this;
	    o.showStatus = true;
	    o.showOn = "both";
	    o.buttonImage = "/misc/images/calendar.gif";
	    o.buttonImageOnly = true;
      
      if (o.start || o.end) {
      	o.onClose = function(text) { 
					var instance = $(this).data("datepicker");
					uidata_vali($(this), text, instance, instance.settings.start ? 'start' : 'end');
        }
      }
      
      if (o.showTime) {
      	o.duration = '';
        o.showTime = true;
        o.constrainInput = false;
      }
      
      $(o.dom).datepicker(o).focus(function() { this.blur(); });
    });
  };
  
  function uidata_vali(obj, text, instance, type) {
  	var val_end, val_start;
  	if (type == 'start') {
  		val_start = text;
  		val_end = $(instance.settings.start).val();
  	} else {
  		val_end = text;
  		val_start = $(instance.settings.end).val();
  	}
  	
  	if (val_end && val_start) {
      re = /[^0-9]/g;
      val_end = val_end.replace(re, '');
      val_start = val_start.replace(re, '');
      if (val_end <= val_start) {
      	obj.val('');
      	alert('结束日期必须大于开始日期');
      }
  	}
  	return false;
  };
});