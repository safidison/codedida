// $Id$
$(function(){
  for (var attr in settings.ahah) {
    var element = settings.ahah[attr];
    $(element).each(function() {
      ahah = this;
      $('#' + attr).bind(ahah.event, function(){
        if(ahah.callFunction) {
          Dida.callFunc(ahah.callFunction, ahah);
        }else{
          var d = '&__default_value=' + $('#' + attr).val();
          $.ajax({
            url: ahah.url,
            data: $('#' + attr).parents('form').serialize() + d,
            success: function(r){
              if(ahah.callFunction) {
                Dida.callFunc(ahah.callFunction, r, $('#' + attr));
              }
            },
            dataType: ahah.dataType || 'html',
            type: ahah.type || 'POST'
          });
        }
      });
    });
  }
});

function call_category(opt){
  url = opt.url;
  ajax_category(url, $('#' + opt.id));
}

function ajax_category(url, $$){
	var tmp = '';
  var d = '&__default_value=' + $$.val();
  $.ajax({
    url: url,
    data: $$.parents('form').serialize() + d,
    success: function(json){
      if(!json.error){
        tmp = '<option value="0">请选择</option>';
        $.each(json.contents, function(i, item){
          tmp += '<option value="'+ item.tid +'">'+ item.name +'</option>';
        });
        if($$.next('.term_options').length != 0){
          $$.next('.term_options').html(tmp);
          $$.next().nextAll('.term_options').remove();
        }else{
          $$.after('<select class="term_options" onchange="ajax_category(url, $(this));" name="term_test[]">' + tmp + '</select>'); 
        }
      }else{
        $$.next('.term_ajax_options').remove();
      }
    },
    dataType: 'json',
    type: 'POST'
  });
}