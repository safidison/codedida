// $Id$
var Dida ={};
Dida.dejson = function(data){
  if ((data.substring(0, 1) != '{') && (data.substring(0, 1) != '[')) {
    return false;
  }
  return eval('(' + data + ');');
};

Dida.getck = function(name){
  var arr = document.cookie.match(new RegExp("(^| )"+name+"=([^;]*)(;|$)"));
  if(arr != null) return unescape(arr[2]); return null;
};

Dida.gettime = function(){
  var d = new Date();
  return d.getTime();
};

Dida.getsize = function(){
	var de = document.documentElement;
	var w = window.innerWidth || self.innerWidth || (de&&de.clientWidth) || document.body.clientWidth;
	var h = window.innerHeight || self.innerHeight || (de&&de.clientHeight) || document.body.clientHeight;
	arrayPageSize = [w,h];
	return arrayPageSize;
};

Dida.url = function(q, opt){
  var url;
  url = settings.base_path + q;
  if(opt){
    if(settings.base_path.indexOf('?') != -1){
      url += '?';
    }else{
      url += '&';
    }
    for(var attr in opt){
      url += attr + '=' +opt[attr] + '&';
    }
  }
  return url;
};

Dida.favorite = function(title, url){
  if($.browser.msie){
    window.external.addFavorite(url, title);
  }else{
    window.sidebar.addPanel(title, url, '');
  }
};

Dida.loca = function(url){
  if(url == 1) url = '';
  location.href = url ? url : location.href;
};

Dida.onbe = function(msg){  
  window.onbeforeunload = onbeforeunload_handler;
  function onbeforeunload_handler(){
    var warning = msg ? msg : '确认退出？';
    return warning;
  }
};

Dida.onun = function(msg){  
  window.onunload = onunload_handler;
  function onunload_handler(){
      alert(msg); 
  }
};

