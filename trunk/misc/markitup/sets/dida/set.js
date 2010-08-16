// $Id$

Dida.markitup = {html: {}};

/*******html******/
/**
 * 列表
 */
Dida.markitup.html.extUl = function(h, type) {
	var t = '<'+type+'>\n';
	
	if(h.selection) {
		var l = h.selection.split('\n');
		
		$(l).each(function(){
			if (this != '') {
				t += '  <li>'+this+'</li>\n';
			}
		});
		
	}
	
	t += '</'+type+'>\n';
	return t;
}
	
	/**
	 * 代码高亮
	 */
Dida.markitup.html.extCode = function(h, type) {
	return '[code='+type+']\n' + h.selection + '\n[/code]\n';
}
	
/**
 * html 按钮
 */
Dida.markitup.html.sets = {
	h1: {name:'H1', key:'1', openWith:'<h1(!( class="[![Class]!]")!)>', closeWith:'</h1>'},
	h2: {name:'H2', key:'2', openWith:'<h2(!( class="[![Class]!]")!)>', closeWith:'</h2>'},
	h3: {name:'H3', key:'3', openWith:'<h3(!( class="[![Class]!]")!)>', closeWith:'</h3>'},
	h4: {name:'H4', key:'4', openWith:'<h4(!( class="[![Class]!]")!)>', closeWith:'</h4>'},
	h5: {name:'H5', key:'5', openWith:'<h5(!( class="[![Class]!]")!)>', closeWith:'</h5>'},
	h6: {name:'H6', key:'6', openWith:'<h6(!( class="[![Class]!]")!)>', closeWith:'</h6>'},
	p: {name:'P', openWith:'<p(!( class="[![Class]!]")!)>', closeWith:'</p>'  },
	strong: {name:'粗体', key:'B', openWith:'<strong>', closeWith:'</strong>' },
	em: {name:'斜体', key:'I', openWith:'<em>', closeWith:'</em>'  },
	u: {name:'下划线', key:'U', openWith:'<u>', closeWith:'</u>'  },
	del: {name:'删除线', key:'S', openWith:'<del>', closeWith:'</del>' },
	ul: {name:'无序列表', replaceWith: function(h) { return Dida.markitup.html.extUl(h, 'ul'); }},
	ol: {name:'有序列表', replaceWith: function(h) { return Dida.markitup.html.extUl(h, 'ol'); }},
	img: {name:'加图片', key:'P', replaceWith:'<img src="[![图片地址:!:http://]!]" alt="[![提示文本]!]" />' },
	a: {name:'超链接', key:'L', openWith:'<a href="[![链接网址:!:http://]!]"(!( title="[![Title]!]")!)>', closeWith:'</a>', placeHolder:'' },
	blockquote: {name: '引用', key: 'Q', openWith: '<blockquote>', closeWith: '</blockquote>'},
	code: {name: '插入代码', key: 'C', dropMenu: 
		[
			 {name: 'PHP', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'php');} },
			 {name: 'ActionScript3', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'as3');} },
			 {name: 'Bash/shell', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'bash');} },
			 {name: 'ColdFusion', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'cf');} },
			 {name: 'C#', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'csharp');} },
			 {name: 'C++', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'c');} },
			 {name: 'CSS', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'css');} },
			 {name: 'Delphi', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'delphi');} },
			 {name: 'Diff', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'diff');} },
			 {name: 'Erlang', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'erl');} },
			 {name: 'Groovy', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'groovy');} },
			 {name: 'JavaScript', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'js');} },
			 {name: 'Java', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'java');} },
			 {name: 'JavaFX', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'jfx');} },
			 {name: 'Perl', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'pl');} },
			 {name: 'Plain Text', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'plain');} },
			 {name: 'PowerShell', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'ps');} },
			 {name: 'Python', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'py');} },
			 {name: 'Ruby', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'ruby');} },
			 {name: 'SQL', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'sql');} },
			 {name: 'Visual Basic', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'vb');} },
			 {name: 'XML', replaceWith: function(h) { return Dida.markitup.html.extCode(h, 'xml');} }
		 ]},
	clean: {name:'清除格式', replaceWith:function(h) { return h.selection.replace(/<(.*?)>/g, "") }},
	preview: {name:'预览', call:'preview', className:'preview'}
}

Dida.markitup.html.menus = function(menu) {
	
	if (typeof(menu) == 'object') {
		var l = menu.length, m = [];
		
		for (var i=0; i < l; i++) {
			if (Dida.markitup.html.sets[menu[i]]) {
				Dida.markitup.html.sets[menu[i]].className = 'markItUpButton_' + menu[i];
				m[i] = Dida.markitup.html.sets[menu[i]];
			}
		}
	} else {
		
		var i = 0, m = [];;
		for (var attr in Dida.markitup.html.sets) {
			Dida.markitup.html.sets[attr].className = 'markItUpButton_' + attr;
			m[i] = Dida.markitup.html.sets[attr];
			i++;
		}
		
	}
	return m;
}

Dida.markitup.html.settings = function(obj) {
	var o = obj || {};
	var opt = {
		nameSpace: 'html',
		nameSpace: 'html',
		onShiftEnter: {keepDefault:false, replaceWith:'<br />\n'},
		onCtrlEnter: {keepDefault:false, openWith:'\n<p>', closeWith:'</p>\n'},
		onTab: {keepDefault:false, openWith:''}
	};
	
	if (o.sets) {
		// 添加自定义按钮
		for (var attr in o.sets) {
			Dida.markitup.html.sets[attr] = o.sets[attr];
		}
	}
	
	// 获取按钮
	opt.markupSet = Dida.markitup.html.menus(o.toolbar);
	
	if (o.options) {
		// 获取自定义配置
		for (var attr in o.options) {
			opt[attr] = o.options[attr];
		}
	}
	
	return opt;
}

/*******bbcode******/