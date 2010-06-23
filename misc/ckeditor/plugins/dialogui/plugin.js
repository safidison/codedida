﻿/*
Copyright (c) 2003-2009, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

CKEDITOR.plugins.add('dialogui');(function() {var a=function(h) {var k=this;k._||(k._={});k._['default']=k._.initValue=h['default']||'';var i=[k._];for (var j=1;j<arguments.length;j++)i.push(arguments[j]);i.push(true);CKEDITOR.tools.extend.apply(CKEDITOR.tools,i);return k._;},b={build:function(h,i,j) {return new CKEDITOR.ui.dialog.textInput(h,i,j);}},c={build:function(h,i,j) {return new CKEDITOR.ui.dialog[i.type](h,i,j);}},d={isChanged:function() {return this.getValue()!=this.getInitValue();},reset:function() {this.setValue(this.getInitValue());},setInitValue:function() {this._.initValue=this.getValue();},resetInitValue:function() {this._.initValue=this._['default'];},getInitValue:function() {return this._.initValue;}},e=CKEDITOR.tools.extend({},CKEDITOR.ui.dialog.uiElement.prototype.eventProcessors,{onChange:function(h,i) {if (!this._.domOnChangeRegistered) {h.on('load',function() {this.getInputElement().on('change',function() {this.fire('change',{value:this.getValue()});},this);},this);this._.domOnChangeRegistered=true;}this.on('change',i);}},true),f=/^on([A-Z]\w+)/,g=function(h) {for (var i in h)if (f.test(i)||i=='title'||i=='type')delete h[i];return h;};CKEDITOR.tools.extend(CKEDITOR.ui.dialog,{labeledElement:function(h,i,j,k) {if (arguments.length<4)return;var l=a.call(this,i);l.labelId=CKEDITOR.tools.getNextNumber()+'_label';var m=this._.children=[],n=function() {var o=[];if (i.labelLayout!='horizontal')o.push('<div class="cke_dialog_ui_labeled_label" id="',l.labelId,'" >',i.label,'</div>','<div class="cke_dialog_ui_labeled_content">',k(h,i),'</div>');else{var p={type:'hbox',widths:i.widths,padding:0,children:[{type:'html',html:'<span class="cke_dialog_ui_labeled_label" id="'+l.labelId+'">'+CKEDITOR.tools.htmlEncode(i.label)+'</span>'},{type:'html',html:'<span class="cke_dialog_ui_labeled_content">'+k(h,i)+'</span>'}]};CKEDITOR.dialog._.uiElementBuilders.hbox.build(h,p,o);}return o.join('');};CKEDITOR.ui.dialog.uiElement.call(this,h,i,j,'div',null,null,n);},textInput:function(h,i,j) {if (arguments.length<3)return;a.call(this,i);var k=this._.inputId=CKEDITOR.tools.getNextNumber()+'_textInput',l={'class':'cke_dialog_ui_input_'+i.type,id:k,type:'text'},m;if (i.validate)this.validate=i.validate;if (i.maxLength)l.maxlength=i.maxLength;if (i.size)l.size=i.size;var n=this,o=false;h.on('load',function() {n.getInputElement().on('keydown',function(q) {if (q.data.getKeystroke()==13)o=true;});n.getInputElement().on('keyup',function(q) {if (q.data.getKeystroke()==13&&o) {h.getButton('ok')&&h.getButton('ok').click();
o=false;}},null,null,1000);});var p=function() {var q=['<div class="cke_dialog_ui_input_',i.type,'"'];if (i.width)q.push('style="width:'+i.width+'" ');q.push('><input ');for (var r in l)q.push(r+'="'+l[r]+'" ');q.push(' /></div>');return q.join('');};CKEDITOR.ui.dialog.labeledElement.call(this,h,i,j,p);},textarea:function(h,i,j) {if (arguments.length<3)return;a.call(this,i);var k=this,l=this._.inputId=CKEDITOR.tools.getNextNumber()+'_textarea',m={};if (i.validate)this.validate=i.validate;m.rows=i.rows||5;m.cols=i.cols||20;var n=function() {var o=['<div class="cke_dialog_ui_input_textarea"><textarea class="cke_dialog_ui_input_textarea" id="',l,'" '];for (var p in m)o.push(p+'="'+CKEDITOR.tools.htmlEncode(m[p])+'" ');o.push('>',CKEDITOR.tools.htmlEncode(k._['default']),'</textarea></div>');return o.join('');};CKEDITOR.ui.dialog.labeledElement.call(this,h,i,j,n);},checkbox:function(h,i,j) {if (arguments.length<3)return;var k=a.call(this,i,{'default':!!i['default']});if (i.validate)this.validate=i.validate;var l=function() {var m=CKEDITOR.tools.extend({},i,{id:i.id?i.id+'_checkbox':CKEDITOR.tools.getNextNumber()+'_checkbox'},true),n=[],o={'class':'cke_dialog_ui_checkbox_input',type:'checkbox'};g(m);if (i['default'])o.checked='checked';k.checkbox=new CKEDITOR.ui.dialog.uiElement(h,m,n,'input',null,o);n.push(' <label for="',o.id,'">',CKEDITOR.tools.htmlEncode(i.label),'</label>');return n.join('');};CKEDITOR.ui.dialog.uiElement.call(this,h,i,j,'span',null,null,l);},radio:function(h,i,j) {if (arguments.length<3)return;a.call(this,i);if (!this._['default'])this._['default']=this._.initValue=i.items[0][1];if (i.validate)this.validate=i.valdiate;var k=[],l=this,m=function() {var n=[],o=[],p={'class':'cke_dialog_ui_radio_item'},q=i.id?i.id+'_radio':CKEDITOR.tools.getNextNumber()+'_radio';for (var r=0;r<i.items.length;r++) {var s=i.items[r],t=s[2]!==undefined?s[2]:s[0],u=s[1]!==undefined?s[1]:s[0],v=CKEDITOR.tools.extend({},i,{id:CKEDITOR.tools.getNextNumber()+'_radio_input',title:null,type:null},true),w=CKEDITOR.tools.extend({},v,{id:null,title:t},true),x={type:'radio','class':'cke_dialog_ui_radio_input',name:q,value:u},y=[];if (l._['default']==u)x.checked='checked';g(v);g(w);k.push(new CKEDITOR.ui.dialog.uiElement(h,v,y,'input',null,x));y.push(' ');new CKEDITOR.ui.dialog.uiElement(h,w,y,'label',null,{'for':x.id},s[0]);n.push(y.join(''));}new CKEDITOR.ui.dialog.hbox(h,[],n,o);return o.join('');};CKEDITOR.ui.dialog.labeledElement.call(this,h,i,j,m);this._.children=k;},button:function(h,i,j) {if (!arguments.length)return;
if (typeof i=='function')i=i(h.getParentEditor());a.call(this,i,{disabled:i.disabled||false});CKEDITOR.event.implementOn(this);var k=this;h.on('load',function(m) {var n=this.getElement();(function() {n.on('click',function(o) {k.fire('click',{dialog:k.getDialog()});o.data.preventDefault();});})();n.unselectable();},this);var l=CKEDITOR.tools.extend({},i);delete l.style;CKEDITOR.ui.dialog.uiElement.call(this,h,l,j,'a',null,{style:i.style,href:'javascript:void(0)',title:i.label,hidefocus:'true','class':i['class']},'<span class="cke_dialog_ui_button">'+CKEDITOR.tools.htmlEncode(i.label)+'</span>');},select:function(h,i,j) {if (arguments.length<3)return;var k=a.call(this,i);if (i.validate)this.validate=i.validate;var l=function() {var m=CKEDITOR.tools.extend({},i,{id:i.id?i.id+'_select':CKEDITOR.tools.getNextNumber()+'_select'},true),n=[],o=[],p={'class':'cke_dialog_ui_input_select'};if (i.size!=undefined)p.size=i.size;if (i.multiple!=undefined)p.multiple=i.multiple;g(m);for (var q=0,r;q<i.items.length&&(r=i.items[q]);q++)o.push('<option value="',CKEDITOR.tools.htmlEncode(r[1]!==undefined?r[1]:r[0]),'" /> ',CKEDITOR.tools.htmlEncode(r[0]));k.select=new CKEDITOR.ui.dialog.uiElement(h,m,n,'select',null,p,o.join(''));return n.join('');};CKEDITOR.ui.dialog.labeledElement.call(this,h,i,j,l);},file:function(h,i,j) {if (arguments.length<3)return;if (i['default']===undefined)i['default']='';var k=CKEDITOR.tools.extend(a.call(this,i),{definition:i,buttons:[]});if (i.validate)this.validate=i.validate;var l=function() {k.frameId=CKEDITOR.tools.getNextNumber()+'_fileInput';var m=CKEDITOR.env.ie&&document.domain!=window.location.hostname,n=['<iframe frameborder="0" allowtransparency="0" class="cke_dialog_ui_input_file" id="',k.frameId,'" title="',i.label,'" src="javascript:void('];n.push(m?"(function() {document.open();document.domain='"+document.domain+"';"+'document.close();'+'})()':'0');n.push(')"></iframe>');return n.join('');};h.on('load',function() {var m=CKEDITOR.document.getById(k.frameId),n=m.getParent();n.addClass('cke_dialog_ui_input_file');});CKEDITOR.ui.dialog.labeledElement.call(this,h,i,j,l);},fileButton:function(h,i,j) {if (arguments.length<3)return;var k=a.call(this,i),l=this;if (i.validate)this.validate=i.validate;var m=CKEDITOR.tools.extend({},i),n=m.onClick;m.className=(m.className?m.className+' ':'')+('cke_dialog_ui_button');m.onClick=function(o) {var p=i['for'];if (!n||n.call(this,o)!==false) {h.getContentElement(p[0],p[1]).submit();this.disable();}};h.on('load',function() {h.getContentElement(i['for'][0],i['for'][1])._.buttons.push(l);
});CKEDITOR.ui.dialog.button.call(this,h,m,j);},html:(function() {var h=/^\s*<[\w:]+\s+([^>]*)?>/,i=/^(\s*<[\w:]+(?:\s+[^>]*)?)((?:.|\r|\n)+)$/,j=/\/$/;return function(k,l,m) {if (arguments.length<3)return;var n=[],o,p=l.html,q,r;if (p.charAt(0)!='<')p='<span>'+p+'</span>';if (l.focus) {var s=this.focus;this.focus=function() {s.call(this);l.focus.call(this);this.fire('focus');};if (l.isFocusable) {var t=this.isFocusable;this.isFocusable=t;}this.keyboardFocusable=true;}CKEDITOR.ui.dialog.uiElement.call(this,k,l,n,'span',null,null,'');o=n.join('');q=o.match(h);r=p.match(i)||['','',''];if (j.test(r[1])) {r[1]=r[1].slice(0,-1);r[2]='/'+r[2];}m.push([r[1],' ',q[1]||'',r[2]].join(''));};})()},true);CKEDITOR.ui.dialog.html.prototype=new CKEDITOR.ui.dialog.uiElement();CKEDITOR.ui.dialog.labeledElement.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.uiElement(),{setLabel:function(h) {var i=CKEDITOR.document.getById(this._.labelId);if (i.getChildCount()<1)new CKEDITOR.dom.text(h,CKEDITOR.document).appendTo(i);else i.getChild(0).$.nodeValue=h;return this;},getLabel:function() {var h=CKEDITOR.document.getById(this._.labelId);if (!h||h.getChildCount()<1)return '';else return h.getChild(0).getText();},eventProcessors:e},true);CKEDITOR.ui.dialog.button.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.uiElement(),{click:function() {var h=this;if (!h._.disabled)return h.fire('click',{dialog:h._.dialog});h.getElement().$.blur();return false;},enable:function() {this._.disabled=false;var h=this.getElement();h&&h.removeClass('disabled');},disable:function() {this._.disabled=true;this.getElement().addClass('disabled');},isVisible:function() {return!!this.getElement().$.firstChild.offsetHeight;},isEnabled:function() {return!this._.disabled;},eventProcessors:CKEDITOR.tools.extend({},CKEDITOR.ui.dialog.uiElement.prototype.eventProcessors,{onClick:function(h,i) {this.on('click',i);}},true),accessKeyUp:function() {this.click();},accessKeyDown:function() {this.focus();},keyboardFocusable:true},true);CKEDITOR.ui.dialog.textInput.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.labeledElement(),{getInputElement:function() {return CKEDITOR.document.getById(this._.inputId);},focus:function() {var h=this.selectParentTab();setTimeout(function() {var i=h.getInputElement();i&&i.$.focus();},0);},select:function() {var h=this.selectParentTab();setTimeout(function() {var i=h.getInputElement();if (i) {i.$.focus();i.$.select();}},0);},accessKeyUp:function() {this.select();},setValue:function(h) {h=h||'';return CKEDITOR.ui.dialog.uiElement.prototype.setValue.call(this,h);
},keyboardFocusable:true},d,true);CKEDITOR.ui.dialog.textarea.prototype=new CKEDITOR.ui.dialog.textInput();CKEDITOR.ui.dialog.select.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.labeledElement(),{getInputElement:function() {return this._.select.getElement();},add:function(h,i,j) {var k=new CKEDITOR.dom.element('option',this.getDialog().getParentEditor().document),l=this.getInputElement().$;k.$.text=h;k.$.value=i===undefined||i===null?h:i;if (j===undefined||j===null) {if (CKEDITOR.env.ie)l.add(k.$);else l.add(k.$,null);}else l.add(k.$,j);return this;},remove:function(h) {var i=this.getInputElement().$;i.remove(h);return this;},clear:function() {var h=this.getInputElement().$;while(h.length>0)h.remove(0);return this;},keyboardFocusable:true},d,true);CKEDITOR.ui.dialog.checkbox.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.uiElement(),{getInputElement:function() {return this._.checkbox.getElement();},setValue:function(h) {this.getInputElement().$.checked=h;this.fire('change',{value:h});},getValue:function() {return this.getInputElement().$.checked;},accessKeyUp:function() {this.setValue(!this.getValue());},eventProcessors:{onChange:function(h,i) {if (!CKEDITOR.env.ie)return e.onChange.apply(this,arguments);else{h.on('load',function() {var j=this._.checkbox.getElement();j.on('propertychange',function(k) {k=k.data.$;if (k.propertyName=='checked')this.fire('change',{value:j.$.checked});},this);},this);this.on('change',i);}return null;}},keyboardFocusable:true},d,true);CKEDITOR.ui.dialog.radio.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.uiElement(),{setValue:function(h) {var i=this._.children,j;for (var k=0;k<i.length&&(j=i[k]);k++)j.getElement().$.checked=j.getValue()==h;this.fire('change',{value:h});},getValue:function() {var h=this._.children;for (var i=0;i<h.length;i++)if (h[i].getElement().$.checked)return h[i].getValue();return null;},accessKeyUp:function() {var h=this._.children,i;for (i=0;i<h.length;i++)if (h[i].getElement().$.checked) {h[i].getElement().focus();return;}h[0].getElement().focus();},eventProcessors:{onChange:function(h,i) {if (!CKEDITOR.env.ie)return e.onChange.apply(this,arguments);else{h.on('load',function() {var j=this._.children,k=this;for (var l=0;l<j.length;l++) {var m=j[l].getElement();m.on('propertychange',function(n) {n=n.data.$;if (n.propertyName=='checked'&&this.$.checked)k.fire('change',{value:this.getAttribute('value')});});}},this);this.on('change',i);}return null;}},keyboardFocusable:true},d,true);CKEDITOR.ui.dialog.file.prototype=CKEDITOR.tools.extend(new CKEDITOR.ui.dialog.labeledElement(),d,{getInputElement:function() {var h=CKEDITOR.document.getById(this._.frameId).getFrameDocument();
return h.$.forms.length>0?new CKEDITOR.dom.element(h.$.forms[0].elements[0]):this.getElement();},submit:function() {this.getInputElement().getParent().$.submit();return this;},getAction:function(h) {return this.getInputElement().getParent().$.action;},reset:function() {var h=CKEDITOR.document.getById(this._.frameId),i=h.getFrameDocument(),j=this._.definition,k=this._.buttons;function l() {i.$.open();if (CKEDITOR.env.isCustomDomain())i.$.domain=document.domain;var m='';if (j.size)m=j.size-(CKEDITOR.env.ie?7:0);i.$.write(['<html><head><title></title></head><body style="margin: 0; overflow: hidden; background: transparent;">','<form enctype="multipart/form-data" method="POST" action="',CKEDITOR.tools.htmlEncode(j.action),'">','<input type="file" name="',CKEDITOR.tools.htmlEncode(j.id||'cke_upload'),'" size="',CKEDITOR.tools.htmlEncode(m>0?m:''),'" />','</form>','</body></html>'].join(''));i.$.close();for (var n=0;n<k.length;n++)k[n].enable();};if (CKEDITOR.env.gecko)setTimeout(l,500);else l();},getValue:function() {return '';},eventProcessors:e,keyboardFocusable:true},true);CKEDITOR.ui.dialog.fileButton.prototype=new CKEDITOR.ui.dialog.button();CKEDITOR.dialog.addUIElement('text',b);CKEDITOR.dialog.addUIElement('password',b);CKEDITOR.dialog.addUIElement('textarea',c);CKEDITOR.dialog.addUIElement('checkbox',c);CKEDITOR.dialog.addUIElement('radio',c);CKEDITOR.dialog.addUIElement('button',c);CKEDITOR.dialog.addUIElement('select',c);CKEDITOR.dialog.addUIElement('file',c);CKEDITOR.dialog.addUIElement('fileButton',c);CKEDITOR.dialog.addUIElement('html',c);})();
