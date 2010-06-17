/**
 * autosave
 */
var AutoSaveString;
CKEDITOR.plugins.add('autosave',
{
	init : function(a){
    if(a.config.AutoSaveUrl && a.config.AutoSaveUrl != 'undefined'){
      a.addCommand('autosave', new CKEDITOR.autosaveCommand());
  		a.ui.addButton('AutoSave',{
  				label : '保存草稿',
					command : 'autosave',
          icon : this.path + 'autosave.gif'
  			});
    }
	},
  afterInit: function(a){
    if(a.config.AutoSaveUrl && a.config.AutoSaveUrl != 'undefined'){
      var t = a.config.AutoSaveTime || 60000;
      window.setInterval(function(){PluginAutoSave(a)}, t);
    }
  }
});

CKEDITOR.autosaveCommand = function () {};
CKEDITOR.autosaveCommand.prototype = {
    exec: function (a) {
      PluginAutoSave(a);
    }
};

function PluginAutoSave(a){
  $(a.document).ready(function(){
    var b = a.document.getBody().getHtml();
    $('textarea[name='+a.name+']').val(b);
    var s = $.param($('textarea[name='+a.name+']').closest('form').formToArray());
    
    if(AutoSaveString != s) { // 表单数据与上次请求保存时不一致时才执行
      AutoSaveString = s;
      $.post(a.config.AutoSaveUrl, s, function(data){
        if(data == 'ok'){
          var myDate = new Date();
          Dida.messageShow(myDate.toLocaleString() + ' 草稿保存成功');
        }else{
          alert('草稿保存失败');
        }
      });
    };
  })
}