Dida.callFunc = function(cb) {
	if(typeof cb == 'function'){
		eval(cb);
	}else{
    var func;
    func = eval(cb);
    return func.apply(null, Array.prototype.slice.call(arguments, 1));
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

Dida.getUrl = function(){
  var url = location.href;
  return url.replace(/^[^\?]*\??/,'');
}

Dida.messageShow = function(text, opt){
  var o;
  if(opt){
    o = {
      timeOut: opt.timeOut || 5000,
      status: opt.status || 'status'
    };
  }else{
    o = {timeOut: 5000, status: 'status'};
  }
  
  $('body').append('<div style="position:absolute;top:0;right:0;z-index: 1000;" class="js_messageShow '+o.status+'">'+text+'</div>');
  window.setTimeout(function(){$('.js_messageShow').remove();}, o.timeOut);
}

Dida.callFunc = function(cb) {
	if(typeof cb == 'function'){
		eval(cb);
	}else{
    var func;
    func = eval(cb);
    return func.apply(null, Array.prototype.slice.call(arguments, 1));
  }
}

function In_array(str, array) {
  for (i = 0; i < array.length; i++) {
    thisEntry = array[i].toString();
    if (thisEntry == str) {
      return true;
    }
  }
  return false;
}
$(function(){
  $('#keywords').one('click', function(){
    $(this).val('');
  });
  $('.button_goto').click(function(){
    location.href = $(this).attr('href');
  });
  $('.homepage_button').click(function(){
    if($.browser.msie){
      this.style.behavior='url(#default#homepage)';this.setHomePage($(this).attr('href'))
    }else{
      alert('你的浏览器安全设置过高，不支持此操作。');
    }
    return false;
  });
  $('.favorites_button').click(function(){
    Dida.favorite($(this).attr('title'), $(this).attr('href'));
    return false;
  });
  $('.confirm').click(function(){
    msg = $(this).attr('alt');
    if(!confirm((msg ? msg : '确认此操作吗？'))){
      return false;
    }else if($(this).attr('type') == 'button'){
      location.href = $(this).attr('href');
    }
  });
  $('.confirmajax').click(function(){
    msg = $(this).attr('alt');
    if(confirm((msg ? msg : '确认此操作吗？'))){
      var $$ = $(this);
      var url = $$.attr('href');
      $.get(url, {'timestamp': Dida.gettime()}, function(data){
        if($$.attr('type') != 'js'){
          var fun = $$.attr('fun');
          if(data == 1){
            var text = $$.attr('replace');
            if(fun){
              eval(fun + '('+data+');');
            }else if(text){
              $$.attr('href', '#').unbind('click').text(text);
            }else{
              switch($$.attr('level')){
                case 'parent':
                  $$.parent().remove();
                break;
                case 'own':
                  $$.remove();
                break;
                default:
                  $$.parents('tr').remove();
              }
            }
          }else if(data && fun){
            eval(fun + '('+data+');');
          }else{
            alert(data ? data : '操作失败');
          }
        }else{
          eval(data);
        }
      })
    }
    return false;
  });
  $('.form_all_check').click(function(){
    dom = $(this).attr('alt');
    $('.' + dom).attr('checked', this.checked ? true : false);
  });
  $('a.all_menu_ext').click(function(){
    dom = $(this).attr('alt');
    if($(this).attr('rel') == 1){
      $(this).attr('rel', 0).text('闭合');
      $('.' + dom).removeClass('div_none');
    }else{
      $(this).attr('rel', 1).text('展开');
      $('.' + dom).addClass('div_none');
    }
    return false;
  });
  
  $('.dc_form_ajax_form_button').click(function(){
  	var s = $.param($(this).parents('form').formToArray());
  	var getUrl = $(this).attr('alt');
  	if(getUrl){
	  	$.ajax({
	  		type: 'POST',
	  		url: getUrl,
	  		dataType: 'script',
	  		data: s,
	  		error: function(e){
	  			alert('error');
	  		}
	  	});
  	};
  	return false;
  });
  
	$('.thickbox, .dialog').click(function(){
    $('body').append('<div id="dialog_wrapper" style="display: none;"></div>');
		var o = $(this);
		var url = $(this).attr('href');
		var queryString = url.replace(/^[^\?]*\??/,'');
    
		var params = Dida.parseQuery(queryString);
    if(params['iframe']){
  		$('#dialog_wrapper').dialog({
  			title: o.attr('title') || o.text(),
  			closeText: '关闭',
  			width: parseInt(params['width']) || 700,
  			height: parseInt(params['height']) || 450,
        modal: params['modal'] || false,
  			autoOpen: true,
  			bgiframe: true,
  			close: function(event, ui) {
          $('#dialog_wrapper').remove();
  				$(this).dialog('destroy');
          
          if(params['closeCall']){
            Dida.callFunc(params['closeCall'], event, ui, params);
          }
          
  				if(params['reload']){
  					location.reload();
  				}
  			},
  			open: function(){
  				$(this).append('<div id="dialog_wrapper_loading">加载中…</div><iframe id="dialog_iframe_wrapper" frameborder="no" border="0" src="'+url+'" width="100%" height="100%" style="display:none"></iframe>');
        	$('#dialog_iframe_wrapper').load(function(){
            $('#dialog_wrapper_loading').hide();
            $('#dialog_iframe_wrapper').show();
          });
  			}
  		});
    }else if(params['inlineId']){
  		$('#'+params['inlineId']).dialog({
  			title: o.attr('title') || o.text(),
  			closeText: '关闭',
  			width: params['width'] || 700,
  			height: params['height'] || 450,
  			autoOpen: true,
  			bgiframe: true,
  			close: function(event, ui) {
  				$(this).dialog('destroy');
  				if(params['reload']){
  					location.reload();
  				}
  			}
  		});
    }
		return false;
	});
  
  $('input[class="admin_delete_button"]').click(function(){
    if(confirm('确认此操作吗？')){
      var href = $(this).attr('alt');
      var c = $(this).attr('rel');
      $('.' + c).not($('input.form_all_check')).each(function(){
        if(this.checked){
          var $$ = $(this);
          url = $$.attr('href') ? $$.attr('href') : href;
          var opt = {};
          opt.timestamp = Dida.gettime();
          if(!$$.attr('href')){
            opt.id = $(this).val();
            opt.name = $(this).attr('alt');
          }
          $.get(url, opt, function(data){
            if($$.attr('type') != 'js'){
              var fun = $$.attr('fun');
              if(data == 1){
                var text = $$.attr('rel');
                if(fun){
                  eval(fun + '('+data+');');
                }else if(text){
                  $$.attr('disabled', true).after('<span class="msgjs">' + text + '</span>');
                }else{
                  $$.parent().parent().remove();
                }
              }else if(data && fun){
                eval(fun + '('+data+');');
              }else{
                $$.after('<span class="msgjs">' + (data ? data : '操作失败') + '</span>');
              }
            }else{
              eval(data);
            }
          })
        }
      })
    }
    return false;
  });
  $('.login_msg').click(function(){
    if(confirm('你需要登录才能进行此操作，立即登录？')){
      location.href = Dida.url('user/login', {redirect: location.pathname});
    }
    return false;
  });
  if(settings.multi){
    for (var attr in settings.multi) {
      var element = settings.multi[attr];
      $(element).each(function() {
      	$('#multi_' + this.dom).MultiFile({'list' : '#multi_list_' + this.dom});
      });
    }
  }
  if(settings.edit){
    var element = settings.edit;
    $(element).each(function() {
    	$(this.dom).editable(null, this.opt);
    });
  }
  $('.pager_form_go_input').blur(function(){
    var id = $(this).attr('alt');
    var s = settings.pager[id];
    var t = parseInt($(this).val());
    var go = 0;
    if(t){
      t = t - 1;
      if(t != s.current && t < s.sum){
        go =  t > s.sum ? s.sum : t;
        var h = location.href;
        if(s.current != 0){
          var re = /page=\d*/i
          h = h.replace(re, 'page=' + go);
          location.href = h;
        }else{
          location.href = Dida.url(h, {page: go});
        }
      }
    }
  });
  if(settings.farbtastic){
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
  //时间控件
  if(settings.uidata){
    $.datepicker.setDefaults($.datepicker.regional['zh-CN']);
    var element = settings.uidata;
    $(element).each(function(i) {
      o = this;
	    o.showStatus = true;
	    o.showOn = "both";
	    o.buttonImage = "/misc/images/calendar.gif";
	    o.buttonImageOnly = true;
      
      if(o.start || o.end){
      	o.onClose = function(text){ 
					var instance = $(this).data("datepicker");
					uidata_vali($(this), text, instance, o.start ? 'start' : 'end');
        }
      }
      
      if(o.showTime){
      	o.duration = '';
        o.showTime = true;
        o.constrainInput = false;
      }
      
      $(o.dom).datepicker(o).focus(function(){ this.blur(); });
    });
  };
  function uidata_vali(obj, text, instance, type){
  	var val_end, val_start;
  	if(type == 'start'){
  		val_start = text;
  		val_end = $(instance.settings.start).val();
  	}else{
  		val_end = text;
  		val_start = $(instance.settings.end).val();
  	}
  	
  	if(val_end && val_start){
      re = /[^0-9]/g;
      val_end = val_end.replace(re, '');
      val_start = val_start.replace(re, '');
      if(val_end <= val_start){
      	obj.val('');
      	alert('结束日期必须大于开始日期');
      }
  	}
  	return false;
  };
  $('#tour_get_search .category_form_selects_button').remove();
});