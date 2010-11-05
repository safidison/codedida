jQuery.fn.extend({
  menuLevel: function(o) {
	
    $(this).addClass('js_menu_level_root').find('ul').css({position: 'absolute', display:'none', visibility:'visible'}).each(function() {
      if ($(this).parent('li').parent('ul').hasClass('js_menu_level_root')) {
        $(this).parent('li').addClass('js_menu_level_parent');
      } else {
        $(this).parent('li').addClass('js_menu_level_children');
      }
    });
    
    $(this).children('li').addClass('js_menu_level_one_li').children('a')
    .addClass('js_menu_level_one_a').wrapInner(function() {
    	return '<span/>';
    });
    
    $('.js_menu_level_parent').css({position: 'relative'}).hover(function() {
      var _c = $(this).children('ul').eq(0);
      _c.css({left: (this.offsetWidth - _c.width())/2 + 'px', top: this.offsetHeight, display:'block'});
      $(this).addClass('active');
    },function() {
      $(this).removeClass('active').children('ul').eq(0).hide();
    });
    
    $('.js_menu_level_children').css({position: 'relative'}).hover(function() {
      var _c = $(this).children('ul').eq(0);
      var _s = {left:this.offsetWidth-2, top: -1, display: 'block'};
      
      if ($(window).width() - $(this).offset().left -  this.offsetWidth - _c.width() < 20) {
        _s.left = -(this.offsetWidth > _c.width() ? this.offsetWidth : _c.width());
      }
      _c.css(_s);
    },function() {
      $(this).children('ul').eq(0).hide();
    }); 
  },
  
  countdown: function(o) {
    var count = 0;
    var interId = null;
    var $$ = this;
    $$.html('请稍候，正在同步时间');
    function show() {
      if (count > 0) {
        count -= 1;
        var second = Math.floor(count % 60);
        var minite = Math.floor((count / 60) % 60);
        var hour = Math.floor((count / 3600) % 24);
        var day = Math.floor((count / 3600) / 24);
        $$.html('<span class="day"><em>'+day + '</em>天</span><span class="hour"><em>' + hour + '</em>小时</span><span class="minite"><em>' + minite + '</em>分</span><span class="second"><em>' + second + '</em>秒</span>');
      } else {
        $(o.button).attr('disabled', false);
        window.clearInterval(interId);
      }
    }
    $.post(o.path, o, function(t) {
    	if (t < 0) {
    		Dida.loca();
    	}
      count = t;
      interId = window.setInterval(show, 1000);
    });
  },
  
  didaTabs: function(o){
    var opt = {
      event: 'click',
      className: '.dida_tabs',
      varyTime: 5000,
      mouseout: null,
      bodyDom: 'href',
      auto: false
    }
    
    $.extend(opt, o);
    
    var autoDom, domCount, currentId, showTimer;;
    
    var root = this;
    $$ = this.find(opt.className);
    
    function show(obj){
      if(typeof obj != 'object'){
        if(currentId == domCount) currentId = 0;
        obj = $$.eq(currentId);
      }
      
      if(typeof obj == 'object'){
       var dom = obj.attr(opt.bodyDom);
        
        currentId = obj.index();
        currentId += 1;
        root.find(opt.className).each(function(){
          $(this).removeClass('active');
          var dom = $(this).attr(opt.bodyDom);
          $(dom).hide();
        });
        
        $(dom).show();
        obj.addClass('active');
      }
    }
    
    function hide(obj){
      if(typeof obj == 'object'){
        var dom = obj.attr(opt.bodyDom);
        $(dom).hide();
        $(dom).mouseover(function(){$(this).show();});
      }
    }
    
    $$.each(function(i){
      $(this).css({cursor: 'pointer'});
      var dom = $(this).attr(opt.bodyDom);
 
      if(opt.auto){
        $(dom).hover(function(){
          clearInterval(showTimer);
        },function(){
          showTimer = window.setInterval(show, opt.varyTime);
        });
      }
      
      if($(dom).attr('alt') != 'default'){
        $(dom).hide();
      }
      
    });
    
    if(opt.auto){
      currentId = 0;
      domCount = $$.size();
      showTimer = window.setInterval(show, opt.varyTime);
      $$.hover(function(){
        clearInterval(showTimer);
      },function(){
        showTimer = window.setInterval(show, opt.varyTime);
      });
    }
    
    if(opt.mouseout == 'hide'){
    	$(this).find(opt.className).each(function(){
    		$($(this).attr(opt.bodyDom)).hover(function(){ 
    			$(this).show().attr('alt', 'block');
    		}, function(){
    			$(this).hide().attr('alt', 'none');
    		});
    	});
	    root.mouseout(function(){
	    	$(this).find(opt.className).each(function(){
	    		if($($(this).attr(opt.bodyDom)).attr('alt') == 'none'){
	    			$(this).removeClass('active');
	    			$($(this).attr(opt.bodyDom)).hide();
	    		}
	    	});
	    });
    }
    
    switch(opt.event){
      case 'click':
        $$.click(function(){ 
        	show($(this));
        	return false;
       });
      break;
      default:
        $$.hover(function(){
          show($(this));
          return false;
        },function(){
          return false;
        });
    }
  }
});