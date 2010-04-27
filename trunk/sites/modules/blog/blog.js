/* $Id$ */
$(function(){
  $('.blog_comment_quote').click(function(){
    var cid = $(this).attr('alt');
    if(cid){
      var q = $('#blog_comment_'+cid).find('.blog_comment_body').eq(0).text();
      if(q){
        var n = $('#blog_comment_'+cid).find('.blog_comment_name').eq(0).text() + '说：';
        n = n.replace(/\s/ig, '');
        q = n + q + "\n------------------------------------------\n";
        var v = $('#blog_comment_form_type_body').val();
        $('#blog_comment_form_type_body').val(v + q);
      }
    }
    return false;
  });
  $('.blog_trackback').click(function(){
    return false;
  });
});