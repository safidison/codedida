jQuery.fn.extend({
  menuLevel: function(o){
    $(this).addClass('js_menu_level_root').find('ul').css({position: 'absolute', display:'none', visibility:'visible'}).each(function(){
      if($(this).parent('li').parent('ul').hasClass('js_menu_level_root')){
        $(this).parent('li').addClass('js_menu_level_parent');
      }else{
        $(this).parent('li').addClass('js_menu_level_children');
      }
    });
    $(this).children('li').addClass('js_menu_level_one_li').children('a').addClass('js_menu_level_one_a');
    $('.js_menu_level_parent').css({position: 'relative'}).hover(function(){
      var _c = $(this).children('ul').eq(0);
      _c.css({left: (this.offsetWidth - _c.width())/2 + 'px', top: this.offsetHeight, display:'block'});
    },function(){
      $(this).children('ul').eq(0).hide();
    });
    
    $('.js_menu_level_children').css({position: 'relative'}).hover(function(){
      var _c = $(this).children('ul').eq(0);
      var _s = {left:this.offsetWidth-2, top: -1, display: 'block'};
      
      if($(window).width() - $(this).offset().left -  this.offsetWidth - _c.width() < 20){
        _s.left = -(this.offsetWidth > _c.width() ? this.offsetWidth : _c.width());
      }
      _c.css(_s);
    },function(){
      $(this).children('ul').eq(0).hide();
    });
    
  }
});