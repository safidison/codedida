﻿/*
Copyright (c) 2003-2009, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/

(function() {var a=/^[\t\r\n ]*(?:&nbsp;|\xa0)$/,b='{cke_protected}';function c(A,B) {var C=A.children,D=C[C.length-1];if (D) {if ((B||!CKEDITOR.env.ie)&&(D.type==CKEDITOR.NODE_ELEMENT&&D.name=='br'))C.pop();if (D.type==CKEDITOR.NODE_TEXT&&a.test(D.value))C.pop();}};function d(A) {if (A.children.length<1)return true;var B=A.children[A.children.length-1];return B.type==CKEDITOR.NODE_ELEMENT&&B.name=='br';};function e(A) {c(A,true);if (d(A))if (CKEDITOR.env.ie)A.add(new CKEDITOR.htmlParser.text('\xa0'));else A.add(new CKEDITOR.htmlParser.element('br',{}));};function f(A) {c(A);if (d(A))A.add(new CKEDITOR.htmlParser.text('\xa0'));};var g=CKEDITOR.dtd,h=CKEDITOR.tools.extend({},g.$block,g.$listItem,g.$tableContent);for (var i in h)if (!('br' in g[i]))delete h[i];delete h.pre;var j={attributeNames:[[/^on/,'_cke_pa_on']]},k={elements:{}};for (i in h)k.elements[i]=e;var l={elementNames:[[/^cke:/,''],[/^\?xml:namespace$/,'']],attributeNames:[[/^_cke_(saved|pa)_/,''],[/^_cke.*/,'']],elements:{$:function(A) {var B=A.attributes;if (B) {var C=['name','href','src'],D;for (var E=0;E<C.length;E++) {D='_cke_saved_'+C[E];D in B&&delete B[C[E]];}}},embed:function(A) {var B=A.parent;if (B&&B.name=='object') {var C=B.attributes.width,D=B.attributes.height;C&&(A.attributes.width=C);D&&(A.attributes.height=D);}},param:function(A) {A.children=[];A.isEmpty=true;return A;},a:function(A) {if (!(A.children.length||A.attributes.name||A.attributes._cke_saved_name))return false;}},attributes:{'class':function(A,B) {return CKEDITOR.tools.ltrim(A.replace(/(?:^|\s+)cke_[^\s]*/g,''))||false;}},comment:function(A) {if (A.substr(0,b.length)==b)return new CKEDITOR.htmlParser.cdata(decodeURIComponent(A.substr(b.length)));return A;}},m={elements:{}};for (i in h)m.elements[i]=f;if (CKEDITOR.env.ie)l.attributes.style=function(A,B) {return A.toLowerCase();};var n=/<(?:a|area|img|input).*?\s((?:href|src|name)\s*=\s*(?:(?:"[^"]*")|(?:'[^']*')|(?:[^ "'>]+)))/gi;function o(A) {return A.replace(n,'$& _cke_saved_$1');};var p=/<(style)(?=[ >])[^>]*>[^<]*<\/\1>/gi,q=/<cke:encoded>([^<]*)<\/cke:encoded>/gi,r=/(<\/?)((?:object|embed|param).*?>)/gi,s=/<cke:param(.*?)\/>/gi;function t(A) {return '<cke:encoded>'+encodeURIComponent(A)+'</cke:encoded>';};function u(A) {return A.replace(p,t);};function v(A) {return A.replace(r,'$1cke:$2');};function w(A) {return A.replace(s,'<cke:param$1></cke:param>');};function x(A,B) {return decodeURIComponent(B);};function y(A) {return A.replace(q,x);};function z(A,B) {var C=[],D=/<\!--\{cke_temp\}(\d*?)-->/g,E=[/<!--[\s\S]*?-->/g,/<script[\s\S]*?<\/script>/gi,/<noscript[\s\S]*?<\/noscript>/gi].concat(B);
for (var F=0;F<E.length;F++)A=A.replace(E[F],function(G) {G=G.replace(D,function(H,I) {return C[I];});return '<!--{cke_temp}'+(C.push(G)-1)+'-->';});A=A.replace(D,function(G,H) {return '<!--'+b+encodeURIComponent(C[H]).replace(/--/g,'%2D%2D')+'-->';});return A;};CKEDITOR.plugins.add('htmldataprocessor',{requires:['htmlwriter'],init:function(A) {var B=A.dataProcessor=new CKEDITOR.htmlDataProcessor(A);B.writer.forceSimpleAmpersand=A.config.forceSimpleAmpersand;B.dataFilter.addRules(j);B.dataFilter.addRules(k);B.htmlFilter.addRules(l);B.htmlFilter.addRules(m);}});CKEDITOR.htmlDataProcessor=function(A) {var B=this;B.editor=A;B.writer=new CKEDITOR.htmlWriter();B.dataFilter=new CKEDITOR.htmlParser.filter();B.htmlFilter=new CKEDITOR.htmlParser.filter();};CKEDITOR.htmlDataProcessor.prototype={toHtml:function(A,B) {A=z(A,this.editor.config.protectedSource);A=o(A);if (CKEDITOR.env.ie)A=u(A);A=v(A);A=w(A);var C=document.createElement('div');C.innerHTML='a'+A;A=C.innerHTML.substr(1);if (CKEDITOR.env.ie)A=y(A);var D=CKEDITOR.htmlParser.fragment.fromHtml(A,B),E=new CKEDITOR.htmlParser.basicWriter();D.writeHtml(E,this.dataFilter);return E.getHtml(true);},toDataFormat:function(A,B) {var C=this.writer,D=CKEDITOR.htmlParser.fragment.fromHtml(A,B);C.reset();D.writeHtml(C,this.htmlFilter);return C.getHtml(true);}};})();CKEDITOR.config.forceSimpleAmpersand=false;
