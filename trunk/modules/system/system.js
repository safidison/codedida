// $Id$
$(function(){
  var adminMenu = Dida.getck('admin_menu');
  if(adminMenu){
    var m = adminMenu.split(',');
    $.each(m, function(i, c){
      if(c){
        $(".item_admin_menus .admin_menus:eq("+c+")").hide().attr('alt', 'none')
        .prev('h3').children('.item_click_op').addClass('item-list-hide');
      }
    });
  }
  $('.item_admin_menus .item_click_op').click(function(){
    if($(this).parent().next('.admin_menus').css('display') == 'block'){
      $(this).addClass('item-list-hide')
      .parent().next('.admin_menus').hide(300).attr('alt', 'none');
    }else{
      $(this).removeClass('item-list-hide')
      .parent().next('.admin_menus').show(300).attr('alt', 'block');
    }
    
    var n = '';
    $('.item_admin_menus .admin_menus').each(function(i){
      if($(this).attr('alt') == 'none'){
        n += i + ',';
      }
    });
    var date = new Date();
    date.setTime(date.getTime() + 9999999999);
    document.cookie = "admin_menu=" + n + ";path=/;expires=" + date.toGMTString();
  });
  $('.files_view_ckeditorbrowser_insert a').click(function(){
    if(window.opener){
      var params = Dida.parseQuery(Dida.getUrl());
      if(params['CKEditorFuncNum']){
        window.opener.CKEDITOR.tools.callFunction(params['CKEditorFuncNum'], $(this).attr('href'), '');
        window.close();
      }
    }
    return false;
  });
  $('.files_view_custombrowser_insert a').click(function(){
    if(window.parent){
      window.parent.ck_insert_img($(this).attr('href'), $(this).attr('title'), $(this).attr('alt'), $(this).attr('link'));
      $(this).remove();
    }
    return false;
  });
});