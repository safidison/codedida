(function() {
    var a = {
        exec: function(c){
          c.insertHtml('<span class="ck_body_break" title="摘要分隔符">&nbsp;<!-- break --></span>');
        }
    };
    CKEDITOR.plugins.add('bodybreak',{
        init: function(c) {
            c.addCommand('bodybreak', a);
            c.ui.addButton('BodyBreak',{
                label: '摘要',
                command: 'bodybreak',
                icon : this.path + 'break.gif'
            });
        }
    });
})();