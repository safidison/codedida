jQuery.fn.extend({
  menuLevel: function(o) {
    $(this).addClass('js_menu_level_root').find('ul').css({position: 'absolute', display:'none', visibility:'visible'}).each(function() {
      if ($(this).parent('li').parent('ul').hasClass('js_menu_level_root')) {
        $(this).parent('li').addClass('js_menu_level_parent');
      } else {
        $(this).parent('li').addClass('js_menu_level_children');
      }
    });
    $(this).children('li').addClass('js_menu_level_one_li').children('a').addClass('js_menu_level_one_a');
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
  }
});