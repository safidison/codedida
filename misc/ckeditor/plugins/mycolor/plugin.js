/*
	mycolor
*/

CKEDITOR.plugins.add('mycolor',{
	init : function(a)
	{
		a.addCommand('mycolor', new CKEDITOR.dialogCommand('mycolor'));
    
		a.ui.addButton( 'MyColor',
			{
				label : '编辑器颜色',
				command : 'mycolor',
				icon : this.path + 'uicolor.gif'
			});
		CKEDITOR.dialog.add('mycolor', this.path + 'dialogs/mycolor.js' );
  }
});