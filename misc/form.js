// $Id$
$(function(){
  $("input@[type='text'], input@[type='password']").hover(function(){
    $(this).addClass('inputhover');
  },function(){
    $(this).removeClass('inputhover');
  })
  $('.fieldset-hide').children('.fieldset-wrapper').hide();
  $('fieldset.collapsible > legend.collapse-processed').click(function() {
    $(this).toggleClass('asc').toggleClass('desc').parent().toggleClass('fieldset-hide');
    $(this).next('.fieldset-wrapper').slideToggle(100);
    return false;
  });
  /*
  $('.form_field_input_captcha').one('click', function(){
    $(this).next('.form_captcha').trigger('click');
  });
  */
  $('.form_captcha').click(function(){
    if(settings.captcha){
      var k = $(this).attr('alt');
      var j = settings.captcha[k];
      var $$ = $(this);
      
      var opt = j;
      opt.timestamp = Dida.gettime();
      
      if(j.image){
        $$.html('<img class="form_captcha_img" src="'+ Dida.url('captcha', opt) +'" />');
      }else{
        $.post(settings.base_path + 'captcha', opt, function(data){
          $$.html(data);
        });
      }
    }
  });
  $('a.form_file_click').click(function(){
    length = $(this).prev().attr('multi');
    dom = $(this).attr('alt');
    sum = $('input', '#' + dom).length;
    if(sum != length){
      $($($(this).prev()).clone()).insertAfter($(this).parent());
      $('#' + dom).children('input').not($(this).prev()).wrapAll('<div></div>');
      document.cookie = dom + "=" + (sum+1);
      if(sum == (length - 1)){
         $(this).remove();
      }
    }else{
      $(this).remove();
      alert('最多允许同时上传 '+length+' 个文件');
    }
    return false;
  });
  
  $('input#user_login').keypress(function(event){
  	if(event.which == 0) {
  		$('input#user_login_form_type_pass').focus();
  		return false;
  	}
  });
  $('span.option_label, span.form_radio_text').click(function(){
  	$(this).prev('input').attr('checked', !$(this).prev('input').attr('checked'));
  });
  $('span.form_description').click(function(){
  	$(this).prev('input').focus();
  });
  $('label.dd_label').click(function(){
  	$(this).next().focus();
  });
});