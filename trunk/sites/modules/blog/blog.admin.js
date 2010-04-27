/* $Id$ */
$(function(){
  if($('#blog_node_form_status input[type=radio][checked]').val() != 9){
    $('#blog_node_form_info_pass').hide();
  }
  $('#blog_node_form_info_status input[type=radio]').click(function(){
    if($(this).val() == 9){
      $('#blog_node_form_info_pass').show(300);
    }else{
      $('#blog_node_form_info_pass').hide(300);
    }
  });
});

function blog_comment_delete(cid){
  if(cid){
    $('.blog_admin_table_tr_'+cid).remove();
  }else{
    alert('Error!');
  }
}