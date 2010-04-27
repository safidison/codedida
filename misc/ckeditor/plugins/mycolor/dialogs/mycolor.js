/*
	mycolor
*/
CKEDITOR.dialog.add('mycolor', function (a) {
    return {
        title: '编辑器换肤',
        minWidth: 390,
        minHeight: 230,
        /*onLoad: function () {
        },*/
        contents: [{
            id: 'tab1',
            label: '',
            title: '',
            expand: true,
            padding: 0,
            elements: [{
                type: 'html',
                children: [{'width' : '200px'}],
                html: '<div id="farbtastic"></div><input type="text" id="mycolor-value" value="" class="cke_dialog_ui_input_text" style="text-align: center;"/>'}]
        }],
        buttons: [CKEDITOR.dialog.okButton, CKEDITOR.dialog.cancelButton],
        onShow: function(){
          $(function(){
            var f = $.farbtastic('#farbtastic', function(color){$('#mycolor-value').val(color);a.setUiColor(color);});
            var c = a.getUiColor();
            if(!c){
              c = '#D3D3D3';
            }
            $('#mycolor-value').val(c).attr('alt', c);
            f.setColor(c);
          });
        },
        /*onShow: function(){
          var t = document.getElementById('mycolor-value');
          if(!t.value || !t.value == 'undefined'){
            if(!a.config.uiColor || a.config.uiColor == 'undefined'){
              a.config.uiColor = '#D3D3D3';
            }
            t.value = a.config.uiColor;
            t.setAttribute('alt', a.config.uiColor);
          }
        },*/
        onOk: function(){
          document.cookie='editorColor=' + document.getElementById('mycolor-value').value + ';path=/';
        },
        onCancel: function(){
	        a.setUiColor(document.getElementById('mycolor-value').getAttribute('alt'));
        }
    };
});